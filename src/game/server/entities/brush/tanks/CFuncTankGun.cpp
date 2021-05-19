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

#include "CFuncTankGun.hpp"

LINK_ENTITY_TO_CLASS(func_tank, CFuncTankGun);

void CFuncTankGun::Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker)
{
	if (m_fireLast != 0)
	{
		// FireBullets needs gpGlobals->v_up, etc.
		UTIL_MakeAimVectors(GetAbsAngles());

		const int bulletCount = (gpGlobals->time - m_fireLast) * m_fireRate;
		if (bulletCount > 0)
		{
			for (int i = 0; i < bulletCount; i++)
			{
				switch (m_bulletType)
				{
				case TankBullet::Cal9mm:
					FireBullets(1, barrelEnd, forward, gTankSpread[m_spread], WORLD_BOUNDARY, BULLET_MONSTER_9MM, 1, m_iBulletDamage, pAttacker);
					break;

				case TankBullet::MP5:
					FireBullets(1, barrelEnd, forward, gTankSpread[m_spread], WORLD_BOUNDARY, BULLET_MONSTER_MP5, 1, m_iBulletDamage, pAttacker);
					break;

				case TankBullet::Cal12mm:
					FireBullets(1, barrelEnd, forward, gTankSpread[m_spread], WORLD_BOUNDARY, BULLET_MONSTER_12MM, 1, m_iBulletDamage, pAttacker);
					break;

				default:
				case TankBullet::None:
					break;
				}
			}
			CFuncTank::Fire(barrelEnd, forward, pAttacker);
		}
	}
	else
		CFuncTank::Fire(barrelEnd, forward, pAttacker);
}
