#pragma once

#include "header_raw.h"
#include "common.h"

#include <vector>


// MPEG Header
class CHeader final
{
public:
	static bool					isValid				(uint f_header)
	{
		auto& h = reinterpret_cast<const Header&>(f_header);
		return h.isValid();
	}

	static size_t				getSize				() { return sizeof(m_header); }

	static const std::string&	str					(MPEG::Version		f_ver);
	static const std::string&	str					(MPEG::ChannelMode	f_mode);
	static const std::string&	str					(MPEG::Emphasis		f_emphasis);

public:
								CHeader				(uint f_header): m_header(f_header) {}
								CHeader				() = delete;

	MPEG::Version				getVersion			() const { return static_cast<MPEG::Version>(m_header.Version); }
	uint						getLayer			() const { return 4 - m_header.Layer; }
	bool						isProtected			() const { return m_header.isProtected(); }
	uint						getBitrate			() const;
	uint						getSamplingRate		() const;
	bool						isPadded			() const { return m_header.Padding; }
	bool						isPrivate			() const { return m_header.Private; }
	MPEG::ChannelMode			getChannelMode		() const { return static_cast<MPEG::ChannelMode>(m_header.Channel); }
	bool						isCopyrighted		() const { return m_header.Copyright; }
	bool						isOriginal			() const { return m_header.Original; }
	MPEG::Emphasis				getEmphasis			() const { return static_cast<MPEG::Emphasis>(m_header.Emphasis); }

	// Complex
	uint						getFrameSize		() const;
	float						getFrameLength		() const;
	uint						getFrameDataOffset	() const { return getSize() + getSideInfoSize(); }

	bool						operator==			(const CHeader& f_header) const { return (m_header == f_header.m_header); }
	bool						operator!=			(const CHeader& f_header) const { return !(*this == f_header); }

	// Private methods
private:
	const Header&				header				() const { return m_header; }

	uint						getSideInfoSize		() const;

private:
	Header m_header;
};

// ====================================
// Xing Header (Zone A)
// XING / VBRI header is in the 1-st frame after a side information block (layer 3 only)
// https://www.codeproject.com/articles/8295/mpeg-audio-frame-header#XINGHeader
class CXingHeader
{
public:
	static bool isValid(const uchar* f_data, size_t f_size)
	{
		if(f_size < sizeof(uint))
			return false;
		auto h = *reinterpret_cast<const uint*>(f_data);
		return (isVBR(h) || isCBR(h));
	}

public:
	CXingHeader(const uchar* f_data, size_t f_size);
	CXingHeader() = delete;

	// Getters
	bool isVBR			() const { return m_vbr;        }
	uint getFrameCount	() const { return m_frames;     }
	uint getByteCount	() const { return m_bytes;      }
	uint getTOCsOffset	() const { return m_TOCsOffset; }
	uint getQuality		() const { return m_quality;    }
	// Setters
	void setFrameCount(uint f_frames)
	{
		if(f_frames != m_frames)
		{
			m_frames = f_frames;
			m_modified = true;
		}
	}
	void setByteCount(uint f_bytes)
	{
		if(f_bytes != m_bytes)
		{
			m_bytes = f_bytes;
			m_modified = true;
		}
	}

private:
	static bool isVBR(uint f_header) { return (f_header == FOUR_CC('X','i','n','g')); }
	static bool isCBR(uint f_header) { return (f_header == FOUR_CC('I','n','f','o')); }

private:
	// All the serialized fields are Big-Endian
	enum class Flags
	{
		Frames	= 0x0001,
		Bytes	= 0x0002,
		TOC		= 0x0004,
		Quality	= 0x0008
	};
	uint m_flags;

	// "Xing" vs. "Info"
	bool m_vbr;
	// The number of "real" data frames (without XING)
	uint m_frames;
	// The size of "real" data stream (without XING)
	uint m_bytes;
	// 100 TOC (Table Of Contents) seek point entries (uchars)
	// stream_offset = (TOC[%] / 256.0) * stream_bytes
	uint m_TOCsOffset;
	// 0 - best, 100 - worst
	uint m_quality;

	bool m_modified;
};
// Zone B - Initial LAME Info (20 bytes, i.e. "LAME...")
// http://gabriel.mp3-tech.org/mp3infotag.html

// Zone C - LAME Tag


class CXingFrame
{
public:
	static size_t getSize(const uchar* f_data, size_t f_size)
	{
		if(f_size < sizeof(uint))
			return 0;
		CHeader header(*reinterpret_cast<const uint*>(f_data));

		auto size = header.getFrameSize();
		auto dataOffset = header.getFrameDataOffset();
		if(size > f_size || dataOffset >= f_size)
			return 0;

		return CXingHeader::isValid(f_data + dataOffset, f_size - dataOffset) ? size : 0;
	}

	CXingFrame(const uchar* f_data, size_t f_size):
		m_header(f_data, f_size),
		m_data(f_size)
	{
		memcpy(&m_data[0], f_data, f_size);
	}
	CXingFrame() = delete;

	CXingHeader& getHeader() { return m_header; }

	void serialize(std::vector<uchar>& /*f_outStream*/)
	{
		ASSERT(!"Not implemented");
		//if(m_header.isModified())
		//	sync();
	}

private:
	CXingHeader m_header;
	std::vector<uchar> m_data;
};

