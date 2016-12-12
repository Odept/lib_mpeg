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
		static const uint HeaderSequenceLimit = 2;

		for(size_t n = HeaderSequenceLimit, offset = 0;
			(offset + CHeader::getSize()) <= f_size;
			--n)
		{
			auto rawHeader = *reinterpret_cast<const uint*>(f_data + offset);
			if(!CHeader::isValid(rawHeader))
				break;

			CHeader h(rawHeader);
			offset += h.getFrameSize();

			if(!n)
				return true;
		}

		return false;
	}


	bool IStream::isIncompleteFrame(const unsigned char* f_data, size_t f_size)
	{
		// Don't consider a block as a frame if its size is less than a frame size
		if(f_size < CHeader::getSize())
			return false;

		CHeader header( *reinterpret_cast<const uint*>(f_data) );
		return (header.getFrameSize() > f_size);
	}


	const std::string& IStream::str(MPEG::Version f_version)	{ return CHeader::str(f_version);	}
	const std::string& IStream::str(MPEG::ChannelMode f_mode)	{ return CHeader::str(f_mode);		}
	const std::string& IStream::str(MPEG::Emphasis f_emphasis)	{ return CHeader::str(f_emphasis);	}


	IStream::~IStream() {}
}

