#include "Analyzer.h"

#include <sstream>
#include <iomanip>

#include "AnalyzerHelpers.h"


struct ClockGeneratorData
{
    double mSamplesPerHalfPeriod;
    double mCurrentError;
    double mSampleTime;
};

ClockGenerator::ClockGenerator()
:	mData( new ClockGeneratorData() )
{
}

ClockGenerator::~ClockGenerator()
{
    delete mData;
    mData = NULL;
}

void ClockGenerator::Init( double target_frequency, U32 sample_rate_hz )
{
    mData->mSamplesPerHalfPeriod = double( sample_rate_hz ) / target_frequency;
    mData->mCurrentError = 0;
    mData->mSampleTime = 1.0 / sample_rate_hz;
}

U32 ClockGenerator::AdvanceByHalfPeriod( double multiple )
{
    double target_half_period =  ( mData->mSamplesPerHalfPeriod * multiple ) + mData->mCurrentError;
    U32 samples_to_advance = U32( target_half_period );
    mData->mCurrentError = target_half_period - double( samples_to_advance );
    return samples_to_advance;
}

U32 ClockGenerator::AdvanceByTimeS( double time_s )
{
    double target_num_samples =  ( time_s / mData->mSampleTime ) + mData->mCurrentError;
    U32 samples_to_advance = U32( target_num_samples );
    mData->mCurrentError = target_num_samples - double( samples_to_advance );
    return samples_to_advance;
}

struct BitExtractorData
{
    U64 mData;
    AnalyzerEnums::ShiftOrder mShiftOrder;
    U64 mMask;
};


BitExtractor::BitExtractor( U64 data,  AnalyzerEnums::ShiftOrder shift_order, U32 num_bits )
:	mData( new BitExtractorData() )
{
    mData->mData = data;
    mData->mShiftOrder = shift_order;

    if( shift_order ==  AnalyzerEnums::LsbFirst )
        mData->mMask = 0x1;
    else
        mData->mMask = 0x1ULL << ( num_bits - 1 );
}

BitExtractor::~BitExtractor()
{
    delete mData;
    mData = NULL;
}

BitState BitExtractor::GetNextBit()
{
    BitState result;

    if( ( mData->mMask & mData->mData ) == 0 )
        result = BIT_LOW;
    else
        result = BIT_HIGH;

    if( mData->mShiftOrder == AnalyzerEnums::LsbFirst )
        mData->mMask <<= 1;
    else
        mData->mMask >>= 1;

    return result;
}

struct DataBuilderData
{
    U64* mData;
    AnalyzerEnums::ShiftOrder mShiftOrder;
    U64 mMask;
};

DataBuilder::DataBuilder()
:	mData( new DataBuilderData() )
{
}

DataBuilder::~DataBuilder()
{
    delete mData;
    mData = NULL;
}

void DataBuilder::Reset( U64* data, AnalyzerEnums::ShiftOrder shift_order, U32 num_bits )
{
    mData->mData = data;
    mData->mShiftOrder = shift_order;

    if( shift_order ==  AnalyzerEnums::LsbFirst )
        mData->mMask = 0x1;
    else
        mData->mMask = 0x1ULL << ( num_bits - 1 );

    *data = 0;
}

void DataBuilder::AddBit( BitState bit )
{
    if( bit == BIT_HIGH )
        *mData->mData |= mData->mMask;

    if( mData->mShiftOrder == AnalyzerEnums::LsbFirst )
        mData->mMask <<= 1;
    else
        mData->mMask >>= 1;
}

void AnalyzerHelpers::GetNumberString(U64 number, DisplayBase display_base, U32 num_data_bits, char *result_string, U32 result_string_max_length)
{

}

void AnalyzerHelpers::GetTimeString( U64 sample, U64 trigger_sample, U32 sample_rate_hz, char* result_string, U32 result_string_max_length )
{
    std::stringstream ss;
    ss.precision( 15 );
    double time = double( S64(sample) - S64(trigger_sample) );
    time = time / double( sample_rate_hz );
    ss << std::dec << std::fixed << time;

    std::string result = ss.str();
    if( ( result.size() + 1 ) > result_string_max_length )
        result = result.substr( 0, result_string_max_length - 1 );

    strcpy( result_string, result.c_str() );
}

U64 AnalyzerHelpers::AdjustSimulationTargetSample(U64 target_sample, U32 sample_rate, U32 simulation_sample_rate)
{

}

/////////////////////////////////////////////////////////////////////////////



struct SimpleArchiveData
{
};

SimpleArchive::SimpleArchive()
:	mData( new SimpleArchiveData() )
{
}

SimpleArchive::~SimpleArchive()
{
    delete mData;
    mData = NULL;
}

void SimpleArchive::SetString( const char* archive_string )
{
 }

const char* SimpleArchive::GetString()
{
 }

bool SimpleArchive::operator<<( U64 data )
{
    return true;
}
bool SimpleArchive::operator<<( U32 data )
{

    return true;
}
bool SimpleArchive::operator<<( S64 data )
{
    return true;
}
bool SimpleArchive::operator<<( S32 data )
{
    return true;
}
bool SimpleArchive::operator<<( double data )
{

    return true;
}
bool SimpleArchive::operator<<( bool data )
{
    return true;
}
bool SimpleArchive::operator<<( const char* data )
{
    return true;
}
bool SimpleArchive::operator<<( Channel& data )
{
    return true;
}

bool SimpleArchive::operator>>( U64& data )
{
    return true;
}
bool SimpleArchive::operator>>( U32& data )
{
    return true;
}
bool SimpleArchive::operator>>( S64& data )
{
    return true;
}
bool SimpleArchive::operator>>( S32& data )
{
    return true;
}
bool SimpleArchive::operator>>( double& data )
{
    return true;
}
bool SimpleArchive::operator>>( bool& data )
{
    return true;
}
bool SimpleArchive::operator>>( char const ** data )
{

    return true;
}
bool SimpleArchive::operator>>( Channel& data )
{

    return true;
}
