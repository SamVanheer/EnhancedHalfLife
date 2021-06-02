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

#pragma once

#include "CBaseWeapon.hpp"
#include "CEgon.generated.hpp"

enum egon_e
{
	EGON_IDLE1 = 0,
	EGON_FIDGET1,
	EGON_ALTFIREON,
	EGON_ALTFIRECYCLE,
	EGON_ALTFIREOFF,
	EGON_FIRE1,
	EGON_FIRE2,
	EGON_FIRE3,
	EGON_FIRE4,
	EGON_DRAW,
	EGON_HOLSTER
};

constexpr int EGON_PRIMARY_VOLUME = 450;
constexpr std::string_view EGON_BEAM_SPRITE{"sprites/xbeam1.spr"};
constexpr std::string_view EGON_FLARE_SPRITE{"sprites/XSpark1.spr"};
constexpr std::string_view EGON_SOUND_OFF{"weapons/egon_off1.wav"};
constexpr std::string_view EGON_SOUND_RUN{"weapons/egon_run3.wav"};
constexpr std::string_view EGON_SOUND_STARTUP{"weapons/egon_windup2.wav"};

class EHL_CLASS(EntityName=weapon_egon) CEgon : public CBaseWeapon
{
	EHL_GENERATED_BODY()

public:
	enum class FireState
	{
		Off,
		Charge
	};

	enum class FireMode
	{
		Narrow,
		Wide
	};

	void OnConstruct() override
	{
		CBaseWeapon::OnConstruct();
		m_iId = WEAPON_EGON;
		m_iDefaultAmmo = EGON_DEFAULT_GIVE;
		SetWorldModelName("models/w_egon.mdl");
	}

	void OnRemove() override;
	void Spawn() override;
	void Precache() override;
	int WeaponSlot() override { return 4; }
	bool GetWeaponInfo(WeaponInfo & p) override;
	bool AddToPlayer(CBasePlayer * pPlayer) override;

	bool Deploy() override;
	void Holster() override;

	void UpdateEffect(const Vector & startPoint, const Vector & endPoint, float timeBlend);

	void CreateEffect();
	void DestroyEffect();

	void EndAttack();
	void Attack();
	void PrimaryAttack() override;
	bool ShouldWeaponIdle() override { return true; }
	void WeaponIdle() override;

	EHL_FIELD(Persisted, Type=Time)
	float m_flAmmoUseTime = 0;// since we use < 1 point of ammo per update, we subtract ammo on a timer.

	float GetPulseInterval();
	float GetDischargeInterval();

	void Fire(const Vector & vecOrigSrc, const Vector & vecDir);

	bool HasAmmo();

	void UseAmmo(int count);

	EHandle<CBeam> m_hBeam;
	EHandle<CBeam> m_hNoise;
	EHandle<CSprite> m_hSprite;

	EHL_FIELD(Persisted)
	FireState m_fireState = FireState::Off;

	bool UseDecrement() override
	{
#if defined( CLIENT_WEAPONS )
		return true;
#else
		return false;
#endif
	}

	void GetWeaponData(weapon_data_t & data) override;
	void SetWeaponData(const weapon_data_t & data) override;

	unsigned short m_usEgonStop = 0;

private:
	EHL_FIELD(Persisted, Type=Time)
	float m_shootTime = 0;

	EHL_FIELD(Persisted)
	FireMode m_fireMode = FireMode::Narrow;

	EHL_FIELD(Persisted, Type=Time)
	float m_shakeTime = 0;

	bool m_deployed = 0; //TODO: bool

	unsigned short m_usEgonFire = 0;
};
