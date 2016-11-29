// check vbri consistency
// mp3 padding fix

#include "stream.h"

#include "header.h"


CStream::CStream(const uchar* f_data, uint f_size):
	m_length(0.0f),
	m_abr(0),
	m_vbr(false)
{
	uint offset = 0;

	CHeader first(*(const uint*)f_data);
	m_version		= first.getVersion();
	m_layer			= first.getLayer();
	m_sampling_rate	= first.getSamplingRate();
	m_channel_mode	= first.getChannelMode();
	m_emphasis		= first.getEmphasis();

	// Handle Xing-header frame
	uint frameDataOffset = first.getFrameDataOffset();
	if(frameDataOffset + first.getFrameSize() < f_size)
	{
		const uchar* pData = f_data + frameDataOffset;
		if(CXingHeader::isValid(pData))
		{
			offset += first.getFrameSize();
			ASSERT(!"XING-header");
		}
	}

	// Parse MPEG frames
	uint br = first.getBitrate();

	for(uint next; ; offset += next)
	{
		auto rawHeader = *(const uint*)(f_data + offset);
		if(!CHeader::isValid(rawHeader))
			break;

		CHeader h(rawHeader);
		ASSERT(h == first);

		next = h.getFrameSize();
		if(offset + next > f_size)
		{
			WARNING("Unexpected end of MPEG frame @ relative offset 0x" << std::hex << offset << std::dec);
			break;
		}
		m_frames.push_back( FrameInfo(offset, next, m_length) );

		uint bitrate = h.getBitrate();
		m_length += h.getFrameLength();
		m_abr += bitrate / 1000;
		if(!m_vbr && br != bitrate)
			m_vbr = true;
	}

	// Copy all frame data
	m_data.resize(offset);
	memcpy(&m_data[0], f_data, offset);

	m_abr /= m_frames.size();
}


uint CStream::truncate(uint f_frames)
{
	if(!f_frames)
		return 0;

	uint n = getFrameCount();
	uint frames = (f_frames <= n) ? (n - f_frames) : 0;

	m_data.resize( getFrameOffset(frames) );

	frames = n - frames;
	for(uint i = frames; i; i--)
		m_frames.pop_back();

	return frames;
}


/*const CHeader* CStream::getFrameHeader() const
{
	if(f_index >= m_frames.size())
		return NULL;
	m_frames[f_index].Time : 0.0f;
	return CHeader::gen( *(const uint*)(m_data + getFrameOffset(f_index)) );
}
*/

/*uint CStream::getFrameNumber(float f_time) const
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

