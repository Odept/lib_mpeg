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

	struct FrameInfo
	{
		FrameInfo(uint f_offset, uint f_size, float f_time):
			Offset(f_offset),
			Size(f_size),
			Time(f_time)
		{}

		uint	Offset;
		uint	Size;
		float	Time;
	};

// Static Section
public:
	static CMPEGStream*	gen(const uchar* f_data, uint f_size);

	static uint calcFirstHeaderOffset(const uchar* f_data, uint f_size);
	static bool verifyFrameSequence  (const uchar* f_data, uint f_size);

	static bool isIncompleteFrame(const uchar* f_data, uint f_size);

private:
	static uint findHeader(const uchar* f_data, uint f_size);

// Public Section
public:
	virtual ~CMPEGStream();

	uint	getSize()					const;
	uint	getFrameCount()				const;
	float	getLength()					const;

	const char*	getVersion()		const;
	uint		getLayer()			const;
	uint		getBitrate()		const;
	bool		isVBR()				const;
	uint		getSamplingRate()	const;
	const char*	getChannelMode()	const;
	const char*	getEmphasis()		const;

	uint	getFrameOffset(uint f_index)	const;
	uint	getFrameSize(uint f_index)		const;
	float	getFrameTime(uint f_index)		const;

	uint truncate(uint f_frames);

private:
	CMPEGStream(const uchar* f_data, uint f_size);
	CMPEGStream();

private:
	std::vector<uchar> m_data;

	float	m_length;

	uint	m_version;
	uint	m_layer;
	uint	m_abr;
	bool	m_vbr;
	uint	m_sampling_rate;
	uint	m_channel_mode;
	uint	m_emphasis;

	std::vector<FrameInfo> m_frames;
};

#endif	// _MPEG_STREAM_H_

