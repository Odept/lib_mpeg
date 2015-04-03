#include "common.h"

#include "header.h"
#include "stream.h"

#include <stdio.h>


void test_header(uint f_val)
{
	const CMPEGHeader* pH = CMPEGHeader::gen(f_val);
	if(!pH)
	{
		std::cout << "Header is not valid" << std::endl;
		return;
	}

	MPEGVersion ver = pH->getMpegVersion();

	std::cout << "Header:        0x"	<< std::hex << f_val << std::dec << std::endl <<
				 "Version:       "		<< ((ver == MPEGv1) ? "1" : "2") << ((ver == MPEGv25) ? ".5" : "") << std::endl <<
				 "Layer:         "		<< pH->getLayer() << std::endl <<
				 "Bitrate:       "		<< pH->getBitrate() << std::endl <<
				 "Sampling Rate: "		<< pH->getSamplingRate() << std::endl <<
				 "Protected:     "		<< pH->isProtected() << std::endl <<
				 "Padded:        "		<< pH->isPadded() << std::endl <<
				 "Private:       "		<< pH->isPrivate() << std::endl <<
				 "Copyrighted:   "		<< pH->isCopyrighted() << std::endl <<
				 "Original:      "		<< pH->isOriginal() << std::endl <<

				 "Emphasis:     *"		<< pH->getEmphasis() << std::endl <<
				 "Channel:      *"		<< pH->getChannelMode() << std::endl <<

				 "Frame size:    "		<< pH->getFrameSize() << std::endl <<
				 "Frame length:  "		<< pH->getFrameLength() << std::endl;
	delete pH;
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

		uint offset = CMPEGStream::calcFirstHeaderOffset(pBuf, fsize);
		if(offset >= fsize)
		{
			std::cout << "Failed to init MPEG stream" << std::endl;
			break;
		}

		pMPEG = CMPEGStream::gen(pBuf + offset, fsize - offset);

		uint uDataOffset = pMPEG->getFirstDataFrameOffset();
		std::cout << f_path << std::endl << "================" << std::endl <<
					 "Offset : " << (offset + uDataOffset) << (uDataOffset ? "*" : "") << std::endl <<
					 "Frames : " << pMPEG->getFrameCount() << std::endl <<
					 "Length : " << pMPEG->getLength() << std::endl <<
					 "Bitrate: " << pMPEG->getBitrate() << std::endl;
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

