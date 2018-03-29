#ifndef ANALYZER_TEST_MOCK_SIMULATED_CHANNEL_DESCRIPTOR_H
#define ANALYZER_TEST_MOCK_SIMULATED_CHANNEL_DESCRIPTOR_H

#include "SimulationChannelDescriptor.h"

namespace AnalyzerTest
{

class SimulatedChannel
{
public:
    SimulatedChannel();

    Channel GetChannel() const {
        return mChannel;
    }

    static SimulatedChannel* FromSimulatedChannelDescriptor(SimulationChannelDescriptor* sim);

private:
    friend ::SimulationChannelDescriptor;

    Channel mChannel;
};



} // of namespace AnalyzerTest

#endif // of ANALYZER_TEST_MOCK_SIMULATED_CHANNEL_DESCRIPTOR_H
