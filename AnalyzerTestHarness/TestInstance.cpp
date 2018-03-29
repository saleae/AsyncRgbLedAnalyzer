#include "TestInstance.h"

#include "TestAnalyzerData.h"
#include "MockSimulatedChannelDescriptor.h"

#include <cassert>

extern "C" {
    Analyzer* __cdecl CreateAnalyzer( );
}

namespace AnalyzerTest
{


AnalyzerResults *Instance::GetResults()
{
    return GetResultsFromAnalyzer(mAnalyzerInstance.get());
}

Instance::~Instance()
{

}

void Instance::CreatePlugin()
{
    mAnalyzerInstance.reset(CreateAnalyzer());
}

void Instance::SetChannelData(const Channel& chan, MockChannelData *mockData)
{
    GetDataFromAnalyzer(mAnalyzerInstance.get())->channelData[chan] = mockData;
}

void Instance::RunAnalyzerWorker(int timeoutSec)
{
    assert(mAnalyzerInstance);

    // setup a watch-dog to timeout this


    mAnalyzerInstance->WorkerThread();

}


void Instance::RunSimulation(U64 num_samples, U32 sample_rate_hz)
{
    mSimulatedChannels.clear();
    SimulationChannelDescriptor* channels[16];


    U32 count = mAnalyzerInstance->GenerateSimulationData(num_samples, sample_rate_hz, channels);
    if (count == 0) {
        return;
    }

    mSimulatedChannels.resize(count);
    memcpy(mSimulatedChannels.data(), channels, count * sizeof(SimulationChannelDescriptor*));
}

SimulatedChannel *Instance::GetSimulationChannel(const Channel &chan)
{
    if (mSimulatedChannels.empty())
        return nullptr;

    for (auto sc : mSimulatedChannels) {
        if (sc->GetChannel() == chan) {
            return SimulatedChannel::FromSimulatedChannelDescriptor(sc);
        }
    }

    return nullptr;
}


} // of namespace AnalyzerTest

