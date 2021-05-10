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
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "effects.h"
#include "gamerules.h"

constexpr int	TRIPMINE_PRIMARY_VOLUME = 450;

#ifndef CLIENT_DLL

constexpr int SF_TRIPMINE_INSTANT_ON = 1 << 0;

class CTripmineGrenade : public CGrenade
{
	void OnRemove() override;
	void Spawn() override;
	void Precache() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	bool TakeDamage(const TakeDamageInfo& info) override;

	void EXPORT WarningThink();
	void EXPORT PowerupThink();
	void EXPORT BeamBreakThink();
	void EXPORT DelayDeathThink();
	void Killed(const KilledInfo& info) override;

	void MakeBeam();
	void KillBeam();

	float		m_flPowerUp = 0;
	Vector		m_vecDir;
	Vector		m_vecEnd;
	float		m_flBeamLength = 0;

	EHANDLE		m_hOwner;
	EHandle<CBeam> m_hBeam;
	Vector		m_posOwner;
	Vector		m_angleOwner;
	EHANDLE m_hRealOwner;// tracelines don't hit PEV->OWNER, which means a player couldn't detonate his own trip mine, so we store the owner here.
};

LINK_ENTITY_TO_CLASS(monster_tripmine, CTripmineGrenade);

TYPEDESCRIPTION	CTripmineGrenade::m_SaveData[] =
{
	DEFINE_FIELD(CTripmineGrenade, m_flPowerUp, FIELD_TIME),
	DEFINE_FIELD(CTripmineGrenade, m_vecDir, FIELD_VECTOR),
	DEFINE_FIELD(CTripmineGrenade, m_vecEnd, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CTripmineGrenade, m_flBeamLength, FIELD_FLOAT),
	DEFINE_FIELD(CTripmineGrenade, m_hOwner, FIELD_EHANDLE),
	DEFINE_FIELD(CTripmineGrenade, m_hBeam, FIELD_EHANDLE),
	DEFINE_FIELD(CTripmineGrenade, m_posOwner, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CTripmineGrenade, m_angleOwner, FIELD_VECTOR),
	DEFINE_FIELD(CTripmineGrenade, m_hRealOwner, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CTripmineGrenade, CGrenade);

void CTripmineGrenade::OnRemove()
{
	KillBeam();
	CGrenade::OnRemove();
}

void CTripmineGrenade::Spawn()
{
	Precache();
	// motor
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::Not);

	SetModel("models/v_tripmine.mdl");
	pev->frame = 0;
	pev->body = 3;
	pev->sequence = TRIPMINE_WORLD;
	ResetSequenceInfo();
	pev->framerate = 0;

	SetSize(Vector(-8, -8, -8), Vector(8, 8, 8));
	SetAbsOrigin(GetAbsOrigin());

	if (pev->spawnflags & SF_TRIPMINE_INSTANT_ON)
	{
		// power up quickly
		m_flPowerUp = gpGlobals->time + 1.0;
	}
	else
	{
		// power up in 2.5 seconds
		m_flPowerUp = gpGlobals->time + 2.5;
	}

	SetThink(&CTripmineGrenade::PowerupThink);
	pev->nextthink = gpGlobals->time + 0.2;

	SetDamageMode(DamageMode::Yes);
	pev->dmg = gSkillData.plrDmgTripmine;
	pev->health = 1; // don't let die normally

	if (auto owner = GetOwner(); owner)
	{
		// play deploy sound
		EmitSound(SoundChannel::Voice, "weapons/mine_deploy.wav");
		EmitSound(SoundChannel::Body, "weapons/mine_charge.wav", 0.2); // chargeup

		m_hRealOwner = owner;// see CTripmineGrenade for why.
	}

	UTIL_MakeAimVectors(GetAbsAngles());

	m_vecDir = gpGlobals->v_forward;
	m_vecEnd = GetAbsOrigin() + m_vecDir * 2048;
}

void CTripmineGrenade::Precache()
{
	PRECACHE_MODEL("models/v_tripmine.mdl");
	PRECACHE_SOUND("weapons/mine_deploy.wav");
	PRECACHE_SOUND("weapons/mine_activate.wav");
	PRECACHE_SOUND("weapons/mine_charge.wav");
}

void CTripmineGrenade::WarningThink()
{
	// play warning sound
	// EmitSound(SoundChannel::Voice, "buttons/Blip2.wav");

	// set to power up
	SetThink(&CTripmineGrenade::PowerupThink);
	pev->nextthink = gpGlobals->time + 1.0;
}

void CTripmineGrenade::PowerupThink()
{
	if (m_hOwner == nullptr)
	{
		// find an owner
		auto oldowner = GetOwner();
		SetOwner(nullptr);
		TraceResult tr;
		UTIL_TraceLine(GetAbsOrigin() + m_vecDir * 8, GetAbsOrigin() - m_vecDir * 32, IgnoreMonsters::No, this, &tr);
		if (tr.fStartSolid || (oldowner && InstanceOrNull(tr.pHit) == oldowner))
		{
			SetOwner(oldowner);
			m_flPowerUp += 0.1;
			pev->nextthink = gpGlobals->time + 0.1;
			return;
		}
		if (tr.flFraction < 1.0)
		{
			auto newOwner = InstanceOrNull(tr.pHit);
			SetOwner(newOwner);
			m_hOwner = newOwner;
			m_posOwner = newOwner->GetAbsOrigin();
			m_angleOwner = newOwner->GetAbsAngles();
		}
		else
		{
			StopSound(SoundChannel::Voice, "weapons/mine_deploy.wav");
			StopSound(SoundChannel::Body, "weapons/mine_charge.wav");
			SetThink(&CTripmineGrenade::SUB_Remove);
			pev->nextthink = gpGlobals->time + 0.1;
			ALERT(at_console, "WARNING:Tripmine at %.0f, %.0f, %.0f removed\n", GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
			KillBeam();
			return;
		}
	}
	else if (m_posOwner != m_hOwner->GetAbsOrigin() || m_angleOwner != m_hOwner->GetAbsAngles())
	{
		// disable
		StopSound(SoundChannel::Voice, "weapons/mine_deploy.wav");
		StopSound(SoundChannel::Body, "weapons/mine_charge.wav");
		auto pMine = static_cast<CTripmine*>(Create("weapon_tripmine", GetAbsOrigin() + m_vecDir * 24, GetAbsAngles()));
		pMine->m_RespawnMode = ItemRespawnMode::Never;

		SetThink(&CTripmineGrenade::SUB_Remove);
		KillBeam();
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}
	// ALERT( at_console, "%d %.0f %.0f %0.f\n", GetOwner(), m_pOwner->GetAbsOrigin().x, m_pOwner->GetAbsOrigin().y, m_pOwner->GetAbsOrigin().z );

	if (gpGlobals->time > m_flPowerUp)
	{
		// make solid
		SetSolidType(Solid::BBox);
		SetAbsOrigin(GetAbsOrigin());

		MakeBeam();

		// play enabled sound
		EmitSound(SoundChannel::Voice, "weapons/mine_activate.wav", 0.5, ATTN_NORM, 75);
	}
	pev->nextthink = gpGlobals->time + 0.1;
}

void CTripmineGrenade::KillBeam()
{
	m_hBeam.Remove();
}

void CTripmineGrenade::MakeBeam()
{
	// ALERT( at_console, "serverflags %f\n", gpGlobals->serverflags );
	TraceResult tr;
	UTIL_TraceLine(GetAbsOrigin(), m_vecEnd, IgnoreMonsters::No, this, &tr);

	m_flBeamLength = tr.flFraction;

	// set to follow laser spot
	SetThink(&CTripmineGrenade::BeamBreakThink);
	pev->nextthink = gpGlobals->time + 0.1;

	const Vector vecTmpEnd = GetAbsOrigin() + m_vecDir * 2048 * m_flBeamLength;

	auto beam = m_hBeam = CBeam::BeamCreate(g_pModelNameLaser, 10);
	beam->PointEntInit(vecTmpEnd, entindex());
	beam->SetColor(0, 214, 198);
	beam->SetScrollRate(255);
	beam->SetBrightness(64);
}

void CTripmineGrenade::BeamBreakThink()
{
	// HACKHACK Set simple box using this really nice global!
	gpGlobals->trace_flags = FTRACE_SIMPLEBOX;
	TraceResult tr;
	UTIL_TraceLine(GetAbsOrigin(), m_vecEnd, IgnoreMonsters::No, this, &tr);

	// ALERT( at_console, "%f : %f\n", tr.flFraction, m_flBeamLength );

	// respawn detect. 
	if (!m_hBeam)
	{
		MakeBeam();
		if (tr.pHit)
			m_hOwner = CBaseEntity::Instance(tr.pHit);	// reset owner too
	}

	bool bBlowup = false;

	if (fabs(m_flBeamLength - tr.flFraction) > 0.001)
	{
		bBlowup = true;
	}
	else
	{
		if (m_hOwner == nullptr)
			bBlowup = true;
		else if (m_posOwner != m_hOwner->GetAbsOrigin())
			bBlowup = true;
		else if (m_angleOwner != m_hOwner->GetAbsAngles())
			bBlowup = true;
	}

	if (bBlowup)
	{
		// a bit of a hack, but all CGrenade code passes pev->owner along to make sure the proper player gets credit for the kill
		// so we have to restore pev->owner from pRealOwner, because an entity's tracelines don't strike it's pev->owner which meant
		// that a player couldn't trigger his own tripmine. Now that the mine is exploding, it's safe the restore the owner so the 
		// CGrenade code knows who the explosive really belongs to.
		SetOwner(m_hRealOwner);
		pev->health = 0;
		Killed({GetOwner(), GetOwner(), GibType::Normal});
		return;
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

bool CTripmineGrenade::TakeDamage(const TakeDamageInfo& info)
{
	if (gpGlobals->time < m_flPowerUp && info.GetDamage() < pev->health)
	{
		// disable
		// Create("weapon_tripmine", GetAbsOrigin() + m_vecDir * 24, GetAbsAngles());
		SetThink(&CTripmineGrenade::SUB_Remove);
		pev->nextthink = gpGlobals->time + 0.1;
		KillBeam();
		return false;
	}
	return CGrenade::TakeDamage(info);
}

void CTripmineGrenade::Killed(const KilledInfo& info)
{
	SetDamageMode(DamageMode::No);

	if (auto attacker = info.GetAttacker(); attacker && (attacker->pev->flags & FL_CLIENT))
	{
		// some client has destroyed this mine, he'll get credit for any kills
		SetOwner(attacker);
	}

	SetThink(&CTripmineGrenade::DelayDeathThink);
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.1, 0.3);

	EmitSound(SoundChannel::Body, "common/null.wav", 0.5); // shut off chargeup
}

void CTripmineGrenade::DelayDeathThink()
{
	KillBeam();
	TraceResult tr;
	UTIL_TraceLine(GetAbsOrigin() + m_vecDir * 8, GetAbsOrigin() - m_vecDir * 64, IgnoreMonsters::No, this, &tr);

	Explode(&tr, DMG_BLAST);
}
#endif

LINK_ENTITY_TO_CLASS(weapon_tripmine, CTripmine);

void CTripmine::Spawn()
{
	Precache();
	pev->frame = 0;
	pev->body = 3;
	pev->sequence = TRIPMINE_GROUND;
	// ResetSequenceInfo( );
	pev->framerate = 0;

	FallInit();// get ready to fall down

#ifdef CLIENT_DLL
	if (!bIsMultiplayer())
#else
	if (!g_pGameRules->IsMultiplayer())
#endif
	{
		SetSize(Vector(-16, -16, 0), Vector(16, 16, 28));
	}
}

void CTripmine::Precache()
{
	CBaseWeapon::Precache();

	PRECACHE_MODEL("models/v_tripmine.mdl");
	PRECACHE_MODEL("models/p_tripmine.mdl");
	UTIL_PrecacheOther("monster_tripmine");

	m_usTripFire = PRECACHE_EVENT(1, "events/tripfire.sc");
}

bool CTripmine::GetWeaponInfo(WeaponInfo& p)
{
	p.pszName = GetClassname();
	p.pszAmmo1 = "Trip Mine";
	p.iMaxAmmo1 = TRIPMINE_MAX_CARRY;
	p.pszAmmo2 = nullptr;
	p.iMaxAmmo2 = -1;
	p.iMaxClip = WEAPON_NOCLIP;
	p.iSlot = 4;
	p.iPosition = 2;
	p.iId = m_iId = WEAPON_TRIPMINE;
	p.iWeight = TRIPMINE_WEIGHT;
	p.iFlags = WEAPON_FLAG_LIMITINWORLD | WEAPON_FLAG_EXHAUSTIBLE;

	return true;
}

bool CTripmine::Deploy()
{
	pev->body = 0;
	return DefaultDeploy("models/v_tripmine.mdl", "models/p_tripmine.mdl", TRIPMINE_DRAW, "trip");
}

void CTripmine::Holster()
{
	auto player = m_hPlayer.Get();

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if (!player->m_rgAmmo[m_iPrimaryAmmoType])
	{
		// out of mines
		player->pev->weapons &= ~(1 << WEAPON_TRIPMINE);
		SetThink(&CTripmine::DestroyWeapon);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	SendWeaponAnim(TRIPMINE_HOLSTER);
	player->EmitSound(SoundChannel::Weapon, "common/null.wav");
}

void CTripmine::PrimaryAttack()
{
	auto player = m_hPlayer.Get();

	if (player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	UTIL_MakeVectors(player->pev->v_angle + player->pev->punchangle);
	const Vector vecSrc = player->GetGunPosition();
	const Vector vecAiming = gpGlobals->v_forward;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + vecAiming * 128, IgnoreMonsters::No, player, &tr);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	UTIL_PlaybackEvent(flags, player, m_usTripFire);

	if (tr.flFraction < 1.0)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
		if (pEntity && !(pEntity->pev->flags & FL_CONVEYOR))
		{
			const Vector angles = VectorAngles(tr.vecPlaneNormal);

			CBaseEntity* pEnt = CBaseEntity::Create("monster_tripmine", tr.vecEndPos + tr.vecPlaneNormal * 8, angles, player);

			player->m_rgAmmo[m_iPrimaryAmmoType]--;

			// player "shoot" animation
			player->SetAnimation(PlayerAnim::Attack1);

			if (player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			{
				// no more mines! 
				RetireWeapon();
				return;
			}
		}
		else
		{
			// ALERT( at_console, "no deploy\n" );
		}
	}
	else
	{
	}

	m_flNextPrimaryAttack = GetNextAttackDelay(0.3);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
}

void CTripmine::WeaponIdle()
{
	auto player = m_hPlayer.Get();

	//If we're here then we're in a player's inventory, and need to use this body
	pev->body = 0;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (player->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		SendWeaponAnim(TRIPMINE_DRAW);
	}
	else
	{
		RetireWeapon();
		return;
	}

	int iAnim;
	const float flRand = UTIL_SharedRandomFloat(player->random_seed, 0, 1);
	if (flRand <= 0.25)
	{
		iAnim = TRIPMINE_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 30.0;
	}
	else if (flRand <= 0.75)
	{
		iAnim = TRIPMINE_IDLE2;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 30.0;
	}
	else
	{
		iAnim = TRIPMINE_FIDGET;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 100.0 / 30.0;
	}

	SendWeaponAnim(iAnim);
}