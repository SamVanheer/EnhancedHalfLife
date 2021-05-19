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

constexpr int SPEAKER_START_SILENT = 1;	//!< wait for trigger 'on' to start announcements

constexpr float ANNOUNCE_MINUTES_MIN = 0.25;
constexpr float ANNOUNCE_MINUTES_MAX = 2.25;

/**
*	@brief Used for announcements per level, for door lock/unlock spoken voice.
*/
class CSpeaker : public CBaseEntity
{
public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Precache() override;

	/**
	*	@brief if an announcement is pending, cancel it.  If no announcement is pending, start one.
	*/
	void EXPORT ToggleUse(const UseInfo& info);
	void EXPORT SpeakerThink();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	int	m_preset = 0; // preset number
};
