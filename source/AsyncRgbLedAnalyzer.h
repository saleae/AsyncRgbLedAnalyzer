#ifndef ASYNCRGBLED_ANALYZER_H
#define ASYNCRGBLED_ANALYZER_H

#include <Analyzer.h>

#include "AsyncRgbLedSimulationDataGenerator.h"

// forward decls
class AsyncRgbLedAnalyzerSettings;
class AsyncRgbLedAnalyzerResults;

class ANALYZER_EXPORT AsyncRgbLedAnalyzer : public Analyzer2
{
public:
	AsyncRgbLedAnalyzer();
	virtual ~AsyncRgbLedAnalyzer();

	void SetupResults() override;
	void WorkerThread() override;

	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels ) override;
	U32 GetMinimumSampleRateHz() override;

	const char* GetAnalyzerName() const override;
	bool NeedsRerun() override;

protected: //vars
	std::unique_ptr< AsyncRgbLedAnalyzerSettings > mSettings;
	std::unique_ptr< AsyncRgbLedAnalyzerResults > mResults;
	AnalyzerChannelData* mChannelData = nullptr;

	AsyncRgbLedSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitialized = false;

	// analysis vars:
	U32 mSampleRateHz = 0;
	U32 mStartOfStopBitOffset = 0;
	U32 mEndOfStopBitOffset = 0;
};

extern "C" {
	ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
	ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
	ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );
}

#endif //ASYNCRGBLED_ANALYZER_H
