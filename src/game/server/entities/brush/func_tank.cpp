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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "effects.h"
#include "weapons.h"
#include "explode.h"

#include "player.h"

constexpr int SF_TANK_ACTIVE = 0x0001;
constexpr int SF_TANK_PLAYER = 0x0002;
constexpr int SF_TANK_HUMANS = 0x0004;
constexpr int SF_TANK_ALIENS = 0x0008;
constexpr int SF_TANK_LINEOFSIGHT = 0x0010;
constexpr int SF_TANK_CANCONTROL = 0x0020;
constexpr int SF_TANK_SOUNDON = 0x8000;

enum class TankBullet
{
	None = 0,
	Cal9mm = 1,
	MP5 = 2,
	Cal12mm = 3,
};

/**
*	@details Custom damage
*	env_laser (duration is 0.5 rate of fire)
*	rockets
*	explosion?
*/
class CFuncTank : public CBaseEntity
{
public:
	void	Spawn() override;
	void	Precache() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void	Use(const UseInfo& info) override;
	void	Think() override;
	void	TrackTarget();

	/**
	*	@brief Fire targets and spawn sprites
	*/
	virtual void Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker);
	virtual Vector UpdateTargetPosition(CBaseEntity* pTarget)
	{
		return pTarget->BodyTarget(pev->origin);
	}

	void	StartRotSound();
	void	StopRotSound();

	/**
	*	@brief Bmodels don't go across transitions
	*/
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	inline bool IsActive() { return (pev->spawnflags & SF_TANK_ACTIVE) != 0; }

	inline void TankActivate()
	{
		pev->spawnflags |= SF_TANK_ACTIVE;
		pev->nextthink = pev->ltime + 0.1;
		m_fireLast = 0;
	}

	inline void TankDeactivate()
	{
		pev->spawnflags &= ~SF_TANK_ACTIVE;
		m_fireLast = 0;
		StopRotSound();
	}

	inline bool CanFire() { return (gpGlobals->time - m_lastSightTime) < m_persist; }
	bool		InRange(float range);

	/**
	*	@brief Acquire a target.  pPlayer is a player in the PVS
	*/
	CBaseEntity* FindTarget(CBaseEntity* pPlayer);

	void		TankTrace(const Vector& vecStart, const Vector& vecForward, const Vector& vecSpread, TraceResult& tr);

	Vector		BarrelPosition()
	{
		Vector forward, right, up;
		AngleVectors(pev->angles, forward, right, up);
		return pev->origin + (forward * m_barrelPos.x) + (right * m_barrelPos.y) + (up * m_barrelPos.z);
	}

	/**
	*	@brief If barrel is offset, add in additional rotation
	*/
	void		AdjustAnglesForBarrel(Vector& angles, float distance);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	/**
	*	@brief TANK CONTROLLING
	*/
	bool OnControls(CBaseEntity* pTest) override;
	bool StartControl(CBasePlayer* pController);
	void StopControl();

	/**
	*	@brief Called each frame by the player's ItemPostFrame
	*/
	void ControllerPostFrame();


protected:
	EHandle<CBasePlayer> m_hController;
	float		m_flNextAttack;
	Vector		m_vecControllerUsePos;

	float		m_yawCenter;	// "Center" yaw
	float		m_yawRate;		// Max turn rate to track targets
	float		m_yawRange;		// Range of turning motion (one-sided: 30 is +/- 30 degress from center)
								// Zero is full rotation
	float		m_yawTolerance;	// Tolerance angle

	float		m_pitchCenter;	// "Center" pitch
	float		m_pitchRate;	// Max turn rate on pitch
	float		m_pitchRange;	// Range of pitch motion as above
	float		m_pitchTolerance;	// Tolerance angle

	float		m_fireLast;		// Last time I fired
	float		m_fireRate;		// How many rounds/second
	float		m_lastSightTime;// Last time I saw target
	float		m_persist;		// Persistence of firing (how long do I shoot when I can't see)
	float		m_minRange;		// Minimum range to aim/track
	float		m_maxRange;		// Max range to aim/track

	Vector		m_barrelPos;	// Length of the freakin barrel
	float		m_spriteScale;	// Scale of any sprites we shoot
	string_t	m_iszSpriteSmoke;
	string_t	m_iszSpriteFlash;
	TankBullet	m_bulletType;	// Bullet type
	int			m_iBulletDamage; // 0 means use Bullet type's default damage

	Vector		m_sightOrigin;	// Last sight of target
	int			m_spread;		// firing spread
	string_t	m_iszMaster;	// Master entity (game_team_master or multisource)
};

TYPEDESCRIPTION	CFuncTank::m_SaveData[] =
{
	DEFINE_FIELD(CFuncTank, m_yawCenter, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_yawRate, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_yawRange, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_yawTolerance, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_pitchCenter, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_pitchRate, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_pitchRange, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_pitchTolerance, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_fireLast, FIELD_TIME),
	DEFINE_FIELD(CFuncTank, m_fireRate, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_lastSightTime, FIELD_TIME),
	DEFINE_FIELD(CFuncTank, m_persist, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_minRange, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_maxRange, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_barrelPos, FIELD_VECTOR),
	DEFINE_FIELD(CFuncTank, m_spriteScale, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_iszSpriteSmoke, FIELD_STRING),
	DEFINE_FIELD(CFuncTank, m_iszSpriteFlash, FIELD_STRING),
	DEFINE_FIELD(CFuncTank, m_bulletType, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTank, m_sightOrigin, FIELD_VECTOR),
	DEFINE_FIELD(CFuncTank, m_spread, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTank, m_hController, FIELD_EHANDLE),
	DEFINE_FIELD(CFuncTank, m_vecControllerUsePos, FIELD_VECTOR),
	DEFINE_FIELD(CFuncTank, m_flNextAttack, FIELD_TIME),
	DEFINE_FIELD(CFuncTank, m_iBulletDamage, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTank, m_iszMaster, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CFuncTank, CBaseEntity);

static constexpr Vector gTankSpread[] =
{
	Vector(0, 0, 0),		// perfect
	Vector(0.025, 0.025, 0.025),	// small cone
	Vector(0.05, 0.05, 0.05),  // medium cone
	Vector(0.1, 0.1, 0.1),	// large cone
	Vector(0.25, 0.25, 0.25),	// extra-large cone
};

constexpr int MAX_FIRING_SPREADS = ArraySize(gTankSpread);

void CFuncTank::Spawn()
{
	Precache();

	pev->movetype = Movetype::Push;  // so it doesn't get pushed by anything
	pev->solid = Solid::BSP;
	SetModel(STRING(pev->model));

	m_yawCenter = pev->angles.y;
	m_pitchCenter = pev->angles.x;

	if (IsActive())
		pev->nextthink = pev->ltime + 1.0;

	m_sightOrigin = BarrelPosition(); // Point at the end of the barrel

	if (m_fireRate <= 0)
		m_fireRate = 1;
	if (m_spread > MAX_FIRING_SPREADS)
		m_spread = 0;

	pev->oldorigin = pev->origin;
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

	if ((m_vecControllerUsePos - pTest->pev->origin).Length() < 30)
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
	if (auto activeItem = pController->m_hActiveItem.Get(); activeItem)
	{
		activeItem->Holster();
		pController->pev->weaponmodel = iStringNull;
		pController->pev->viewmodel = iStringNull;

	}

	pController->m_iHideHUD |= HIDEHUD_WEAPONS;
	m_vecControllerUsePos = pController->pev->origin;

	pev->nextthink = pev->ltime + 0.1;

	return true;
}

void CFuncTank::StopControl()
{
	CBasePlayer* controller = m_hController;
	if (!controller)
		return;

	//TODO: don't have this code outside CBasePlayer
	if (auto activeItem = controller->m_hActiveItem.Get(); activeItem)
		activeItem->Deploy();

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
		AngleVectors(pev->angles, &vecForward, nullptr, nullptr);

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
		const Vector targetPosition = pTarget->pev->origin + pTarget->pev->view_ofs;
		const float range = (targetPosition - barrelEnd).Length();

		if (!InRange(range))
			return;

		UTIL_TraceLine(barrelEnd, targetPosition, IgnoreMonsters::No, edict(), &tr);

		bool lineOfSight = false;
		// No line of sight, don't track
		if (tr.flFraction == 1.0 || tr.pHit == pTarget->edict())
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
		direction = m_sightOrigin - pev->origin;
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
	const float distY = UTIL_AngleDistance(angles.y, pev->angles.y);
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
	const float distX = UTIL_AngleDistance(angles.x, pev->angles.x);
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
		AngleVectors(pev->angles, &forward, nullptr, nullptr);

		if (pev->spawnflags & SF_TANK_LINEOFSIGHT)
		{
			float length = direction.Length();
			UTIL_TraceLine(barrelEnd, barrelEnd + forward * length, IgnoreMonsters::No, edict(), &tr);
			if (tr.pHit == pTarget->edict())
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
			pSprite->SetTransparency(RenderMode::TransAlpha, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, 255, RenderFX::None);
			pSprite->pev->velocity.z = RANDOM_FLOAT(40, 80);
			pSprite->SetScale(m_spriteScale);
		}
		if (!IsStringNull(m_iszSpriteFlash))
		{
			CSprite* pSprite = CSprite::SpriteCreate(STRING(m_iszSpriteFlash), barrelEnd, true);
			pSprite->AnimateAndDie(60);
			pSprite->SetTransparency(RenderMode::TransAdd, 255, 255, 255, 255, RenderFX::NoDissipation);
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
	UTIL_TraceLine(vecStart, vecEnd, IgnoreMonsters::No, edict(), &tr);
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

class CFuncTankGun : public CFuncTank
{
public:
	void Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker) override;
};

LINK_ENTITY_TO_CLASS(func_tank, CFuncTankGun);

void CFuncTankGun::Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker)
{
	if (m_fireLast != 0)
	{
		// FireBullets needs gpGlobals->v_up, etc.
		UTIL_MakeAimVectors(pev->angles);

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

class CFuncTankLaser : public CFuncTank
{
public:
	void	Activate() override;
	void	KeyValue(KeyValueData* pkvd) override;
	void	Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker) override;
	void	Think() override;
	CLaser* GetLaser();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

private:
	EHandle<CLaser> m_hLaser;
	float	m_laserTime;
};

LINK_ENTITY_TO_CLASS(func_tanklaser, CFuncTankLaser);

TYPEDESCRIPTION	CFuncTankLaser::m_SaveData[] =
{
	DEFINE_FIELD(CFuncTankLaser, m_hLaser, FIELD_EHANDLE),
	DEFINE_FIELD(CFuncTankLaser, m_laserTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CFuncTankLaser, CFuncTank);

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
		UTIL_MakeAimVectors(pev->angles);

		const int bulletCount = (gpGlobals->time - m_fireLast) * m_fireRate;
		if (bulletCount)
		{
			CLaser* laser = m_hLaser;

			TraceResult tr;
			for (int i = 0; i < bulletCount; i++)
			{
				laser->pev->origin = barrelEnd;
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

class CFuncTankRocket : public CFuncTank
{
public:
	void Precache() override;
	void Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker) override;
};

LINK_ENTITY_TO_CLASS(func_tankrocket, CFuncTankRocket);

void CFuncTankRocket::Precache()
{
	UTIL_PrecacheOther("rpg_rocket");
	CFuncTank::Precache();
}

void CFuncTankRocket::Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker)
{
	if (m_fireLast != 0)
	{
		const int bulletCount = (gpGlobals->time - m_fireLast) * m_fireRate;
		if (bulletCount > 0)
		{
			for (int i = 0; i < bulletCount; i++)
			{
				CBaseEntity* pRocket = CBaseEntity::Create("rpg_rocket", barrelEnd, pev->angles, this);
			}
			CFuncTank::Fire(barrelEnd, forward, pAttacker);
		}
	}
	else
		CFuncTank::Fire(barrelEnd, forward, pAttacker);
}

class CFuncTankMortar : public CFuncTank
{
public:
	void KeyValue(KeyValueData* pkvd) override;
	void Fire(const Vector& barrelEnd, const Vector& forward, CBaseEntity* pAttacker) override;
};

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
			UTIL_MakeAimVectors(pev->angles);

			TankTrace(barrelEnd, forward, gTankSpread[m_spread], tr);

			UTIL_CreateExplosion(tr.vecEndPos, pev->angles, this, pev->impulse, true);

			CFuncTank::Fire(barrelEnd, forward, pAttacker);
		}
	}
	else
		CFuncTank::Fire(barrelEnd, forward, pAttacker);
}

/**
*	@brief FUNC TANK CONTROLS
*/
class CFuncTankControls : public CBaseEntity
{
public:
	int	ObjectCaps() override;
	void Spawn() override;
	void Use(const UseInfo& info) override;
	void Think() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	EHandle<CFuncTank> m_hTank;
};

LINK_ENTITY_TO_CLASS(func_tankcontrols, CFuncTankControls);

TYPEDESCRIPTION	CFuncTankControls::m_SaveData[] =
{
	DEFINE_FIELD(CFuncTankControls, m_hTank, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CFuncTankControls, CBaseEntity);

int	CFuncTankControls::ObjectCaps()
{
	return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_IMPULSE_USE;
}

void CFuncTankControls::Use(const UseInfo& info)
{ // pass the Use command onto the controls
	if (auto tank = m_hTank.Get(); tank)
		tank->Use(info);

	ASSERT(m_hTank != nullptr);	// if this fails,  most likely means save/restore hasn't worked properly
}

void CFuncTankControls::Think()
{
	CBaseEntity* pTarget = nullptr;

	while ((pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target))) != nullptr)
	{
		if (!strncmp(STRING(pTarget->pev->classname), "func_tank", 9))
		{
			break;
		}
	}

	if (IsNullEnt(pTarget))
	{
		ALERT(at_console, "No tank %s\n", STRING(pev->target));
		return;
	}

	m_hTank = (CFuncTank*)pTarget;
}

void CFuncTankControls::Spawn()
{
	pev->solid = Solid::Trigger;
	pev->movetype = Movetype::None;
	pev->effects |= EF_NODRAW;
	SetModel(STRING(pev->model));

	SetSize(pev->mins, pev->maxs);
	SetAbsOrigin(pev->origin);

	//TODO: maybe use Activate() instead?
	pev->nextthink = gpGlobals->time + 0.3;	// After all the func_tank's have spawned

	CBaseEntity::Spawn();
}
