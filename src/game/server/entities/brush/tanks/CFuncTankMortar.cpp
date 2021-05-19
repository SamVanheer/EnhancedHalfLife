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

#include "CFuncTankMortar.hpp"
#include "effects/CEnvExplosion.hpp"

LINK_ENTITY_TO_CLASS(func_tankmortar, CFuncTankMortar);

void CFuncTankMortar::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "iMagnitude"))
	{
		pev->impulse = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CFuncTank::KeyValue(pkvd);
}

void CFuncTankMortar::Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker)
{
	if (m_fireLast != 0)
	{
		const int bulletCount = (gpGlobals->time - m_fireLast) * m_fireRate;
		// Only create 1 explosion
		if (bulletCount > 0)
		{
			TraceResult tr;

			// TankTrace needs gpGlobals->v_up, etc.
			UTIL_MakeAimVectors(GetAbsAngles());

			TankTrace(barrelEnd, forward, gTankSpread[m_spread], tr);

			UTIL_CreateExplosion(tr.vecEndPos, GetAbsAngles(), this, pev->impulse, true);

			CFuncTank::Fire(barrelEnd, forward, pAttacker);
		}
	}
	else
		CFuncTank::Fire(barrelEnd, forward, pAttacker);
}
