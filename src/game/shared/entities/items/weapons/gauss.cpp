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
#include "soundent.h"
#include "shake.h"
#include "gamerules.h"
#include "UserMessages.h"
#include "weaponinfo.h"

LINK_ENTITY_TO_CLASS(weapon_gauss, CGauss);

float CGauss::GetFullChargeTime()
{
#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		return 1.5;
	}

	return 4;
}

#ifdef CLIENT_DLL
extern int g_irunninggausspred;
#endif

void CGauss::Spawn()
{
	Precache();
	m_iId = WEAPON_GAUSS;
	SetModel("models/w_gauss.mdl");

	m_iDefaultAmmo = GAUSS_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}

void CGauss::Precache()
{
	PRECACHE_MODEL("models/w_gauss.mdl");
	PRECACHE_MODEL("models/v_gauss.mdl");
	PRECACHE_MODEL("models/p_gauss.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/gauss2.wav");
	PRECACHE_SOUND("weapons/electro4.wav");
	PRECACHE_SOUND("weapons/electro5.wav");
	PRECACHE_SOUND("weapons/electro6.wav");
	PRECACHE_SOUND("ambience/pulsemachine.wav");

	m_iGlow = PRECACHE_MODEL("sprites/hotglow.spr");
	m_iBalls = PRECACHE_MODEL("sprites/hotglow.spr");
	m_iBeam = PRECACHE_MODEL("sprites/smoke.spr");

	m_usGaussFire = PRECACHE_EVENT(1, "events/gauss.sc");
	m_usGaussSpin = PRECACHE_EVENT(1, "events/gaussspin.sc");
}

bool CGauss::AddToPlayer(CBasePlayer* pPlayer)
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

bool CGauss::GetItemInfo(ItemInfo* p)
{
	p->pszName = GetClassname();
	p->pszAmmo1 = "uranium";
	p->iMaxAmmo1 = URANIUM_MAX_CARRY;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_GAUSS;
	p->iFlags = 0;
	p->iWeight = GAUSS_WEIGHT;

	return true;
}

bool CGauss::Deploy()
{
	m_flPlayAftershock = 0.0;
	return DefaultDeploy("models/v_gauss.mdl", "models/p_gauss.mdl", GAUSS_DRAW, "gauss");
}

void CGauss::Holster()
{
	auto player = m_hPlayer.Get();

	UTIL_PlaybackEvent(FEV_GLOBAL | FEV_RELIABLE, player, m_usGaussFire,
		{.delay = 0.01f, .origin = player->GetAbsOrigin(), .angles = player->GetAbsAngles(), .fparam1 = 0.0f, .bparam1 = false, .bparam2 = true});

	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(GAUSS_HOLSTER);
	m_fInAttack = AttackState::NotAttacking;
}

void CGauss::PrimaryAttack()
{
	auto player = m_hPlayer.Get();

	// don't fire underwater
	if (player->pev->waterlevel == WaterLevel::Head)
	{
		PlayEmptySound();
		m_flNextSecondaryAttack = m_flNextPrimaryAttack = GetNextAttackDelay(0.15);
		return;
	}

	if (player->m_rgAmmo[m_iPrimaryAmmoType] < 2)
	{
		PlayEmptySound();
		player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
		return;
	}

	player->m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;
	m_fPrimaryFire = true;

	player->m_rgAmmo[m_iPrimaryAmmoType] -= 2;

	StartFire();
	m_fInAttack = AttackState::NotAttacking;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
	player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.2;
}

void CGauss::SecondaryAttack()
{
	auto player = m_hPlayer.Get();

	// don't fire underwater
	if (player->pev->waterlevel == WaterLevel::Head)
	{
		if (m_fInAttack != AttackState::NotAttacking)
		{
			player->EmitSound(SoundChannel::Weapon, "weapons/electro4.wav", VOL_NORM, ATTN_NORM, 80 + RANDOM_LONG(0, 0x3f));
			SendWeaponAnim(GAUSS_IDLE);
			m_fInAttack = AttackState::NotAttacking;
		}
		else
		{
			PlayEmptySound();
		}

		m_flNextSecondaryAttack = m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		return;
	}

	if (m_fInAttack == AttackState::NotAttacking)
	{
		if (player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		{
			player->EmitSound(SoundChannel::Weapon, "weapons/357_cock1.wav", 0.8);
			player->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
			return;
		}

		m_fPrimaryFire = false;

		player->m_rgAmmo[m_iPrimaryAmmoType]--;// take one ammo just to start the spin
		m_flNextAmmoBurn = UTIL_WeaponTimeBase();

		// spin up
		player->m_iWeaponVolume = GAUSS_PRIMARY_CHARGE_VOLUME;

		SendWeaponAnim(GAUSS_SPINUP);
		m_fInAttack = AttackState::SpinUp;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		m_flStartCharge = gpGlobals->time;
		m_flAmmoStartCharge = UTIL_WeaponTimeBase() + GetFullChargeTime();

		UTIL_PlaybackEvent(FEV_NOTHOST, player, m_usGaussSpin, {.iparam1 = 110, .bparam1 = false});

		m_iSoundState = SND_CHANGE_PITCH;
	}
	else if (m_fInAttack == AttackState::SpinUp)
	{
		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			SendWeaponAnim(GAUSS_SPIN);
			m_fInAttack = AttackState::Charging;
		}
	}
	else
	{
		// during the charging process, eat one bit of ammo every once in a while
		if (UTIL_WeaponTimeBase() >= m_flNextAmmoBurn && m_flNextAmmoBurn != 1000)
		{
#ifdef CLIENT_DLL
			if (bIsMultiplayer())
#else
			if (g_pGameRules->IsMultiplayer())
#endif
			{
				player->m_rgAmmo[m_iPrimaryAmmoType]--;
				m_flNextAmmoBurn = UTIL_WeaponTimeBase() + 0.1;
			}
			else
			{
				player->m_rgAmmo[m_iPrimaryAmmoType]--;
				m_flNextAmmoBurn = UTIL_WeaponTimeBase() + 0.3;
			}
		}

		if (player->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		{
			// out of ammo! force the gun to fire
			StartFire();
			m_fInAttack = AttackState::NotAttacking;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
			player->m_flNextAttack = UTIL_WeaponTimeBase() + 1;
			return;
		}

		if (UTIL_WeaponTimeBase() >= m_flAmmoStartCharge)
		{
			// don't eat any more ammo after gun is fully charged.
			m_flNextAmmoBurn = 1000;
		}

		const int pitch = std::min(250, static_cast<int>((gpGlobals->time - m_flStartCharge) * (150 / GetFullChargeTime()) + 100));

		// ALERT( at_console, "%d %d %d\n", m_fInAttack, m_iSoundState, pitch );

		if (m_iSoundState == 0)
			ALERT(at_console, "sound state %d\n", m_iSoundState);

		UTIL_PlaybackEvent(FEV_NOTHOST, player, m_usGaussSpin, {.iparam1 = pitch, .bparam1 = m_iSoundState == SND_CHANGE_PITCH});

		m_iSoundState = SND_CHANGE_PITCH; // hack for going through level transitions

		player->m_iWeaponVolume = GAUSS_PRIMARY_CHARGE_VOLUME;

		// m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;
		if (m_flStartCharge < gpGlobals->time - 10)
		{
			// Player charged up too long. Zap him.
			player->EmitSound(SoundChannel::Weapon, "weapons/electro4.wav", VOL_NORM, ATTN_NORM, 80 + RANDOM_LONG(0, 0x3f));
			player->EmitSound(SoundChannel::Item, "weapons/electro6.wav", VOL_NORM, ATTN_NORM, 75 + RANDOM_LONG(0, 0x3f));

			m_fInAttack = AttackState::NotAttacking;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
			player->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;

#ifndef CLIENT_DLL
			player->TakeDamage({UTIL_GetWorld(), UTIL_GetWorld(), 50, DMG_SHOCK});
			UTIL_ScreenFade(player, Vector(255, 128, 0), 2, 0.5, 128, FFADE_IN);
#endif
			SendWeaponAnim(GAUSS_IDLE);

			// Player may have been killed and this weapon dropped, don't execute any more code after this!
			return;
		}
	}
}

void CGauss::StartFire()
{
	auto player = m_hPlayer.Get();

	UTIL_MakeVectors(player->pev->v_angle + player->pev->punchangle);
	const Vector vecAiming = gpGlobals->v_forward;
	const Vector vecSrc = player->GetGunPosition(); // + gpGlobals->v_up * -8 + gpGlobals->v_right * 8;

	float flDamage;
	if (gpGlobals->time - m_flStartCharge > GetFullChargeTime())
	{
		flDamage = 200;
	}
	else
	{
		flDamage = 200 * ((gpGlobals->time - m_flStartCharge) / GetFullChargeTime());
	}

	if (m_fPrimaryFire)
	{
		// fixed damage on primary attack
#ifdef CLIENT_DLL
		flDamage = 20;
#else 
		flDamage = gSkillData.plrDmgGauss;
#endif
	}

	//TODO: Aftershock state is never set so what is this exactly?
	if (m_fInAttack != AttackState::Aftershock)
	{
		//ALERT ( at_console, "Time:%f Damage:%f\n", gpGlobals->time - player->m_flStartCharge, flDamage );

#ifndef CLIENT_DLL
		const float flZVel = player->GetAbsVelocity().z;

		Vector velocity = player->GetAbsVelocity();

		if (!m_fPrimaryFire)
		{
			velocity = velocity - gpGlobals->v_forward * flDamage * 5;
		}

		if (!g_pGameRules->IsMultiplayer())
		{
			// in deathmatch, gauss can pop you up into the air. Not in single play.
			velocity.z = flZVel;
		}

		player->SetAbsVelocity(velocity);
#endif
		// player "shoot" animation
		player->SetAnimation(PlayerAnim::Attack1);
	}

	// time until aftershock 'static discharge' sound
	m_flPlayAftershock = gpGlobals->time + UTIL_SharedRandomFloat(player->random_seed, 0.3, 0.8);

	Fire(vecSrc, vecAiming, flDamage);
}

void CGauss::Fire(const Vector& vecOrigSrc, Vector vecDir, float flDamage)
{
	auto player = m_hPlayer.Get();

	player->m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;

#ifdef CLIENT_DLL
	if (!m_fPrimaryFire)
		g_irunninggausspred = true;
#endif

	// The main firing event is sent unreliably so it won't be delayed.
	UTIL_PlaybackEvent(FEV_NOTHOST, player, m_usGaussFire,
		{.origin = player->GetAbsOrigin(), .angles = player->GetAbsAngles(), .fparam1 =flDamage, .bparam1 = m_fPrimaryFire, .bparam2 = false});

	// This reliable event is used to stop the spinning sound
	// It's delayed by a fraction of second to make sure it is delayed by 1 frame on the client
	// It's sent reliably anyway, which could lead to other delays

	UTIL_PlaybackEvent(FEV_NOTHOST | FEV_RELIABLE, player, m_usGaussFire,
		{.delay = 0.01f, .origin = player->GetAbsOrigin(), .angles = player->GetAbsAngles(), .fparam1 = 0.0f, .bparam1 = false, .bparam2 = true});

	/*ALERT( at_console, "%f %f %f\n%f %f %f\n",
		vecSrc.x, vecSrc.y, vecSrc.z,
		vecDest.x, vecDest.y, vecDest.z );*/


		//	ALERT( at_console, "%f %f\n", tr.flFraction, flMaxFrac );

#ifndef CLIENT_DLL
	Vector vecSrc = vecOrigSrc;
	Vector vecDest = vecSrc + vecDir * WORLD_SIZE;
	TraceResult tr, beam_tr;
	float flMaxFrac = 1.0;
	int	nTotal = 0;
	bool fHasPunched = false;
	bool fFirstBeam = true;
	int	nMaxHits = 10;

	CBaseEntity* pIgnore = player;

	while (flDamage > 10 && nMaxHits > 0)
	{
		nMaxHits--;

		// ALERT( at_console, "." );
		UTIL_TraceLine(vecSrc, vecDest, IgnoreMonsters::No, pIgnore, &tr);

		if (tr.fAllSolid)
			break;

		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		if (pEntity == nullptr)
			break;

		if (fFirstBeam)
		{
			player->pev->effects |= EF_MUZZLEFLASH;
			fFirstBeam = false;

			nTotal += 26;
		}

		if (pEntity->pev->takedamage)
		{
			ClearMultiDamage();
			pEntity->TraceAttack({player, flDamage, vecDir, tr, DMG_BULLET});
			ApplyMultiDamage(player, player);
		}

		if (pEntity->ReflectGauss())
		{
			pIgnore = nullptr;

			float n = -DotProduct(tr.vecPlaneNormal, vecDir);

			if (n < 0.5) // 60 degrees
			{
				// ALERT( at_console, "reflect %f\n", n );
				// reflect
				const Vector r = 2.0 * tr.vecPlaneNormal * n + vecDir;
				flMaxFrac = flMaxFrac - tr.flFraction;
				vecDir = r;
				vecSrc = tr.vecEndPos + vecDir * 8;
				vecDest = vecSrc + vecDir * WORLD_SIZE;

				// explode a bit
				player->RadiusDamage(tr.vecEndPos, this, player, flDamage * n, CLASS_NONE, DMG_BLAST);

				nTotal += 34;

				// lose energy
				if (n == 0) n = 0.1;
				flDamage = flDamage * (1 - n);
			}
			else
			{
				nTotal += 13;

				// limit it to one hole punch
				if (fHasPunched)
					break;
				fHasPunched = true;

				// try punching through wall if secondary attack (primary is incapable of breaking through)
				if (!m_fPrimaryFire)
				{
					UTIL_TraceLine(tr.vecEndPos + vecDir * 8, vecDest, IgnoreMonsters::No, pIgnore, &beam_tr);
					if (!beam_tr.fAllSolid)
					{
						// trace backwards to find exit point
						UTIL_TraceLine(beam_tr.vecEndPos, tr.vecEndPos, IgnoreMonsters::No, pIgnore, &beam_tr);

						float n = (beam_tr.vecEndPos - tr.vecEndPos).Length();

						if (n < flDamage)
						{
							if (n == 0) n = 1;
							flDamage -= n;

							// ALERT( at_console, "punch %f\n", n );
							nTotal += 21;

							// exit blast damage
							//player->RadiusDamage( beam_tr.vecEndPos + vecDir * 8, pev, player->pev, flDamage, CLASS_NONE, DMG_BLAST );
							float damage_radius;

							if (g_pGameRules->IsMultiplayer())
							{
								damage_radius = flDamage * 1.75;  // Old code == 2.5
							}
							else
							{
								damage_radius = flDamage * 2.5;
							}

							::RadiusDamage(beam_tr.vecEndPos + vecDir * 8, this, player, flDamage, damage_radius, CLASS_NONE, DMG_BLAST);

							CSoundEnt::InsertSound(bits_SOUND_COMBAT, GetAbsOrigin(), NORMAL_EXPLOSION_VOLUME, 3.0);

							nTotal += 53;

							vecSrc = beam_tr.vecEndPos + vecDir;
						}
					}
					else
					{
						//ALERT( at_console, "blocked %f\n", n );
						flDamage = 0;
					}
				}
				else
				{
					//ALERT( at_console, "blocked solid\n" );

					flDamage = 0;
				}
			}
		}
		else
		{
			vecSrc = tr.vecEndPos + vecDir;
			pIgnore = pEntity;
		}
	}
#endif
	// ALERT( at_console, "%d bytes\n", nTotal );
}

void CGauss::WeaponIdle()
{
	auto player = m_hPlayer.Get();

	ResetEmptySound();

	// play aftershock static discharge
	if (m_flPlayAftershock && m_flPlayAftershock < gpGlobals->time)
	{
		switch (RANDOM_LONG(0, 3))
		{
		case 0:	player->EmitSound(SoundChannel::Weapon, "weapons/electro4.wav", RANDOM_FLOAT(0.7, 0.8)); break;
		case 1:	player->EmitSound(SoundChannel::Weapon, "weapons/electro5.wav", RANDOM_FLOAT(0.7, 0.8)); break;
		case 2:	player->EmitSound(SoundChannel::Weapon, "weapons/electro6.wav", RANDOM_FLOAT(0.7, 0.8)); break;
		case 3:	break; // no sound
		}
		m_flPlayAftershock = 0.0;
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_fInAttack != AttackState::NotAttacking)
	{
		StartFire();
		m_fInAttack = AttackState::NotAttacking;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
	}
	else
	{
		int iAnim;
		const float flRand = RANDOM_FLOAT(0, 1);
		if (flRand <= 0.5)
		{
			iAnim = GAUSS_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
		}
		else if (flRand <= 0.75)
		{
			iAnim = GAUSS_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(player->random_seed, 10, 15);
		}
		else
		{
			iAnim = GAUSS_FIDGET;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
		}

		return;
		SendWeaponAnim(iAnim);
	}
}

void CGauss::GetWeaponData(weapon_data_t& data)
{
	data.iuser2 = static_cast<int>(m_fInAttack);
	data.fuser2 = m_flNextAmmoBurn;
	data.fuser3 = m_flAmmoStartCharge;
}

void CGauss::SetWeaponData(const weapon_data_t& data)
{
	m_fInAttack = static_cast<AttackState>(data.iuser2);
	m_flNextAmmoBurn = data.fuser2;
	m_flAmmoStartCharge = data.fuser3;
}

void CGauss::DecrementTimers()
{
	if (m_flNextAmmoBurn != 1000)
	{
		m_flNextAmmoBurn -= gpGlobals->frametime;

		if (m_flNextAmmoBurn < -0.001)
			m_flNextAmmoBurn = -0.001;
	}

	if (m_flAmmoStartCharge != 1000)
	{
		m_flAmmoStartCharge -= gpGlobals->frametime;

		if (m_flAmmoStartCharge < -0.001)
			m_flAmmoStartCharge = -0.001;
	}
}

class CGaussAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SetModel("models/w_gaussammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_gaussammo.mdl");
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
LINK_ENTITY_TO_CLASS(ammo_gaussclip, CGaussAmmo);
