// check vbri consistency
// mp3 padding fix

#include "stream.h"

#include "common.h"
#include "header.h"

#include <memory> // auto_ptr
#include <cstring> // memcpy

/******************************************************************************
 * Public Section
 *****************************************************************************/
uint	CMPEGStream::getSize()					const { return m_data.size();			}
uint	CMPEGStream::getFirstDataFrameOffset()	const { return m_offset;				}
uint	CMPEGStream::getFrameCount()			const { return (uint)m_frames.size();	}
float	CMPEGStream::getLength()				const { return m_length;				}

const char*	CMPEGStream::getVersion()		const { return CMPEGHeader::strVer(m_version);				}
uint		CMPEGStream::getLayer()			const { return m_layer;										}
uint		CMPEGStream::getBitrate()		const { return m_abr;										}
bool		CMPEGStream::isVBR()			const { return m_vbr;										}
uint		CMPEGStream::getSamplingRate()	const { return m_sampling_rate;								}
const char*	CMPEGStream::getChannelMode()	const { return CMPEGHeader::strChannelMode(m_channel_mode);	}
const char*	CMPEGStream::getEmphasis()		const { return CMPEGHeader::strEmphasis(m_emphasis);		}

uint CMPEGStream::getFrameOffset(uint f_index) const
{
	return (f_index < m_frames.size()) ? m_frames[f_index].Offset : (uint)m_data.size();
}
uint CMPEGStream::getFrameSize(uint f_index) const
{
	return (f_index < m_frames.size()) ? m_frames[f_index].Size : 0;
}
float CMPEGStream::getFrameTime(uint f_index) const
{
	return (f_index < m_frames.size()) ? m_frames[f_index].Time : 0.0f;
}

/*const CMPEGHeader* CMPEGStream::getFrameHeader() const
{
	if(f_index >= m_frames.size())
		return NULL;
	m_frames[f_index].Time : 0.0f;
	return CMPEGHeader::gen( *(const uint*)(m_data + getFrameOffset(f_index)) );
}
*/

/*uint CMPEGStream::getFrameNumber(float f_time) const
{
	uint i, n = getFrameCount();

	for(i = 0; i < n; i++)
	{
		if(m_frames[i].length >= f_time)
			break;
	}

	return i;
}
*/

CMPEGStream::~CMPEGStream() {}

/******************************************************************************
 * Static Section
 *****************************************************************************/
CMPEGStream* CMPEGStream::gen(const uchar* f_data, uint f_size)
{
	if( !verifyFrameSequence(f_data, f_size) )
		return NULL;
	return new CMPEGStream(f_data, f_size);
}


uint CMPEGStream::calcFirstHeaderOffset(const uchar* f_data, uint f_size)
{
	for(uint offset = 0;; offset++)
	{
		offset += findHeader(f_data + offset, f_size - offset);
		if( verifyFrameSequence(f_data + offset, f_size - offset) )
			return offset;
	}
	return f_size;
}

bool CMPEGStream::verifyFrameSequence(const uchar* f_data, uint f_size)
{
	static const uint HeaderSequenceLimit = 2;

	for(uint n = HeaderSequenceLimit, offset = 0;
		(offset + CMPEGHeader::getSize()) <= f_size;
		n--)
	{
		static char mem[sizeof(CMPEGHeader)] __attribute__(( aligned(sizeof(void*)) ));
		const CMPEGHeader* pH = CMPEGHeader::gen(*(const uint*)(f_data + offset), mem);
		if(!pH)
			break;

		offset += pH->getFrameSize();
		pH->~CMPEGHeader();

		if(!n)
			return true;
	}

	return false;
}

// ====================================
uint CMPEGStream::findHeader(const uchar* f_data, uint f_size)
{
	uint i;
	uint limit = f_size - CMPEGHeader::getSize();

	for(i = 0; i < limit; i++)
	{
		if(const CMPEGHeader* pH = CMPEGHeader::gen( *(const uint*)(f_data + i) ))
		{
			delete pH;
			break;
		}
	}

	return ((i == limit) ? f_size : i);
}

/******************************************************************************
 * Private Section
 *****************************************************************************/
CMPEGStream::CMPEGStream(const uchar* f_data, uint f_size):
	m_offset(0),
	m_length(0.0f),
	m_abr(0),
	m_vbr(false)
{
	std::auto_ptr<const CMPEGHeader> first( CMPEGHeader::gen(*(const uint*)f_data) );
	m_version		= first->getVersion();
	m_layer			= first->getLayer();
	m_sampling_rate	= first->getSamplingRate();
	m_channel_mode	= first->getChannelMode();
	m_emphasis		= first->getEmphasis();

	// Handle Xing-header frame
	uint frameDataOffset = first->getFrameDataOffset();
	if(frameDataOffset + first->getFrameSize() < f_size)
	{
		if(const CXingHeader* pXing = CXingHeader::gen(f_data + frameDataOffset))
		{
			m_offset += first->getFrameSize();
			delete pXing;
		}
	}

	// Parse MPEG frames
	uint offset = m_offset;
	uint br = first->getBitrate();

	char mem[sizeof(CMPEGHeader)] __attribute__(( aligned(sizeof(void*)) ));
	for(uint next; ; offset += next)
	{
		const CMPEGHeader* pH = CMPEGHeader::gen(*(const uint*)(f_data + offset), mem);
		if(!pH)
			break;
		ASSERT(*pH == *first);

		next = pH->getFrameSize();
		ASSERT(offset + next <= f_size);
		m_frames.push_back( FrameInfo(offset, next, m_length) );

		uint bitrate = pH->getBitrate();
		m_length += pH->getFrameLength();
		m_abr += bitrate / 1000;
		if(!m_vbr && br != bitrate)
			m_vbr = true;
		pH->~CMPEGHeader();
	}

	// Copy all frame data
	m_data.resize(offset);
	memcpy(&m_data[0], f_data, offset);

	m_abr /= m_frames.size();
}

