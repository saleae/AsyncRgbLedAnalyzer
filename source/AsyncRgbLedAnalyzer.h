#ifndef ASYNCRGBLED_ANALYZER_H
#define ASYNCRGBLED_ANALYZER_H

#include <Analyzer.h>
#include "AsyncRgbLedAnalyzerResults.h"
#include "AsyncRgbLedSimulationDataGenerator.h"

class AsyncRgbLedAnalyzerSettings;
class ANALYZER_EXPORT AsyncRgbLedAnalyzer : public Analyzer2
{
public:
	AsyncRgbLedAnalyzer();
	virtual ~AsyncRgbLedAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	std::auto_ptr< AsyncRgbLedAnalyzerSettings > mSettings;
	std::auto_ptr< AsyncRgbLedAnalyzerResults > mResults;
	AnalyzerChannelData* mSerial;

	AsyncRgbLedSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //ASYNCRGBLED_ANALYZER_H
