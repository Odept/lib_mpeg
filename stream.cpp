// check vbri consistency
// mp3 padding fix

#include "stream.h"
#include "header.h"

#include <sstream>


CStream::CStream(const uchar* f_data, size_t f_size):
	m_warnings(0)
{
	size_t offset = 0;

	// Handle Xing-header frame
	if(auto size = CXingFrame::getSize(f_data, f_size))
	{
		m_xing = std::make_unique<CXingFrame>(f_data, size);
		offset += size;
	}
	CHeader first(*reinterpret_cast<const uint*>(f_data + offset));

	offset = init(f_data, offset, f_size, true);

	// Basic XING validation
	if(m_xing)
	{
		auto& h = m_xing->getHeader();

		if(h != first)
			WARNING("XING: MPEG info differs from the rest of the stream");
		if((h.isVBR() && !m_vbr) || (!h.isVBR() && m_vbr))
			WARNING("XING: VBR status mismatch (expected " << h.isVBR() << ", actual " << m_vbr << ')');
		if(h.getFrameCount() != m_frames.size())
			WARNING("XING: frame count mismatch (expected " << h.getFrameCount() << ", actual " << m_frames.size() << ')');
		if(h.getByteCount() != offset)
			WARNING("XING: stream size mismatch (expected " << h.getByteCount() << ", actual " << offset << ')');
	}

	// Copy all frame data
	m_data.resize(offset);
	memcpy(&m_data[0], f_data, offset);
}


size_t CStream::init(const uchar* f_data, size_t f_offset, size_t f_size, bool f_bFirstInit)
{
	m_length = 0.0f;
	m_abr = 0;
	m_vbr = false;
	m_frames.clear();

	auto offset = f_offset;

	ASSERT(offset + CHeader::getSize() <= f_size);
	CHeader first(*reinterpret_cast<const uint*>(f_data + offset));

	// Parse MPEG frames
	auto firstFrameBitrate = first.getBitrate();
	uint nFreeBitrateFrames = 0;

	for(size_t next; offset != f_size/*condition for ideal pure stream*/; offset += next)
	{
		if(offset + sizeof(uint) > f_size)
		{
			ASSERT(f_bFirstInit);
			WARNING("unexpected end of MPEG stream @ relative offset " << offset << " (0x" << OUT_HEX(offset) << ')');
			break;
		}
		auto rawHeader = *reinterpret_cast<const uint*>(f_data + offset);
		if(!CHeader::isValid(rawHeader))
			break;

		CHeader h(rawHeader);
		if(h.isFreeBitrate())
		{
			next = h.calcFrameSize(f_data + offset, f_size - offset);
			if(!next)
			{
				ASSERT(f_bFirstInit);
				WARNING("failed to calculate a size of a free-bitrate frame @ relative offset " << offset << " (0x" << OUT_HEX(offset) << ')');
				break;
			}
			++nFreeBitrateFrames;
		}
		else
		{
			if(first.isFreeBitrate())
			{
				first = CHeader(rawHeader);
				firstFrameBitrate = first.getBitrate();
			}
			// Check for non-free-bitrate frames only
			ASSERT_MSG(h == first, "(frame #" + std::to_string(m_frames.size()) + ')');
			next = h.getFrameSize();
		}

		if(offset + next > f_size)
		{
			ASSERT(f_bFirstInit);
			WARNING("unexpected end of MPEG frame @ relative offset " << offset << " (0x" << OUT_HEX(offset) << ')');
			break;
		}
		m_frames.push_back( FrameInfo(offset, next, m_length, h.getFrameDataOffset()) );

		m_length += h.getFrameLength();
		if(!h.isFreeBitrate())
		{
			auto bitrate = h.getBitrate();
			m_abr += bitrate / 1000;
			if(!m_vbr && bitrate != firstFrameBitrate)
				m_vbr = true;
		}
	}
	ASSERT(m_frames.size() != nFreeBitrateFrames);
	m_abr /= (m_frames.size() - nFreeBitrateFrames);

	// The assert is not really needed, because the assert above is actually the same
	ASSERT(!first.isFreeBitrate());
	// Get values here, where the first non-free-bitrate frame is guaranteed to be found
	m_version		= first.getVersion();
	m_layer			= first.getLayer();
	m_sampling_rate	= first.getSamplingRate();
	m_channel_mode	= first.getChannelMode();
	m_emphasis		= first.getEmphasis();
	//m_bCRC			= first.isProtected();
	//m_copyrighted	= first.isCopyrighted();
	//m_original		= first.isOriginal();

	if(nFreeBitrateFrames && f_bFirstInit)
		WARNING(nFreeBitrateFrames << " free-bitrate frame" << ((nFreeBitrateFrames > 1) ? "s" : "") << " found");

	return offset;
}


unsigned CStream::cut(unsigned f_frame, unsigned f_count)
{
	auto nFramesPrev = m_frames.size();
	if(f_frame >= nFramesPrev)
	{
		std::ostringstream oss;
		oss << "the start frame #" << f_frame << " is greater than the total number of frames (" << nFramesPrev << ") in the stream";
		throw std::out_of_range(oss.str());
	}

	auto count = f_count;
	if(f_frame + count > nFramesPrev)
		count = nFramesPrev - f_frame;
	if(!count)
		return 0;

		auto it = m_frames.cbegin();
	auto offsetFirst = it->Offset;
		it += f_frame;
	auto offsetBegin = it->Offset;
		it += count - 1;
	auto offsetEnd = it->Offset + it->Size;

	m_data.erase(m_data.cbegin() + offsetBegin, m_data.cbegin() + offsetEnd);

	init(&m_data[0], offsetFirst, m_data.size(), false);
	if(m_xing)
	{
		ASSERT(!"Not implemented: cut with XING");
	}
	else
		ASSERT(offsetFirst == 0);

	return nFramesPrev - m_frames.size();
}


unsigned CStream::truncate(unsigned f_frames)
{
	ASSERT(!m_xing);
	if(!f_frames)
		return 0;

	auto n = m_frames.size();
	auto nFramesNew = (f_frames <= n) ? (n - f_frames) : 0;

	m_data.resize( getFrameOffset(nFramesNew) );
	// n - number of deleted frames
	n -= nFramesNew;
	for(auto i = n; i; --i)
		m_frames.pop_back();

	return n;
}


void CStream::serialize(std::vector<unsigned char>& f_outStream)
{
	ASSERT(!"Not implemented"); (void)f_outStream;
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

