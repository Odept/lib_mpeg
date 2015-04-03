#ifndef _MPEG_STREAM_H_
#define _MPEG_STREAM_H_

#pragma once

#include <vector>


class CMPEGStream
{
// Declarations
private:
	typedef unsigned int	uint;
	typedef unsigned char	uchar;

/*	struct SFrameInfo
	{
		SFrameInfo(uint f_offset, uint f_size, float f_length):
			offset(f_offset),
			size(f_size),
			length(f_length)
		{}

		uint offset;
		uint size;
		float		 length;
	};
//*/

// Static Section
public:
	static CMPEGStream*	gen(const uchar* f_data, uint f_size);

	static uint calcFirstHeaderOffset(const uchar* f_data, uint f_size);
	static bool verifyFrameSequence  (const uchar* f_data, uint f_size);

private:
	static uint findHeader(const uchar* f_data, uint f_size);

// Public Section
public:
	virtual ~CMPEGStream();

	uint	getFirstDataFrameOffset()	const;
	uint	getFrameCount()				const;
	float	getLength()					const;
	uint	getBitrate()				const;

private:
	CMPEGStream(const uchar* f_data, uint f_size);
	CMPEGStream();

private:
	std::vector<uchar> m_data;
	uint	m_size;

	uint	m_offset;

	float	m_length;
	uint	m_abr;

	//std::vector<SFrameInfo> m_frames;
	uint	m_frames;
};

#endif	// _MPEG_STREAM_H_

