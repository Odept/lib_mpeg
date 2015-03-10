#include "header.h"

/******************************************************************************
 * MPEG Version Class
 *****************************************************************************/
CMPEGVer::CMPEGVer()			{ init(0x01 /*invalid (reserved) mask*/); }
CMPEGVer::CMPEGVer(uint f_mask)	{ init(f_mask); }

bool CMPEGVer::isValid()	const { return m_v2 || !m_v25; }
bool CMPEGVer::isV2()		const { return m_v2; }

uint CMPEGVer::getIndex() const { return (m_v2 ? (m_v25 ? 2 : 1) : 0); }

void CMPEGVer::init(uint f_mask)
{
	m_v2	= (f_mask == 0x02) || (f_mask == 0x00);
	m_v25	= (f_mask == 0x00) || (f_mask == 0x01);
}

/******************************************************************************
 * Public Section
 *****************************************************************************/
// Basic routines
CMPEGHeader::CMPEGHeader(uint f_header):
	m_header(f_header),
	m_valid(false),
	m_layer(0),
	m_bitrate(0),
	m_frequency(0)
{
	m_valid = isValidInternal();
	if(m_valid)
	{
		m_mpeg = calcMpegVersion();
		m_layer = calcLayer();
		m_bitrate = calcBitrate(m_mpeg, m_layer);
		m_frequency = calcFrequency(m_mpeg);
	}
}

bool CMPEGHeader::isValid() const { return m_valid; }

const CMPEGVer&	CMPEGHeader::getMpegVersion()	const { return m_mpeg;		}
uint			CMPEGHeader::getLayer()			const { return m_layer;		}
uint			CMPEGHeader::getBitrate()		const { return m_bitrate;	}
uint			CMPEGHeader::getFrequency()		const { return m_frequency;	}

bool CMPEGHeader::isProtected()		const { return !((m_header >> 8) & 0x01); }
bool CMPEGHeader::isPadded()		const { return ((m_header >> 17) & 0x01); }
bool CMPEGHeader::isPrivate()		const { return ((m_header >> 16) & 0x01); }
bool CMPEGHeader::isCopyrighted()	const { return ((m_header >> 27) & 0x01); }
bool CMPEGHeader::isOriginal()		const { return ((m_header >> 26) & 0x01); }

EMPHASIS CMPEGHeader::getEmphasis() const
{
	ASSERT(((m_header >> 24) & 0x03) != 0x02);

	switch((m_header >> 24) & 0x03)
	{
		case 0x01: return EMPHASIS_50_15;
		case 0x03: return EMPHASIS_CCIT_J17;
		case 0x00:

		default:
			return EMPHASIS_NONE;
	}
}

CHANNEL_MODE CMPEGHeader::getChannelMode()	const
{
	return (CHANNEL_MODE)((m_header >> 30) & 0x03);
}

// Complex routines
uint CMPEGHeader::getFrameSize() const
{
	if(!m_valid)
	{
		ASSERT(!"Invalid frame");
		return 0;
	}

	// Samples Per Frame / 8
	static const uint SPF8[][3] =
	{
		// 12 must be multiplied by 4 because of slot size
		{12, 144, 144},
		{12, 144,  72}
	};
	static const uint slotSize[] = {4, 1, 1};

	return ((SPF8[m_mpeg.isV2()][m_layer - 1] * m_bitrate / m_frequency) + isPadded()) * slotSize[m_layer - 1];
};


float CMPEGHeader::getFrameLength() const
{
	if(!m_valid)
	{
		ASSERT(!"Invalid frame");
		return 0;
	}

	static const float SPF[][3] =
	{
		{384.0f, 1152.0f, 1152.0f},
		{384.0f, 1152.0f,  576.0f}
	};

	return SPF[m_mpeg.isV2()][m_layer - 1] / m_frequency;
}


uint CMPEGHeader::getNextFrame() const
{
	ASSERT(!isCopyrighted());
	return (/*sizeof(m_header) + */getFrameSize() + isCopyrighted() * sizeof(short)/* + getSideInfoSize()*/);
}

// version, layer, sampling rate, channel mode, emphasis (________ ___xxxx_ ____xx__ xx____xx)
static const uint HeaderMask = 0xC30C1E00;
bool CMPEGHeader::operator==(const CMPEGHeader& f_header) const
{
	return ((m_header & HeaderMask) == (f_header.m_header & HeaderMask));
}

bool CMPEGHeader::operator!=(const CMPEGHeader& f_header) const
{
	return !(f_header == *this);
}

/******************************************************************************
 * Private Section
 *****************************************************************************/
// Basic routines
CMPEGVer CMPEGHeader::calcMpegVersion() const { return CMPEGVer((m_header >> 11) & 0x3); }

uint CMPEGHeader::calcLayer() const
{
	ASSERT((m_header >> 9) & 0x03);
	return (4 - ((m_header >> 9) & 0x03));
}

// Complex routines
bool CMPEGHeader::isValidInternal() const
{
	if((m_header & 0xE0FF) != 0xE0FF)
		return false;

	if(!(m_header & 0x0600) || !(m_header & 0xF00000) ||	/*!(m_header & 0x030000) ||*/
	   !(~m_header & 0xF00000) || !(~m_header & 0x0C0000))
		return false;

	if(((m_header & 0x1800) == 0x0800) || ((m_header & 0x03000000) == 0x02000000))
		return false;

	// MPEG 1, layer 2 additional mode check
	if(((m_header & 0x1800) == 0x1800) && ((m_header & 0x0600) == 0x0400))
	{
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

		return consistent[((m_header >> 20) & 0x0F)]			// bitrate index
						 [(((m_header >> 30) & 0x3) == 0x3)];	// mono
	}

	return true;
}


uint CMPEGHeader::calcBitrate(const CMPEGVer& f_ver, uint f_layer) const
{
	ASSERT(f_ver.isValid() && f_layer && (f_layer < 4));

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

	return bitrate[ index[f_ver.isV2()][f_layer - 1] ]
				  [ (m_header >> 20) & 0x0F ] * 1000;
}


uint CMPEGHeader::calcFrequency(const CMPEGVer& f_ver) const
{
	ASSERT(f_ver.isValid());

	static const uint frequency[][3] =
	{
		{44100, 48000, 32000},
		{22050, 24000, 16000},
		{11025, 12000,  8000},
		{    0,     0,     0}
	};
	
	return frequency[f_ver.getIndex()][(m_header >> 18) & 0x03];
}


uint CMPEGHeader::getSideInfoSize() const
{
	static const uint size[][2] =
	{
		{32, 17},
		{17,  9}
	};

	ASSERT(m_valid);
	return (m_valid && (m_layer == 3)) ? size[m_mpeg.isV2()][getChannelMode() == CHANNEL_MONO] : 0;
}

