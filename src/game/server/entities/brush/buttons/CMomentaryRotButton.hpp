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

#include "CBaseToggle.hpp"
#include "CMomentaryRotButton.generated.hpp"

/**
*	@brief Make this button behave like a door (HACKHACK)
*	@details This will disable use and make the button solid
*	rotating buttons were made Solid::Not by default since their were some collision problems with them...
*/
constexpr int SF_MOMENTARY_DOOR = 0x0001;
constexpr int SF_MOMENTARY_AUTO_RETURN = 16;

class EHL_CLASS(EntityName=momentary_rot_button) CMomentaryRotButton : public CBaseToggle
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void	KeyValue(KeyValueData* pkvd) override;
	int	ObjectCaps() override
	{
		const int flags = CBaseToggle::ObjectCaps() & (~FCAP_ACROSS_TRANSITION);
		if (pev->spawnflags & SF_MOMENTARY_DOOR)
			return flags;
		return flags | FCAP_CONTINUOUS_USE;
	}
	void	Use(const UseInfo& info) override;
	void	EXPORT Off();
	void	EXPORT Return();
	void	UpdateSelf(float value);
	void	UpdateSelfReturn(float value);
	void	UpdateAllButtons(float value, bool start);

	void	PlaySound();
	void	UpdateTarget(float value);

	EHL_FIELD(Persisted)
	bool m_lastUsed = false;

	EHL_FIELD(Persisted)
	int m_direction = 0;

	EHL_FIELD(Persisted)
	float m_returnSpeed = 0;

	EHL_FIELD(Persisted)
	Vector m_start;

	EHL_FIELD(Persisted)
	Vector m_end;

	EHL_FIELD(Persisted)
	int m_sounds = 0;

	EHL_FIELD(Persisted, Type=SoundName)
	string_t m_iszActivateSound = iStringNull;
};
