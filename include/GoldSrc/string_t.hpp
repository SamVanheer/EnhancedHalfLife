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
*	@brief Offset to a string
*/
struct string_t
{
public:
	static const string_t Null;

	constexpr string_t() = default;

	constexpr explicit string_t(unsigned int value)
		: _offset(value)
	{
	}

	constexpr string_t(const string_t&) = default;
	constexpr string_t& operator=(const string_t& other) = default;

	constexpr string_t& operator=(unsigned int value)
	{
		_offset = value;
		return *this;
	}

	constexpr unsigned int GetOffset() const { return _offset; }

	constexpr bool operator==(const string_t& other) const = default;
	constexpr bool operator!=(const string_t& other) const = default;

	constexpr bool IsValid() const { return *this != Null; }

private:
	unsigned int _offset = 0;
};

inline constexpr string_t string_t::Null{};

// Testing strings for nullity
inline bool IsStringNull(string_t iString) { return !iString.IsValid(); }
