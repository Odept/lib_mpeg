#pragma once

#include "common.h"
#include "mpeg.h"

#include <vector>


class CStream : public MPEG::IStream
{
public:
						CStream			(const uchar* f_data, uint f_size);
						CStream			() = delete;
	virtual				~CStream		();

	uint				getSize			() const override { return static_cast<uint>(m_data.size());		}
	uint				getFrameCount	() const override { return static_cast<uint>(m_frames.size());	}
	float				getLength		() const override { return m_length;								}

	MPEG::Version		getVersion		() const override { return m_version;		}
	uint				getLayer		() const override { return m_layer;			}
	uint				getBitrate		() const override { return m_abr;			}
	bool				isVBR			() const override { return m_vbr;			}
	uint				getSamplingRate	() const override { return m_sampling_rate;	}
	MPEG::ChannelMode	getChannelMode	() const override { return m_channel_mode;	}
	MPEG::Emphasis		getEmphasis		() const override { return m_emphasis;		}

	uint getFrameOffset(uint f_index) const override
	{
		return (f_index < m_frames.size()) ? m_frames[f_index].Offset : (uint)m_data.size();
	}
	uint getFrameSize(uint f_index) const override
	{
		return (f_index < m_frames.size()) ? m_frames[f_index].Size : 0;
	}
	float getFrameTime(uint f_index) const override
	{
		return (f_index < m_frames.size()) ? m_frames[f_index].Time : 0.0f;
	}

	uint				truncate		(uint f_frames) override;

private:
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

private:
	std::vector<uchar>		m_data;

	float					m_length;

	MPEG::Version			m_version;
	uint					m_layer;
	uint					m_abr;
	bool					m_vbr;
	uint					m_sampling_rate;
	MPEG::ChannelMode		m_channel_mode;
	MPEG::Emphasis			m_emphasis;

	std::vector<FrameInfo>	m_frames;
};

