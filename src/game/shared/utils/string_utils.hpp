/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

#include "vector.h"

/**
*	@brief Copies src to dest and always null terminates the result
*	@param dst Buffer to copy the string to
*	@param src String to copy
*	@param len_dst Size of the destination buffer, in bytes
*	@return If the destination buffer length is not 0, returns dst. Otherwise returns nullptr
*/
inline char* safe_strcpy(char* dst, const char* src, std::size_t len_dst)
{
	if (len_dst <= 0)
	{
		return nullptr; // this is bad
	}

	strncpy(dst, src, len_dst - 1);
	dst[len_dst - 1] = '\0';

	return dst;
}

template<std::size_t Size>
inline char* safe_strcpy(char(&dst)[Size], const char* src)
{
	return safe_strcpy(dst, src, Size);
}

inline char* safe_strcpy(char* dst, std::string_view src, std::size_t len_dst)
{
	return safe_strcpy(dst, src.data(), std::min(len_dst, src.length() + 1));
}

template<std::size_t Size>
inline char* safe_strcpy(char(&dst)[Size], std::string_view src)
{
	return safe_strcpy(dst, src, Size);
}

/**
*	@brief Appends src to dest and always null terminates the result
*	@param dst Buffer to append the string to
*	@param src String to copy
*	@param len_dst Size of the destination buffer, in bytes
*	@return If the destination buffer length is not 0 and if there is room to append some or all of src, returns dst. Otherwise returns nullptr
*/
inline char* safe_strcat(char* dst, const char* src, std::size_t len_dst)
{
	if (len_dst <= 0)
	{
		return nullptr; // this is bad
	}

	const std::size_t srcLength = strlen(src);

	//Nothing to add
	if (srcLength == 0)
	{
		return dst;
	}

	const std::size_t dstLengthSoFar = strlen(dst);

	const std::size_t spaceLeft = len_dst - dstLengthSoFar;

	//Include the null terminator in src so everything is measuring the full thing to add
	const std::size_t amountToCopy = std::min(spaceLeft, srcLength + 1);

	strncat(dst, src, amountToCopy - 1);
	dst[dstLengthSoFar + amountToCopy] = '\0';

	return dst;
}

template<std::size_t Size>
inline char* safe_strcat(char(&dst)[Size], const char* src)
{
	return safe_strcat(dst, src, Size);
}

/**
*	@brief determine if a uchar32 represents a valid Unicode code point
*/
bool Q_IsValidUChar32(char32_t uVal);

/**
*	@brief // Decode one character from a UTF-8 encoded string.
*
*	Treats 6-byte CESU-8 sequences as a single character, as if they were a correctly-encoded 4-byte UTF-8 sequence.
*/
std::size_t Q_UTF8ToUChar32(std::string_view str, char32_t& uValueOut, bool& bErrorOut);

bool V_UTF8ToUChar32(std::string_view str, char32_t& uValueOut);

std::size_t Q_UnicodeAdvance(std::string_view str, int nChars);

/**
*	@brief Returns true if UTF-8 string contains invalid sequences.
*/
bool Q_UnicodeValidate(std::string_view str);

Vector UTIL_StringToVector(std::string_view str);

float UTIL_StringToFloat(std::string_view str, float defaultValue = 0);

int UTIL_StringToInt(std::string_view str, int defaultValue = 0);

template<std::size_t Size>
std::array<int, Size> UTIL_StringToIntArray(std::string_view str)
{
	std::array<int, Size> result{{}};

	std::size_t index = 0;

	for (std::size_t j = 0; j < Size; ++j)
	{
		result[j] = UTIL_StringToInt(str.substr(index));

		while (index < str.length() && str[index] != ' ')
		{
			++index;
		}

		if (index >= str.length())
		{
			break;
		}

		++index;
	}

	return result;
}

/**
*	@brief Extracts the base name of a file (no path, no extension, assumes '/' as path separator)
*/
std::string_view COM_FileBase(std::string_view in);

bool UTIL_IEquals(std::string_view lhs, std::string_view rhs);
