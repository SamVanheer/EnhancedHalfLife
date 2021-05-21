/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

#include "cl_engine_int.hpp"
#include "vector.hpp"

constexpr int DMG_IMAGE_LIFE = 2;	// seconds that image is up

constexpr int DMG_IMAGE_POISON = 0;
constexpr int DMG_IMAGE_ACID = 1;
constexpr int DMG_IMAGE_COLD = 2;
constexpr int DMG_IMAGE_DROWN = 3;
constexpr int DMG_IMAGE_BURN = 4;
constexpr int DMG_IMAGE_NERVE = 5;
constexpr int DMG_IMAGE_RAD = 6;
constexpr int DMG_IMAGE_SHOCK = 7;
//tf defines
constexpr int DMG_IMAGE_CALTROP = 8;
constexpr int DMG_IMAGE_TRANQ = 9;
constexpr int DMG_IMAGE_CONCUSS = 10;
constexpr int DMG_IMAGE_HALLUC = 11;
constexpr int NUM_DMG_TYPES = 12;
// instant damage

struct DAMAGE_IMAGE
{
	float fExpire;
	float fBaseline;
	int	x, y;
};

class CHudHealth : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float fTime) override;
	void Reset() override;
	bool MsgFunc_Health(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_Damage(const char* pszName, int iSize, void* pbuf);
	int m_iHealth = 0;
	int m_HUD_dmg_bio = 0;
	int m_HUD_cross = 0;
	float m_fAttackFront = 0, m_fAttackRear = 0, m_fAttackLeft = 0, m_fAttackRight = 0;

	/**
	*	@brief Returns back a color from the Green <-> Yellow <-> Red ramp
	*/
	void GetPainColor(int& r, int& g, int& b);
	float m_fFade = 0;

private:
	HSPRITE m_hSprite{};
	HSPRITE m_hDamage{};

	DAMAGE_IMAGE m_dmg[NUM_DMG_TYPES]{};
	int	m_bitsDamage = 0;
	bool DrawPain(float fTime);
	bool DrawDamage(float fTime);
	void CalcDamageDirection(Vector vecFrom);
	void UpdateTiles(float fTime, long bits);
};
