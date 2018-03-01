#include "AsyncRgbLedAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "AsyncRgbLedAnalyzer.h"
#include "AsyncRgbLedAnalyzerSettings.h"
#include <iostream>
#include <fstream>

AsyncRgbLedAnalyzerResults::AsyncRgbLedAnalyzerResults( AsyncRgbLedAnalyzer* analyzer, AsyncRgbLedAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

AsyncRgbLedAnalyzerResults::~AsyncRgbLedAnalyzerResults()
{
}

void AsyncRgbLedAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	ClearResultStrings();
	Frame frame = GetFrame( frame_index );

	U32 ledIndex = frame.mData2;
	U8 red = frame.mData1 >> 16;
	U8 green = (frame.mData1 >> 8) & 0xff;
	U8 blue = frame.mData1 & 0xff;
	char colorBuffer[128];
	::snprintf(colorBuffer, sizeof(colorBuffer), "LED %d #%02x%02x%02x", ledIndex, red, green, blue);
	AddResultString( colorBuffer );
}

void AsyncRgbLedAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	file_stream << "Time [s],Value" << std::endl;

	U64 num_frames = GetNumFrames();
	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
		
		char time_str[128];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

		char number_str[128];
		AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );

		file_stream << time_str << "," << number_str << std::endl;

		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}

	file_stream.close();
}

void AsyncRgbLedAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
#ifdef SUPPORTS_PROTOCOL_SEARCH
	Frame frame = GetFrame( frame_index );
	ClearTabularText();

	char number_str[128];
	AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
	AddTabularText( number_str );
#endif
}

void AsyncRgbLedAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	//not supported

}

void AsyncRgbLedAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	//not supported
}