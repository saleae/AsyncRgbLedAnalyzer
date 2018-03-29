#include "AnalyzerSettings.h"
#include "AnalyzerSettingInterface.h"

AnalyzerSettings::AnalyzerSettings()
{

}

AnalyzerSettings::~AnalyzerSettings()
{

}

void AnalyzerSettings::ClearChannels()
{

}

void AnalyzerSettings::AddChannel( Channel& channel, const char* channel_label, bool is_used )
{

}

void AnalyzerSettings::SetErrorText( const char* error_text )
{

}

void AnalyzerSettings::AddInterface( AnalyzerSettingInterface* analyzer_setting_interface )
{

}


void AnalyzerSettings::AddExportOption( U32 user_id, const char* menu_text )
{

}

void AnalyzerSettings::AddExportExtension(U32 user_id, const char *extension_description, const char *extension)
{

}

const char* AnalyzerSettings::SetReturnString( const char* str )
{

}

//////////////////////////////////////////////////////////////////////////////

AnalyzerSettingInterface::AnalyzerSettingInterface()
{

}

AnalyzerSettingInterface::~AnalyzerSettingInterface()
{

}

void AnalyzerSettingInterface::operator delete ( void* p )
{

}

void* AnalyzerSettingInterface::operator new( size_t size )
{

}

AnalyzerInterfaceTypeId AnalyzerSettingInterface::GetType()
{

}

void AnalyzerSettingInterface::SetTitleAndTooltip(const char *title, const char *tooltip)
{

}

//////////////////////////////////////////////////////////////////////////////


AnalyzerSettingInterfaceChannel::AnalyzerSettingInterfaceChannel()
{

}

AnalyzerSettingInterfaceChannel::~AnalyzerSettingInterfaceChannel()
{

}

AnalyzerInterfaceTypeId AnalyzerSettingInterfaceChannel::GetType()
{

}

Channel AnalyzerSettingInterfaceChannel::GetChannel()
{

}

void AnalyzerSettingInterfaceChannel::SetChannel(const Channel &channel)
{

}

//////////////////////////////////////////////////////////////////////////////


AnalyzerSettingInterfaceNumberList::AnalyzerSettingInterfaceNumberList()
{

}

AnalyzerSettingInterfaceNumberList::~AnalyzerSettingInterfaceNumberList()
{

}

AnalyzerInterfaceTypeId AnalyzerSettingInterfaceNumberList::GetType()
{

}

double AnalyzerSettingInterfaceNumberList::GetNumber()
{

}

void AnalyzerSettingInterfaceNumberList::SetNumber(double number)
{

}

void AnalyzerSettingInterfaceNumberList::AddNumber( double number, const char* str, const char* tooltip )
{

}

