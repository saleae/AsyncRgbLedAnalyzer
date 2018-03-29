#ifndef ANALYZER_TEST_MOCK_CHANNEL_DATA
#define ANALYZER_TEST_MOCK_CHANNEL_DATA

#include "AnalyzerChannelData.h"

namespace AnalyzerTest
{

class MockChannelData : public AnalyzerChannelData
{
public:
    MockChannelData();

    // test interface

    void TestSetInitialBitState(BitState bs);

    void TestAppendTransitionAfterSamples(U64 sampleCount);

    void TestAppendTransitions(const std::vector<U64>& transitions);

private:
    BitState mCurrentState = BIT_LOW;
    U64 mCurrentSample = 0;

    BitState mInitialState = BIT_LOW
            ;
    // absolute sample numbers of transitions
    std::vector<U64> mTransitions;
};


} // of namespace AnalyzerTest


#endif // of ANALYZER_TEST_MOCK_CHANNEL_DATA
