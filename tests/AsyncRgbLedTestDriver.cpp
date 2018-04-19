#include "TestInstance.h"

#include "MockChannelData.h"
#include "MockResults.h"
#include "MockSettings.h"
#include "MockSimulatedChannelDescriptor.h"
#include "TestMacros.h"

#include <cmath>
#include <cassert>
#include <exception>

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

class LedChannelDataGenerator
{
public:
    struct PulseTiming
    {
        double min, mean, max; // all in seconds
    };

    struct BitTiming
    {
        PulseTiming hiPulse;
        PulseTiming lowPulse;
    };

    struct ModeTiming
    {
        BitTiming zero;
        BitTiming one;
    };

    void SetMockChannel(MockChannelData* mock)
    {
        mChannelData = mock;
    }

    void AddMode(const ModeTiming& mode)
    {
        mModes.push_back(mode);
    }

    void SetResetDuration(double d)
    {
        mResetDuration = d;
    }

    void SetTolerance(double d)
    {
        mTolerance = d;
    }

    void SetSampleRate(U32 sampleRateHz)
    {
        mSampleRate = sampleRateHz;
    }

    void SetGRBLayout()
    {
        mGRBLayout = true;

    }
    double computePulse(const PulseTiming& t)
    {
        const double actualMean = (t.min + t.max) * 0.5,
              range = t.max - t.min;

        // we want to offset by a random amount of the range that's less
        // than the tolerance.
        return actualMean + (drand48() * range * mTolerance * 0.5);
    }

    void appendChannelWord(std::vector<double>& result, U16 byte)
    {
        const ModeTiming& tm = mModes.at(mModeIndex);

        for (int bitIndex = mChannelBitSize - 1; bitIndex >= 0; --bitIndex) {
            const bool b = byte >> (bitIndex) & 1;
            const auto bitTiming = b ? tm.one : tm.zero;
            result.push_back(computePulse(bitTiming.hiPulse));
            result.push_back(computePulse(bitTiming.lowPulse));
        }
    }

    void appendRGB(std::vector<double>& result, U16 red, U16 green, U16 blue)
    {
        if (mGRBLayout) {
            appendChannelWord(result, green);
            appendChannelWord(result, red);
        } else {
            appendChannelWord(result, red);
            appendChannelWord(result, green);
        }
        appendChannelWord(result, blue);
    }

    U8 parseHexChar(const char c)
    {
        if (isdigit(c)) return c - '0';
        if ((c >= 'a') && (c <= 'f')) return c - 'a';
        if ((c >= 'A') && (c <= 'F')) return c - 'A';
        throw std::invalid_argument("Can't parse CSS color");
    }

    void appendCSSColor(std::vector<double>& result, const std::string& css)
    {
        // basic CSS color parsing
        assert(css.at(0) == '#');
        appendRGB(result,
                  parseHexChar(css.at(1)) << 4 | parseHexChar(css.at(2)),
                  parseHexChar(css.at(3)) << 4 | parseHexChar(css.at(4)),
                  parseHexChar(css.at(5)) << 4 | parseHexChar(css.at(6))
                  );
    }

    double resetPulseDuration() const
    {
        return mResetDuration;
    }

    void appendFrameFromCSS(const std::string& cssColors, bool reset = true)
    {
        auto currentPos = 0;
        std::vector<double> result;
        for ( ;; ) {
            auto nextComma = cssColors.find(',', currentPos);
            std::string color;
            if (nextComma == std::string::npos) {
                // last value
                color = cssColors.substr(currentPos);
            } else {
                color = cssColors.substr(currentPos, nextComma - currentPos);
                currentPos = nextComma;
            }

            appendCSSColor(result, color);
            if (nextComma == std::string::npos) {
                break;
            }
        }

        if (reset) {
            result.back() = resetPulseDuration();
        }

        mAccumulatedError =
                mChannelData->TestAppendIntervals(mSampleRate, mAccumulatedError, result);
    }

    void appendFromText(const std::string& text)
    {
        for (int currentPos = 0 ; currentPos != std::string::npos ; ) {
            std::string token;
            auto nextComma = text.find(',', currentPos);
            if (nextComma == std::string::npos) {
                token = text.substr(currentPos);
                currentPos = nextComma;
            } else {
                token = text.substr(currentPos, nextComma - currentPos);
                currentPos = nextComma + 1;
            }

            if (token.empty()) {
                continue;
            } else if (token == "reset") {
                mAccumulatedError =
                        mChannelData->TestAppendIntervals(mSampleRate, mAccumulatedError, resetPulseDuration());
            } else if (token.at(0) == '#') {
                // append a simple color
                std::vector<double> result;
                appendCSSColor(result, token);

                if (token.rfind("_reset") != std::string::npos) {
                    result.back() = resetPulseDuration();
                }

                mAccumulatedError =
                        mChannelData->TestAppendIntervals(mSampleRate, mAccumulatedError, result);
            }
        } // of token parsing loop
    }
private:
    const U8 mChannelBitSize = 8;
    U8 mModeIndex = 0; // 0 - normal speed, 1 = hi-speed if available


    U32 mSampleRate = 12000000;
    double mAccumulatedError = 0.0;
    double mResetDuration = 55_us;
    double mTolerance = 0.0;
    std::vector<ModeTiming> mModes;
    bool mGRBLayout = false;
    MockChannelData* mChannelData = nullptr;
};

const LedChannelDataGenerator::ModeTiming WS2811_normal_speed = {
    {{350_ns, 500_ns, 650_ns}, {1850_ns, 2000_ns, 2150_ns}},  // zero bit
    {{1050_ns, 1200_ns, 1350_ns}, {1150_ns, 1300_ns, 1450_ns}}  // one bit
};

const LedChannelDataGenerator::ModeTiming WS2811_high_speed = {
    {{175_ns, 250_ns, 325_ns}, {925_ns, 1000_ns, 1075_ns}},      // 0-bit times
    {{525_ns, 600_ns, 675_ns}, {575_ns, 650_ns, 725_ns}}      // 1-bit times
};

const LedChannelDataGenerator::ModeTiming WS2812B = {
    {{250_ns, 400_ns, 550_ns}, {700_ns, 850_ns, 1000_ns}},     // 0-bit times
    {{650_ns, 800_ns, 950_ns}, {300_ns, 450_ns, 600_ns}}  // 1-bit times
};

const LedChannelDataGenerator::ModeTiming WS2813 = {
    {{300_ns, 375_ns, 450_ns}, {300_ns, 875_ns, 100_us}},     // 0-bit times
    {{750_ns, 875_ns, 1000_ns}, {300_ns, 375_ns, 100_us}},  // 1-bit times
};

const LedChannelDataGenerator::ModeTiming TM1809_normal_speed = {
    {{450_ns, 600_ns, 750_ns}, {1050_ns, 1200_ns, 1350_ns}},     // 0-bit times
    {{1050_ns, 1200_ns, 1350_ns}, {450_ns, 600_ns, 750_ns}}  // 1-bit times
};

const LedChannelDataGenerator::ModeTiming TM1809_high_speed = {
    {{250_ns, 320_ns, 390_ns}, {530_ns, 600_ns, 670_ns}},      // 0-bit times
    {{530_ns, 600_ns, 670_ns}, {250_ns, 320_ns, 390_ns}}       // 1-bit times
};

const LedChannelDataGenerator::ModeTiming UCS1903_normal_speed = {
    {{350_ns, 500_ns, 650_ns}, {1850_ns, 2000_ns, 2150_ns}},     // 0-bit times
    {{1850_ns, 2000_ns, 2150_ns}, {350_ns, 500_ns, 650_ns}},  // 1-bit times
};

const LedChannelDataGenerator::ModeTiming UCS1903_high_speed = {
    {{175_ns, 250_ns, 325_ns}, {925_ns, 1000_ns, 1075_ns}},      // 0-bit times
    {{925_ns, 1000_ns, 1075_ns}, {175_ns, 250_ns, 325_ns}}      // 1-bit times
};

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
    plugin.SetSampleRate(20000000);

    auto mockSettings = MockSettings::MockFromSettings(plugin.GetSettings());

    mockSettings->GetSetting("LED Channel")->mChannel = TEST_CHANNEL;
    mockSettings->GetSetting("LED Controller")->SetNumberedListIndexByLabel(controllerName);

    // push values into the plugin
    plugin.GetSettings()->SetSettingsFromInterfaces();
}

void testBasicAnalysis(const std::string& controller,
                       LedChannelDataGenerator* generator)
{
    Instance pluginInstance{"Addressable LEDs (Async)"};
    setupStandardTestSettings(pluginInstance, controller);

    MockChannelData channelData(&pluginInstance);
    channelData.TestSetInitialBitState(BIT_LOW);

    generator->SetSampleRate(pluginInstance.GetSampleRate());
    generator->SetMockChannel(&channelData);
    generator->appendFromText("reset,"
                                 "#aabbcc,#223344,#667788,#cfcfcf,#deadbe,#7f7f7f_reset,"
                                 "#aabbcc,#223344,#667788,#998877,#eeddff,#123456_reset"
                                 );

    pluginInstance.SetChannelData(TEST_CHANNEL, &channelData);
    auto rr= pluginInstance.RunAnalyzerWorker();
    // ensure we consumed all the data
    TEST_VERIFY_EQ(rr, Instance::WorkerRanOutOfData);

    // validation
    auto results = MockResultData::MockFromResults(pluginInstance.GetResults());

    TEST_VERIFY_EQ(results->TotalFrameCount(), 12);
    TEST_VERIFY_EQ(results->TotalCommitCount(), 14) // commit per frame and per packet right now;
    TEST_VERIFY_EQ(results->TotalPacketCount(), 3); // FIXME - analyzer is appending a final packet

    // verify LED indices between reset pulses
    TEST_VERIFY_EQ(results->GetFrame(2).mData2, 2);
    TEST_VERIFY_EQ(results->GetFrame(3).mData2, 3);
    TEST_VERIFY_EQ(results->GetFrame(5).mData2, 5);
    TEST_VERIFY_EQ(results->GetFrame(6).mData2, 0);
    TEST_VERIFY_EQ(results->GetFrame(7).mData2, 1);
    TEST_VERIFY_EQ(results->GetFrame(11).mData2, 5);

    TEST_VERIFY_EQ(results->GetFrame(2).mData1, rgb_triple_as_u64(0x66, 0x77, 0x88));
    TEST_VERIFY_EQ(results->GetFrame(7).mData1, rgb_triple_as_u64(0x22, 0x33, 0x44));

    // verify packets
    TEST_VERIFY_EQ(results->GetFrameRangeForPacket(0), MockResultData::FrameRange(0, 5));
    TEST_VERIFY_EQ(results->GetFrameRangeForPacket(1), MockResultData::FrameRange(6, 11));

    // bubble text generation
    pluginInstance.GenerateBubbleText(2, TEST_CHANNEL, Decimal);
    TEST_VERIFY_EQ(results->TotalStringCount(), 4);
    TEST_VERIFY_EQ(results->GetString(0), "LED 2 Red: 102 Green: 119 Blue: 136 #667788")
    TEST_VERIFY_EQ(results->GetString(1), "2 R: 102 G: 119 B: 136 #667788")
    TEST_VERIFY_EQ(results->GetString(2), "(2) #667788")
    TEST_VERIFY_EQ(results->GetString(3), "#667788")

    // tabular text generation
     pluginInstance.GenerateTabularText(2, Decimal);
    TEST_VERIFY_EQ(results->TotalTabularTextCount(), 1);
    TEST_VERIFY_EQ(results->GetTabularText(0), "[2] 102, 119, 136");

    std::cout << "passed test basic analysis ok for " << controller << std::endl;
}

void testSettings()
{
    Instance pluginInstance{"Addressable LEDs (Async)"};
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
    Instance pluginInstance{"Addressable LEDs (Async)"};
    AnalyzerSettings* settings = pluginInstance.GetSettings();
}

void testSynchronizeMidData()
{
    Instance pluginInstance{"Addressable LEDs (Async)"};
    setupStandardTestSettings(pluginInstance, "WS2811");

    MockChannelData channelData(&pluginInstance);
    channelData.TestSetInitialBitState(BIT_LOW);
    channelData.TestAppendIntervals(pluginInstance.GetSampleRate(), 0.0,
                                    50_us,
                                    // first packet
                                    ws2811_test_rgb(0xaa, 0xbb, 0xcc),
                                    ws2811_test_rgb(0x22, 0x33, 0x44),
                                    ws2811_test_rgb_and_reset(0x11, 0x22, 0x33, 55_us),
                                    // second packet
                                    ws2811_test_rgb(0x44, 0x55, 0x66),
                                    ws2811_test_rgb(0x22, 0x33, 0x44),
                                    ws2811_test_rgb_and_reset(0x66, 0x77, 0x88, 55_us),
                                    // third packet
                                    ws2811_test_rgb(0xaa, 0xbb, 0xcc),
                                    ws2811_test_rgb(0x99, 0x88, 0x77),
                                    ws2811_test_rgb_and_reset(0x66, 0x77, 0x88, 55_us)
                                    );


    // advance part-way through the first packet, so the analyzer has to sync
    // to the next reset pulse
    channelData.ResetCurrentSample(1000);
    pluginInstance.SetChannelData(TEST_CHANNEL, &channelData);
    auto rr= pluginInstance.RunAnalyzerWorker();
    // ensure we consumed all the data
    TEST_VERIFY_EQ(rr, Instance::WorkerRanOutOfData);

    // validation
    auto results = MockResultData::MockFromResults(pluginInstance.GetResults());

    TEST_VERIFY_EQ(results->TotalFrameCount(), 6);
    TEST_VERIFY_EQ(results->TotalPacketCount(), 3); // FIXME, analyzer is appending an empty packet

    // verify LED indices between reset pulses
    TEST_VERIFY_EQ(results->GetFrame(1).mData2, 1);
    TEST_VERIFY_EQ(results->GetFrame(3).mData2, 0);

    TEST_VERIFY_EQ(results->GetFrame(0).mData1, rgb_triple_as_u64(0x44, 0x55, 0x66));
    TEST_VERIFY_EQ(results->GetFrame(4).mData1, rgb_triple_as_u64(0x99, 0x88, 0x77));

    // verify packets
    TEST_VERIFY_EQ(results->GetFrameRangeForPacket(0), MockResultData::FrameRange(0, 2));
    TEST_VERIFY_EQ(results->GetFrameRangeForPacket(1), MockResultData::FrameRange(3, 5));

    std::cout << "passed test: synchornize mid-stream" << std::endl;
}

void testResynchronizeAfterBadData()
{
    Instance pluginInstance{"Addressable LEDs (Async)"};
    setupStandardTestSettings(pluginInstance, "WS2811");

    MockChannelData channelData(&pluginInstance);
    channelData.TestSetInitialBitState(BIT_LOW);
    channelData.TestAppendIntervals(pluginInstance.GetSampleRate(), 0.0,
                                    50_us,
                                    // first packet
                                    ws2811_test_rgb(0xaa, 0xbb, 0xcc),
                                    ws2811_test_rgb(0x22, 0x33, 0x44),
                                    ws2811_test_rgb_and_reset(0x11, 0x22, 0x33, 55_us),
                                    // second packet (mangled!)
                                    ws2811_test_rgb(0x44, 0x55, 0x66),
                                    // garbage data
                                    150_ns, 1000_ns, 300_ns, 2000_ns, 400_ns, 3000_ns,
                                    ws2811_test_rgb_and_reset(0x66, 0x77, 0x88, 55_us),
                                    // third packet
                                    ws2811_test_rgb(0x11, 0x33, 0x22),
                                    ws2811_test_rgb(0x99, 0x88, 0x77),
                                    ws2811_test_rgb_and_reset(0x66, 0x77, 0x88, 55_us)
                                    );


    pluginInstance.SetChannelData(TEST_CHANNEL, &channelData);
    auto rr= pluginInstance.RunAnalyzerWorker();
    // ensure we consumed all the data
    TEST_VERIFY_EQ(rr, Instance::WorkerRanOutOfData);

    // validation
    auto results = MockResultData::MockFromResults(pluginInstance.GetResults());

    TEST_VERIFY_EQ(results->TotalFrameCount(), 7);
    TEST_VERIFY_EQ(results->TotalPacketCount(), 4); // FIXME, analyzer is appending an empty packet

    // verify LED indices between reset pulses
    TEST_VERIFY_EQ(results->GetFrame(1).mData2, 1);
    TEST_VERIFY_EQ(results->GetFrame(4).mData2, 0);

    TEST_VERIFY_EQ(results->GetFrame(0).mData1, rgb_triple_as_u64(0xaa, 0xbb, 0xcc));
    TEST_VERIFY_EQ(results->GetFrame(4).mData1, rgb_triple_as_u64(0x11, 0x33, 0x22));

    // verify packets
    TEST_VERIFY_EQ(results->GetFrameRangeForPacket(0), MockResultData::FrameRange(0, 2));
    TEST_VERIFY_EQ(results->GetFrameRangeForPacket(1), MockResultData::FrameRange(3, 3));
    TEST_VERIFY_EQ(results->GetFrameRangeForPacket(2), MockResultData::FrameRange(4, 6));

    std::cout << "passed test: re-synchronize after bad data mid-stream" << std::endl;
}

struct BitTiming {
    double highSec;
    double lowSec;
};

struct ModeTiming
{
    BitTiming zeroTiming;
    BitTiming oneTiming;
};

void verifyReset(SimulatedChannel* sim_chan,
                 double reset_time)
{
    TEST_EQ_EPSILON(reset_time, sim_chan->GetDurationToNextTransition(), sim_chan->GetSampleDuration());
}

/**
 * @brief canClassify - check if a pulse-pair matches the given timing spec
 * @param timing - nominal bit timings
 * @param hiSec
 * @param lowSec
 * @param epsilon
 * @return
 */
bool canClassify(const BitTiming& timing, double hiSec, double lowSec, double epsilon)
{
    return std::fabs(timing.highSec - hiSec) < epsilon &&
            std::fabs(timing.lowSec - lowSec) < epsilon;
}

bool classifyBit(const std::vector<ModeTiming>& timingData,
                 double hiSec, double lowSec, double epsilon,
                 BitState* state, int* modeIndex)
{
    int i = 0;
    for (auto mode : timingData) {
        if (canClassify(mode.oneTiming, hiSec, lowSec, epsilon)) {
            *state = BIT_HIGH;
            *modeIndex = i;
            return true;
        }

        if (canClassify(mode.zeroTiming, hiSec, lowSec, epsilon)) {
            *state = BIT_HIGH;
            *modeIndex = i;
            return true;
        }

        ++i;
    }

    std::cerr << "classifciation failure:" << hiSec << " / " << lowSec << std::endl;
    return false;
}

bool isResetPulse(double resetTime, double lowTime, double epsilon)
{
    return lowTime >= (resetTime - epsilon);
}

double fixupLowPulseDurationForReset(const ModeTiming& timing, double hiPulseSec, double epsilon)
{
    if (std::fabs(timing.zeroTiming.highSec - hiPulseSec) < epsilon)
        return timing.zeroTiming.lowSec;

    if (std::fabs(timing.oneTiming.highSec - hiPulseSec) < epsilon)
        return timing.oneTiming.lowSec;

    std::cerr << "fixupLowPulseDurationForReset: couldn't classify high pulse " << hiPulseSec << std::endl;
    exit(-1);
}

void parseSimulationData(SimulatedChannel* sim_chan,
                         double reset_time,
                         const std::vector<ModeTiming>& timingData)
{
    // assume simulation starts with a reset
    sim_chan->ResetToStart();
    TEST_VERIFY(sim_chan->GetCurrentState() ==  BIT_LOW);
    const double epsilon = sim_chan->GetSampleDuration();

    verifyReset(sim_chan, reset_time);
    sim_chan->AdvanceToNextTransition();
    int modeIndex = -1; // no mode selected
    int bitCount = 0;

    int channel = 0;
    U16 channelData[4] = {0,0,0,0};
    const int channelDataSize = 8;

    for ( ; ; ) {
        bool sawReset = false;
        double hiPulseSec = sim_chan->GetDurationToNextTransition();
        if (!sim_chan->AdvanceToNextTransition()) {
            break;
        }

        double lowPulseSec = sim_chan->GetDurationToNextTransition();
        if (isResetPulse(reset_time, lowPulseSec, epsilon)) {
            TEST_VERIFY(modeIndex >= 0);
            // fudge the length back to something sensible so that classifyBit doesn't
            // need to deal with reset pulses
            lowPulseSec = fixupLowPulseDurationForReset(timingData.at(modeIndex),
                                                        hiPulseSec, epsilon);

            // reset the mode detection, since the simulator might switch
            // to or from high-speed mode on a reset
            modeIndex = -1;
            sawReset = true;
        }

        BitState bs;
        bool ok = classifyBit(timingData, hiPulseSec, lowPulseSec, epsilon, &bs, &modeIndex);
        if (!ok) {
            std::cerr << "failed to classify at " << sim_chan->GetCurrentSample() << std::endl;
            break;
        } else {
            // shift and add the bit into channelData
            channelData[channel] =  (channelData[channel] << 1) | bs;
            if (++bitCount == channelDataSize) {
                bitCount = 0;
                ++channel;

                if (channel == 3) {
                    memset(&channelData, 0, sizeof(channelData));
                    channel = 0;
                }
            }
        }

        if (!sim_chan->AdvanceToNextTransition()) {
            break;
        }
    } // of pulse detection loop
}

void testSimulationData1()
{
    Instance pluginInstance{"Addressable LEDs (Async)"};
    setupStandardTestSettings(pluginInstance, "WS2811");

    const U64 numSamplesToGenerate = 1000000;
    // use 20Mhz mode to generate mixture of low and high-speed samples
    pluginInstance.RunSimulation(numSamplesToGenerate, 20000000);
    auto mockSimulationData = pluginInstance.GetSimulationChannel(TEST_CHANNEL);
    TEST_VERIFY(mockSimulationData);
    TEST_VERIFY(mockSimulationData->GetSampleCount() >= 10000);

    // verify the generated simulation data
    parseSimulationData(mockSimulationData, 50_us, {
                            ModeTiming{ {500_ns, 2000_ns}, {1200_ns, 1300_ns} },
                            ModeTiming{ {250_ns, 1000_ns}, {600_ns, 650_ns} }
                        });

    TEST_VERIFY(mockSimulationData->GetCurrentSample() >= numSamplesToGenerate);

    std::cout << "did parse simulation data" << std::endl;
}

int main(int argc, char* argv[])
{
    testSettings();

    std::vector<double> tolerances = {0.0, 0.5, 0.7};
    for (double t : tolerances)
    {
        LedChannelDataGenerator gen;
        gen.SetTolerance(t);
        gen.AddMode(WS2811_normal_speed);
        testBasicAnalysis("WS2811", &gen);
    }

    for (double t : tolerances)
    {
        LedChannelDataGenerator gen;
        gen.SetTolerance(t);
        gen.AddMode(WS2811_high_speed);
        testBasicAnalysis("WS2811", &gen);
    }

    for (double t : tolerances)
    {
        LedChannelDataGenerator gen;
        gen.SetTolerance(t);
        gen.AddMode(WS2812B);
        gen.SetGRBLayout();
        testBasicAnalysis("WS2812B", &gen);
    }

    for (double t : tolerances)
    {
        LedChannelDataGenerator gen;
        gen.SetTolerance(t);
        gen.AddMode(TM1809_normal_speed);
        testBasicAnalysis("TM1809", &gen);
    }

    for (double t : tolerances)
    {
        LedChannelDataGenerator gen;
        gen.SetTolerance(t);
        gen.AddMode(TM1809_high_speed);
        testBasicAnalysis("TM1809", &gen);
    }

    for (double t : tolerances)
    {
        LedChannelDataGenerator gen;
        gen.SetTolerance(t);
        gen.AddMode(UCS1903_normal_speed);
        testBasicAnalysis("UCS1903", &gen);
    }

    for (double t : tolerances)
    {
        LedChannelDataGenerator gen;
        gen.SetTolerance(t);
        gen.AddMode(UCS1903_high_speed);
        testBasicAnalysis("UCS1903", &gen);
    }

    testSynchronizeMidData();
    testResynchronizeAfterBadData();

    testSimulationData1();

    return EXIT_SUCCESS;
}
