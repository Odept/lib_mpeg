#pragma once


#include <iostream>


#define __STR_INTERNAL(x)	#x
#define __STR(x)			__STR_INTERNAL(x)
#define STR__LINE__			__STR(__LINE__)
#define ASSERT(X)			do { if(!(X)) throw std::logic_error(#X " @ " __FILE__ ":" STR__LINE__); } while(0)
#define ASSERT_MSG(X, MSG)  do { if(!(X)) throw std::logic_error(#X " @ " __FILE__ ":" STR__LINE__ + std::string(" ") + std::string(MSG)); } while(0)

#define OUT_HEX(num)		std::hex << (num) << std::dec
#define WARNING(msg)		do { std::cerr << "WARNING @ " << __FILE__ << ":" << __LINE__ << ": " << msg << std::endl; ++m_warnings; } while(0)

#define FOUR_CC(A, B, C, D)	 \
	(( (A) & 0xFF)			|\
	 (((B) & 0xFF) <<  8)	|\
	 (((C) & 0xFF) << 16)	|\
	 (((D) & 0xFF) << 24))


using uint		= unsigned int;
using ushort	= unsigned short;
using uchar		= unsigned char;

