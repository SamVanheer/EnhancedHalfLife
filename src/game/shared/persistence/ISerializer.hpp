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

#include <cstddef>

#include <rttr/property.h>
#include <rttr/variant.h>

#include "PersistenceDefs.hpp"

namespace persistence
{
class Restore;
class Save;

struct ISerializer
{
	virtual ~ISerializer() = default;

	virtual void Serialize(const rttr::property& property, const rttr::variant& value, Save& save) const = 0;

	virtual void Deserialize(const rttr::property& property, rttr::variant& value, Restore& restore) const = 0;
};
}
