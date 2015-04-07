#ifndef _MPEG_HEADER_H_
#define _MPEG_HEADER_H_

#pragma once


struct Header;

// Declarations
enum MPEGVersion
{
	MPEGv25			= 0,
	MPEGvReserved	= 1,
	MPEGv2			= 2, // ISO/IEC 13818-3
	MPEGv1			= 3	 // ISO/IEC 11172-3
};

enum MPEGChannelMode
{
	ChannelStereo		= 0,
	ChannelJointStereo	= 1,
	ChannelDual			= 2,
	ChannelMono			= 3
};

enum MPEGEmphasis
{
	EmphasisNone		= 0,
	Emphasis5015		= 1,
	EmphasisReserved	= 2,
	EmphasisCCITJ17		= 3
};

// MPEG Header
class CMPEGHeader
{
private:
	typedef unsigned int uint;

// Public Static Interface
public:
	static CMPEGHeader* gen(uint f_header, void* f_pMem = 0);

	static uint getSize();

	static const char* strVer(uint f_ver);
	static const char* strChannelMode(uint f_mode);
	static const char* strEmphasis(uint f_emphasis);

// Interfaces
public:
	virtual ~CMPEGHeader();

	MPEGVersion		getVersion()		const;
	uint			getLayer()			const;
	bool			isProtected()		const;
	uint			getBitrate()		const;
	uint			getSamplingRate()	const;
	bool			isPadded()			const;
	bool			isPrivate()			const;
	MPEGChannelMode	getChannelMode()	const;
	bool			isCopyrighted()		const;
	bool			isOriginal()		const;
	MPEGEmphasis	getEmphasis()		const;

	// Complex
	uint	getFrameSize()			const;
	float	getFrameLength()		const;
	uint	getFrameDataOffset()	const;

	// version, layer, sampling rate, channel mode, emphasis (________ ___xxxx_ ____xx__ xx____xx)
	bool operator==(const CMPEGHeader& f_header) const;
	bool operator!=(const CMPEGHeader& f_header) const;

	// XING / VBRI header is in the 1-st frame after side information block (layer 3 only)

// Private members
private:
	CMPEGHeader(uint f_header);
	CMPEGHeader();

	const Header& header() const;

	uint getSideInfoSize() const;

private:
	uint m_header;
};

// Xing Header
class CXingHeader
{
private:
	typedef unsigned int	uint;
	typedef unsigned char	uchar;

public:
	static CXingHeader* gen(const uchar* f_pData);

public:
	uint getFrameCount()	const;
	uint getByteCount()		const;
	uint getTOCsOffset()	const;
	uint getQuality()		const;

private:
	CXingHeader(const uchar* f_pData);
	CXingHeader();

private:
	uint m_frames;
	uint m_bytes;
	uint m_TOCsOffset;
	uint m_quality;
};

#endif	// _MPEG_HEADER_H_

