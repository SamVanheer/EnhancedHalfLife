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

#include <cctype>
#include <cstdint>

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

char* Q_UnicodeAdvance(char* pUTF8, int nChars)
{
	char32_t uVal;
	bool bError;

	for (;nChars > 0 && *pUTF8; --nChars)
	{
		pUTF8 += Q_UTF8ToUChar32(pUTF8, uVal, bError);
	}

	return pUTF8;
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

static bool s_com_token_unget = false;
bool com_ignorecolons = false;

void COM_UngetToken()
{
	s_com_token_unget = true;
}

//TODO: refactor this to return string_view
char* COM_Parse(char* data)
{
	if (s_com_token_unget)
	{
		s_com_token_unget = false;
		return data;
	}

	com_token[0] = '\0';

	if (!data)
	{
		return nullptr;
	}

	{
		char32_t wchar;
		bool processAnotherLine = false;

		do
		{
			processAnotherLine = false;

			// skip whitespace
			while (!V_UTF8ToUChar32(data, wchar) && wchar <= ' ')
			{
				if (wchar == '\0')
				{
					return nullptr; // end of file;
				}

				data = Q_UnicodeAdvance(data, 1);
			}

			// skip // comments
			if (data[0] == '/' && data[1] == '/')
			{
				data += 2;

				while (data[0] != '\n' && data[0] != '\0')
				{
					++data;
				}

				processAnotherLine = true;
			}
		}
		while (processAnotherLine);
	}

	// handle quoted strings specially
	if (data[0] == '\"')
	{
		++data;

		std::size_t byteCount = 0;

		while (true)
		{
			const char c = *data++;

			if (c == '\"' || c == '\0' || byteCount == (sizeof(com_token) - 1))
			{
				com_token[byteCount] = '\0';
				return data;
			}

			com_token[byteCount++] = c;
		}
	}

	// parse single characters
	{
		const char c = data[0];

		if (c == '}'
			|| c == '{'
			|| c == ')'
			|| c == '('
			|| c == '\''
			|| c == ','
			|| (c == ':' && !com_ignorecolons))
		{
			com_token[0] = c;
			com_token[1] = '\0';
			return data + 1;
		}
	}

	// parse a regular word
	{
		char c = data[0];

		std::size_t byteCount = 0;

		do
		{
			com_token[byteCount++] = c;
			++data;
			c = data[0];

			if (c == '}'
				|| c == '{'
				|| c == ')'
				|| c == '('
				|| c == '\''
				|| c == ','
				|| (c == ':' && !com_ignorecolons))
			{
				break;
			}
		}
		while (byteCount < (sizeof(com_token) - 1) && c > ' ');

		com_token[byteCount] = '\0';
	}

	return data;
}

bool COM_TokenWaiting(char* buffer)
{
	char* p;

	p = buffer;
	while (*p && *p != '\n')
	{
		if (!isspace(*p) || isalnum(*p))
			return true;

		p++;
	}

	return false;
}

Vector UTIL_StringToVector(const char* pString)
{
	const char* pstr = pString;

	Vector result = vec3_origin;

	for (int j = 0; j < 3; ++j)
	{
		result[j] = atof(pstr);

		while (*pstr && *pstr != ' ')
		{
			++pstr;
		}

		if (!*pstr)
		{
			break;
		}

		++pstr;
	}

	return result;
}
