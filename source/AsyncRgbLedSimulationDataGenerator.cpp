#include "AsyncRgbLedSimulationDataGenerator.h"

#include <cmath> // for M_PI, cos
#include <iostream>

#include "AsyncRgbLedAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

const double NSEC_TO_SEC = 1e-9;

AsyncRgbLedSimulationDataGenerator::AsyncRgbLedSimulationDataGenerator()
{
}

AsyncRgbLedSimulationDataGenerator::~AsyncRgbLedSimulationDataGenerator()
{
}

void AsyncRgbLedSimulationDataGenerator::Initialize( U32 simulation_sample_rate, AsyncRgbLedAnalyzerSettings* settings )
{
	// Initialize the random number generator with a literal seed to obtain repeatability
    // Change this for srand(time(NULL)) for "truly" random sequences
    // NOTICE rand() an srand() are *not* thread safe
    srand( 42 );

	mSimulationSampleRateHz = simulation_sample_rate;
	mSettings = settings;

    // TODO pass in the analyzer and call GetMinimumSampleRate?
    mClockGenerator.Init( 12 * 1000000, mSimulationSampleRateHz );

	mLEDSimulationData.SetChannel( mSettings->mInputChannel );
	mLEDSimulationData.SetSampleRate( simulation_sample_rate );
	mLEDSimulationData.SetInitialBitState( BIT_HIGH );
}

U32 AsyncRgbLedSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel )
{
	U64 adjusted_largest_sample_requested = 
	AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

	while( mLEDSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
	{
        WriteReset();

        // six RGB-triple cascade between resets, i.e six discrete LEDs
        // or two of the 3-LED combined drivers. We should perhaps make
        // this adjustable
		for (int t=0; t<6; ++t) {
			CreateRGBWord();
		}
	}

	*simulation_channel = &mLEDSimulationData;
	return 1;
}

void AsyncRgbLedSimulationDataGenerator::CreateRGBWord()
{
	const U32 rgb = RandomRGBValue();
	WriteRGBTriple( rgb >> 16 & 0xff, rgb >> 8 & 0xff, rgb & 0xff );
}

void AsyncRgbLedSimulationDataGenerator::WriteRGBTriple( U32 red, U32 green, U32 blue )
{
    WriteUIntData( red, mSettings->BitSize() );
    WriteUIntData( green, mSettings->BitSize() );
    WriteUIntData( blue, mSettings->BitSize() );
}

void AsyncRgbLedSimulationDataGenerator::WriteReset()
{
    // interstitial after last data bit, before the reset goes low
    mLEDSimulationData.Advance( mClockGenerator.AdvanceByTimeS(1e-5) );
    mLEDSimulationData.Transition(); // go low
    // scale by 120% since reset time is the minimum I believe
    const double resetSec = mClockGenerator.AdvanceByTimeS(mSettings->ResetTimeNSec() * 1.2 * NSEC_TO_SEC);
    mLEDSimulationData.Advance( resetSec );
	mLEDSimulationData.Transition(); // go high to end the reset
}

void AsyncRgbLedSimulationDataGenerator::WriteUIntData( U32 data, U8 bit_count )
{
	U32 mask =  1 << (bit_count - 1);
	for( U32 bit=0; bit < bit_count; ++bit) {
		WriteBit(data & mask);
		mask = mask >> 1;
	}	
}

void AsyncRgbLedSimulationDataGenerator::WriteBit(bool b)
{
    const BitState bs = b ? BIT_HIGH : BIT_LOW;
    const double highSamples = mClockGenerator.AdvanceByTimeS( mSettings->DataTimeNSecHigh( bs ) * NSEC_TO_SEC );
    const double lowSamples = mClockGenerator.AdvanceByTimeS( mSettings->DataTimeNSecLow( bs ) * NSEC_TO_SEC );
    mLEDSimulationData.Advance( highSamples );
	mLEDSimulationData.Transition(); // go low
    mLEDSimulationData.Advance( lowSamples );
	mLEDSimulationData.Transition(); // go high to end this bit
}

U32 AsyncRgbLedSimulationDataGenerator::RandomRGBValue() const
{
    // TODO handle 12-bit data generation
	const U8 red = rand() % 0xff;
	const U8 green = rand() % 0xff;
	const U8 blue = rand() % 0xff;
	return (red << 16) | (green << 8) | blue;
}
