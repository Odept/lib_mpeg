#include "mpeg.h"

#include "stream.h"
#include "header.h"


namespace MPEG
{
	std::shared_ptr<IStream> IStream::create(const unsigned char* f_data, size_t f_size)
	{
		return std::make_shared<CStream>(f_data, f_size);
	}


	static size_t findHeader(const unsigned char* f_data, size_t f_size)
	{
		size_t i;
		auto limit = f_size - CHeader::getSize();

		for(i = 0; i < limit; i++)
		{
			if(CHeader::isValid( *reinterpret_cast<const uint*>(f_data + i)) )
				break;
		}

		return ((i == limit) ? f_size : i);
	}

	size_t IStream::calcFirstHeaderOffset(const uchar* f_data, size_t f_size)
	{
		for(size_t offset = 0;; offset++)
		{
			offset += findHeader(f_data + offset, f_size - offset);
			if( verifyFrameSequence(f_data + offset, f_size - offset) )
				return offset;
		}
		return f_size;
	}


	bool IStream::verifyFrameSequence(const unsigned char* f_data, size_t f_size)
	{
		static const uint HeadersToVerify = 3;
		for(size_t nFrames = HeadersToVerify, offset = 0; nFrames; --nFrames)
		{
			if(offset + CHeader::getSize() > f_size)
				return false;

			auto rawHeader = *reinterpret_cast<const uint*>(f_data + offset);
			if(!CHeader::isValid(rawHeader))
				return false;

			CHeader h(rawHeader);
			if(h.isFreeBitrate())
			{
				// Try handle a free-bitrate frame
				auto size = h.calcFrameSize(f_data + offset, f_size - offset);
				if(!size)
					return false;
				offset += size;
			}
			else
				offset += h.getFrameSize();
		}

		return true;
	}


	bool IStream::isIncompleteFrame(const unsigned char* f_data, size_t f_size)
	{
		// Don't consider a block as a frame if its size is less than a frame size
		if(f_size < CHeader::getSize())
			return false;

		auto rawHeader = *reinterpret_cast<const uint*>(f_data);
		if(!CHeader::isValid(rawHeader))
			return false;

		CHeader header(rawHeader);
		// Don't support free-bitrate frames here for now
		return !header.isFreeBitrate() && (header.getFrameSize() > f_size);
	}


	const std::string& IStream::str(MPEG::Version f_version)	{ return CHeader::str(f_version);	}
	const std::string& IStream::str(MPEG::ChannelMode f_mode)	{ return CHeader::str(f_mode);		}
	const std::string& IStream::str(MPEG::Emphasis f_emphasis)	{ return CHeader::str(f_emphasis);	}


	IStream::~IStream() {}
}

