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

#include <bitset>
#include <cstddef>
#include <string_view>

class CTokenizer final
{
private:
	static constexpr int Flag_HasToken = 0;
	static constexpr int Flag_IgnoreColons = 1;

	static constexpr int FlagCount = Flag_IgnoreColons + 1;

public:
	explicit constexpr CTokenizer() = default;

	explicit constexpr CTokenizer(std::string_view data)
		: _data(data)
	{
	}

	constexpr ~CTokenizer() = default;

	bool HasToken() const { return _flags.test(Flag_HasToken); }

	bool ShouldIgnoreColons() const { return _flags.test(Flag_IgnoreColons); }

	void SetIgnoreColons(bool value)
	{
		_flags.set(Flag_IgnoreColons, value);
	}

	constexpr std::string_view GetData() const { return _data; }

	constexpr std::size_t GetOffset() const { return _offset; }

	constexpr std::string_view GetToken() const { return _token; }

	bool Next();

	bool TokenWaiting() const;

private:
	void SetHasToken(bool value)
	{
		_flags.set(Flag_HasToken, value);
	}

	constexpr bool IsSingleCharacter(char c) const
	{
		return c == '}'
			|| c == '{'
			|| c == ')'
			|| c == '('
			|| c == '\''
			|| c == ','
			|| (c == ':' && !ShouldIgnoreColons());
	}

	bool SkipWhitespace();
	bool TrySkipCommentLine();

	bool TryParseQuotedString();
	bool TryParseSingleCharacter();
	void ParseWord();

private:
	std::string_view _data;
	std::size_t _offset{};
	std::string_view _token;

	std::bitset<FlagCount> _flags;
};
