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
#include "player.h"
#include "monsters.h"
#include "weapons.h"
#include "effects.h"
#include "customentity.h"
#include "gamerules.h"
#include "UserMessages.h"
#include "weaponinfo.h"

constexpr float EGON_SWITCH_NARROW_TIME = 0.75;		// Time it takes to switch fire modes
constexpr float EGON_SWITCH_WIDE_TIME = 1.5;

LINK_ENTITY_TO_CLASS(weapon_egon, CEgon);

void CEgon::OnRemove()
{
	m_hBeam.Remove();
	m_hNoise.Remove();
	m_hSprite.Remove();
	CBasePlayerWeapon::OnRemove();
}

void CEgon::Spawn()
{
	Precache();
	m_iId = WEAPON_EGON;
	SetModel("models/w_egon.mdl");

	m_iDefaultAmmo = EGON_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}

void CEgon::Precache()
{
	PRECACHE_MODEL("models/w_egon.mdl");
	PRECACHE_MODEL("models/v_egon.mdl");
	PRECACHE_MODEL("models/p_egon.mdl");

	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND(EGON_SOUND_OFF.data());
	PRECACHE_SOUND(EGON_SOUND_RUN.data());
	PRECACHE_SOUND(EGON_SOUND_STARTUP.data());

	PRECACHE_MODEL(EGON_BEAM_SPRITE.data());
	PRECACHE_MODEL(EGON_FLARE_SPRITE.data());

	PRECACHE_SOUND("weapons/357_cock1.wav");

	m_usEgonFire = PRECACHE_EVENT(1, "events/egon_fire.sc");
	m_usEgonStop = PRECACHE_EVENT(1, "events/egon_stop.sc");
}

bool CEgon::Deploy()
{
	m_deployed = false;
	m_fireState = FireState::Off;
	return DefaultDeploy("models/v_egon.mdl", "models/p_egon.mdl", EGON_DRAW, "egon");
}

bool CEgon::AddToPlayer(CBasePlayer* pPlayer)
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

void CEgon::Holster()
{
	auto player = m_hPlayer.Get();

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(EGON_HOLSTER);

	EndAttack();
}

bool CEgon::GetItemInfo(ItemInfo* p)
{
	p->pszName = GetClassname();
	p->pszAmmo1 = "uranium";
	p->iMaxAmmo1 = URANIUM_MAX_CARRY;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_EGON;
	p->iFlags = 0;
	p->iWeight = EGON_WEIGHT;

	return true;
}

constexpr float EGON_PULSE_INTERVAL = 0.1;
constexpr float EGON_DISCHARGE_INTERVAL = 0.1;

float CEgon::GetPulseInterval()
{
	return EGON_PULSE_INTERVAL;
}

float CEgon::GetDischargeInterval()
{
	return EGON_DISCHARGE_INTERVAL;
}

bool CEgon::HasAmmo()
{
	auto player = m_hPlayer.Get();

	if (player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return false;

	return true;
}

void CEgon::UseAmmo(int count)
{
	auto player = m_hPlayer.Get();

	if (player->m_rgAmmo[m_iPrimaryAmmoType] >= count)
		player->m_rgAmmo[m_iPrimaryAmmoType] -= count;
	else
		player->m_rgAmmo[m_iPrimaryAmmoType] = 0;
}

void CEgon::Attack()
{
	auto player = m_hPlayer.Get();

	// don't fire underwater
	if (player->pev->waterlevel == WaterLevel::Head)
	{

		if (m_fireState != FireState::Off || m_hBeam)
		{
			EndAttack();
		}
		else
		{
			PlayEmptySound();
		}
		return;
	}

	UTIL_MakeVectors(player->pev->v_angle + player->pev->punchangle);
	const Vector vecAiming = gpGlobals->v_forward;
	const Vector vecSrc = player->GetGunPosition();

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	switch (m_fireState)
	{
	case FireState::Off:
	{
		if (!HasAmmo())
		{
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.25;
			PlayEmptySound();
			return;
		}

		m_flAmmoUseTime = gpGlobals->time;// start using ammo ASAP.

		UTIL_PlaybackEvent(flags, player, m_usEgonFire, {.iparam1 = static_cast<int>(m_fireState), .iparam2 = static_cast<int>(m_fireMode), .bparam1 = true});

		m_shakeTime = 0;

		player->m_iWeaponVolume = EGON_PRIMARY_VOLUME;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;
		pev->fuser1 = UTIL_WeaponTimeBase() + 2;

		pev->dmgtime = gpGlobals->time + GetPulseInterval();
		m_fireState = FireState::Charge;
	}
	break;

	case FireState::Charge:
	{
		Fire(vecSrc, vecAiming);
		player->m_iWeaponVolume = EGON_PRIMARY_VOLUME;

		if (pev->fuser1 <= UTIL_WeaponTimeBase())
		{
			UTIL_PlaybackEvent(flags, player, m_usEgonFire, {.iparam1 = static_cast<int>(m_fireState), .iparam2 = static_cast<int>(m_fireMode), .bparam1 = false});
			pev->fuser1 = 1000;
		}

		if (!HasAmmo())
		{
			EndAttack();
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
		}

	}
	break;
	}
}

void CEgon::PrimaryAttack()
{
	m_fireMode = FireMode::Wide;
	Attack();
}

void CEgon::Fire(const Vector& vecOrigSrc, const Vector& vecDir)
{
	auto player = m_hPlayer.Get();

	const Vector tmpSrc = vecOrigSrc + gpGlobals->v_up * -8 + gpGlobals->v_right * 3;

	// ALERT( at_console, "." );
	TraceResult tr;
	UTIL_TraceLine(vecOrigSrc, vecOrigSrc + vecDir * 2048, IgnoreMonsters::No, player, &tr);

	if (tr.fAllSolid)
		return;

#ifndef CLIENT_DLL
	CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

	if (pEntity == nullptr)
		return;

	if (g_pGameRules->IsMultiplayer())
	{
		if (auto sprite = m_hSprite.Get(); sprite)
		{
			if (pEntity->pev->takedamage)
			{
				sprite->pev->effects &= ~EF_NODRAW;
			}
			else
			{
				sprite->pev->effects |= EF_NODRAW;
			}
		}
	}
#endif

	float timedist;

	switch (m_fireMode)
	{
	case FireMode::Narrow:
#ifndef CLIENT_DLL
		if (pev->dmgtime < gpGlobals->time)
		{
			// Narrow mode only does damage to the entity it hits
			ClearMultiDamage();
			if (pEntity->pev->takedamage)
			{
				pEntity->TraceAttack({player, gSkillData.plrDmgEgonNarrow, vecDir, tr, DMG_ENERGYBEAM});
			}
			ApplyMultiDamage(player, player);

			if (g_pGameRules->IsMultiplayer())
			{
				// multiplayer uses 1 ammo every 1/10th second
				if (gpGlobals->time >= m_flAmmoUseTime)
				{
					UseAmmo(1);
					m_flAmmoUseTime = gpGlobals->time + 0.1;
				}
			}
			else
			{
				// single player, use 3 ammo/second
				if (gpGlobals->time >= m_flAmmoUseTime)
				{
					UseAmmo(1);
					m_flAmmoUseTime = gpGlobals->time + 0.166;
				}
			}

			pev->dmgtime = gpGlobals->time + GetPulseInterval();
		}
#endif
		timedist = (pev->dmgtime - gpGlobals->time) / GetPulseInterval();
		break;

	case FireMode::Wide:
#ifndef CLIENT_DLL
		if (pev->dmgtime < gpGlobals->time)
		{
			// wide mode does damage to the ent, and radius damage
			ClearMultiDamage();
			if (pEntity->pev->takedamage)
			{
				pEntity->TraceAttack({player, gSkillData.plrDmgEgonWide, vecDir, tr, DMG_ENERGYBEAM | DMG_ALWAYSGIB});
			}
			ApplyMultiDamage(player, player);

			if (g_pGameRules->IsMultiplayer())
			{
				// radius damage a little more potent in multiplayer.
				::RadiusDamage(tr.vecEndPos, this, player, gSkillData.plrDmgEgonWide / 4, 128, CLASS_NONE, DMG_ENERGYBEAM | DMG_BLAST | DMG_ALWAYSGIB);
			}

			if (!player->IsAlive())
				return;

			if (g_pGameRules->IsMultiplayer())
			{
				//multiplayer uses 5 ammo/second
				if (gpGlobals->time >= m_flAmmoUseTime)
				{
					UseAmmo(1);
					m_flAmmoUseTime = gpGlobals->time + 0.2;
				}
			}
			else
			{
				// Wide mode uses 10 charges per second in single player
				if (gpGlobals->time >= m_flAmmoUseTime)
				{
					UseAmmo(1);
					m_flAmmoUseTime = gpGlobals->time + 0.1;
				}
			}

			pev->dmgtime = gpGlobals->time + GetDischargeInterval();
			if (m_shakeTime < gpGlobals->time)
			{
				UTIL_ScreenShake(tr.vecEndPos, 5.0, 150.0, 0.75, 250.0);
				m_shakeTime = gpGlobals->time + 1.5;
			}
		}
#endif
		timedist = (pev->dmgtime - gpGlobals->time) / GetDischargeInterval();
		break;
	}

	timedist = 1 - std::clamp(timedist, 0.0f, 1.0f);

	UpdateEffect(tmpSrc, tr.vecEndPos, timedist);
}

void CEgon::UpdateEffect(const Vector& startPoint, const Vector& endPoint, float timeBlend)
{
#ifndef CLIENT_DLL
	if (!m_hBeam)
	{
		CreateEffect();
	}

	auto beam = m_hBeam.Get();

	beam->SetStartPos(endPoint);
	beam->SetBrightness(255 - (timeBlend * 180));
	beam->SetWidth(40 - (timeBlend * 20));

	if (m_fireMode == FireMode::Wide)
		beam->SetColor(30 + (25 * timeBlend), 30 + (30 * timeBlend), 64 + 80 * fabs(sin(gpGlobals->time * 10)));
	else
		beam->SetColor(60 + (25 * timeBlend), 120 + (30 * timeBlend), 64 + 80 * fabs(sin(gpGlobals->time * 10)));

	auto sprite = m_hSprite.Get();

	sprite->SetAbsOrigin(endPoint);
	sprite->pev->frame += 8 * gpGlobals->frametime;
	if (sprite->pev->frame > sprite->Frames())
		sprite->pev->frame = 0;

	m_hNoise->SetStartPos(endPoint);
#endif
}

void CEgon::CreateEffect()
{
#ifndef CLIENT_DLL
	auto player = m_hPlayer.Get();

	DestroyEffect();

	auto beam = m_hBeam = CBeam::BeamCreate(EGON_BEAM_SPRITE.data(), 40);
	beam->PointEntInit(GetAbsOrigin(), player->entindex());
	beam->SetFlags(BEAM_FSINE);
	beam->SetEndAttachment(1);
	beam->pev->spawnflags |= SF_BEAM_TEMPORARY;	// Flag these to be destroyed on save/restore or level transition
	beam->pev->flags |= FL_SKIPLOCALHOST;
	beam->SetOwner(player);

	auto noise = m_hNoise = CBeam::BeamCreate(EGON_BEAM_SPRITE.data(), 55);
	noise->PointEntInit(GetAbsOrigin(), player->entindex());
	noise->SetScrollRate(25);
	noise->SetBrightness(100);
	noise->SetEndAttachment(1);
	noise->pev->spawnflags |= SF_BEAM_TEMPORARY;
	noise->pev->flags |= FL_SKIPLOCALHOST;
	noise->SetOwner(player);

	auto sprite = m_hSprite = CSprite::SpriteCreate(EGON_FLARE_SPRITE.data(), GetAbsOrigin(), false);
	sprite->pev->scale = 1.0;
	sprite->SetTransparency(RenderMode::Glow, {255, 255, 255}, 255, RenderFX::NoDissipation);
	sprite->pev->spawnflags |= SF_SPRITE_TEMPORARY;
	sprite->pev->flags |= FL_SKIPLOCALHOST;
	sprite->SetOwner(player);

	if (m_fireMode == FireMode::Wide)
	{
		beam->SetScrollRate(50);
		beam->SetNoise(20);
		noise->SetColor(50, 50, 255);
		noise->SetNoise(8);
	}
	else
	{
		beam->SetScrollRate(110);
		beam->SetNoise(5);
		noise->SetColor(80, 120, 255);
		noise->SetNoise(2);
	}
#endif
}

void CEgon::DestroyEffect()
{
#ifndef CLIENT_DLL
	m_hBeam.Remove();
	m_hNoise.Remove();
	if (auto sprite = m_hSprite.Get(); sprite)
	{
		if (m_fireMode == FireMode::Wide)
			sprite->Expand(10, 500);
		else
			UTIL_Remove(sprite);
		m_hSprite = nullptr;
	}
#endif
}

void CEgon::WeaponIdle()
{
	auto player = m_hPlayer.Get();

	if (!(player->m_afButtonPressed & IN_ATTACK2) && (player->pev->button & IN_ATTACK))
	{
		return;
	}

	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_fireState != FireState::Off)
		EndAttack();

	int iAnim;

	const float flRand = RANDOM_FLOAT(0, 1);

	if (flRand <= 0.5)
	{
		iAnim = EGON_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
	}
	else
	{
		iAnim = EGON_FIDGET1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
	}

	SendWeaponAnim(iAnim);
	m_deployed = true;
}

void CEgon::EndAttack()
{
	auto player = m_hPlayer.Get();

	const bool bMakeNoise = m_fireState != FireState::Off; //Checking the button just in case!.

	UTIL_PlaybackEvent(FEV_GLOBAL | FEV_RELIABLE, player, m_usEgonStop, {.origin = player->GetAbsOrigin(), .angles = player->GetAbsAngles(), .iparam1 = bMakeNoise});

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;

	m_fireState = FireState::Off;

	DestroyEffect();
}

void CEgon::GetWeaponData(weapon_data_t& data)
{
	data.iuser3 = static_cast<int>(m_fireState);
}

void CEgon::SetWeaponData(const weapon_data_t& data)
{
	m_fireState = static_cast<FireState>(data.iuser3);
}

class CEgonAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SetModel("models/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_chainammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	ItemApplyResult Apply(CBasePlayer* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_URANIUMBOX_GIVE, "uranium", URANIUM_MAX_CARRY) != -1)
		{
			EmitSound(SoundChannel::Item, "items/9mmclip1.wav");
			return ItemApplyResult::Used;
		}
		return ItemApplyResult::NotUsed;
	}
};
LINK_ENTITY_TO_CLASS(ammo_egonclip, CEgonAmmo);
