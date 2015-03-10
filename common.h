#ifndef _MPEG_COMMON_H_
#define _MPEG_COMMON_H_

#include <cstdlib>
#include <iostream>

#define ASSERT(X) if(!(X)) { std::cout << #X << std::endl; std::abort(); }

typedef unsigned int uint;

#endif // _MPEG_COMMON_H_

