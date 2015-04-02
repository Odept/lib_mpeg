#ifndef _MPEG_HEADER_H_
#define _MPEG_HEADER_H_

#pragma once


struct Header;

// Declarations
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

// MPEG Version
class CMPEGVer
{
private:
	typedef unsigned int uint;

public:
	CMPEGVer(uint f_mask);

	bool isValid()	const;
	bool isV2()		const;

	uint getIndex()	const;

private:
	CMPEGVer();

private:
	bool m_v2;
	bool m_v25;
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

// Interfaces
public:

	// Basic
	bool isValid() const;

	const CMPEGVer&	getMpegVersion()	const;
	uint			getLayer()			const;
	uint			getBitrate()		const;
	uint			getFrequency()		const;

	bool			isProtected()		const;
	bool			isPadded()			const;
	bool			isPrivate()			const;
	bool			isCopyrighted()		const;
	bool			isOriginal()		const;

	MPEGEmphasis	getEmphasis()		const;
	MPEGChannelMode	getChannelMode()	const;

	// Complex
	uint	getFrameSize()			const;
	float	getFrameLength()		const;
	uint	getFrameDataOffset()	const;

	uint	getNextFrame()			const;

	// version, layer, sampling rate, channel mode, emphasis (________ ___xxxx_ ____xx__ xx____xx)
	bool operator==(const CMPEGHeader& f_header) const;
	bool operator!=(const CMPEGHeader& f_header) const;

	// XING / VBRI header is in the 1-st frame after side information block (layer 3 only)

// Private members
private:
	CMPEGHeader(uint f_header);
	CMPEGHeader();

	const Header& header() const;

	CMPEGVer	calcMpegVersion()									const;
	uint		calcBitrate(const CMPEGVer& f_ver, uint f_layer)	const;
	uint		calcFrequency(const CMPEGVer& f_ver)				const;

	uint getSideInfoSize() const;

private:
	uint m_header;

	CMPEGVer m_ver;
	uint m_layer;
	uint m_bitrate;
	uint m_frequency;
};

// Xing Header
class CXingHeader
{
private:
	typedef unsigned int uint;

public:
	CXingHeader(const unsigned char* f_data);

	bool isValid()			const;

	uint getFrameCount()	const;
	uint getByteCount()		const;
	uint getTOCsOffset()	const;
	uint getQuality()		const;

private:
	CXingHeader();

private:
	bool m_valid;

	uint m_frames;
	uint m_bytes;
	uint m_TOCsOffset;
	uint m_quality;
};

#endif	// _MPEG_HEADER_H_

