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
#include "gamerules.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(weapon_rpg, CRpg);

#ifndef CLIENT_DLL

LINK_ENTITY_TO_CLASS(laser_spot, CLaserSpot);

CLaserSpot* CLaserSpot::CreateSpot()
{
	CLaserSpot* pSpot = GetClassPtr((CLaserSpot*)nullptr);
	pSpot->Spawn();

	pSpot->SetClassname("laser_spot");

	return pSpot;
}

void CLaserSpot::Spawn()
{
	Precache();
	SetMovetype(Movetype::None);
	SetSolidType(Solid::Not);

	SetRenderMode(RenderMode::Glow);
	SetRenderFX(RenderFX::NoDissipation);
	SetRenderAmount(255);

	SetModel("sprites/laserdot.spr");
	SetAbsOrigin(GetAbsOrigin());
}

void CLaserSpot::Suspend(float flSuspendTime)
{
	pev->effects |= EF_NODRAW;

	SetThink(&CLaserSpot::Revive);
	pev->nextthink = gpGlobals->time + flSuspendTime;
}

void CLaserSpot::Revive()
{
	pev->effects &= ~EF_NODRAW;

	SetThink(nullptr);
}

void CLaserSpot::Precache()
{
	PRECACHE_MODEL("sprites/laserdot.spr");
}

LINK_ENTITY_TO_CLASS(rpg_rocket, CRpgRocket);

CRpgRocket* CRpgRocket::CreateRpgRocket(Vector vecOrigin, Vector vecAngles, CBaseEntity* pOwner, CRpg* pLauncher)
{
	CRpgRocket* pRocket = GetClassPtr((CRpgRocket*)nullptr);

	pRocket->SetAbsOrigin(vecOrigin);
	pRocket->SetAbsAngles(vecAngles);
	pRocket->Spawn();
	pRocket->SetTouch(&CRpgRocket::RocketTouch);
	pRocket->m_hLauncher = pLauncher;// remember what RPG fired me. 
	pLauncher->m_cActiveRockets++;// register this missile as active for the launcher
	pRocket->SetOwner(pOwner);

	return pRocket;
}

void CRpgRocket::Spawn()
{
	Precache();
	// motor
	SetMovetype(Movetype::Bounce);
	SetSolidType(Solid::BBox);

	SetModel("models/rpgrocket.mdl");
	SetSize(vec3_origin, vec3_origin);
	SetAbsOrigin(GetAbsOrigin());

	SetClassname("rpg_rocket");

	SetThink(&CRpgRocket::IgniteThink);
	SetTouch(&CRpgRocket::ExplodeTouch);

	Vector myAngles = GetAbsAngles();

	myAngles.x -= 30;
	UTIL_MakeVectors(myAngles);
	myAngles.x = -(myAngles.x + 30);
	SetAbsAngles(myAngles);

	SetAbsVelocity(gpGlobals->v_forward * 250);
	pev->gravity = 0.5;

	pev->nextthink = gpGlobals->time + 0.4;

	pev->dmg = gSkillData.plrDmgRPG;
}

void CRpgRocket::RocketTouch(CBaseEntity* pOther)
{
	if (auto launcher = m_hLauncher.Get(); launcher)
	{
		// my launcher is still around, tell it I'm dead.
		launcher->m_cActiveRockets--;
	}

	StopSound(SoundChannel::Voice, "weapons/rocket1.wav");
	ExplodeTouch(pOther);
}

void CRpgRocket::Precache()
{
	PRECACHE_MODEL("models/rpgrocket.mdl");
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	PRECACHE_SOUND("weapons/rocket1.wav");
}

void CRpgRocket::IgniteThink()
{
	// SetMovetype(Movetype::Toss);

	SetMovetype(Movetype::Fly);
	pev->effects |= EF_LIGHT;

	// make rocket sound
	EmitSound(SoundChannel::Voice, "weapons/rocket1.wav", VOL_NORM, 0.5);

	// rocket trail
	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);

	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex());	// entity
	WRITE_SHORT(m_iTrail);	// model
	WRITE_BYTE(40); // life
	WRITE_BYTE(5);  // width
	WRITE_BYTE(224);   // r, g, b
	WRITE_BYTE(224);   // r, g, b
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(255);	// brightness

	MESSAGE_END();  // move PHS/PVS data sending into here (SEND_ALL, SEND_PVS, SEND_PHS)

	m_flIgniteTime = gpGlobals->time;

	// set to follow laser spot
	SetThink(&CRpgRocket::FollowThink);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CRpgRocket::FollowThink()
{
	UTIL_MakeAimVectors(GetAbsAngles());

	// Examine all entities within a reasonable radius
	CBaseEntity* pOther = nullptr;
	Vector vecTarget = gpGlobals->v_forward;
	float flMax = WORLD_BOUNDARY;
	TraceResult tr;

	while ((pOther = UTIL_FindEntityByClassname(pOther, "laser_spot")) != nullptr)
	{
		UTIL_TraceLine(GetAbsOrigin(), pOther->GetAbsOrigin(), IgnoreMonsters::No, this, &tr);
		// ALERT( at_console, "%f\n", tr.flFraction );
		if (tr.flFraction >= 0.90)
		{
			Vector vecDir = pOther->GetAbsOrigin() - GetAbsOrigin();
			const float flDist = vecDir.Length();
			vecDir = vecDir.Normalize();
			const float flDot = DotProduct(gpGlobals->v_forward, vecDir);
			if ((flDot > 0) && (flDist * (1 - flDot) < flMax))
			{
				flMax = flDist * (1 - flDot);
				vecTarget = vecDir;
			}
		}
	}

	SetAbsAngles(VectorAngles(vecTarget));

	// this acceleration and turning math is totally wrong, but it seems to respond well so don't change it.
	const float flSpeed = GetAbsVelocity().Length();
	if (gpGlobals->time - m_flIgniteTime < 1.0)
	{
		SetAbsVelocity(GetAbsVelocity() * 0.2 + vecTarget * (flSpeed * 0.8 + 400));
		if (pev->waterlevel == WaterLevel::Head)
		{
			// go slow underwater
			if (GetAbsVelocity().Length() > 300)
			{
				SetAbsVelocity(GetAbsVelocity().Normalize() * 300);
			}
			UTIL_BubbleTrail(GetAbsOrigin() - GetAbsVelocity() * 0.1, GetAbsOrigin(), 4);
		}
		else
		{
			if (GetAbsVelocity().Length() > 2000)
			{
				SetAbsVelocity(GetAbsVelocity().Normalize() * 2000);
			}
		}
	}
	else
	{
		if (pev->effects & EF_LIGHT)
		{
			pev->effects = 0;
			StopSound(SoundChannel::Voice, "weapons/rocket1.wav");
		}
		SetAbsVelocity(GetAbsVelocity() * 0.2 + vecTarget * flSpeed * 0.798);
		if (pev->waterlevel == WaterLevel::Dry && GetAbsVelocity().Length() < 1500)
		{
			Detonate();
		}
	}
	// ALERT( at_console, "%.0f\n", flSpeed );

	pev->nextthink = gpGlobals->time + 0.1;
}
#endif

void CRpg::Reload()
{
	if (m_iClip == 1)
	{
		// don't bother with any of this if don't need to reload.
		return;
	}

	if (m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	// because the RPG waits to autoreload when no missiles are active while  the LTD is on, the
	// weapons code is constantly calling into this function, but is often denied because 
	// a) missiles are in flight, but the LTD is on
	// or
	// b) player is totally out of ammo and has nothing to switch to, and should be allowed to
	//    shine the designator around
	//
	// Set the next attack time into the future so that WeaponIdle will get called more often
	// than reload, allowing the RPG LTD to be updated

	m_flNextPrimaryAttack = GetNextAttackDelay(0.5);

	if (m_cActiveRockets && m_fSpotActive)
	{
		// no reloading when there are active missiles tracking the designator.
		// ward off future autoreload attempts by setting next attack time into the future for a bit. 
		return;
	}

#ifndef CLIENT_DLL
	if (m_pSpot && m_fSpotActive)
	{
		m_pSpot->Suspend(2.1);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.1;
	}
#endif

	bool iResult;

	if (m_iClip == 0)
		iResult = DefaultReload(RPG_MAX_CLIP, RPG_RELOAD, 2);

	if (iResult)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_hPlayer->random_seed, 10, 15);
}

void CRpg::Spawn()
{
	Precache();
	m_iId = WEAPON_RPG;

	SetModel("models/w_rpg.mdl");
	m_fSpotActive = true;

#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		// more default ammo in multiplay. 
		m_iDefaultAmmo = RPG_DEFAULT_GIVE * 2;
	}
	else
	{
		m_iDefaultAmmo = RPG_DEFAULT_GIVE;
	}

	FallInit();// get ready to fall down.
}

void CRpg::Precache()
{
	PRECACHE_MODEL("models/w_rpg.mdl");
	PRECACHE_MODEL("models/v_rpg.mdl");
	PRECACHE_MODEL("models/p_rpg.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	UTIL_PrecacheOther("laser_spot");
	UTIL_PrecacheOther("rpg_rocket");

	PRECACHE_SOUND("weapons/rocketfire1.wav");
	PRECACHE_SOUND("weapons/glauncher.wav"); // alternative fire sound

	m_usRpg = PRECACHE_EVENT(1, "events/rpg.sc");
}

bool CRpg::GetItemInfo(ItemInfo* p)
{
	p->pszName = GetClassname();
	p->pszAmmo1 = "rockets";
	p->iMaxAmmo1 = ROCKET_MAX_CARRY;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = RPG_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_RPG;
	p->iFlags = 0;
	p->iWeight = RPG_WEIGHT;

	return true;
}

bool CRpg::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MessageDest::One, gmsgWeapPickup, pPlayer);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return true;
	}
	return false;
}

bool CRpg::Deploy()
{
	if (m_iClip == 0)
	{
		return DefaultDeploy("models/v_rpg.mdl", "models/p_rpg.mdl", RPG_DRAW_UL, "rpg");
	}

	return DefaultDeploy("models/v_rpg.mdl", "models/p_rpg.mdl", RPG_DRAW1, "rpg");
}

bool CRpg::CanHolster()
{
	if (m_fSpotActive && m_cActiveRockets)
	{
		// can't put away while guiding a missile.
		return false;
	}

	return true;
}

void CRpg::Holster()
{
	m_fInReload = false;// cancel any reload in progress.

	m_hPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(RPG_HOLSTER1);

#ifndef CLIENT_DLL
	if (m_pSpot)
	{
		m_pSpot->Killed({nullptr, nullptr, GibType::Never});
		m_pSpot = nullptr;
	}
#endif
}

void CRpg::PrimaryAttack()
{
	if (m_iClip)
	{
		m_hPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_hPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		// player "shoot" animation
		m_hPlayer->SetAnimation(PlayerAnim::Attack1);

		UTIL_MakeVectors(m_hPlayer->pev->v_angle);
		Vector vecSrc = m_hPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;

		CRpgRocket* pRocket = CRpgRocket::CreateRpgRocket(vecSrc, m_hPlayer->pev->v_angle, m_hPlayer, this);

		UTIL_MakeVectors(m_hPlayer->pev->v_angle);// RpgRocket::Create stomps on globals, so remake.
		pRocket->SetAbsVelocity(pRocket->GetAbsVelocity() + gpGlobals->v_forward * DotProduct(m_hPlayer->GetAbsVelocity(), gpGlobals->v_forward));
#endif

		// firing RPG no longer turns on the designator. ALT fire is a toggle switch for the LTD.
		// Ken signed up for this as a global change (sjb)

		int flags;
#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		UTIL_PlaybackEvent(flags, m_hPlayer, m_usRpg);

		m_iClip--;

		m_flNextPrimaryAttack = GetNextAttackDelay(1.5);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
	}
	else
	{
		PlayEmptySound();
	}
	UpdateSpot();
}

void CRpg::SecondaryAttack()
{
	m_fSpotActive = !m_fSpotActive;

#ifndef CLIENT_DLL
	if (!m_fSpotActive && m_pSpot)
	{
		//TODO: don't use Killed to remove this
		m_pSpot->Killed({nullptr, nullptr, GibType::Normal});
		m_pSpot = nullptr;
	}
#endif

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
}

void CRpg::WeaponIdle()
{
	UpdateSpot();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_hPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		int iAnim;
		const float flRand = UTIL_SharedRandomFloat(m_hPlayer->random_seed, 0, 1);
		if (flRand <= 0.75 || m_fSpotActive)
		{
			if (m_iClip == 0)
				iAnim = RPG_IDLE_UL;
			else
				iAnim = RPG_IDLE;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 15.0;
		}
		else
		{
			if (m_iClip == 0)
				iAnim = RPG_FIDGET_UL;
			else
				iAnim = RPG_FIDGET;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.1;
		}

		ResetEmptySound();
		SendWeaponAnim(iAnim);
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1;
	}
}

void CRpg::UpdateSpot()
{

#ifndef CLIENT_DLL
	if (m_fSpotActive)
	{
		if (!m_pSpot)
		{
			m_pSpot = CLaserSpot::CreateSpot();
		}

		UTIL_MakeVectors(m_hPlayer->pev->v_angle);
		const Vector vecSrc = m_hPlayer->GetGunPosition();
		const Vector vecAiming = gpGlobals->v_forward;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecSrc + vecAiming * WORLD_SIZE, IgnoreMonsters::No, m_hPlayer, &tr);

		m_pSpot->SetAbsOrigin(tr.vecEndPos);
	}
#endif
}

class CRpgAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SetModel("models/w_rpgammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_rpgammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBasePlayer* pOther) override
	{
		int iGive;

#ifdef CLIENT_DLL
		if (bIsMultiplayer())
#else
		if (g_pGameRules->IsMultiplayer())
#endif
		{
			// hand out more ammo per rocket in multiplayer.
			iGive = AMMO_RPGCLIP_GIVE * 2;
		}
		else
		{
			iGive = AMMO_RPGCLIP_GIVE;
		}

		if (pOther->GiveAmmo(iGive, "rockets", ROCKET_MAX_CARRY) != -1)
		{
			EmitSound(SoundChannel::Item, "items/9mmclip1.wav");
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_rpgclip, CRpgAmmo);
