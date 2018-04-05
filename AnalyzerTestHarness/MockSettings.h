#ifndef ANALYZER_TEST_MOCK_SETTINGS_H
#define ANALYZER_TEST_MOCK_SETTINGS_H

#include "AnalyzerSettings.h"

namespace AnalyzerTest
{

class MockSettings
{
public:

    static MockSettings *MockFromSettings(AnalyzerSettings *settings);

    struct SettingChannel {
        Channel channel;
        std::string label;
        bool used;
    };

    std::vector<SettingChannel> mChannels;
    std::vector<AnalyzerSettingInterface*> mInterfaces;
};

class MockSettingInterface
{
public:
    static MockSettingInterface *MockFromInterface(AnalyzerSettingInterface *iface);

    AnalyzerInterfaceTypeId mTypeId;
    std::string mTitle;
    std::string mTooltip;

    // to simplify the test code, we store all the different settings
    // interface types as instances of this class

    // interface-channel
    Channel mChannel;

    int mValue; // bool, integer, number list


    struct NamedValue
    {
        std::string name;
        double value;
        std::string tooltip;
    };

    std::vector<NamedValue> mNamedValueList; // number list

    std::string mText; // text, bool
    AnalyzerSettingInterfaceText::TextType mTextType;

    int mMinValue, mMaxValue; // integer
};

}  // of namespace

#endif // of ANALYZER_TEST_MOCK_SETTINGS_H
