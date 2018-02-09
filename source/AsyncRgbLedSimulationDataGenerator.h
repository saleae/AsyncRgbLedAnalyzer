#ifndef ASYNCRGBLED_SIMULATION_DATA_GENERATOR
#define ASYNCRGBLED_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
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
	void CreateSerialByte();
	std::string mSerialText;
	U32 mStringIndex;

	SimulationChannelDescriptor mSerialSimulationData;

};
#endif //ASYNCRGBLED_SIMULATION_DATA_GENERATOR