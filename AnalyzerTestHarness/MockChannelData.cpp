#include "MockChannelData.h"

#include <cassert>

namespace AnalyzerTest
{

MockChannelData::MockChannelData() :
    AnalyzerChannelData(nullptr)
{

}

void MockChannelData::TestSetInitialBitState(BitState bs)
{
    mInitialState = bs;
}

void MockChannelData::TestAppendTransitionAfterSamples(U64 sampleCount)
{
    assert(sampleCount > 0);
    U64 currentTrans = mTransitions.empty() ? 0 : mTransitions.back();
    mTransitions.push_back(currentTrans + sampleCount);
}

void MockChannelData::TestAppendTransitions(const std::vector<U64> &transitions)
{
    for (auto t : transitions) {
        TestAppendTransitionAfterSamples(t);
    }
}

} // of namespace AnalyzerTest


/////////////////////////////////////////////////////////////////////////////

#define D_PTR() \
    auto * d = reinterpret_cast<AnalyzerTest::MockChannelData*>(this);

AnalyzerChannelData::AnalyzerChannelData(ChannelData *channel_data)
{
}

AnalyzerChannelData::~AnalyzerChannelData()
{

}

U64 AnalyzerChannelData::GetSampleNumber()
{
    D_PTR();
}

BitState AnalyzerChannelData::GetBitState()
{
    D_PTR();
}

U32 AnalyzerChannelData::Advance(U32 num_samples)
{

}

U32 AnalyzerChannelData::AdvanceToAbsPosition(U64 sample_number)
{

}

void AnalyzerChannelData::AdvanceToNextEdge()
{

}

U64 AnalyzerChannelData::GetSampleOfNextEdge()
{

}

bool AnalyzerChannelData::WouldAdvancingCauseTransition(U32 num_samples)
{

}

bool AnalyzerChannelData::WouldAdvancingToAbsPositionCauseTransition(U64 sample_number)
{

}
