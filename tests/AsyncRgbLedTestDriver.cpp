#include "TestInstance.h"

#include "MockChannelData.h"
#include "MockResults.h"

#include <cassert>

#define TEST_VERIFY_EQ(a, b) \
    assert(a == b)

void testFunction1()
{
    AnalyzerTest::Instance pluginInstance;
    pluginInstance.CreatePlugin();

    // setup test settings

    AnalyzerTest::MockChannelData channelData;
    channelData.TestSetInitialBitState(BIT_LOW);
    channelData.TestAppendTransitions({4, 6, 2, 4, 6, 1, 2, 8, 19, 4});

    pluginInstance.SetChannelData(Channel(0, 0, DIGITAL_CHANNEL), &channelData);
    // other channels as required

    pluginInstance.RunAnalyzerWorker();

    // validation

    auto results = AnalyzerTest::MockResultData::MockFromResults(pluginInstance.GetResults());

    TEST_VERIFY_EQ(results->GetFrame(2).mData1, 1234);
}

void testFunction2()
{
    AnalyzerTest::Instance pluginInstance;
    pluginInstance.CreatePlugin();
    
}

void testSimulationData1()
{

    AnalyzerTest::Instance pluginInstance;
    pluginInstance.CreatePlugin();

    pluginInstance.RunSimulation(10000, 12000000);

    auto  mockSimulationData = pluginInstance.GetSimulationChannel(Channel(0, 0, DIGITAL_CHANNEL));

    // verification the generated simulation data
}

int main(int argc, char* argv[])
{
    testFunction1();

    testFunction2();

    testSimulationData1();

    return EXIT_SUCCESS;
}
