#pragma once

#include "common.h"
#include "mpeg.h"


// Raw MPEG Header
struct Header
{
	// 11111111 111VVLLP BBBBSS_p CCXX@OEE
	union
	{
		struct
		{
			uint Sync0		: 8;

			uint Protection	: 1;
			uint Layer		: 2;
			uint Version	: 2;
			uint Sync1		: 3;

			uint Private	: 1;
			uint Padding	: 1;
			uint Sampling	: 2;
			uint Bitrate	: 4;

			uint Emphasis	: 2;
			uint Original	: 1;
			uint Copyright	: 1;
			uint Extension	: 2;
			uint Channel	: 2;
		};
		uint uCell;
	};


	// For simplicity, don't use scoped enum for internal enums
	enum
	{
		LayerReserved	= 0,
		Layer3			= 1,
		Layer2			= 2,
		Layer1			= 3
	};

	enum
	{
		BitrateFree	= 0x0,
		BitrateBad	= 0xF
	};

	enum
	{
		SamplingRateReserved = 3
	};

	// Functions
	Header(uint f_header): uCell(f_header) {}

	bool isValid() const
	{
		static const uint SyncMask = 0xE0FF;
		if((uCell & SyncMask) != SyncMask)
			return false;

		if(Version	== static_cast<uint>(MPEG::Version::vReserved)	||
		   Layer	== LayerReserved								||
		   Bitrate	== BitrateBad									||
		   Sampling	== SamplingRateReserved							||
		   Emphasis	== static_cast<uint>(MPEG::Emphasis::Reserved))
			return false;

		// MPEG 1, layer 2 additional mode check
		if(Version != static_cast<uint>(MPEG::Version::v1) || Layer != Layer2)
			return true;

		static const bool consistent[][16] =
		{
			{ true,  true},	// free
			{false,  true},	// 32
			{false,  true},	// 48
			{false,  true},	// 56
			{ true,  true},	// 64
			{false,  true},	// 80
			{ true,  true},	// 96
			{ true,  true},	// 112
			{ true,  true},	// 128
			{ true,  true},	// 160
			{ true,  true},	// 192
			{ true, false},	// 224
			{ true, false},	// 256
			{ true, false},	// 320
			{ true, false},	// 384
			{false, false}	// reserved
		};
		return consistent[Bitrate][Channel == static_cast<uint>(MPEG::ChannelMode::Mono)];
	}

	bool isV2() const
	{
		return (Version == static_cast<uint>(MPEG::Version::v2) ||
				Version == static_cast<uint>(MPEG::Version::v25));
	}

	bool isProtected() const { return !Protection;	}
	bool isFreeBitrate() const { return (Bitrate == BitrateFree); }

	bool operator==(const Header& f_header) const
	{
		// version, layer, sampling rate, channel mode, emphasis
		// (________  ___xxxx_  ____xx__  xx____xx)
		static const uint CmpMask = 0xC30C1E00;
		return ((uCell ^ f_header.uCell) & CmpMask) == 0;
	}
};
static_assert(sizeof(Header) == sizeof(uint), "Invalid MPEH header structure size");

