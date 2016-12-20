#include "header.h"

#include "common.h"


/******************************************************************************
 * MPEG Header
 *****************************************************************************/
const std::string& CHeader::str(MPEG::Version f_ver)
{
	static const std::string ver[] = {"2.5", "", "2", "1"};
	auto i = static_cast<unsigned>(f_ver);
	ASSERT(i < (sizeof(ver) / sizeof(*ver)));
	return ver[i];
}

const std::string& CHeader::str(MPEG::ChannelMode f_mode)
{
	static const std::string mode[] = {"Stereo", "Joint Stereo", "Dual Channel", "Mono"};
	auto i = static_cast<unsigned>(f_mode);
	ASSERT(i < (sizeof(mode) / sizeof(*mode)));
	return mode[i];
}

const std::string& CHeader::str(MPEG::Emphasis f_emphasis)
{
	static const std::string s_emphasis[] = {"None", "50/15", "", "CCIT J.17"};
	auto i = static_cast<uint>(f_emphasis);
	ASSERT(i < (sizeof(s_emphasis) / sizeof(*s_emphasis)));
	return s_emphasis[i];
}


uint CHeader::getBitrate(uint f_rawIndex) const
{
	ASSERT(f_rawIndex < Header::BitrateBad);

	static const uint s_index[][3] =
	{
		{0, 1, 2},
		{3, 4, 4}
	};
	static const uint s_bitrate[][16] =
	{
		{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0},
		{0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0},
		{0, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 0},
		{0, 32, 48, 56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0},
		{0,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0}
	};
	return s_bitrate[ s_index[m_header.isV2()][getLayer() - 1] ]
					[ f_rawIndex ] * 1000;
}


uint CHeader::getSamplingRate() const
{
	static const uint s_frequency[][3] =
	{
		{11025, 12000,  8000},
		{    0,     0,     0},
		{22050, 24000, 16000},
		{44100, 48000, 32000}
	};
	ASSERT(m_header.Sampling != Header::SamplingRateReserved);
	return s_frequency[m_header.Version][m_header.Sampling];
}


// Samples Per Frame / 8
static const uint s_SPF8[][3] =
{
	// 12 must be multiplied by 4 because of slot size
	{144, 144, 12},
	{ 72, 144, 12}
};
static const uint s_slotSize[] = {1, 1, 4};

uint CHeader::getFrameSize(uint f_bitrate) const
{
	uint i = m_header.Layer - 1;
	return ((s_SPF8[m_header.isV2()][i] * f_bitrate / getSamplingRate()) + m_header.Padding) * s_slotSize[i];
}

uint CHeader::calcFrameSize(const uchar* f_data, size_t f_size)
{
	ASSERT(isFreeBitrate());

	// Calc max frame size
	auto maxBitrate = getBitrate(Header::BitrateBad - 1);
	auto size = getFrameSize(maxBitrate);
	if(size > f_size)
		size = static_cast<uint>(f_size);

	for(size_t o = CHeader::getSize(); o + CHeader::getSize() <= size; ++o)
	{
		auto rawHeader = *reinterpret_cast<const uint*>(f_data + o);
		if(CHeader::isValid(rawHeader) && isValidSize(o))
			return o;
	}

	return 0;
}

bool CHeader::isValidSize(uint f_size) const
{
	uint i = m_header.Layer - 1;

	auto x = f_size / s_slotSize[i];
	if(x * s_slotSize[i] != f_size)
		return false;

	auto spf8 = s_SPF8[m_header.isV2()][i];
	x = (x - m_header.Padding) * getSamplingRate();
	auto y = x / spf8;
	return (y * spf8 == x);
}


float CHeader::getFrameLength() const
{
	static const float s_SPF[][3] =
	{
		{1152.0, 1152.0, 384.0},
		{ 576.0, 1152.0, 384.0}
	};
	return s_SPF[m_header.isV2()][m_header.Layer - 1] / getSamplingRate();
}


uint CHeader::getSideInfoSize() const
{
	static const uint s_size[][2] =
	{
		{32, 17},
		{17,  9}
	};
	return (m_header.Layer == Header::Layer3)
		   ? s_size[m_header.isV2()][m_header.Channel == static_cast<uint>(MPEG::ChannelMode::Mono)]
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
	ASSERT(!header.isFreeBitrate());

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

