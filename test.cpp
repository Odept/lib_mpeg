#include <iostream>

#include "header.h"


int main(int, char**)
{
	CMPEGHeader h(0x40a0fbff);

	if(h.isValid())
		std::cout << "Version:      2" << (h.getMpegVersion().isV2() ? "" : ".5") << std::endl <<
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

	return 0;
}

