#ifndef __COMMON_H__
#define __COMMON_H__

#include <cstdlib>
#include <iostream>

#define ASSERT(X) if(!(X)) { std::cout << "Abort @ " << __FILE__ << ":" << __LINE__ << ": \"" << #X << "\"" << std::endl; std::abort(); }

#define ERROR(msg) \
	do { \
		std::cerr << "ERROR @ " << __FILE__ << ":" << __LINE__ << ": " \
				  << msg << std::endl; \
	} while(0)


#define FOUR_CC(A, B, C, D)	 \
	(( (A) & 0xFF)			|\
	 (((B) & 0xFF) <<  8)	|\
	 (((C) & 0xFF) << 16)	|\
	 (((D) & 0xFF) << 24))


typedef unsigned int	uint;
typedef unsigned short	ushort;
typedef unsigned char	uchar;

#endif // __COMMON_H__

