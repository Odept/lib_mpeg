#pragma once

#include "common.h"
#include "mpeg.h"

// ====================================
// MPEG Header
class CHeader final
{
public:
	static bool					isValid				(uint f_header);

	static size_t				getSize				();

	static const std::string&	str					(MPEG::Version		f_ver);
	static const std::string&	str					(MPEG::ChannelMode	f_mode);
	static const std::string&	str					(MPEG::Emphasis		f_emphasis);

public:
								CHeader				(uint f_header): m_header(f_header) {}
								CHeader				() = delete;

	MPEG::Version				getVersion			() const;
	uint						getLayer			() const;
	bool						isProtected			() const;
	uint						getBitrate			() const;
	uint						getSamplingRate		() const;
	bool						isPadded			() const;
	bool						isPrivate			() const;
	MPEG::ChannelMode			getChannelMode		() const;
	bool						isCopyrighted		() const;
	bool						isOriginal			() const;
	MPEG::Emphasis				getEmphasis			() const;

	// Complex
	uint						getFrameSize		() const;
	float						getFrameLength		() const;
	uint						getFrameDataOffset	() const;

	// version, layer, sampling rate, channel mode, emphasis (________ ___xxxx_ ____xx__ xx____xx)
	bool operator==(const CHeader& f_header) const;
	bool operator!=(const CHeader& f_header) const;

	// XING / VBRI header is in the 1-st frame after side information block (layer 3 only)

	// Private methods
private:
	const struct Header&		header				() const;

	uint						getSideInfoSize		() const;

private:
	uint m_header;
};

// ====================================
// Xing Header
class CXingHeader
{
public:
	static bool isValid(const uchar* f_pData)
	{
		uint id = *reinterpret_cast<const uint*>(f_pData);
		// Xing = VBR & ABR; Info = CBR
		return (id == FOUR_CC('X','i','n','g') || id == FOUR_CC('I','n','f','o'));
	}

public:
	CXingHeader(const uchar* f_pData);
	CXingHeader() = delete;

	uint getFrameCount()	const { return m_frames;     }
	uint getByteCount()		const { return m_bytes;      }
	uint getTOCsOffset()	const { return m_TOCsOffset; }
	uint getQuality()		const { return m_quality;    }

private:
	uint m_frames;
	uint m_bytes;
	uint m_TOCsOffset;
	uint m_quality;
};

