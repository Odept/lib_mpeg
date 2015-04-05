#include "header.h"

#include "common.h"


struct Header
{
	enum
	{
		LayerReserved	= 0,
		Layer3			= 1,
		Layer2			= 2,
		Layer1			= 3
	};

	enum
	{
		BitrateFree	= 0,
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

		if(Version == MPEGvReserved							||
		   Layer == LayerReserved							||
		   Bitrate == BitrateFree || Bitrate == BitrateBad	||
		   Sampling == SamplingRateReserved					||
		   Emphasis == EmphasisReserved)
			return false;

		// MPEG 1, layer 2 additional mode check
		if(Version != MPEGv1 || Layer != Layer2)
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
		return consistent[Bitrate][Channel == ChannelMono];
	}

	bool isV2() const { return (Version == MPEGv2 || Version == MPEGv25); }

	bool isProtected() const { return !Protection;	}
};

/******************************************************************************
 * MPEG Header
 *****************************************************************************/
uint CMPEGHeader::getSize() { return sizeof(Header); }

const char* CMPEGHeader::strVer(uint f_ver)
{
	static const char* ver[] = {"2.5", "", "2", "1"};
	ASSERT(f_ver < (sizeof(ver) / sizeof(*ver)));
	return ver[f_ver];
}
const char* CMPEGHeader::strChannelMode(uint f_mode)
{
	static const char* mode[] = {"Stereo", "Joint Stereo", "Dual", "Mono"};
	ASSERT(f_mode < (sizeof(mode) / sizeof(*mode)));
	return mode[f_mode];
}
const char* CMPEGHeader::strEmphasis(uint f_emphasis)
{
	static const char* emphasis[] = {"", "50/15", "", "CCIT J.17"};
	ASSERT(f_emphasis < (sizeof(emphasis) / sizeof(*emphasis)));
	return emphasis[f_emphasis];
}

// ====================================
const Header& CMPEGHeader::header() const { return (const Header&)m_header; }

MPEGVersion		CMPEGHeader::getMpegVersion()	const { return (MPEGVersion    )header().Version;	}
uint			CMPEGHeader::getLayer()			const { return              4 - header().Layer;		}
MPEGChannelMode	CMPEGHeader::getChannelMode()	const { return (MPEGChannelMode)header().Channel;	}
MPEGEmphasis	CMPEGHeader::getEmphasis()		const { return (MPEGEmphasis   )header().Emphasis;	}

bool CMPEGHeader::isProtected()		const { return header().isProtected();	}
bool CMPEGHeader::isPadded()		const { return header().Padding;		}
bool CMPEGHeader::isPrivate()		const { return header().Private;		}
bool CMPEGHeader::isCopyrighted()	const { return header().Copyright;		}
bool CMPEGHeader::isOriginal()		const { return header().Original;		}

uint CMPEGHeader::getFrameDataOffset() const { return getSize() + getSideInfoSize(); }


uint CMPEGHeader::getBitrate() const
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

uint CMPEGHeader::getSamplingRate() const
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
CMPEGHeader::CMPEGHeader(uint f_header): m_header(f_header) {}
CMPEGHeader::~CMPEGHeader() {}

CMPEGHeader* CMPEGHeader::gen(uint f_header, void* f_pMem)
{
	const Header& h = (const Header&)f_header;
	if(!h.isValid())
		return NULL;
	if(f_pMem)
		return new (f_pMem) CMPEGHeader(f_header);
	else
		return new CMPEGHeader(f_header);
}


uint CMPEGHeader::getFrameSize() const
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

float CMPEGHeader::getFrameLength() const
{
	static const uint SPF[][3] =
	{
		{1152, 1152, 384},
		{ 576, 1152, 384}
	};
	return SPF[header().isV2()][header().Layer - 1] / (float)getSamplingRate();
}

uint CMPEGHeader::getNextFrame() const
{
	ASSERT(!isCopyrighted());
	return (getFrameSize() + isCopyrighted() * sizeof(short)/* + getSideInfoSize()*/);
}


bool CMPEGHeader::operator==(const CMPEGHeader& f_header) const
{
	return ((m_header & Header::CmpMask) == (f_header.m_header & Header::CmpMask));
}

bool CMPEGHeader::operator!=(const CMPEGHeader& f_header) const
{
	return !(f_header == *this);
}


uint CMPEGHeader::getSideInfoSize() const
{
	static const uint size[][2] =
	{
		{32, 17},
		{17,  9}
	};
	return (header().Layer == Header::Layer3) ? size[header().isV2()][header().Channel == ChannelMono] : 0;
}

/******************************************************************************
 * Xing Header
 *****************************************************************************/
#define FOUR_CC(A, B, C, D) \
	(( (A) & 0xFF)			|\
	 (((B) & 0xFF) <<  8)	|\
	 (((C) & 0xFF) << 16)	|\
	 (((D) & 0xFF) << 24))

uint CXingHeader::getFrameCount()	const { return m_frames;     }
uint CXingHeader::getByteCount()	const { return m_bytes;      }
uint CXingHeader::getTOCsOffset()	const { return m_TOCsOffset; }
uint CXingHeader::getQuality()		const { return m_quality;    }

// ====================================
CXingHeader* CXingHeader::gen(const uchar* f_pData)
{
	uint id = *(const uint*)f_pData;
	if(id != FOUR_CC('X','i','n','g') && id != FOUR_CC('I','n','f','o'))
		return NULL;
	return new CXingHeader(f_pData);
}

CXingHeader::CXingHeader(const uchar* f_pData):
	m_frames(0),
	m_bytes(0),
	m_TOCsOffset(0),
	m_quality(0)
{
	const uint* pData = (const uint*)f_pData + 1;

	uint mask = *pData;
	pData++;

	if(mask & 0x0001)
	{
		m_frames = *pData;
		pData++;
	}
	if(mask & 0x0002)
	{
		m_bytes = *pData;
		pData++;
	}
	if(mask & 0x0004)
	{
		m_TOCsOffset = /*(uint)*/((const uchar*)pData - f_pData);
		pData = (const uint*)((const uchar*)pData + 100);
	}
	if(mask & 0x0008)
	{
		m_quality = *pData;
		//pData++;
	}
}

