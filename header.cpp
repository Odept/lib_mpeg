#include "header.h"

#include "common.h"


#define FOUR_CC(A, B, C, D) \
	(( (A) & 0xFF)			|\
	 (((B) & 0xFF) <<  8)	|\
	 (((C) & 0xFF) << 16)	|\
	 (((D) & 0xFF) << 24))


struct Header
{
	enum
	{
		Ver25			= 0,
		VerReserved		= 1,
		Ver2			= 2, // ISO/IEC 13818-3
		Ver1			= 3	 // ISO/IEC 11172-3
	};

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

		if(Version == VerReserved							||
		   Layer == LayerReserved							||
		   Bitrate == BitrateFree || Bitrate == BitrateBad	||
		   Sampling == SamplingRateReserved					||
		   Emphasis == EmphasisReserved)
			return false;

		// MPEG 1, layer 2 additional mode check
		if(Version != Ver1 || Layer != Layer2)
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

	bool isProtected() const { return !Protection;	}
};


/******************************************************************************
 * MPEG Version
 *****************************************************************************/
CMPEGVer::CMPEGVer(uint f_mask)
{
	m_v2	= (f_mask == 0x02) || (f_mask == 0x00);
	m_v25	= (f_mask == 0x00) || (f_mask == 0x01);
}

bool CMPEGVer::isValid()	const { return m_v2 || !m_v25; }
bool CMPEGVer::isV2()		const { return m_v2; }

uint CMPEGVer::getIndex() const { return (m_v2 ? (m_v25 ? 2 : 1) : 0); }

/******************************************************************************
 * MPEG Header
 *
 * Getters & Setters
 *****************************************************************************/
uint CMPEGHeader::getSize() { return sizeof(Header); }

const Header& CMPEGHeader::header() const { return (const Header&)m_header; }

const CMPEGVer&	CMPEGHeader::getMpegVersion()	const { return m_ver;				}
uint			CMPEGHeader::getLayer()			const { return 4 - header().Layer;	}

bool CMPEGHeader::isProtected()		const { return header().isProtected();	}
bool CMPEGHeader::isPadded()		const { return header().Padding;		}
bool CMPEGHeader::isPrivate()		const { return header().Private;		}
bool CMPEGHeader::isCopyrighted()	const { return header().Copyright;		}
bool CMPEGHeader::isOriginal()		const { return header().Original;		}

MPEGEmphasis	CMPEGHeader::getEmphasis()		const { return (MPEGEmphasis   )header().Emphasis; }
MPEGChannelMode	CMPEGHeader::getChannelMode()	const { return (MPEGChannelMode)header().Channel;  }

uint CMPEGHeader::getFrameDataOffset()			const { return getSize() + getSideInfoSize(); }

// Complex Routines
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

CMPEGHeader::CMPEGHeader(uint f_header):
	m_header(f_header),
	m_ver(header().Version)
{}


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
	return bitrate[ index[m_ver.isV2()][getLayer() - 1] ]
				  [ header().Bitrate ] * 1000;
}


uint CMPEGHeader::getSamplingRate() const
{
	static const uint frequency[][3] =
	{
		{44100, 48000, 32000},
		{22050, 24000, 16000},
		{11025, 12000,  8000}
	};
	return frequency[m_ver.getIndex()][header().Sampling];
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
	return ((SPF8[m_ver.isV2()][i] * getBitrate() / getSamplingRate()) + header().Padding) * slotSize[i];
}


float CMPEGHeader::getFrameLength() const
{
	static const uint SPF[][3] =
	{
		{1152, 1152, 384},
		{ 576, 1152, 384}
	};
	return SPF[m_ver.isV2()][header().Layer - 1] / (float)getSamplingRate();
}


uint CMPEGHeader::getNextFrame() const
{
	ASSERT(!isCopyrighted());
	return (getFrameSize() + isCopyrighted() * sizeof(short)/* + getSideInfoSize()*/);
}

// version, layer, sampling rate, channel mode, emphasis (________ ___xxxx_ ____xx__ xx____xx)
bool CMPEGHeader::operator==(const CMPEGHeader& f_header) const
{
	return ((m_header & Header::CmpMask) == (f_header.m_header & Header::CmpMask));
}

bool CMPEGHeader::operator!=(const CMPEGHeader& f_header) const
{
	return !(f_header == *this);
}

/******************************************************************************
 * Private Section
 *****************************************************************************/
uint CMPEGHeader::getSideInfoSize() const
{
	static const uint size[][2] =
	{
		{32, 17},
		{17,  9}
	};
	return (header().Layer == Header::Layer3) ? size[m_ver.isV2()][header().Channel == ChannelMono] : 0;
}

/******************************************************************************
 * Xing Header
 *****************************************************************************/
CXingHeader::CXingHeader(const unsigned char* f_data):
	m_valid(false),
	m_frames(0),
	m_bytes(0),
	m_TOCsOffset(0),
	m_quality(0)
{
	const uint* pData = (const uint*)f_data;

	uint id = *pData;
	if(id != FOUR_CC('X','i','n','g') && id != FOUR_CC('I','n','f','o'))
		return;

	m_valid = true;
	pData++;

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
		m_TOCsOffset = /*(uint)*/((const unsigned char*)pData - f_data);
		pData = (uint*)((unsigned char*)pData + 100);
	}
	if(mask & 0x0008)
	{
		m_quality = *pData;
		//pData++;
	}
}

bool CXingHeader::isValid()			const { return m_valid;      }

uint CXingHeader::getFrameCount()	const { return m_frames;     }
uint CXingHeader::getByteCount()	const { return m_bytes;      }
uint CXingHeader::getTOCsOffset()	const { return m_TOCsOffset; }
uint CXingHeader::getQuality()		const { return m_quality;    }

