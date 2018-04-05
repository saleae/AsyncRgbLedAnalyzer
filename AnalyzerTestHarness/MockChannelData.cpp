#include "MockChannelData.h"

#include <cassert>
#include <algorithm>
#include <exception>

namespace AnalyzerTest
{

BitState InvertBitState(BitState b)
{
    if (b == BIT_HIGH) return BIT_LOW;
    return BIT_HIGH;
}

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

void MockChannelData::ResetCurrentSample(U64 sampleNumber)
{
    mCurrentState = mInitialState;
    mCurrentSample = 0;
    AdvanceToSample(sampleNumber);
}

U32 MockChannelData::AdvanceToSample(U64 sample)
{
    assert(sample >= mCurrentSample);
    if (sample == mCurrentSample)
        return 0;

    auto cur = std::lower_bound(mTransitions.begin(), mTransitions.end(), mCurrentSample);
    if (cur == mTransitions.end()) {
        throw std::runtime_error("AdvanceToSample: out of samples");
    }

    auto it = std::lower_bound(mTransitions.begin(), mTransitions.end(), sample);
    if (it == mTransitions.end()) {
        throw std::runtime_error("AdvanceToSample: out of samples");
    }

    if (it == cur) {
        // no transitions between current and requested sample
        // short circuit to avoud complicating the logic below
        mCurrentSample = sample;
        return 0;
    }

    if (*it > sample) {
        // we didn't match exactly, so take the transition preceeding
        --it;
    }

    // count the distance between the two
    U32 transitionCount = std::distance(cur, it);
    bool oddTransitionCount = transitionCount % 1;
    if (oddTransitionCount) {
        mCurrentState = InvertBitState(mCurrentState);
    }

    mCurrentSample = sample;
    return transitionCount;
}

double MockChannelData::InnerAppendIntervals(U64 sampleRateHz, double startingError, const std::vector<double> &intervals)
{
    // same logic as built-in ClockGenerator, track accumulated error to ensure
    // we don't consistently under-sample by rounding down

    const double sampleDuration = 1.0 / sampleRateHz;
    double currentError = startingError;

    for (auto i : intervals) {
        double nominalSamples = (i / sampleDuration) + currentError;
        const U32 samplesToAdvance = static_cast<U32>(nominalSamples);
        currentError = nominalSamples - static_cast<double>(samplesToAdvance);
        TestAppendTransitionAfterSamples(samplesToAdvance);
    }

    return currentError;
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
    return d->mCurrentSample;
}

BitState AnalyzerChannelData::GetBitState()
{
    D_PTR();
    return d->mCurrentState;
}

U32 AnalyzerChannelData::Advance(U32 num_samples)
{
    D_PTR();
    return d->AdvanceToSample(d->mCurrentSample + num_samples);
}

U32 AnalyzerChannelData::AdvanceToAbsPosition(U64 sample_number)
{
    D_PTR();
    return d->AdvanceToSample(sample_number);
}

void AnalyzerChannelData::AdvanceToNextEdge()
{
    D_PTR();
    U64 nextEdge = GetSampleOfNextEdge();
    d->mCurrentState = AnalyzerTest::InvertBitState(d->mCurrentState);
    d->mCurrentSample = nextEdge;
}

U64 AnalyzerChannelData::GetSampleOfNextEdge()
{
    D_PTR();
    auto next = std::lower_bound(d->mTransitions.begin(), d->mTransitions.end(), d->mCurrentSample);
    if (next == d->mTransitions.end()) {
        throw std::runtime_error("AdvanceToNextEdge: out of samples");
    }

    if (*next == d->mCurrentSample) {
        // if we're exactly on the sample, advance
        ++next;
        if (next == d->mTransitions.end()) {
            throw std::runtime_error("AdvanceToNextEdge: out of samples");
        }
    }

    return *next;
}

bool AnalyzerChannelData::WouldAdvancingCauseTransition(U32 num_samples)
{
    D_PTR();
    return WouldAdvancingToAbsPositionCauseTransition(d->mCurrentSample + num_samples);
}

bool AnalyzerChannelData::WouldAdvancingToAbsPositionCauseTransition(U64 sample_number)
{
    D_PTR();
    auto next = std::lower_bound(d->mTransitions.begin(), d->mTransitions.end(), d->mCurrentSample);
    if (next == d->mTransitions.end()) {
        return false;
    }

    if (*next == d->mCurrentSample) {
        ++next;
        if (next == d->mTransitions.end()) {
            return false;
        }
    }

    return (*next <= sample_number);
}
