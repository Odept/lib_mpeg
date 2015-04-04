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
uint	CMPEGStream::getFirstDataFrameOffset()	const { return m_offset;	}
uint	CMPEGStream::getFrameCount()			const { return m_frames/*(uint)m_frames.size()*/; }
float	CMPEGStream::getLength()				const { return m_length;	}
uint	CMPEGStream::getBitrate()				const { return m_abr;		}

CMPEGStream::~CMPEGStream() {}

	//const void*		getFramePtr(uint f_num) const { return ((f_num < getFrameCount()) ? (m_data + m_frames[f_num].offset) : NULL); }
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
	float CMPEGStream::getFrameTime(uint f_num) const
	{
		return ((f_num < getFrameCount()) ? m_frames[f_num].length : 0.0f);
	}
	*/
//	uint getFrameOffset(uint f_frame) const { return ((f_frame < getFrameCount()) ? m_frames[f_frame].offset : m_size); }

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

		offset += pH->getNextFrame();
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
	m_size(f_size),
	m_offset(0),
	m_length(0.0f),
	m_abr(0),
	m_frames(0)
{
	std::auto_ptr<const CMPEGHeader> first( CMPEGHeader::gen(*(const uint*)f_data) );
	uint offset = 0;

	// Handle Xing-header frame
	uint frameDataOffset = first->getFrameDataOffset();
	if(frameDataOffset + first->getFrameSize() < m_size)
	{
		if(const CXingHeader* pXing = CXingHeader::gen(f_data + frameDataOffset))
		{
			offset += first->getNextFrame();
			m_offset += offset;
			delete pXing;
		}
	}

	// Parse MPEG frames
	char mem[sizeof(CMPEGHeader)] __attribute__(( aligned(sizeof(void*)) ));
	for(uint next; ; offset += next)
	{
		const CMPEGHeader* pH = CMPEGHeader::gen(*(const uint*)(f_data + offset), mem);
		if(!pH)
			break;
		ASSERT(*pH == *first);

		next = pH->getNextFrame();
		ASSERT(offset + next <= m_size);
		//m_frames.push_back( SFrameInfo(offset, next, m_length) );
		m_frames++;

		m_length += pH->getFrameLength();
		m_abr += pH->getBitrate() / 1000;
		pH->~CMPEGHeader();
	}

	// Copy all frame data
	m_data.resize(offset);
	memcpy(&m_data[0], f_data, offset);

	//if(m_frames.size())
	//	m_abr /= (uint)m_frames.size();
	m_abr /= m_frames;
}

