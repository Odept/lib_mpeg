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
	static const std::string emphasis[] = {"None", "50/15", "", "CCIT J.17"};
	auto i = static_cast<unsigned>(f_emphasis);
	ASSERT(i < (sizeof(emphasis) / sizeof(*emphasis)));
	return emphasis[i];
}


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

