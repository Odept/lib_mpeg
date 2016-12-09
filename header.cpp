#include "header.h"

#include "common.h"


struct Header
{
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

	enum { SamplingRateReserved = 3 };

	// Constants
	static const uint SyncMask = 0xE0FF;

	// version, layer, sampling rate, channel mode, emphasis
	// (________  ___xxxx_  ____xx__  xx____xx)
	static const uint CmpMask = 0xC30C1E00;

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
		unsigned int uCell;
	};

	// ================================
	bool isValid() const
	{
		if((uCell & SyncMask) != SyncMask)
			return false;

		if(Version	== static_cast<unsigned>(MPEG::Version::vReserved)	||
		   Layer	== LayerReserved									||
		   Bitrate	== BitrateFree										||
		   Bitrate	== BitrateBad										||
		   Sampling	== SamplingRateReserved								||
		   Emphasis	== static_cast<unsigned>(MPEG::Emphasis::Reserved))
			return false;

		// MPEG 1, layer 2 additional mode check
		if(Version != static_cast<unsigned>(MPEG::Version::v1) || Layer != Layer2)
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
		return consistent[Bitrate][Channel == static_cast<unsigned>(MPEG::ChannelMode::Mono)];
	}

	bool isV2() const
	{
		return (Version == static_cast<unsigned>(MPEG::Version::v2) ||
				Version == static_cast<unsigned>(MPEG::Version::v25));
	}

	bool isProtected() const { return !Protection;	}
};

/******************************************************************************
 * MPEG Header
 *****************************************************************************/
size_t CHeader::getSize() { return sizeof(Header); }

const std::string& CHeader::str(MPEG::Version f_ver)
{
	static std::string ver[] = {"2.5", "", "2", "1"};
	auto i = static_cast<unsigned>(f_ver);
	ASSERT(i < (sizeof(ver) / sizeof(*ver)));
	return ver[i];
}
const std::string& CHeader::str(MPEG::ChannelMode f_mode)
{
	static std::string mode[] = {"Stereo", "Joint Stereo", "Dual Channel", "Mono"};
	auto i = static_cast<unsigned>(f_mode);
	ASSERT(i < (sizeof(mode) / sizeof(*mode)));
	return mode[i];
}
const std::string& CHeader::str(MPEG::Emphasis f_emphasis)
{
	static std::string emphasis[] = {"None", "50/15", "", "CCIT J.17"};
	auto i = static_cast<unsigned>(f_emphasis);
	ASSERT(i < (sizeof(emphasis) / sizeof(*emphasis)));
	return emphasis[i];
}

// ====================================
const Header& CHeader::header() const { return (const Header&)m_header; }

MPEG::Version		CHeader::getVersion()		const { return static_cast<MPEG::Version>	(header().Version);		}
uint				CHeader::getLayer()			const { return							 4 - header().Layer;		}
MPEG::ChannelMode	CHeader::getChannelMode()	const { return static_cast<MPEG::ChannelMode>(header().Channel);	}
MPEG::Emphasis		CHeader::getEmphasis()		const { return static_cast<MPEG::Emphasis>	(header().Emphasis);	}

bool CHeader::isProtected()		const { return header().isProtected();	}
bool CHeader::isPadded()		const { return header().Padding;		}
bool CHeader::isPrivate()		const { return header().Private;		}
bool CHeader::isCopyrighted()	const { return header().Copyright;		}
bool CHeader::isOriginal()		const { return header().Original;		}

uint CHeader::getFrameDataOffset() const { return getSize() + getSideInfoSize(); }


uint CHeader::getBitrate() const
{
	static const uint index[][3] =
	{
		{0, 1, 2},
		{3, 4, 4}
	};
	static const uint bitrate[][16] =
	{
		{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0},
		{0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0},
		{0, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 0},
		{0, 32, 48, 56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0},
		{0,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0}
	};
	return bitrate[ index[header().isV2()][getLayer() - 1] ]
				  [ header().Bitrate ] * 1000;
}

uint CHeader::getSamplingRate() const
{
	static const uint frequency[][3] =
	{
		{11025, 12000,  8000},
		{    0,     0,     0},
		{22050, 24000, 16000},
		{44100, 48000, 32000}
	};
	return frequency[header().Version][header().Sampling];
}

// ============================================================================
bool CHeader::isValid(uint f_header)
{
	auto& h = reinterpret_cast<const Header&>(f_header);
	return h.isValid();
}


uint CHeader::getFrameSize() const
{
	// Samples Per Frame / 8
	static const uint SPF8[][3] =
	{
		// 12 must be multiplied by 4 because of slot size
		{144, 144, 12},
		{ 72, 144, 12}
	};
	static const uint slotSize[] = {1, 1, 4};

	uint i = header().Layer - 1;
	return ((SPF8[header().isV2()][i] * getBitrate() / getSamplingRate()) + header().Padding) * slotSize[i];
}

float CHeader::getFrameLength() const
{
	static const uint SPF[][3] =
	{
		{1152, 1152, 384},
		{ 576, 1152, 384}
	};
	return SPF[header().isV2()][header().Layer - 1] / (float)getSamplingRate();
}


bool CHeader::operator==(const CHeader& f_header) const
{
	return ((m_header & Header::CmpMask) == (f_header.m_header & Header::CmpMask));
}

bool CHeader::operator!=(const CHeader& f_header) const
{
	return !(f_header == *this);
}


uint CHeader::getSideInfoSize() const
{
	static const uint size[][2] =
	{
		{32, 17},
		{17,  9}
	};
	return (header().Layer == Header::Layer3)
		   ? size[header().isV2()][header().Channel == static_cast<unsigned>(MPEG::ChannelMode::Mono)]
		   : 0;
}

/******************************************************************************
 * Xing Header
 *****************************************************************************/
static uint fromBigEndian(const uint* f_pBE)
{
	auto p = reinterpret_cast<const uchar*>(f_pBE);
	return (static_cast<uint>(p[0]) << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
}

CXingHeader::CXingHeader(const uchar* f_data, size_t f_size):
	m_vbr(false),
	m_frames(0),
	m_bytes(0),
	m_TOCsOffset(0),
	m_quality(0),
	m_modified(false)
{
	auto nextFrame = f_data + f_size;

	ASSERT(f_size >= sizeof(uint));
	CHeader header(*reinterpret_cast<const uint*>(f_data));

	auto pData = reinterpret_cast<const uint*>(f_data + header.getFrameDataOffset());

	ASSERT(reinterpret_cast<const uchar*>(pData) + sizeof(uint) <= nextFrame);
	m_vbr = isVBR(*pData);
	++pData;

	ASSERT(reinterpret_cast<const uchar*>(pData) + sizeof(uint) <= nextFrame);
	m_flags = fromBigEndian(pData);
	++pData;

	if(m_flags & static_cast<uint>(Flags::Frames))
	{
		ASSERT(reinterpret_cast<const uchar*>(pData) + sizeof(uint) <= nextFrame);
		m_frames = fromBigEndian(pData);
		++pData;
	}
	if(m_flags & static_cast<uint>(Flags::Bytes))
	{
		ASSERT(reinterpret_cast<const uchar*>(pData) + sizeof(uint) <= nextFrame);
		m_bytes = fromBigEndian(pData);
		++pData;
	}
	if(m_flags & static_cast<uint>(Flags::TOC))
	{
		ASSERT(reinterpret_cast<const uchar*>(pData) + 100 <= nextFrame);
		m_TOCsOffset = reinterpret_cast<const uchar*>(pData) - f_data;
		pData = reinterpret_cast<const uint*>(reinterpret_cast<const uchar*>(pData) + 100);
	}
	if(m_flags & static_cast<uint>(Flags::Quality))
	{
		ASSERT(reinterpret_cast<const uchar*>(pData) + sizeof(uint) <= nextFrame);
		m_quality = fromBigEndian(pData);
		//++pData;
	}
}

