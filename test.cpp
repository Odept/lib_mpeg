#include "common.h"

#include "header.h"
#include "mpeg.h"

#include <vector>


#define LOG(msg)	std::cout << msg << std::endl
#define ERROR(msg)	do { std::cerr << "ERROR @ " << __FILE__ << ":" << __LINE__ << ": " << msg << std::endl; } while(0)


void test_header(uint f_val)
{
	try
	{
		const CHeader h(f_val);

		LOG("Header:        0x"	<< std::hex << f_val << std::dec);
		LOG("Version:       "	<< MPEG::IStream::str(h.getVersion()));
		LOG("Layer:         "	<< h.getLayer());
		LOG("Bitrate:       "	<< h.getBitrate());
		LOG("Sampling Rate: "	<< h.getSamplingRate());
		LOG("Protected:     "	<< h.isProtected());
		LOG("Padded:        "	<< h.isPadded());
		LOG("Private:       "	<< h.isPrivate());
		LOG("Copyrighted:   "	<< h.isCopyrighted());
		LOG("Original:      "	<< h.isOriginal());

		LOG("Emphasis:      "	<< MPEG::IStream::str(h.getEmphasis()));
		LOG("Channel:       "	<< MPEG::IStream::str(h.getChannelMode()));

		LOG("Frame size:    "	<< (h.isFreeBitrate() ? 0 : h.getFrameSize()));
		LOG("Frame length:  "	<< h.getFrameLength());
	}
	catch(const std::invalid_argument& e)
	{
		ERROR(e.what());
	}
}


void test_file(const char* f_path)
{
	FILE* f;

	f = fopen(f_path, "rb");
	if(!f)
	{
		ERROR("Failed to open \"" << f_path << "\"");
		return;
	}

	do
	{
		fseek(f, 0, SEEK_END);
		size_t fsize = ftell(f);
		rewind(f);

		std::vector<uchar> buf(fsize);

		if(fread(&buf[0], buf.size(), 1, f) != 1)
		{
			ERROR("Failed to read " << fsize << " bytes");
			break;
		}

		auto offset = MPEG::IStream::calcFirstHeaderOffset(&buf[0], buf.size());
		if(offset >= fsize)
		{
			ERROR("Failed to init MPEG stream");
			break;
		}

		auto mpeg = MPEG::IStream::create(&buf[offset], fsize - offset);

		auto uDataOffset = mpeg->getFrameOffset(0);
		LOG(f_path << std::endl << "================");
		LOG("Offset : " << (offset + uDataOffset) << (uDataOffset ? "*" : ""));
		LOG("Frames : " << mpeg->getFrameCount());
		LOG("Length : " << mpeg->getLength());
		LOG("MPEG " << MPEG::IStream::str(mpeg->getVersion()) << " Layer " << mpeg->getLayer());
		LOG("Bitrate      : " << mpeg->getBitrate() << " kbps" << (mpeg->isVBR() ? " (VBR)" : ""));
		LOG("Sampling Rate: " << mpeg->getSamplingRate() << " Hz");
		LOG("Channel Mode : " << MPEG::IStream::str(mpeg->getChannelMode()));
		LOG("Emphasis     : " << MPEG::IStream::str(mpeg->getEmphasis()));
	}
	while(0);

	fclose(f);
}

int main(int, char**)
{
	//test_header(0x00A2FBFF);
	test_header(0x1B00FBFF);
	//LOG("===");
	//test_header(0x430FFBFF);
	//LOG("===");
	//test_header(0x44C0FBFF);
	LOG("================");
	test_file("test.mp3");

	return 0;
}

