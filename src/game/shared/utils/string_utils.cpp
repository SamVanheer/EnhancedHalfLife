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

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <string>

#include "Platform.h"
#include "mathlib.h"
#include "string_utils.hpp"

bool Q_IsValidUChar32(char32_t uVal)
{
	// Values > 0x10FFFF are explicitly invalid; ditto for UTF-16 surrogate halves,
	// values ending in FFFE or FFFF, or values in the 0x00FDD0-0x00FDEF reserved range
	return (uVal < 0x110000u) && ((uVal - 0x00D800u) > 0x7FFu) && ((uVal & 0xFFFFu) < 0xFFFEu) && ((uVal - 0x00FDD0u) > 0x1Fu);
}

//TODO: refactor this so that it doesn't use goto
int Q_UTF8ToUChar32(const char* pUTF8_, char32_t& uValueOut, bool& bErrorOut)
{
	const std::uint8_t* pUTF8 = (const std::uint8_t*)pUTF8_;

	int nBytes = 1;
	char32_t uValue = pUTF8[0];
	char32_t uMinValue = 0;

	// 0....... single byte
	if (uValue < 0x80)
		goto decodeFinishedNoCheck;

	// Expecting at least a two-byte sequence with 0xC0 <= first <= 0xF7 (110...... and 11110...)
	if ((uValue - 0xC0u) > 0x37u || (pUTF8[1] & 0xC0) != 0x80)
		goto decodeError;

	uValue = (uValue << 6) - (0xC0 << 6) + pUTF8[1] - 0x80;
	nBytes = 2;
	uMinValue = 0x80;

	// 110..... two-byte lead byte
	if (!(uValue & (0x20 << 6)))
		goto decodeFinished;

	// Expecting at least a three-byte sequence
	if ((pUTF8[2] & 0xC0) != 0x80)
		goto decodeError;

	uValue = (uValue << 6) - (0x20 << 12) + pUTF8[2] - 0x80;
	nBytes = 3;
	uMinValue = 0x800;

	// 1110.... three-byte lead byte
	if (!(uValue & (0x10 << 12)))
		goto decodeFinishedMaybeCESU8;

	// Expecting a four-byte sequence, longest permissible in UTF-8
	if ((pUTF8[3] & 0xC0) != 0x80)
		goto decodeError;

	uValue = (uValue << 6) - (0x10 << 18) + pUTF8[3] - 0x80;
	nBytes = 4;
	uMinValue = 0x10000;

	// 11110... four-byte lead byte. fall through to finished.

decodeFinished:
	if (uValue >= uMinValue && Q_IsValidUChar32(uValue))
	{
	decodeFinishedNoCheck:
		uValueOut = uValue;
		bErrorOut = false;
		return nBytes;
	}
decodeError:
	uValueOut = '?';
	bErrorOut = true;
	return nBytes;

decodeFinishedMaybeCESU8:
	// Do we have a full UTF-16 surrogate pair that's been UTF-8 encoded afterwards?
	// That is, do we have 0xD800-0xDBFF followed by 0xDC00-0xDFFF? If so, decode it all.
	if ((uValue - 0xD800u) < 0x400u && pUTF8[3] == 0xED && (std::uint8_t)(pUTF8[4] - 0xB0) < 0x10 && (pUTF8[5] & 0xC0) == 0x80)
	{
		uValue = 0x10000 + ((uValue - 0xD800u) << 10) + ((std::uint8_t)(pUTF8[4] - 0xB0) << 6) + pUTF8[5] - 0x80;
		nBytes = 6;
		uMinValue = 0x10000;
	}
	goto decodeFinished;
}

bool V_UTF8ToUChar32(const char* pUTF8_, char32_t& uValueOut)
{
	bool bError;

	Q_UTF8ToUChar32(pUTF8_, uValueOut, bError);
	return bError;
}

std::size_t Q_UnicodeAdvance(const char* pUTF8, int nChars)
{
	char32_t uVal;
	bool bError;

	auto next = pUTF8;

	for (; nChars > 0 && *pUTF8; --nChars)
	{
		next += Q_UTF8ToUChar32(next, uVal, bError);
	}

	return next - pUTF8;
}

bool Q_UnicodeValidate(const char* pUTF8)
{
	bool bError = false;
	while (*pUTF8)
	{
		char32_t uVal;
		// Our UTF-8 decoder silently fixes up 6-byte CESU-8 (improperly re-encoded UTF-16) sequences.
		// However, these are technically not valid UTF-8. So if we eat 6 bytes at once, it's an error.
		int nCharSize = Q_UTF8ToUChar32(pUTF8, uVal, bError);
		if (bError || nCharSize == 6)
			return false;
		pUTF8 += nCharSize;
	}
	return true;
}

Vector UTIL_StringToVector(std::string_view str)
{
	Vector result = vec3_origin;

	std::size_t index = 0;

	for (int j = 0; j < 3; ++j)
	{
		result[j] = UTIL_StringToFloat(str.substr(index));

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

float UTIL_StringToFloat(std::string_view str, float defaultValue)
{
	//TODO GCC doesn't support from_chars with floats yet
#ifdef WIN32
	float result = defaultValue;
	std::from_chars(str.data(), str.data() + str.length(), result);
	return result;
#else
	char* endPos = nullptr;
	float result = 0;
	bool isValid = false;
	
	//Try to use a stack buffer if possible, fall back to allocating only in case of large strings
	const std::size_t MaxLocalBufferSize = 256;
	
	if (str.length() < MaxLocalBufferSize)
	{
		char buffer[MaxLocalBufferSize];
		
		safe_strcpy(buffer, str);
		
		result = std::strtod(buffer, &endPos);
		
		isValid = &buffer[0] != endPos;
	}
	else
	{
		//This allocates memory but it consistently returns matching results to from-chars
		const std::string fullString{str};
		result = std::strtod(fullString.c_str(), &endPos);
		isValid = fullString.c_str() != endPos;
	}

	if (isValid)
	{
		return result;
	}

	return defaultValue;
#endif
}

int UTIL_StringToInt(std::string_view str, int defaultValue)
{
	int result = defaultValue;
	std::from_chars(str.data(), str.data() + str.length(), result);
	return result;
}

std::string_view COM_FileBase(std::string_view in)
{
	// scan backward for '.'
	std::size_t end = in.find_last_of("./\\");

	if (end == std::string_view::npos)
	{
		// no '.', copy to end
		end = in.length() - 1;
	}
	else
	{
		// Found '.', copy to left of '.'
		--end;
	}

	// Scan backward for '/'
	std::size_t start = in.find_last_of("/\\");

	if (start == std::string_view::npos)
	{
		start = 0;
	}
	else
	{
		++start;
	}

	// Length of new sting
	const size_t len = end - start + 1;

	return in.substr(start, len);
}

bool UTIL_IEquals(std::string_view lhs, std::string_view rhs)
{
	auto result = std::mismatch(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](char l, char r)
		{
			return toupper(l) == toupper(r);
		});

	return result.first == lhs.end() && result.second == rhs.end();
}
