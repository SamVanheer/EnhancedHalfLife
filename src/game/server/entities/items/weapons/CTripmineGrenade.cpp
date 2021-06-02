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

#include "CTripmine.hpp"
#include "CTripmineGrenade.hpp"

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
