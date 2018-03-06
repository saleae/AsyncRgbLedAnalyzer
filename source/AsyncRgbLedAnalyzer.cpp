#include "AsyncRgbLedAnalyzer.h"
#include "AsyncRgbLedAnalyzerSettings.h"
#include "AsyncRgbLedAnalyzerResults.h"

#include <AnalyzerChannelData.h>

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

	if( mChannelData->GetBitState() == BIT_LOW )
		mChannelData->AdvanceToNextEdge();

	U32 bitRate = 9600;
	U32 samples_per_bit = mSampleRateHz / bitRate;
	U32 samples_to_first_center_of_first_data_bit = U32( 1.5 * double( mSampleRateHz ) / double(bitRate) );

	for( ; ; )
	{
		U8 data = 0;
		U8 mask = 1 << 7;
		
		mChannelData->AdvanceToNextEdge(); //falling edge -- beginning of the start bit

		U64 starting_sample = mChannelData->GetSampleNumber();

		mChannelData->Advance( samples_to_first_center_of_first_data_bit );

		for( U32 i=0; i<8; i++ )
		{
			//let's put a dot exactly where we sample this bit:
			mResults->AddMarker( mChannelData->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mInputChannel );

			if( mChannelData->GetBitState() == BIT_HIGH )
				data |= mask;

			mChannelData->Advance( samples_per_bit );

			mask = mask >> 1;
		}


		//we have a byte to save. 
		Frame frame;
		frame.mData1 = data;
		frame.mFlags = 0;
		frame.mStartingSampleInclusive = starting_sample;
		frame.mEndingSampleInclusive = mChannelData->GetSampleNumber();

		mResults->AddFrame( frame );
		mResults->CommitResults();
		ReportProgress( frame.mEndingSampleInclusive );
	}
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