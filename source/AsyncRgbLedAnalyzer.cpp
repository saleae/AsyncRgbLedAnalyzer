#include "AsyncRgbLedAnalyzer.h"
#include "AsyncRgbLedAnalyzerSettings.h"
#include "AsyncRgbLedAnalyzerResults.h"

#include <AnalyzerChannelData.h>

#include <iostream>

AsyncRgbLedAnalyzer::AsyncRgbLedAnalyzer()
:	Analyzer2(),  
	mSettings( new AsyncRgbLedAnalyzerSettings )
{
	SetAnalyzerSettings( mSettings.get() );
}

AsyncRgbLedAnalyzer::~AsyncRgbLedAnalyzer()
{
	KillThread();
}

void AsyncRgbLedAnalyzer::SetupResults()
{
	mResults.reset( new AsyncRgbLedAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mInputChannel );
}

void AsyncRgbLedAnalyzer::WorkerThread()
{
	mSampleRateHz = GetSampleRate();
	mChannelData = GetAnalyzerChannelData( mSettings->mInputChannel );

    // cache this value here to avoid recomputing this every bit-read
    if (mSettings->IsHighSpeedSupported()) {
        mMinimumLowDurationSec = std::min(mSettings->DataTiming(BIT_LOW, true).mNegativeTiming.mMinimumSec,
                                          mSettings->DataTiming(BIT_HIGH, true).mNegativeTiming.mMinimumSec);
    } else {
        mMinimumLowDurationSec = std::min(mSettings->DataTiming(BIT_LOW).mNegativeTiming.mMinimumSec,
                                          mSettings->DataTiming(BIT_HIGH).mNegativeTiming.mMinimumSec);
    }

    bool isResyncNeeded = true;
	for( ; ; )
	{
        if (isResyncNeeded) {
            SynchronizeToReset();
            isResyncNeeded = false;
        }

		U32 frameInPacketIndex = 0;
		mResults->CommitPacketAndStartNewPacket();

		// data word reading loop
		for ( ; ; ) {
            auto result = ReadRGBTriple();
            if (result.mValid) {
                Frame frame;
                frame.mFlags = 0;
                frame.mStartingSampleInclusive = result.mValueBeginSample;
                frame.mStartingSampleInclusive = result.mValueEndSample;
                frame.mData1 = result.mRGB.ConvertToU64();
                frame.mData2 = frameInPacketIndex++;
                mResults->AddFrame( frame );
            } else {
                // something error occurred, let's resynchronise
                isResyncNeeded = true;
            }

            if (isResyncNeeded || result.mIsReset) {
                break;
            }
		}

		mResults->CommitResults();
        ReportProgress( mChannelData->GetSampleNumber() );
	}
}

void AsyncRgbLedAnalyzer::SynchronizeToReset()
{
    if (mChannelData->GetBitState() == BIT_HIGH) {
        mChannelData->AdvanceToNextEdge();
    }

    for ( ; ; ) {
        const U64 lowTransition = mChannelData->GetSampleNumber();
        const U64 highTransition = mChannelData->GetSampleOfNextEdge();
        double lowTimeSec = (highTransition - lowTransition) / mSampleRateHz;
        if (lowTimeSec > mSettings->ResetTiming().mMinimumSec) {
            // it's a reset, we are done
            // advance to the end of the reset, ready for the first
            // ReadRGB / ReadBit
            mChannelData->AdvanceToAbsPosition(highTransition);
            return;
        }

        // advance past the rising edge, to the next falling edge,
        // which is our next candidate for the beginning of a RESET
        mChannelData->AdvanceToAbsPosition(highTransition);
        mChannelData->AdvanceToNextEdge();
    }
}

auto AsyncRgbLedAnalyzer::ReadRGBTriple() -> RGBResult
{
    const U8 bitSize =  mSettings->BitSize();
    U16 channels[3] = {0,0,0};
    RGBResult result;

    DataBuilder builder;
    int channel = 0;
    for ( ; channel < 3; ) {
        U64 value = 0;
        builder.Reset(&value, AnalyzerEnums::MsbFirst, bitSize);
        int i = 0;
        for ( ; i<bitSize; ++i) {
            auto bitResult = ReadBit();
            if (!bitResult.mValid) {
                break;
            }

            // for the first bit of channel 0, record the beginning time
            // for accurate frame positions in the results
            if ((i == 0) && (channel == 0)) {
                result.mValueBeginSample = bitResult.mBeginSample;
            }

            result.mValueEndSample = bitResult.mEndSample;
            builder.AddBit(bitResult.mBitValue);
            if (bitResult.mIsReset) {
                result.mIsReset = true;
                break;
            }
        }

        if (i == bitSize) {
            // we saw a complete channel, save it
            channels[channel++] = value;
        } else {
            // partial data due to reset or invalid timing, discard
            break;
        }
    }

    if (channel == 3) {
        // we saw three complete channels, we can use this
        result.mRGB = RGBValue::CreateFromControllerOrder(mSettings->GetColorLayout(), channels);
        result.mValid = true;
    } // in all other cases, mValid stays false - no RGB data was written

    return result;
}

auto AsyncRgbLedAnalyzer::ReadBit() -> ReadResult
{
    ReadResult result;
    result.mValid = false;
    if (mChannelData->GetBitState() == BIT_LOW) {
        mChannelData->AdvanceToNextEdge();
    }

    result.mBeginSample = mChannelData->GetSampleNumber();
    mChannelData->AdvanceToNextEdge();
    const U64 fallingEdgeSample = mChannelData->GetSampleNumber();

    bool isHighSpeed = false;
    const double highTimeSec = (fallingEdgeSample - result.mBeginSample) / mSampleRateHz;

    // try regular speed
    if (mSettings->DataTiming(BIT_LOW).mPositiveTiming.WithinTolerance(highTimeSec)) {
        result.mBitValue = BIT_LOW;
    } else if (mSettings->DataTiming(BIT_HIGH).mPositiveTiming.WithinTolerance(highTimeSec)) {
        result.mBitValue = BIT_HIGH;
    } else if (mSettings->IsHighSpeedSupported()) {
        // try high-speed modes
        if (mSettings->DataTiming(BIT_LOW, true).mPositiveTiming.WithinTolerance(highTimeSec)) {
            result.mBitValue = BIT_LOW;
            isHighSpeed = true;
        } else if (mSettings->DataTiming(BIT_HIGH, true).mPositiveTiming.WithinTolerance(highTimeSec)) {
            result.mBitValue = BIT_HIGH;
            isHighSpeed = true;
        } else {
            mChannelData->AdvanceToAbsPosition(fallingEdgeSample);
            return result; // invalid result, reset required
        }
    } else {
        // no high speed mode, so we are done
        mChannelData->AdvanceToAbsPosition(fallingEdgeSample);
        return result; // invalid result, reset required
    }

    // the positive (high) sample looks valid and we've classified it
    // and set isHighSpeed accordingly. Now we need to see if the negative
    // side corresponds.

    // check for a too-short low timing
    if (mChannelData->WouldAdvancingCauseTransition(mMinimumLowDurationSec * mSampleRateHz)) {
        mChannelData->AdvanceToNextEdge();
        return result; // invalid result, reset required
    }

    // check for a low period exceeding the minimum reset time
    // if we exceed that, this is a reset
    const int minResetSamples = static_cast<int>(mSettings->ResetTiming().mMinimumSec * mSampleRateHz);
    if (!mChannelData->WouldAdvancingCauseTransition(minResetSamples)) {
        mChannelData->Advance(minResetSamples);
        result.mIsReset = true;
    } else {
        // we saw a transition, let's see the timing
        mChannelData->AdvanceToNextEdge();

        // the -1 is so the end of this frame, and start of the next, don't
        // overlap.
        result.mEndSample = mChannelData->GetSampleNumber() - 1;
    }

    // consistency checks - classify low time, and ensure it corresponds to
    // the high time value
    if (result.mIsReset) {
        // if this bit is also a reset, we can't check the low time since it
        // will exceed the maximums, but we still want to accept that case
        // as valid
        result.mValid = true;

        // use the nominal negative pulse timing for the frame ending.
        double nominalNegativeSec = mSettings->DataTiming(result.mBitValue, isHighSpeed).mNegativeTiming.mNominalSec;
        result.mEndSample = fallingEdgeSample + (nominalNegativeSec * mSampleRateHz);
    } else {
        const double lowTimeSec = (result.mEndSample - fallingEdgeSample) / mSampleRateHz;
        if (mSettings->DataTiming(result.mBitValue, isHighSpeed).mNegativeTiming.WithinTolerance(lowTimeSec)) {
            // we are good
            result.mValid = true;
        } else {
            // we could do further classification here on the error, eg speed mismatch,
            // or bit value mismatch
            result.mValid = false;
        }
    }

    return result;
}

bool AsyncRgbLedAnalyzer::NeedsRerun()
{
	return false;
}

U32 AsyncRgbLedAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitialized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitialized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 AsyncRgbLedAnalyzer::GetMinimumSampleRateHz()
{
	return 12 * 100000; // 12Mhz minimum sample rate
}

const char* AsyncRgbLedAnalyzer::GetAnalyzerName() const
{
	return "Addressable LEDs (Async)";
}

const char* GetAnalyzerName()
{
	return "Addressable LEDs (Async)";
}

Analyzer* CreateAnalyzer()
{
	return new AsyncRgbLedAnalyzer;
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}
