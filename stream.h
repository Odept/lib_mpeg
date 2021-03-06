#pragma once

#include "common.h"
#include "mpeg.h"
#include "header.h"

#include <vector>


class CStream final : public MPEG::IStream
{
public:
						CStream			(const uchar* f_data, size_t f_size);
						CStream			() = delete;
	bool				hasIssues		() const final override { return m_warnings; }

	size_t				getSize			() const final override { return m_data.size();		}
	uint				getFrameCount	() const final override { return static_cast<uint>(m_frames.size()); }
	float				getLength		() const final override { return m_length;			}

	MPEG::Version		getVersion		() const final override { return m_version;			}
	uint				getLayer		() const final override { return m_layer;			}
	uint				getBitrate		() const final override { return m_abr;				}
	bool				isVBR			() const final override { return m_vbr;				}
	uint				getSamplingRate	() const final override { return m_sampling_rate;	}
	MPEG::ChannelMode	getChannelMode	() const final override { return m_channel_mode;	}
	MPEG::Emphasis		getEmphasis		() const final override { return m_emphasis;		}
	//bool				isCopyrighted	() const final override { return m_copyrighted;		}
	//bool				isOriginal		() const final override { return m_original;		}
	//bool				hasCRC			() const final override { return m_bCRC;			}

	size_t getFrameOffset(unsigned int f_index) const final override
	{
		return (f_index < m_frames.size()) ? m_frames[f_index].Offset : m_data.size();
	}
	unsigned int getFrameSize(unsigned int f_index) const final override
	{
		return (f_index < m_frames.size()) ? m_frames[f_index].Size : 0;
	}
	float getFrameTime(unsigned int f_index) const final override
	{
		return (f_index < m_frames.size()) ? m_frames[f_index].Time : 0.0f;
	}

	void				serialize		(std::vector<unsigned char>& f_outStream) final override;

	// Functional
	unsigned			cut				(unsigned f_frame, unsigned f_count) final override;
	unsigned			truncate		(unsigned f_frames) final override;

private:
	size_t	init	(const uchar* f_data, size_t f_offset, size_t f_size, bool f_bFirstInit);

private:
	struct FrameInfo
	{
		FrameInfo(size_t f_offset, uint f_size, float f_time, uint f_dataRelOffset):
			Offset(f_offset),
			Size(f_size),
			Time(f_time),
			DataRelOffset(f_dataRelOffset)
		{}

		size_t	Offset;
		uint	Size;
		float	Time;
		uint	DataRelOffset;
	};

private:
	float						m_length;

	MPEG::Version				m_version;
	uint						m_layer;
	uint						m_abr;
	bool						m_vbr;
	uint						m_sampling_rate;
	MPEG::ChannelMode			m_channel_mode;
	MPEG::Emphasis				m_emphasis;
	//bool						m_copyrighted;
	//bool						m_original;
	//bool						m_bCRC;

	std::unique_ptr<CXingFrame>	m_xing;
	std::vector<FrameInfo>		m_frames;
	std::vector<uchar>			m_data;

	uint						m_warnings;
};

