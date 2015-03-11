// check xing / vbri consistency
// mp3 padding fix

#include "stream.h"
#include "header.h"

/******************************************************************************
 * Static Section
 *****************************************************************************/
CMPEGStream* CMPEGStream::get(const unsigned char* f_data, uint f_size)
{
	CMPEGStream* p = new(std::nothrow) CMPEGStream(f_data, f_size);
	ASSERT(p);
	while(p)
	{
		if(!p->parse())
			break;
		return p;
	}
	delete p;
	return NULL;
}

/******************************************************************************
 * Public Section
 *****************************************************************************/
uint	CMPEGStream::getFrameCount()		const { return m_frames/*(uint)m_frames.size()*/; }
float	CMPEGStream::getLength()			const { return				m_length; }
uint	CMPEGStream::getBitrate()			const { return				   m_abr; }
uint	CMPEGStream::getFirstHeaderOffset()	const { return m_first_header_offset; }

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
 * Private Section
 *****************************************************************************/
CMPEGStream::CMPEGStream(const unsigned char* f_data, uint f_size):
	m_data(f_data),
	m_size(f_size),
	m_first_header_offset(0),
	m_length(0.0f),
	m_abr(0),
	m_frames(0)
{
	ASSERT(m_size < (1 << (sizeof(uint) * 8 - 1)));
}


bool CMPEGStream::parse()
{
	m_length = 0.0f;
	m_abr = 0;

	//m_frames.clear();
	m_frames = 0;

	m_first_header_offset = calcFirstHeaderOffset();
	if(m_first_header_offset >= m_size)
		return false;

	const CMPEGHeader first( *(const uint*)(m_data + m_first_header_offset) );


	for(uint offset = m_first_header_offset, next; offset < m_size; offset += next)
	{
		const CMPEGHeader h( *(const uint*)(m_data + offset) );
		if(!h.isValid() || h != first)
			break;

		next = h.getNextFrame();
		//m_frames.push_back( SFrameInfo(offset, next, m_length) );
		m_frames++;

		m_length += h.getFrameLength();
		m_abr += h.getBitrate() / 1000;
	}

	//if(m_frames.size())
	//	m_abr /= (uint)m_frames.size();
	m_abr /= m_frames;

	return true;
}


uint CMPEGStream::calcFirstHeaderOffset() const
{
	static const uint HeaderSequenceLimit = 3;

	for(uint offsetFirst = 0;;)
	{
		offsetFirst += findHeader(m_data + offsetFirst, m_size - offsetFirst);

		for(uint i = 0, offset = offsetFirst;
			(i < HeaderSequenceLimit) && (offset < m_size);
			i++)
		{
			const CMPEGHeader h( *(const uint*)(m_data + offset) );

			if(!h.isValid())
			{
				offsetFirst++;
				break;
			}

			if(i == HeaderSequenceLimit - 1)
				return offsetFirst;

			offset += h.getNextFrame();
		}
	}

	return m_size;
}


uint CMPEGStream::findHeader(const unsigned char* f_data, uint f_size) const
{
	uint i;
	uint limit = f_size - HeaderSize;

	for(i = 0; i < limit; i++)
	{
		CMPEGHeader header( *(const uint*)(f_data + i) );
		if(header.isValid())
			break;
	}

	return ((i == limit) ? m_size : i);
}
