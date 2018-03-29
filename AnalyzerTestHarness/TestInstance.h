#ifndef ANALYZER_TEST_INSTANCE
#define ANALYZER_TEST_INSTANCE

#include "AnalyzerChannelData.h"
#include "Analyzer.h"

namespace AnalyzerTest
{

// forward decls
class MockChannelData;
class SimulatedChannel;

class Instance
{
public:
    ~Instance();

    void CreatePlugin();

    void SetChannelData(const Channel& chan, MockChannelData* mockData);

    void RunAnalyzerWorker(int timeoutSec = 0);

    AnalyzerResults* GetResults();

    void RunSimulation(U64 num_samples, U32 sample_rate_hz);

    SimulatedChannel* GetSimulationChannel(const Channel& chan);

private:
    std::unique_ptr<Analyzer> mAnalyzerInstance;

    std::vector<SimulationChannelDescriptor*> mSimulatedChannels;
};


} // of namespace AnalyzerTest


#endif // of ANALYZER_TEST_INSTANCE
