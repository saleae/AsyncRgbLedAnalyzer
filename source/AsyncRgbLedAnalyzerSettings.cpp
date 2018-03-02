#include "AsyncRgbLedAnalyzerSettings.h"
#include <AnalyzerHelpers.h>

const char* DEFAULT_CHANNEL_NAME = "Addressable LEDs (Async)";

AsyncRgbLedAnalyzerSettings::AsyncRgbLedAnalyzerSettings()
{
    InitControllerData();

	mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputChannelInterface->SetTitleAndTooltip( "LED Channel", "Standard Addressable LEDs (Async)" );
	mInputChannelInterface->SetChannel( mInputChannel );

	mControllerInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mControllerInterface->SetTitleAndTooltip( "LED Controller", "Specify the LED controller in use." );

    int index = 0;
    for (const auto& controllerData : mControllers) {
        mControllerInterface->AddNumber(index++, controllerData.mName.c_str(),
                                        controllerData.mDescription.c_str());
    }

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

void AsyncRgbLedAnalyzerSettings::InitControllerData()
{
    // order of values here must correspond to the Controller enum
    mControllers = {
        // name, description, bits per channel, channels per frame, reset time nsec, low-speed data nsec, has high speed, high speed data nsec, color layout

        // https://cdn-shop.adafruit.com/datasheets/WS2811.pdf
        {"WS2811", "Worldsemi 24-bit RGB controller", 8, 3, 50000, {{500, 2000}, {1200, 1300}}, true, {{250, 1000}, {600, 650}}, LAYOUT_RGB},

        // https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf
        {"WS2812B", "Worldsemi 24-bit RGB integrated light-source", 8, 3, 50000, {{400, 850}, {800, 450}}, false, {{0, 0}, {0, 0}}, LAYOUT_GRB},

        // http://www.led-color.com/upload/201609/WS2813%20LED.pdf
        {"WS2813", "Worldsemi 24-bit RGB integrated light-source", 8, 3, 300000, {{375, 200}, {875, 200}}, false, {{0, 0}, {0, 0}}, LAYOUT_GRB},

        // https://www.deskontrol.net/descargas/datasheets/TM1809.pdf
        {"TM1809", "Titan Micro 9-chanel 24-bit RGB controller", 8, 9, 24000, {{600, 1200}, {1200, 600}}, true, {{320, 600}, {600, 320}}, LAYOUT_RGB},

        // https://www.deskontrol.net/descargas/datasheets/TM1804.pdf
        {"TM1804", "Titan Micro 24-bit RGB controller", 8, 3, 10000, {{1000, 2000}, {2000, 1000}}, false, {{0, 0}, {0, 0}}, LAYOUT_RGB},

        // http://www.bestlightingbuy.com/pdf/UCS1903%20datasheet.pdf
        {"UCS1903", "UCS1903 24-bit RGB controller", 8, 3, 24000, {{500, 2000}, {2000, 500}}, true, {{250, 1000}, {1000, 250}}, LAYOUT_RGB},

        // https://www.syncrolight.co.uk/datasheets/LPD1886%20datasheet.pdf
        {"LPD1886 - 24 bit", "LPD1886 RGB controller in 24-bit mode", 8, 3, 24000, {{200, 600}, {600, 200}}, false, {{0, 0}, {0, 0}}, LAYOUT_RGB},
        {"LPD1886 - 36 bit", "LPD1886 RGB controller in 36-bit mode", 12, 3, 24000, {{200, 600}, {600, 200}}, false, {{0, 0}, {0, 0}}, LAYOUT_RGB}
    };
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
    return mControllers.at(mLEDController).mBitsPerChannel;
}

U8 AsyncRgbLedAnalyzerSettings::LEDChannelCount() const
{
    return mControllers.at(mLEDController).mChannelCount;
}

U32 AsyncRgbLedAnalyzerSettings::DataTimeNSecHigh(BitState value) const
{
    const auto& d = mControllers.at(mLEDController);
    return mIsHighSpeedMode ? d.mDataTimingHighSpeedNsec[value][0]
                            : d.mDataTimingNSec[value][1];
}

U32 AsyncRgbLedAnalyzerSettings::DataTimeNSecLow(BitState value) const
{
    const auto& d = mControllers.at(mLEDController);
    return mIsHighSpeedMode ? d.mDataTimingHighSpeedNsec[value][0]
                            : d.mDataTimingNSec[value][1];
}

bool AsyncRgbLedAnalyzerSettings::IsHighSpeedSupported() const
    {
    return mControllers.at(mLEDController).mHasHighSpeed;
}

U32 AsyncRgbLedAnalyzerSettings::ResetTimeNSec() const
{
    return mControllers.at(mLEDController).mResetTimeNSec;
}

auto AsyncRgbLedAnalyzerSettings::GetColorLayout() const -> ColorLayout
{
    return mControllers.at(mLEDController).mLayout;
}
