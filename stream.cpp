// check vbri consistency
// mp3 padding fix

#include "stream.h"
#include "header.h"


CStream::CStream(const uchar* f_data, uint f_size):
	m_length(0.0f),
	m_abr(0),
	m_vbr(false)
{
	size_t offset = 0;

	CHeader first(*reinterpret_cast<const uint*>(f_data));
	m_version		= first.getVersion();
	m_layer			= first.getLayer();
	m_sampling_rate	= first.getSamplingRate();
	m_channel_mode	= first.getChannelMode();
	m_emphasis		= first.getEmphasis();

	// Handle Xing-header frame
	if(auto size = CXingFrame::getSize(f_data, f_size))
	{
		m_xing = std::make_unique<CXingFrame>(f_data, size);
		offset += size;
	}

	// Parse MPEG frames
	uint br = first.getBitrate();

	for(size_t next; ; offset += next)
	{
		if(offset + sizeof(uint) > f_size)
		{
			WARNING("unexpected end of MPEG stream @ relative offset " << offset << " (0x" << OUT_HEX(offset) << ')');
			break;
		}
		auto rawHeader = *reinterpret_cast<const uint*>(f_data + offset);
		if(!CHeader::isValid(rawHeader))
			break;

		CHeader h(rawHeader);
		ASSERT(h == first);

		next = h.getFrameSize();
		if(offset + next > f_size)
		{
			WARNING("unexpected end of MPEG frame @ relative offset " << offset << " (0x" << OUT_HEX(offset) << ')');
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
	// Basic validation
	if(m_xing)
	{
		auto& h = m_xing->getHeader();

		if((h.isVBR() && !m_vbr) || (!h.isVBR() && m_vbr))
			WARNING("XING: VBR status mismatch (expected " << h.isVBR() << ", actual " << m_vbr << ')');
		if(h.getFrameCount() != m_frames.size())
			WARNING("XING: frame count mismatch (expected " << h.getFrameCount() << ", actual " << m_frames.size() << ')');
		if(h.getByteCount() != offset)
			WARNING("XING: stream size mismatch (expected " << h.getByteCount() << ", actual " << offset << ')');
	}

	m_abr /= m_frames.size();
}


uint CStream::truncate(uint f_frames)
{
	ASSERT(!m_xing);
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

