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

#include "CBaseEntity.hpp"

class CCyclerSprite : public CBaseEntity
{
public:
	void Spawn() override;
	void Think() override;
	void Use(const UseInfo& info) override;
	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() | FCAP_DONT_SAVE | FCAP_IMPULSE_USE); }
	bool	TakeDamage(const TakeDamageInfo& info) override;
	void	Animate(float frames);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	inline bool		ShouldAnimate() { return m_animate && m_maxFrame > 1.0; }
	bool m_animate = false;
	float m_lastTime = 0;
	float m_maxFrame = 0;
};
