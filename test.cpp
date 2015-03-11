#include "common.h"

#include "header.h"
#include "stream.h"


void test_header(uint f_val)
{
	CMPEGHeader h(f_val);

	if(h.isValid())
		std::cout << "Header:       0x" << std::hex << f_val << std::dec << std::endl <<
					 "Version:      2" << (h.getMpegVersion().isV2() ? "" : ".5") << std::endl <<
					 "Layer:        " << h.getLayer() << std::endl <<
					 "Bitrate:      " << h.getBitrate() << std::endl <<
					 "Frequency:    " << h.getFrequency() << std::endl <<
					 "Protected:    " << h.isProtected() << std::endl <<
					 "Padded:       " << h.isPadded() << std::endl <<
					 "Private:      " << h.isPrivate() << std::endl <<
					 "Copyrighted:  " << h.isCopyrighted() << std::endl <<
					 "Original:     " << h.isOriginal() << std::endl <<

					 "Emphasis:     *" << h.getEmphasis() << std::endl <<
					 "Channel:      *" << h.getChannelMode() << std::endl <<

					 "Frame size:   " << h.getFrameSize() << std::endl <<
					 "Frame length: " << h.getFrameLength() << std::endl;
	else
		std::cout << "Header is not valid" << std::endl;
}


void test_file(const char* f_path)
{
	FILE* f;

	f = fopen(f_path, "rb");
	if(!f)
	{
		std::cout << "Failed to open \"" << f_path << "\"" << std::endl;
		return;
	}

	unsigned char* pBuf = NULL;
	CMPEGStream* pMPEG = NULL;
	do
	{
		fseek(f, 0, SEEK_END);
		uint fsize = ftell(f);
		rewind(f);

		pBuf = new(std::nothrow) unsigned char[fsize];
		if(!pBuf)
		{
			std::cout << "Failed to allocate " << fsize << " bytes" << std::endl;
			break;
		}

		if(fread(pBuf, fsize, 1, f) != 1)
		{
			std::cout << "Failed to read " << fsize << " bytes" << std::endl;
			break;
		}

		pMPEG = CMPEGStream::get(pBuf, fsize);
		if(!pMPEG)
		{
			std::cout << "Failed to init MPEG stream" << std::endl;
			break;
		}

		std::cout << f_path << std::endl << "================" << std::endl <<
					 "Frames:  " << pMPEG->getFrameCount() << std::endl <<
					 "Length:  " << pMPEG->getLength() << std::endl <<
					 "Bitrate: " << pMPEG->getBitrate() << std::endl <<
					 "Offset:  " << pMPEG->getFirstHeaderOffset() << std::endl;
	}
	while(0);

	delete(pMPEG);
	delete(pBuf);
	fclose(f);
}

int main(int, char**)
{
	test_file("test.mp3");
	std::cout << "================" << std::endl;
	test_header(0x44e0fbff);

	return 0;
}

