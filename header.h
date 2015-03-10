#ifndef _MPEG_HEADER_H_
#define _MPEG_HEADER_H_


#include <iostream>


#define ASSERT(X) if(!(X)) { std::cout << #X << std::endl; abort(); }


class CMPEGHeader
{
public:
	enum CHANNEL_MODE
	{
		CHANNEL_STEREO = 0,
		CHANNEL_JOINT_STEREO,
		CHANNEL_DUAL,
		CHANNEL_MONO
	};

	enum EMPHASIS
	{
		EMPHASIS_NONE,
		EMPHASIS_50_15,
		EMPHASIS_CCIT_J17
	};

public:
	class CMPEGVer
	{
	public:
		CMPEGVer(unsigned int f_mask):
			m_v2 ((f_mask == 0x02) || (f_mask == 0x00)),
			m_v25((f_mask == 0x00) || (f_mask == 0x01))
		{}
		
		bool isValid()	const { return m_v2 || !m_v25; }
		bool isV2()		const { return m_v2; }

		unsigned int getIndex() const { return (m_v2 ? (m_v25 ? 2 : 1) : 0); }

	private:
		bool m_v2;
		bool m_v25;
	};

public:
	CMPEGHeader(unsigned int f_header):
		m_header(f_header),
		m_valid(false),
		m_mpeg(0x01),	// invalid (reserved) mask
		m_layer(0),
		m_bitrate(0),
		m_frequency(0)
	{
		m_valid = isValidInternal();
		if(m_valid)
		{
			m_mpeg = calcMpegVersion();
			m_layer = calcLayer();
			m_bitrate = calcBitrate(m_mpeg, m_layer);
			m_frequency = calcFrequency(m_mpeg);
		}
	}

	bool isValid() const { return m_valid; }

	const CMPEGVer&	getMpegVersion()	const { return m_mpeg;		}
	unsigned int	getLayer()			const { return m_layer;		}
	unsigned int	getBitrate()		const { return m_bitrate;	}
	unsigned int	getFrequency()		const { return m_frequency;	}

	bool isProtected()		const { return !((m_header >> 8) & 0x01); }
	bool isPadded()			const { return ((m_header >> 17) & 0x01); }
	bool isPrivate()		const { return ((m_header >> 16) & 0x01); }
	bool isCopyrighted()	const { return ((m_header >> 27) & 0x01); }
	bool isOriginal()		const { return ((m_header >> 26) & 0x01); }

	EMPHASIS		getEmphasis()		const;
	CHANNEL_MODE	getChannelMode()	const { return (CHANNEL_MODE)((m_header >> 30) & 0x03); }

	unsigned int getFrameSize()		const;
	float		 getFrameLength()	const;

	unsigned int getNextFrame() const
	{
		ASSERT(!isCopyrighted());
		return (/*sizeof(m_header) + */getFrameSize() + isCopyrighted() * sizeof(short)/* + getSideInfoSize()*/);
	}

	// version, layer, sampling rate, channel mode, emphasis (________ ___xxxx_ ____xx__ xx____xx)
	bool operator==(const CMPEGHeader& f_header) const { return ((m_header & 0xC30C1E00) == (f_header.m_header & 0xC30C1E00)); }
	bool operator!=(const CMPEGHeader& f_header) const { return !(f_header == *this); }

	// XING / VBRI header is in the 1-st frame after side information block (layer 3 only)

private:
	CMPEGHeader();

	bool isValidInternal() const;

	CMPEGVer calcMpegVersion() const { return CMPEGVer((m_header >> 11) & 0x3); }

	unsigned int calcLayer()												const { ASSERT((m_header >> 9) & 0x03); return (4 - ((m_header >> 9) & 0x03)); }
	unsigned int calcBitrate(const CMPEGVer& f_ver, unsigned int f_layer)	const;
	unsigned int calcFrequency(const CMPEGVer& f_ver)						const;

	unsigned int getSideInfoSize() const;

private:
	unsigned int m_header;

	bool m_valid;

	CMPEGVer m_mpeg;
	unsigned int m_layer;
	unsigned int m_bitrate;
	unsigned int m_frequency;
};

#endif	// _MPEG_HEADER_H_
