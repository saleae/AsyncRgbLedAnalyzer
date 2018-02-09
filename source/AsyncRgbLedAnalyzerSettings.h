#ifndef ASYNCRGBLED_ANALYZER_SETTINGS
#define ASYNCRGBLED_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class AsyncRgbLedAnalyzerSettings : public AnalyzerSettings
{
public:
	AsyncRgbLedAnalyzerSettings();
	virtual ~AsyncRgbLedAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	U32 mBitRate;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;
};

#endif //ASYNCRGBLED_ANALYZER_SETTINGS
