#ifndef _MPEG_COMMON_H_
#define _MPEG_COMMON_H_

#include <cstdlib>
#include <iostream>

#define ASSERT(X) if(!(X)) { std::cout << #X << std::endl; std::abort(); }

#define FOUR_CC(A, B, C, D) \
	((((A) & 0xFF) << 24) |\
	 (((B) & 0xFF) << 16) |\
	 (((C) & 0xFF) <<  8) |\
	 ( (D) & 0xFF))

typedef unsigned int uint;

#endif // _MPEG_COMMON_H_

