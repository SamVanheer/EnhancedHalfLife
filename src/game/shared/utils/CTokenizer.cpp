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

#include "Platform.h"
#include "mathlib.h"
#include "CTokenizer.hpp"
#include "string_utils.hpp"

bool CTokenizer::Next()
{
	_token = {};
	SetHasToken(false);

	if (_offset >= _data.length())
	{
		return false;
	}

	{
		bool processAnotherLine = false;

		do
		{
			processAnotherLine = false;

			if (!SkipWhitespace())
			{
				return false;
			}

			processAnotherLine = TrySkipCommentLine();
		}
		while (processAnotherLine);
	}

	if (TryParseQuotedString())
	{
		return true;
	}

	if (TryParseSingleCharacter())
	{
		return true;
	}

	ParseWord();

	return true;
}

bool CTokenizer::TokenWaiting() const
{
	auto offset = _offset;

	while (offset < _data.length() && _data[offset] != '\n')
	{
		if (!isspace(_data[offset]) || isalnum(_data[offset]))
		{
			return true;
		}

		++offset;
	}

	return false;
}

bool CTokenizer::SkipWhitespace()
{
	char32_t wchar;

	while (true)
	{
		if (_offset >= _data.length())
		{
			return false; // end of file;
		}

		if (V_UTF8ToUChar32(_data.substr(_offset), wchar) || wchar > ' ')
		{
			break;
		}

		_offset += Q_UnicodeAdvance(_data.substr(_offset), 1);
	}

	return true;
}

bool CTokenizer::TrySkipCommentLine()
{
	// skip // comments
	if ((_offset + 1) < _data.length() && _data[_offset] == '/' && _data[_offset + 1] == '/')
	{
		_offset += 2;

		while (_offset < _data.length() && _data[_offset] != '\n')
		{
			++_offset;
		}

		return true;
	}

	return false;
}

bool CTokenizer::TryParseQuotedString()
{
	// handle quoted strings specially
	if (_data[_offset] != '\"')
	{
		return false;
	}

	++_offset;

	const std::size_t startOffset = _offset;

	while (_offset < _data.length() && _data[_offset] != '\"')
	{
		++_offset;
	}

	_token = _data.substr(startOffset, _offset - startOffset);

	//If this is not the end of the buffer skip past the closing quote
	if (_offset < _data.length())
	{
		++_offset;
	}

	SetHasToken(true);

	return true;
}

bool CTokenizer::TryParseSingleCharacter()
{
	const char c = _data[_offset];

	if (IsSingleCharacter(c))
	{
		_token = _data.substr(_offset, 1);
		++_offset;
		SetHasToken(true);
		return true;
	}

	return false;
}

void CTokenizer::ParseWord()
{
	// parse a regular word
	const std::size_t startOffset = _offset;

	do
	{
		++_offset;
	}
	while (_offset < _data.length()
		&& !IsSingleCharacter(_data[_offset])
		&& _data[_offset] > ' ');

	_token = _data.substr(startOffset, _offset - startOffset);
	SetHasToken(true);
}
