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

#include <string_view>

#include <rttr/string_view.h>

namespace detail
{
constexpr std::string_view ClassPrefix{"class"};
constexpr std::string_view StructPrefix{"struct"};
constexpr std::string_view EnumPrefix{"enum"};
}

constexpr std::string_view ToStringView(rttr::string_view view)
{
	return {view.data(), view.length()};
}

constexpr std::string_view RemoveTypePrefix(std::string_view view)
{
	if (view.starts_with(detail::ClassPrefix))
	{
		view.remove_prefix(detail::ClassPrefix.length());
	}
	else if (view.starts_with(detail::StructPrefix))
	{
		view.remove_prefix(detail::StructPrefix.length());
	}
	else if (view.starts_with(detail::EnumPrefix))
	{
		view.remove_prefix(detail::EnumPrefix.length());
	}

	return view;
}
