#include "MockSimulatedChannelDescriptor.h"

namespace {
    class DataExtractor : public SimulationChannelDescriptor {
    public:
        AnalyzerTest::SimulatedChannel* testData() {
            return reinterpret_cast<AnalyzerTest::SimulatedChannel*>(mData);
        }
    };
}

namespace AnalyzerTest
{

SimulatedChannel::SimulatedChannel()
{

}

SimulatedChannel *SimulatedChannel::FromSimulatedChannelDescriptor(SimulationChannelDescriptor *sim)
{
    return static_cast<DataExtractor*>(sim)->testData();
}

} // of namespace AnalyzerTest

#define D_PTR() \
    auto d = reinterpret_cast<AnalyzerTest::SimulatedChannel*>(mData);


SimulationChannelDescriptor::SimulationChannelDescriptor()
{
    auto d = new AnalyzerTest::SimulatedChannel();
    mData = reinterpret_cast<struct SimulationChannelDescriptorData*>(d);
}

SimulationChannelDescriptor::~SimulationChannelDescriptor()
{
    D_PTR();
    delete d; // ensure we delete as the correct type
}

void SimulationChannelDescriptor::SetChannel(Channel &channel)
{
    D_PTR();
    d->mChannel = channel;
}

void SimulationChannelDescriptor::SetSampleRate(U32 sample_rate_hz)
{

}

void SimulationChannelDescriptor::SetInitialBitState(BitState intial_bit_state)
{

}

Channel SimulationChannelDescriptor::GetChannel()
{
    D_PTR();
    return d->mChannel;
}

void SimulationChannelDescriptor::Transition()
{

}

void SimulationChannelDescriptor::TransitionIfNeeded(BitState bit_state)
{

}

void SimulationChannelDescriptor::Advance(U32 num_samples_to_advance)
{

}

BitState SimulationChannelDescriptor::GetCurrentBitState()
{
    D_PTR();
    return BIT_LOW;
}

U64 SimulationChannelDescriptor::GetCurrentSampleNumber()
{
    D_PTR();
    return 0;
}
