#pragma once

#include "Platform.hpp"

struct sizebuf_t
{
	const char* buffername;
	unsigned short flags;
	byte* data;
	int maxsize;
	int cursize;
};
