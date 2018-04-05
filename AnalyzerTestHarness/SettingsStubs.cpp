#include "AnalyzerSettings.h"
#include "AnalyzerSettingInterface.h"

#include <cmath>

#include "MockSettings.h"

#define D_PTR() \
    auto d = reinterpret_cast<AnalyzerTest::MockSettings*>(mData);

AnalyzerSettings::AnalyzerSettings()
{
    AnalyzerTest::MockSettings* d = new AnalyzerTest::MockSettings;
    mData = reinterpret_cast<struct AnalyzerSettingsData*>(d);
}

AnalyzerSettings::~AnalyzerSettings()
{
    D_PTR();
   // delete d;
}

void AnalyzerSettings::ClearChannels()
{
    D_PTR();
    d->mChannels.clear();
}

void AnalyzerSettings::AddChannel( Channel& channel, const char* channel_label, bool is_used )
{
    D_PTR();
    d->mChannels.push_back({channel, channel_label, is_used});
}

void AnalyzerSettings::SetErrorText( const char* error_text )
{

}

void AnalyzerSettings::AddInterface( AnalyzerSettingInterface* analyzer_setting_interface )
{
    D_PTR();
    d->mInterfaces.push_back(analyzer_setting_interface);
}


void AnalyzerSettings::AddExportOption( U32 user_id, const char* menu_text )
{

}

void AnalyzerSettings::AddExportExtension(U32 user_id, const char *extension_description, const char *extension)
{

}

const char* AnalyzerSettings::SetReturnString( const char* str )
{
    return "";
}

namespace {

// helper class to access mData outside the class. Compiler will flatten
// all this code into nothing
class DataExtractor : public AnalyzerSettings {
public:
    AnalyzerTest::MockSettings* mockData() {
        return reinterpret_cast<AnalyzerTest::MockSettings*>(mData);
    }
};

// helper class to access mData outside the class. Compiler will flatten
// all this code into nothing
class InterfaceDataExtractor : public AnalyzerSettingInterface {
public:
    AnalyzerTest::MockSettingInterface* mockData() {
        return reinterpret_cast<AnalyzerTest::MockSettingInterface*>(mData);
    }
};

} // of anonymous namespace

namespace AnalyzerTest {

MockSettings* MockSettings::MockFromSettings(AnalyzerSettings* settings)
{
    return static_cast<DataExtractor*>(settings)->mockData();
}

MockSettingInterface* MockSettingInterface::MockFromInterface(AnalyzerSettingInterface *iface)
{
    return static_cast<InterfaceDataExtractor*>(iface)->mockData();
}

} // of namespace AnalyzerTest

//////////////////////////////////////////////////////////////////////////////

#undef D_PTR
#define D_PTR() \
    auto d = reinterpret_cast<AnalyzerTest::MockSettingInterface*>(mData);

AnalyzerSettingInterface::AnalyzerSettingInterface()
{
    AnalyzerTest::MockSettingInterface* d = new AnalyzerTest::MockSettingInterface;
    d->mTypeId = INTERFACE_BASE;
    mData = reinterpret_cast<struct AnalyzerSettingInterfaceData*>(d);
}

AnalyzerSettingInterface::~AnalyzerSettingInterface()
{
    D_PTR();
    delete d;
}

void AnalyzerSettingInterface::operator delete ( void* p )
{
    free(p);
}

void* AnalyzerSettingInterface::operator new( size_t size )
{
    return malloc(size);
}

AnalyzerInterfaceTypeId AnalyzerSettingInterface::GetType()
{
    D_PTR();
    return d->mTypeId;
}

void AnalyzerSettingInterface::SetTitleAndTooltip(const char *title, const char *tooltip)
{
    D_PTR();
    d->mTitle = title;
    d->mTooltip = tooltip;
}

//////////////////////////////////////////////////////////////////////////////


AnalyzerSettingInterfaceChannel::AnalyzerSettingInterfaceChannel()
{
   D_PTR();
   d->mTypeId = INTERFACE_CHANNEL;
}

AnalyzerSettingInterfaceChannel::~AnalyzerSettingInterfaceChannel()
{

}

AnalyzerInterfaceTypeId AnalyzerSettingInterfaceChannel::GetType()
{
    return INTERFACE_CHANNEL;
}

Channel AnalyzerSettingInterfaceChannel::GetChannel()
{
    D_PTR();
    return d->mChannel;
}

void AnalyzerSettingInterfaceChannel::SetChannel(const Channel &channel)
{
    D_PTR();
    d->mChannel = channel;
}

//////////////////////////////////////////////////////////////////////////////


AnalyzerSettingInterfaceNumberList::AnalyzerSettingInterfaceNumberList()
{
    D_PTR();
    d->mTypeId = INTERFACE_NUMBER_LIST;
}

AnalyzerSettingInterfaceNumberList::~AnalyzerSettingInterfaceNumberList()
{

}

AnalyzerInterfaceTypeId AnalyzerSettingInterfaceNumberList::GetType()
{
    return INTERFACE_NUMBER_LIST;
}

double AnalyzerSettingInterfaceNumberList::GetNumber()
{
    D_PTR();
    return d->mNamedValueList.at(d->mValue).value;
}

void AnalyzerSettingInterfaceNumberList::SetNumber(double number)
{
    D_PTR();
    auto it = std::find_if(d->mNamedValueList.begin(), d->mNamedValueList.end(),
                           [number](const AnalyzerTest::MockSettingInterface::NamedValue& nv)
    { return std::fabs(nv.value - number) < 1e-12;});
    assert(it != d->mNamedValueList.end());
    d->mValue =  std::distance(d->mNamedValueList.begin(), it);
}

void AnalyzerSettingInterfaceNumberList::AddNumber( double number, const char* str, const char* tooltip )
{
    D_PTR();
    d->mNamedValueList.push_back({str, number, tooltip});
}


