/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#pragma once

/**
*	@file
*
*	Platform abstractions, common header includes, workarounds for compiler warnings
*/

// Allow "DEBUG" in addition to default "_DEBUG"
#ifdef _DEBUG
#define DEBUG 1
#endif

// Silence certain warnings
#pragma warning(disable : 4244)		// int or float down-conversion
#pragma warning(disable : 4305)		// int or float data truncation
#pragma warning(disable : 4201)		// nameless struct/union
#pragma warning(disable : 4514)		// unreferenced inline function removed
#pragma warning(disable : 4100)		// unreferenced formal parameter

#include "steam/steamtypes.h"

// Misc C-runtime library headers
#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using byte = unsigned char;
using word = unsigned short;
using func_t = unsigned int;
using string_t = unsigned int;
using qboolean = int;

// Prevent tons of unused windows definitions
#ifdef _WIN32

//Avoid the ISO conformant warning
#define stricmp _stricmp
#define strnicmp _strnicmp
#define itoa _itoa
#define strupr _strupr

#define DLLEXPORT __declspec( dllexport )
#else // _WIN32

#define stricmp strcasecmp
#define strnicmp strncasecmp
#define _alloca alloca

#define DLLEXPORT __attribute__ ( ( visibility( "default" ) ) )
#endif //_WIN32

template<typename T, std::size_t Size>
constexpr std::size_t ArraySize(const T(&)[Size])
{
	return Size;
}
