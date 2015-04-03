#ifndef _MPEG_STREAM_H_
#define _MPEG_STREAM_H_

#pragma once

//#include <vector>


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

// Routines
public:
	static CMPEGStream*	gen(const uchar* f_data, uint f_size);
	static bool			verifyFrameSequence(const uchar* f_data, uint f_size);

public:
	uint	getFrameCount()			const;
	float	getLength()				const;
	uint	getBitrate()			const;
	uint	getFirstHeaderOffset()	const;

private:
	CMPEGStream(const uchar* f_data, uint f_size);
	CMPEGStream();

	bool parse();

	uint calcFirstHeaderOffset() const;
	uint findHeader(const uchar* f_data, uint f_size) const;

// Members
private:
	const uchar* m_data;
	uint m_size;

	uint m_first_header_offset;

	float m_length;
	uint m_abr;

	//std::vector<SFrameInfo> m_frames;
	uint m_frames;
};

#endif	// _MPEG_STREAM_H_

