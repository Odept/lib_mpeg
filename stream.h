#pragma once

#include "common.h"
#include "mpeg.h"
#include "header.h"

#include <vector>


class CStream final : public MPEG::IStream
{
public:
						CStream			(const uchar* f_data, uint f_size);
						CStream			() = delete;

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

	size_t getFrameOffset(uint f_index) const final override
	{
		return (f_index < m_frames.size()) ? m_frames[f_index].Offset : m_data.size();
	}
	uint getFrameSize(uint f_index) const final override
	{
		return (f_index < m_frames.size()) ? m_frames[f_index].Size : 0;
	}
	float getFrameTime(uint f_index) const final override
	{
		return (f_index < m_frames.size()) ? m_frames[f_index].Time : 0.0f;
	}

	uint				truncate		(uint f_frames) final override;

private:
	struct FrameInfo
	{
		FrameInfo(size_t f_offset, uint f_size, float f_time):
			Offset(f_offset),
			Size(f_size),
			Time(f_time)
		{}

		size_t	Offset;
		uint	Size;
		float	Time;
	};

private:
	std::vector<uchar>			m_data;

	float						m_length;

	MPEG::Version				m_version;
	uint						m_layer;
	uint						m_abr;
	bool						m_vbr;
	uint						m_sampling_rate;
	MPEG::ChannelMode			m_channel_mode;
	MPEG::Emphasis				m_emphasis;

	std::unique_ptr<CXingFrame>	m_xing;
	std::vector<FrameInfo>		m_frames;
};

