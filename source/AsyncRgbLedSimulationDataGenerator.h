#ifndef ASYNCRGBLED_SIMULATION_DATA_GENERATOR
#define ASYNCRGBLED_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <AnalyzerHelpers.h>
#include <string>

class AsyncRgbLedAnalyzerSettings;

class AsyncRgbLedSimulationDataGenerator
{
public:
	AsyncRgbLedSimulationDataGenerator();
	~AsyncRgbLedSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, AsyncRgbLedAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	AsyncRgbLedAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected:
	void CreateRGBWord();
	U32 RandomRGBValue() const;

    void WriteRGBTriple( U32 red, U32 green, U32 blue );
	void WriteUIntData( U32 data, U8 bit_count );
	void WriteBit(bool b);

    void WriteReset();
	
    ClockGenerator mClockGenerator;
	SimulationChannelDescriptor mLEDSimulationData;
};
#endif //ASYNCRGBLED_SIMULATION_DATA_GENERATOR
