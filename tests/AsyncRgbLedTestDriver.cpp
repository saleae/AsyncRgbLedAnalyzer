#include "TestInstance.h"

#include "MockChannelData.h"
#include "MockResults.h"
#include "MockSettings.h"

#include <cassert>
#include <iostream>

// output operators for the test macro
std::ostream& operator<<(std::ostream& out, const AnalyzerTest::MockResultData::FrameRange& range)
{
    out << "Frames " << range.first << ":" << range.second;
    return out;
}

std::ostream& operator<<(std::ostream& out, const Channel& chan)
{
    out << "Channel{" << chan.mDeviceId << "/" << chan.mChannelIndex << " ";
    if (chan.mDataType ==ANALOG_CHANNEL) {
        out << "A}";
    } else {
        out << "D}";
    }
    return out;
}


#define TEST_VERIFY_EQ(a, b) \
    if ( !((a) == (b)) )  { \
        std::cerr << "failed: " << #a << " == " << #b << std::endl; \
       std::cerr << "\tgot '" << (a) << "' and '" << (b) << "'" << std::endl; \
        std::cerr << "\tat " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(1); \
    }

#define TEST_VERIFY_EQ_CHARS(a, b) \
    assert(!strcmp(a, b))

namespace {

double operator "" _ns( unsigned long long x )
{
    return x * 1e-9;
}

double operator "" _us( unsigned long long x )
{
    return x * 1e-6;
}

} // of anonymous namespace

const Channel TEST_CHANNEL = Channel(0, 0, DIGITAL_CHANNEL);

using namespace AnalyzerTest;

void ws2811_test_append_byte(std::vector<double>& result, U8 byte)
{
    for (int i=0; i<8; ++i) {
        const bool b = byte >> (7 - i) & 1;
        if (b) {
            result.push_back(1200_ns);
            result.push_back(1300_ns);
        } else {
            result.push_back(500_ns);
            result.push_back(2000_ns);
        }
    }
}

std::vector<double> ws2811_test_rgb(U8 red, U8 green, U8 blue)
{
    std::vector<double> r;
    r.reserve(48); // 48 transitions per 8-bit RGB word
    ws2811_test_append_byte(r, red);
    ws2811_test_append_byte(r, green);
    ws2811_test_append_byte(r, blue);
    return r;
}

std::vector<double> ws2811_test_rgb_and_reset(U8 red, U8 green, U8 blue, double resetDuration)
{
    std::vector<double> r = ws2811_test_rgb(red, green, blue);
    r.back() = resetDuration;
    return r;
}

U64 rgb_triple_as_u64(U16 red, U16 green, U16 blue)
{
    U64 result = 0;
    U16* channels = reinterpret_cast<U16*>(&result);
    channels[0] = red;
    channels[1] = green;
    channels[2] = blue;
    return result;
}

void setupStandardTestSettings(Instance& plugin, const std::string& controllerName)
{
    auto mockSettings = MockSettings::MockFromSettings(plugin.GetSettings());

    mockSettings->GetSetting("LED Channel")->mChannel = TEST_CHANNEL;
    mockSettings->GetSetting("LED Controller")->SetNumberedListIndexByLabel(controllerName);

    // push values into the plugin
    plugin.GetSettings()->SetSettingsFromInterfaces();
}

void testFunction1()
{
    Instance pluginInstance;
    pluginInstance.CreatePlugin("Addressable LEDs (Async)");

    setupStandardTestSettings(pluginInstance, "WS2811");

    pluginInstance.SetSampleRate(20000000);

    MockChannelData channelData(&pluginInstance);
    channelData.TestSetInitialBitState(BIT_LOW);
    channelData.TestAppendIntervals(pluginInstance.GetSampleRate(), 0.0,
                                    55_us,
                                    // first packet
                                    ws2811_test_rgb(0xaa, 0xbb, 0xcc),
                                    ws2811_test_rgb(0x22, 0x33, 0x44),
                                    ws2811_test_rgb_and_reset(0x66, 0x77, 0x88, 55_us),
                                    // second packet
                                    ws2811_test_rgb(0xaa, 0xbb, 0xcc),
                                    ws2811_test_rgb(0x22, 0x33, 0x44),
                                    ws2811_test_rgb_and_reset(0x66, 0x77, 0x88, 55_us)
                                    );

    pluginInstance.SetChannelData(TEST_CHANNEL, &channelData);
    auto rr= pluginInstance.RunAnalyzerWorker();
    // ensure we consumed all the data
    TEST_VERIFY_EQ(rr, Instance::WorkerRanOutOfData);

    // validation
    auto results = MockResultData::MockFromResults(pluginInstance.GetResults());

    TEST_VERIFY_EQ(results->TotalFrameCount(), 6);
    TEST_VERIFY_EQ(results->TotalCommitCount(), 8) // commit per frame and per packet right now;
    TEST_VERIFY_EQ(results->TotalPacketCount(), 3); // FIXME - analyzer is appending a final packet

    // verify LED indices between reset pulses
    TEST_VERIFY_EQ(results->GetFrame(2).mData2, 2);
    TEST_VERIFY_EQ(results->GetFrame(3).mData2, 0);
    TEST_VERIFY_EQ(results->GetFrame(5).mData2, 2);

    TEST_VERIFY_EQ(results->GetFrame(2).mData1, rgb_triple_as_u64(0x66, 0x77, 0x88));
    TEST_VERIFY_EQ(results->GetFrame(4).mData1, rgb_triple_as_u64(0x22, 0x33, 0x44));

    // verify packets
    TEST_VERIFY_EQ(results->GetFrameRangeForPacket(0), MockResultData::FrameRange(0, 2));
    TEST_VERIFY_EQ(results->GetFrameRangeForPacket(1), MockResultData::FrameRange(3, 5));

    std::cout << "passed test 1 ok" << std::endl;
}

void testSettings()
{
    Instance pluginInstance;
    pluginInstance.CreatePlugin("Addressable LEDs (Async)");


    auto mock = MockSettings::MockFromSettings(pluginInstance.GetSettings());

    // should default to channel zero
    TEST_VERIFY_EQ(mock->mChannels.size(), 1);
    TEST_VERIFY_EQ(mock->mChannels.at(0).channel, UNDEFINED_CHANNEL);
    TEST_VERIFY_EQ(mock->mChannels.at(0).label, "Addressable LEDs (Async)");
    TEST_VERIFY_EQ(mock->mChannels.at(0).used, false);

    // check which settings were defined
    TEST_VERIFY_EQ(mock->mInterfaces.size(), 2);

    auto channelSetting = mock->mInterfaces.at(0);
    TEST_VERIFY_EQ(channelSetting->GetType(), INTERFACE_CHANNEL);

    auto setting = mock->mInterfaces.at(1);
    TEST_VERIFY_EQ(setting->GetType(), INTERFACE_NUMBER_LIST);
    auto controllerSettingMock = MockSettingInterface::MockFromInterface(setting);

    TEST_VERIFY_EQ_CHARS(setting->GetTitle(), "LED Controller");
}

void testLoadSettings()
{
    AnalyzerTest::Instance pluginInstance;
    pluginInstance.CreatePlugin("Addressable LEDs (Async)");

    AnalyzerSettings* settings = pluginInstance.GetSettings();
}

void testFunction2()
{
    AnalyzerTest::Instance pluginInstance;
    pluginInstance.CreatePlugin("Addressable LEDs (Async)");
    
}

void testSimulationData1()
{

    AnalyzerTest::Instance pluginInstance;
    pluginInstance.CreatePlugin("Addressable LEDs (Async)");

    pluginInstance.RunSimulation(10000, 12000000);

    auto  mockSimulationData = pluginInstance.GetSimulationChannel(Channel(0, 0, DIGITAL_CHANNEL));

    // verification the generated simulation data
}

int main(int argc, char* argv[])
{
    testSettings();

    testFunction1();

    testFunction2();

    testSimulationData1();

    return EXIT_SUCCESS;
}
