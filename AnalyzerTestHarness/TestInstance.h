#ifndef ANALYZER_TEST_INSTANCE_H
#define ANALYZER_TEST_INSTANCE_H

#include "AnalyzerChannelData.h"
#include "Analyzer.h"

namespace AnalyzerTest
{

// forward decls
class MockChannelData;
class SimulatedChannel;
class MockSettings;

class Instance
{
public:
    ~Instance();

    void CreatePlugin(const std::string &name);

    void SetChannelData(const Channel& chan, MockChannelData* mockData);

    void SetSampleRate(U64 sample_rate_hz);
    U64 GetSampleRate() const;

    void RunAnalyzerWorker(int timeoutSec = 0);

    AnalyzerResults* GetResults();

    void RunSimulation(U64 num_samples, U32 sample_rate_hz);

    SimulatedChannel* GetSimulationChannel(const Channel& chan);

    AnalyzerSettings* GetSettings();

private:
    std::unique_ptr<Analyzer> mAnalyzerInstance;

    std::vector<SimulationChannelDescriptor*> mSimulatedChannels;
};


} // of namespace AnalyzerTest


#endif // of ANALYZER_TEST_INSTANCE_H
