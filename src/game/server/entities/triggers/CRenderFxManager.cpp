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

#include "CRenderFxManager.hpp"

void CRenderFxManager::Spawn()
{
	SetSolidType(Solid::Not);
}

void CRenderFxManager::Use(const UseInfo& info)
{
	if (HasTarget())
	{
		CBaseEntity* pTarget = nullptr;

		while ((pTarget = UTIL_FindEntityByTargetname(pTarget, GetTarget())) != nullptr)
		{
			if (!IsBitSet(pev->spawnflags, SF_RENDER_MASKFX))
				pTarget->SetRenderFX(GetRenderFX());
			if (!IsBitSet(pev->spawnflags, SF_RENDER_MASKAMT))
				pTarget->SetRenderAmount(GetRenderAmount());
			if (!IsBitSet(pev->spawnflags, SF_RENDER_MASKMODE))
				pTarget->SetRenderMode(GetRenderMode());
			if (!IsBitSet(pev->spawnflags, SF_RENDER_MASKCOLOR))
				pTarget->SetRenderColor(GetRenderColor());
		}
	}
}
