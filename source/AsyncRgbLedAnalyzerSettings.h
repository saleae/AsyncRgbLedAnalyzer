#ifndef ASYNCRGBLED_ANALYZER_SETTINGS
#define ASYNCRGBLED_ANALYZER_SETTINGS

#include <vector>

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

#include "AsyncRgbLedHelpers.h"

class AsyncRgbLedAnalyzerSettings : public AnalyzerSettings
{
public:
	AsyncRgbLedAnalyzerSettings();
	virtual ~AsyncRgbLedAnalyzerSettings();

	// delete all the copy/move operators for rule-of-5 compliance
	AsyncRgbLedAnalyzerSettings(const AsyncRgbLedAnalyzerSettings&) = delete;
	AsyncRgbLedAnalyzerSettings(AsyncRgbLedAnalyzerSettings&&) = delete;	
	const AsyncRgbLedAnalyzerSettings& operator=(const AsyncRgbLedAnalyzerSettings&) = delete;
	AsyncRgbLedAnalyzerSettings& operator=(AsyncRgbLedAnalyzerSettings&&) = delete;


	bool SetSettingsFromInterfaces() override;
	void UpdateInterfacesFromSettings();
	void LoadSettings( const char* settings ) override;
	const char* SaveSettings() override;

	enum Controller {
		LED_WS2811 = 0,
		LED_WS2812B,
		LED_WS2813,
		LED_TM1809,
		LED_TM1804,
		LED_UCS1903,
		LED_LPD1886_8bit,
		LED_LPD1886_12bit
	};

	Controller mLEDController = LED_WS2811;
	Channel mInputChannel = UNDEFINED_CHANNEL;

    /// bits ber LED channel, either 8 or 12 at present
	U8 BitSize() const; 

	/// LED channel count, either 3 (RGB) or 9 (three RGB outputs) at present
	U8 LEDChannelCount() const;

	/// interval to consider a reset rather than a data value

    void SetHighSpeedMode(bool isHighSpeed);

    bool IsHighSpeedSupported() const;

    U32 DataTimeNSecHigh(BitState value) const;

    U32 DataTimeNSecLow(BitState value) const;

    U32 ResetTimeNSec() const;

    ColorLayout GetColorLayout() const;

protected:
    void InitControllerData();

	std::unique_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::unique_ptr< AnalyzerSettingInterfaceNumberList >	mControllerInterface;

    // we can't do direct defualt initialisation here, since according to C++11
    // that makes this type non-POD and hence unsuitable for direct initialisation.
    // C++14 fixes this.
    struct LedControllerData
    {
        std::string mName;
        std::string mDescription;
        U8 mBitsPerChannel; // = 8;
        U8 mChannelCount; // = 3;
        U32 mResetTimeNSec;
        U32 mDataTimingNSec[2][2];

        bool mHasHighSpeed; // = trye
        U32 mDataTimingHighSpeedNsec[2][2];

        ColorLayout mLayout; // = LAYOUT_RGB
    };

    std::vector<LedControllerData> mControllers;

    bool mIsHighSpeedMode = false;
};

#endif //ASYNCRGBLED_ANALYZER_SETTINGS
