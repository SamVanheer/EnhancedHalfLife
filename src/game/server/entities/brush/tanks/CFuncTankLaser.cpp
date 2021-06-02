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

#include "CFuncTankLaser.hpp"
#include "effects/CLaser.hpp"

void CFuncTankLaser::Activate()
{
	if (!GetLaser())
	{
		UTIL_Remove(this);
		ALERT(at_error, "Laser tank with no env_laser!\n");
	}
	else
	{
		m_hLaser->TurnOff();
	}
}

void CFuncTankLaser::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "laserentity"))
	{
		pev->message = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CFuncTank::KeyValue(pkvd);
}

CLaser* CFuncTankLaser::GetLaser()
{
	if (auto laser = m_hLaser.Get(); laser)
		return laser;

	CBaseEntity* pLaser = nullptr;

	while ((pLaser = UTIL_FindEntityByTargetname(pLaser, STRING(pev->message))) != nullptr)
	{
		if (pLaser->ClassnameIs("env_laser"))
		{
			m_hLaser = (CLaser*)pLaser;
			break;
		}
	}

	return static_cast<CLaser*>(pLaser);
}

void CFuncTankLaser::Think()
{
	if (auto laser = m_hLaser.Get(); laser && (gpGlobals->time > m_laserTime))
		laser->TurnOff();

	CFuncTank::Think();
}

void CFuncTankLaser::Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker)
{
	if (m_fireLast != 0 && GetLaser())
	{
		// TankTrace needs gpGlobals->v_up, etc.
		UTIL_MakeAimVectors(GetAbsAngles());

		const int bulletCount = (gpGlobals->time - m_fireLast) * m_fireRate;
		if (bulletCount)
		{
			CLaser* laser = m_hLaser;

			TraceResult tr;
			for (int i = 0; i < bulletCount; i++)
			{
				laser->SetAbsOrigin(barrelEnd);
				TankTrace(barrelEnd, forward, gTankSpread[m_spread], tr);

				m_laserTime = gpGlobals->time;
				laser->TurnOn();
				laser->pev->dmgtime = gpGlobals->time - 1.0;
				laser->FireAtPoint(tr);
				laser->pev->nextthink = 0;
			}
			CFuncTank::Fire(barrelEnd, forward, pAttacker);
		}
	}
	else
	{
		CFuncTank::Fire(barrelEnd, forward, pAttacker);
	}
}
