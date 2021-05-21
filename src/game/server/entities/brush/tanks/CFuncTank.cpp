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

#include "CBaseWeapon.hpp"
#include "CFuncTank.hpp"

void CFuncTank::Spawn()
{
	Precache();

	SetMovetype(Movetype::Push);  // so it doesn't get pushed by anything
	SetSolidType(Solid::BSP);
	SetModel(STRING(pev->model));

	m_yawCenter = GetAbsAngles().y;
	m_pitchCenter = GetAbsAngles().x;

	if (IsActive())
		pev->nextthink = pev->ltime + 1.0;

	m_sightOrigin = BarrelPosition(); // Point at the end of the barrel

	if (m_fireRate <= 0)
		m_fireRate = 1;
	if (m_spread > MAX_FIRING_SPREADS)
		m_spread = 0;

	pev->oldorigin = GetAbsOrigin();
}

void CFuncTank::Precache()
{
	if (!IsStringNull(m_iszSpriteSmoke))
		PRECACHE_MODEL(STRING(m_iszSpriteSmoke));
	if (!IsStringNull(m_iszSpriteFlash))
		PRECACHE_MODEL(STRING(m_iszSpriteFlash));

	if (!IsStringNull(pev->noise))
		PRECACHE_SOUND(STRING(pev->noise));
}

void CFuncTank::KeyValue(KeyValueData* pkvd)
{
	if (AreStringsEqual(pkvd->szKeyName, "yawrate"))
	{
		m_yawRate = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "yawrange"))
	{
		m_yawRange = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "yawtolerance"))
	{
		m_yawTolerance = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "pitchrange"))
	{
		m_pitchRange = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "pitchrate"))
	{
		m_pitchRate = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "pitchtolerance"))
	{
		m_pitchTolerance = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "firerate"))
	{
		m_fireRate = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "barrel"))
	{
		m_barrelPos.x = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "barrely"))
	{
		m_barrelPos.y = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "barrelz"))
	{
		m_barrelPos.z = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "spritescale"))
	{
		m_spriteScale = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "spritesmoke"))
	{
		m_iszSpriteSmoke = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "spriteflash"))
	{
		m_iszSpriteFlash = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "rotatesound"))
	{
		pev->noise = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "persistence"))
	{
		m_persist = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "bullet"))
	{
		m_bulletType = (TankBullet)atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "bullet_damage"))
	{
		m_iBulletDamage = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "firespread"))
	{
		m_spread = atoi(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "minRange"))
	{
		m_minRange = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "maxRange"))
	{
		m_maxRange = atof(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else if (AreStringsEqual(pkvd->szKeyName, "master"))
	{
		m_iszMaster = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = true;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

bool CFuncTank::OnControls(CBaseEntity* pTest)
{
	if (!(pev->spawnflags & SF_TANK_CANCONTROL))
		return false;

	if ((m_vecControllerUsePos - pTest->GetAbsOrigin()).Length() < 30)
		return true;

	return false;
}

bool CFuncTank::StartControl(CBasePlayer* pController)
{
	if (m_hController != nullptr)
		return false;

	// Team only or disabled?
	if (!IsStringNull(m_iszMaster))
	{
		if (!UTIL_IsMasterTriggered(m_iszMaster, pController))
			return false;
	}

	ALERT(at_console, "using TANK!\n");

	m_hController = pController;
	//TODO: don't have this code outside CBasePlayer
	if (auto activeWeapon = pController->m_hActiveWeapon.Get(); activeWeapon)
	{
		activeWeapon->Holster();
		pController->pev->weaponmodel = iStringNull;
		pController->pev->viewmodel = iStringNull;

	}

	pController->m_iHideHUD |= HIDEHUD_WEAPONS;
	m_vecControllerUsePos = pController->GetAbsOrigin();

	pev->nextthink = pev->ltime + 0.1;

	return true;
}

void CFuncTank::StopControl()
{
	CBasePlayer* controller = m_hController;
	if (!controller)
		return;

	//TODO: don't have this code outside CBasePlayer
	if (auto activeWeapon = controller->m_hActiveWeapon.Get(); activeWeapon)
		activeWeapon->Deploy();

	ALERT(at_console, "stopped using TANK\n");

	controller->m_iHideHUD &= ~HIDEHUD_WEAPONS;

	pev->nextthink = 0;
	m_hController = nullptr;

	if (IsActive())
		pev->nextthink = pev->ltime + 1.0;
}

void CFuncTank::ControllerPostFrame()
{
	ASSERT(m_hController != nullptr);

	if (gpGlobals->time < m_flNextAttack)
		return;

	if (CBasePlayer* controller = m_hController; controller->pev->button & IN_ATTACK)
	{
		Vector vecForward;
		AngleVectors(GetAbsAngles(), &vecForward, nullptr, nullptr);

		m_fireLast = gpGlobals->time - (1 / m_fireRate) - 0.01;  // to make sure the gun doesn't fire too many bullets

		Fire(BarrelPosition(), vecForward, controller);

		// HACKHACK -- make some noise (that the AI can hear)
		if (controller->IsPlayer())
			controller->m_iWeaponVolume = LOUD_GUN_VOLUME;

		m_flNextAttack = gpGlobals->time + (1 / m_fireRate);
	}
}

void CFuncTank::Use(const UseInfo& info)
{
	if (pev->spawnflags & SF_TANK_CANCONTROL)
	{  // player controlled turret

		if (!info.GetActivator()->IsPlayer())
			return;

		if (info.GetValue() == 2 && info.GetUseType() == UseType::Set)
		{
			ControllerPostFrame();
		}
		else if (!m_hController && info.GetUseType() != UseType::Off)
		{
			((CBasePlayer*)info.GetActivator())->m_pTank = this;
			StartControl((CBasePlayer*)info.GetActivator());
		}
		else
		{
			StopControl();
		}
	}
	else
	{
		if (!ShouldToggle(info.GetUseType(), IsActive()))
			return;

		if (IsActive())
			TankDeactivate();
		else
			TankActivate();
	}
}

CBaseEntity* CFuncTank::FindTarget(CBaseEntity* pPlayer)
{
	return pPlayer;
}

bool CFuncTank::InRange(float range)
{
	if (range < m_minRange)
		return false;
	if (m_maxRange > 0 && range > m_maxRange)
		return false;

	return true;
}

void CFuncTank::Think()
{
	pev->avelocity = vec3_origin;
	TrackTarget();

	if (fabs(pev->avelocity.x) > 1 || fabs(pev->avelocity.y) > 1)
		StartRotSound();
	else
		StopRotSound();
}

void CFuncTank::TrackTarget()
{
	//TODO: clean this up so that it's easier to follow for the two main cases
	TraceResult tr;
	bool updateTime = false;
	Vector angles, direction, barrelEnd;
	CBaseEntity* pTarget;

	// Get a position to aim for
	if (auto controller = m_hController.Get(); controller)
	{
		// Tanks attempt to mirror the player's angles
		angles = controller->pev->v_angle;
		angles[0] = 0 - angles[0];
		pev->nextthink = pev->ltime + 0.05;
	}
	else
	{
		if (IsActive())
			pev->nextthink = pev->ltime + 0.1;
		else
			return;

		CBaseEntity* pPlayer = UTIL_FindClientInPVS(this);
		if (IsNullEnt(pPlayer))
		{
			if (IsActive())
				pev->nextthink = pev->ltime + 2;	// Wait 2 secs
			return;
		}
		pTarget = FindTarget(pPlayer);
		if (!pTarget)
			return;

		// Calculate angle needed to aim at target
		barrelEnd = BarrelPosition();
		const Vector targetPosition = pTarget->GetAbsOrigin() + pTarget->pev->view_ofs;
		const float range = (targetPosition - barrelEnd).Length();

		if (!InRange(range))
			return;

		UTIL_TraceLine(barrelEnd, targetPosition, IgnoreMonsters::No, this, &tr);

		bool lineOfSight = false;
		// No line of sight, don't track
		if (tr.flFraction == 1.0 || InstanceOrNull(tr.pHit) == pTarget)
		{
			lineOfSight = true;

			if (InRange(range) && pTarget->IsAlive())
			{
				updateTime = true;
				m_sightOrigin = UpdateTargetPosition(pTarget);
			}
		}

		// Track sight origin

// !!! I'm not sure what i changed
		direction = m_sightOrigin - GetAbsOrigin();
		//		direction = m_sightOrigin - barrelEnd;
		angles = VectorAngles(direction);

		// Calculate the additional rotation to point the end of the barrel at the target (not the gun's center) 
		AdjustAnglesForBarrel(angles, direction.Length());
	}

	angles.x = -angles.x;

	// Force the angles to be relative to the center position
	angles.y = m_yawCenter + UTIL_AngleDistance(angles.y, m_yawCenter);
	angles.x = m_pitchCenter + UTIL_AngleDistance(angles.x, m_pitchCenter);

	// Limit against range in y
	if (angles.y > m_yawCenter + m_yawRange)
	{
		angles.y = m_yawCenter + m_yawRange;
		updateTime = false;	// Don't update if you saw the player, but out of range
	}
	else if (angles.y < (m_yawCenter - m_yawRange))
	{
		angles.y = (m_yawCenter - m_yawRange);
		updateTime = false; // Don't update if you saw the player, but out of range
	}

	if (updateTime)
		m_lastSightTime = gpGlobals->time;

	// Move toward target at rate or less
	const float distY = UTIL_AngleDistance(angles.y, GetAbsAngles().y);
	pev->avelocity.y = distY * 10;
	if (pev->avelocity.y > m_yawRate)
		pev->avelocity.y = m_yawRate;
	else if (pev->avelocity.y < -m_yawRate)
		pev->avelocity.y = -m_yawRate;

	// Limit against range in x
	if (angles.x > m_pitchCenter + m_pitchRange)
		angles.x = m_pitchCenter + m_pitchRange;
	else if (angles.x < m_pitchCenter - m_pitchRange)
		angles.x = m_pitchCenter - m_pitchRange;

	// Move toward target at rate or less
	const float distX = UTIL_AngleDistance(angles.x, GetAbsAngles().x);
	pev->avelocity.x = distX * 10;

	if (pev->avelocity.x > m_pitchRate)
		pev->avelocity.x = m_pitchRate;
	else if (pev->avelocity.x < -m_pitchRate)
		pev->avelocity.x = -m_pitchRate;

	if (m_hController)
		return;

	if (CanFire() && ((fabs(distX) < m_pitchTolerance && fabs(distY) < m_yawTolerance) || (pev->spawnflags & SF_TANK_LINEOFSIGHT)))
	{
		bool fire = false;
		Vector forward;
		AngleVectors(GetAbsAngles(), &forward, nullptr, nullptr);

		if (pev->spawnflags & SF_TANK_LINEOFSIGHT)
		{
			float length = direction.Length();
			UTIL_TraceLine(barrelEnd, barrelEnd + forward * length, IgnoreMonsters::No, this, &tr);
			if (InstanceOrNull(tr.pHit) == pTarget)
				fire = true;
		}
		else
			fire = true;

		if (fire)
		{
			Fire(BarrelPosition(), forward, this);
		}
		else
			m_fireLast = 0;
	}
	else
		m_fireLast = 0;
}

void CFuncTank::AdjustAnglesForBarrel(Vector& angles, float distance)
{
	if (m_barrelPos.y != 0 || m_barrelPos.z != 0)
	{
		distance -= m_barrelPos.z;
		const float d2 = distance * distance;
		if (m_barrelPos.y)
		{
			const float r2 = m_barrelPos.y * m_barrelPos.y;
			angles.y += (180.0 / M_PI) * atan2(m_barrelPos.y, sqrt(d2 - r2));
		}
		if (m_barrelPos.z)
		{
			const float r2 = m_barrelPos.z * m_barrelPos.z;
			angles.x += (180.0 / M_PI) * atan2(-m_barrelPos.z, sqrt(d2 - r2));
		}
	}
}

void CFuncTank::Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker)
{
	if (m_fireLast != 0)
	{
		if (!IsStringNull(m_iszSpriteSmoke))
		{
			CSprite* pSprite = CSprite::SpriteCreate(STRING(m_iszSpriteSmoke), barrelEnd, true);
			pSprite->AnimateAndDie(RANDOM_FLOAT(15.0, 20.0));
			pSprite->SetTransparency(RenderMode::TransAlpha, GetRenderColor(), 255, RenderFX::None);
			pSprite->SetAbsVelocity({0, 0, RANDOM_FLOAT(40, 80)});
			pSprite->SetScale(m_spriteScale);
		}
		if (!IsStringNull(m_iszSpriteFlash))
		{
			CSprite* pSprite = CSprite::SpriteCreate(STRING(m_iszSpriteFlash), barrelEnd, true);
			pSprite->AnimateAndDie(60);
			pSprite->SetTransparency(RenderMode::TransAdd, {255, 255, 255}, 255, RenderFX::NoDissipation);
			pSprite->SetScale(m_spriteScale);

			// Hack Hack, make it stick around for at least 100 ms.
			pSprite->pev->nextthink += 0.1;
		}
		SUB_UseTargets(this, UseType::Toggle, 0);
	}
	m_fireLast = gpGlobals->time;
}

void CFuncTank::TankTrace(const Vector& vecStart, const Vector& vecForward, const Vector& vecSpread, TraceResult& tr)
{
	// get circular gaussian spread
	float x, y, z;
	do {
		x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
		y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
		z = x * x + y * y;
	}
	while (z > 1);
	const Vector vecDir = vecForward +
		x * vecSpread.x * gpGlobals->v_right +
		y * vecSpread.y * gpGlobals->v_up;

	const Vector vecEnd = vecStart + vecDir * WORLD_BOUNDARY;
	UTIL_TraceLine(vecStart, vecEnd, IgnoreMonsters::No, this, &tr);
}

void CFuncTank::StartRotSound()
{
	if (IsStringNull(pev->noise) || (pev->spawnflags & SF_TANK_SOUNDON))
		return;
	pev->spawnflags |= SF_TANK_SOUNDON;
	EmitSound(SoundChannel::Static, STRING(pev->noise), 0.85);
}

void CFuncTank::StopRotSound()
{
	if (pev->spawnflags & SF_TANK_SOUNDON)
		StopSound(SoundChannel::Static, STRING(pev->noise));
	pev->spawnflags &= ~SF_TANK_SOUNDON;
}
