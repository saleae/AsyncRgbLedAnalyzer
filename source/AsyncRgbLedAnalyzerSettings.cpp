#include "AsyncRgbLedAnalyzerSettings.h"
#include <AnalyzerHelpers.h>

const char* DEFAULT_CHANNEL_NAME = "Addressable LEDs (Async)";

AsyncRgbLedAnalyzerSettings::AsyncRgbLedAnalyzerSettings()
{
	mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputChannelInterface->SetTitleAndTooltip( "LED Channel", "Standard Addressable LEDs (Async)" );
	mInputChannelInterface->SetChannel( mInputChannel );

	mControllerInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mControllerInterface->SetTitleAndTooltip( "LED Controller", "Specify the LED controller in use." );
	mControllerInterface->AddNumber( LED_WS2811, "WS2811", "Worldsemi 24-bit RGB controller" );
	mControllerInterface->AddNumber( LED_WS2812B, "WS2812B", "Worldsemi 24-bit RGB controller" );
	mControllerInterface->AddNumber( LED_WS2813, "WS2813", "Worldsemi 24-bit RGB integrated light-source" );
	mControllerInterface->AddNumber( LED_TM1809, "TM1809", "Titan Micro 9-chanel 24-bit RGB controller" );
	mControllerInterface->AddNumber( LED_TM1804, "TM1804", "Titan Micro 24-bit RGB controller" );
	mControllerInterface->AddNumber( LED_UCS1903, "UCS1903", "UCS1903 24-bit RGB controller" );
	mControllerInterface->AddNumber( LED_LPD1886_8bit, "LPD1886 - 24bit", "LPD1886 RGB controller in 24-bit mode" );
	mControllerInterface->AddNumber( LED_LPD1886_12bit, "LPD1886 - 36bit", "LPD1886 RGB controller in 36-bit mode" );

	mControllerInterface->SetNumber( mLEDController );

	AddInterface( mInputChannelInterface.get() );
	AddInterface( mControllerInterface.get() );

	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mInputChannel, DEFAULT_CHANNEL_NAME, false );
}

AsyncRgbLedAnalyzerSettings::~AsyncRgbLedAnalyzerSettings()
{
}

bool AsyncRgbLedAnalyzerSettings::SetSettingsFromInterfaces()
{
	mInputChannel = mInputChannelInterface->GetChannel();
	mLEDController = static_cast<Controller>(mControllerInterface->GetNumber());

	ClearChannels();
	AddChannel( mInputChannel, DEFAULT_CHANNEL_NAME, true );

	return true;
}

void AsyncRgbLedAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface->SetChannel( mInputChannel );
	mControllerInterface->SetNumber( mLEDController );
}

void AsyncRgbLedAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	U32 controllerInt;
	text_archive >> mInputChannel;
	text_archive >> controllerInt;
	mLEDController = static_cast<Controller>(controllerInt);

	ClearChannels();
	AddChannel( mInputChannel, DEFAULT_CHANNEL_NAME, true );

	UpdateInterfacesFromSettings();
}

const char* AsyncRgbLedAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mInputChannel;
	text_archive << mLEDController;

	return SetReturnString( text_archive.GetString() );
}


U8 AsyncRgbLedAnalyzerSettings::BitSize() const
{
	switch (mLEDController) {
	case LED_LPD1886_12bit:
		return 12;
	default:
		return 8;
	}
}

U8 AsyncRgbLedAnalyzerSettings::LEDChannelCount() const
{
	switch (mLEDController) {
	case LED_WS2811:
	case LED_WS2812B:
	case LED_WS2813:
	case LED_TM1804:
	case LED_UCS1903:
	case LED_LPD1886_8bit:
	case LED_LPD1886_12bit:
		return 3;

	case LED_TM1809:
		return 9;
	}
}