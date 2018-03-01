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
	mNSecPerSample = 1000000000 / GetSampleRate();
	mChannelData = GetAnalyzerChannelData( mSettings->mInputChannel );
	mRGBBitCount = mSettings->BitSize() * 3;

	// find first long low pulse to synchronise with the stream?

	// advance to the first RESET pulse
	if( mChannelData->GetBitState() == BIT_HIGH )
		mChannelData->AdvanceToNextEdge();
	
	U64 resetBeginSample = mChannelData->GetSampleNumber();
	
	for( ; ; )
	{
		U32 frameInPacketIndex = 0;

		mResults->CommitPacketAndStartNewPacket();
		mChannelData->AdvanceToNextEdge(); // rising edge - end of RESET
		U64 firstDataSample = mChannelData->GetSampleNumber();
		U64 resetSampleCount = firstDataSample - resetBeginSample;
        double resetNSec = resetSampleCount * mNSecPerSample;

        if (resetNSec < mSettings->ResetTimeNSec()) {
			// warn about this somehow
            std::cerr << "too-short reset duration observed:" << resetNSec << " vs "
                      << mSettings->ResetTimeNSec() << std::endl;
		}

		// data word reading loop
		for ( ; ; ) {
			Frame frame;
			frame.mFlags = 0;
			frame.mStartingSampleInclusive  = mChannelData->GetSampleNumber();

			bool sawReset = false;
			frame.mData1 = ReadRGBTriple(sawReset);
            frame.mData2 = frameInPacketIndex++;
			if (sawReset) {
				break;
			}

			frame.mEndingSampleInclusive = mChannelData->GetSampleNumber();
			mResults->AddFrame( frame );
		}

		mResults->CommitResults();
		resetBeginSample = mChannelData->GetSampleNumber();
		ReportProgress( resetBeginSample );
	}
}

// TODO U32 won't work for 12-bit mode, we should define a struct
U32 AsyncRgbLedAnalyzer::ReadRGBTriple(bool& sawReset)
{
	U32 rgb = 0;
	for (int i=0; i<mRGBBitCount; ++i) {
		U8 bit = ReadBit();
		if (bit == 0xff) {
			sawReset = true;
			return 0;
		}

		rgb = (rgb << 1) | bit;
	}

	// TODO re-order for GRB controller chips
    AsyncRgbLedAnalyzerSettings::ColorLayout layout = mSettings->GetColorLayout();
    switch (layout) {
    case AsyncRgbLedAnalyzerSettings::LAYOUT_GRB:
  //      std::swap(rgb.red, rgb.green);
        break;

    default:
        // no-op
        break;
    }

	return rgb;
}

U8 AsyncRgbLedAnalyzer::ReadBit()
{
	const U64 bitBeginSample = mChannelData->GetSampleNumber();
	mChannelData->AdvanceToNextEdge(); // falling edge
	const U64 bitTransitionSample = mChannelData->GetSampleNumber();

    // don't advance yet, this might be a reset
	const U64 bitEndSample = mChannelData->GetSampleOfNextEdge();

    const double highNSec = (bitTransitionSample - bitBeginSample) * mNSecPerSample;
    const double lowNSec = (bitEndSample - bitTransitionSample) * mNSecPerSample;

	// if we see a low time that's in the same magnitude as a reset pulse,
	// let's treat it as such (reset pulses are several orders of magnitude larger
	// than a data bit's low pulse)
    if (lowNSec > (mSettings->ResetTimeNSec() * 0.5)) {
		// don't advance here, the outer analysis loop will do that
		return 0xff; // RESET marker value
	}
	
    // if it wasn't a reset, we can safely advance
	mChannelData->AdvanceToAbsPosition(bitEndSample);

	// very tolerant classification; if the high time is longer than the low time,
	// it's a 1, and conversely if the low time is longer than high, it's a 0.
    // TODO replace this since while it's tolerant, it doesn't work for some
    // controllers
    return (highNSec > lowNSec) ? 1 : 0;
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
