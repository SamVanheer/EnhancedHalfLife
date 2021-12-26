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
#include "CMomentaryDoor.generated.hpp"

class EHL_CLASS("EntityName": "momentary_door") CMomentaryDoor : public CBaseToggle
{
	EHL_GENERATED_BODY()

public:
	void	Spawn() override;
	void Precache() override;

	void	KeyValue(KeyValueData* pkvd) override;
	void	Use(const UseInfo& info) override;
	int	ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	/**
	*	@brief The door has reached needed position.
	*/
	void EXPORT DoorMoveDone();

	EHL_FIELD("Persisted": true)
	byte m_bMoveSnd = 0;			// sound a door makes while moving	

	EHL_FIELD("Persisted": true, "Type": "SoundName")
	string_t m_iszMovingSound;

	EHL_FIELD("Persisted": true, "Type": "SoundName")
	string_t m_iszArrivedSound;
};