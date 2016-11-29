#pragma once

#include <stdexcept>
#include <iostream>

#define ASSERT(X) if(!(X)) { std::cerr << "Abort @ " << __FILE__ << ":" << __LINE__ << ": \"" << #X << "\"" << std::endl; \
		throw std::logic_error(#X); \
	}

#define ERROR(msg) \
	do { \
		std::cerr << "ERROR @ " << __FILE__ << ":" << __LINE__ << ": " \
				  << msg << std::endl; \
	} while(0)
#define WARNING(msg) \
	do { \
		std::cerr << "WARNING @ " << __FILE__ << ":" << __LINE__ << ": " \
				  << msg << std::endl; \
	} while(0)


#define FOUR_CC(A, B, C, D)	 \
	(( (A) & 0xFF)			|\
	 (((B) & 0xFF) <<  8)	|\
	 (((C) & 0xFF) << 16)	|\
	 (((D) & 0xFF) << 24))


using uint		= unsigned int;
using ushort	= unsigned short;
using uchar		= unsigned char;

