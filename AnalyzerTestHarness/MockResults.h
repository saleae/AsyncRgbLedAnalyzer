#ifndef ANALYZER_TEST_MOCK_RESULTS
#define ANALYZER_TEST_MOCK_RESULTS

#include "AnalyzerResults.h"

namespace AnalyzerTest
{

class MockResultData 
{
public:
    /**
     * @brief extract the result test data from an AnalyzerResults
     * */
    static MockResultData* MockFromResults(AnalyzerResults* results);

    U64 AddFrame(const Frame& f);
    const Frame& GetFrame(U64 index) const;

    struct MarkerInfo
    {
        U64 frame;
        AnalyzerResults::MarkerType type;
    };

    U64 CurrentFrame() const;
    U64 NextFrame() const;

    struct StringInfo
    {
        U64 frame;
        std::string string;
    };

    void AddString(const std::string& s);
private:
    friend ::AnalyzerResults;

    std::vector<Frame> mFrames;
    std::vector<U64> mPacketStartFrames;
    std::vector<U64> mCommitFrames;
    std::vector<MarkerInfo> mMarkers;
    std::vector<StringInfo> mStrings;

};

}  // of namespace

#endif
