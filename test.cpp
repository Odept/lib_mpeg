#include "common.h"

#include "header.h"
#include "stream.h"

#include <stdio.h>


void test_header(uint f_val)
{
	try
	{
		const CMPEGHeader h(f_val);

		MPEGVersion ver = h.getVersion();

		std::cout << "Header:        0x"	<< std::hex << f_val << std::dec << std::endl <<
					 "Version:       "		<< ((ver == MPEGv1) ? "1" : "2") << ((ver == MPEGv25) ? ".5" : "") << std::endl <<
					 "Layer:         "		<< h.getLayer() << std::endl <<
					 "Bitrate:       "		<< h.getBitrate() << std::endl <<
					 "Sampling Rate: "		<< h.getSamplingRate() << std::endl <<
					 "Protected:     "		<< h.isProtected() << std::endl <<
					 "Padded:        "		<< h.isPadded() << std::endl <<
					 "Private:       "		<< h.isPrivate() << std::endl <<
					 "Copyrighted:   "		<< h.isCopyrighted() << std::endl <<
					 "Original:      "		<< h.isOriginal() << std::endl <<

					 "Emphasis:     *"		<< h.getEmphasis() << std::endl <<
					 "Channel:      *"		<< h.getChannelMode() << std::endl <<

					 "Frame size:    "		<< h.getFrameSize() << std::endl <<
					 "Frame length:  "		<< h.getFrameLength() << std::endl;
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
			ERROR("Failed to allocate " << fsize << " bytes");
			break;
		}

		if(fread(pBuf, fsize, 1, f) != 1)
		{
			ERROR("Failed to read " << fsize << " bytes");
			break;
		}

		uint offset = CMPEGStream::calcFirstHeaderOffset(pBuf, fsize);
		if(offset >= fsize)
		{
			ERROR("Failed to init MPEG stream");
			break;
		}

		pMPEG = CMPEGStream::gen(pBuf + offset, fsize - offset);

		uint uDataOffset = pMPEG->getFrameOffset(0);
		std::cout << f_path << std::endl << "================" << std::endl <<
					 "Offset : " << (offset + uDataOffset) << (uDataOffset ? "*" : "") << std::endl <<
					 "Frames : " << pMPEG->getFrameCount() << std::endl <<
					 "Length : " << pMPEG->getLength() << std::endl <<
					 "MPEG " << pMPEG->getVersion() << " Layer " << pMPEG->getLayer() << std::endl <<
					 "Bitrate      : " << pMPEG->getBitrate() << " kbps" << (pMPEG->isVBR() ? " (VBR)" : "") << std::endl <<
					 "Sampling Rate: " << pMPEG->getSamplingRate() << " Hz" << std::endl <<
					 "Channel Mode : " << pMPEG->getChannelMode() << std::endl <<
					 "Emphasis     : " << pMPEG->getEmphasis() << std::endl;
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
	test_header(0x00A2FBFF);

	return 0;
}

