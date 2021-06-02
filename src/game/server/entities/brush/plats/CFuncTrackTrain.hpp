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
#include "CPathTrack.hpp"
#include "CFuncTrackTrain.generated.hpp"

// Tracktrain spawn flags
constexpr int SF_TRACKTRAIN_NOPITCH = 0x0001;
constexpr int SF_TRACKTRAIN_NOCONTROL = 0x0002;
constexpr int SF_TRACKTRAIN_FORWARDONLY = 0x0004;
constexpr int SF_TRACKTRAIN_PASSABLE = 0x0008;

class EHL_CLASS(EntityName=func_tracktrain) CFuncTrackTrain : public CBaseEntity
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;

	void Blocked(CBaseEntity* pOther) override;
	void Use(const UseInfo& info) override;
	void KeyValue(KeyValueData* pkvd) override;

	void EXPORT Next();
	void EXPORT Find();
	void EXPORT NearestPath();
	void EXPORT DeadEnd();

	void		NextThink(float thinkTime, bool alwaysThink);

	void SetTrack(CPathTrack* track) { m_hPath = track->Nearest(GetAbsOrigin()); }
	void SetControls(CBaseEntity* pControls);
	bool OnControls(CBaseEntity* pTest) override;

	void StopSound();

	/**
	*	@brief update pitch based on speed, start sound if not playing
	*	NOTE: when train goes through transition, m_soundPlaying should go to false,
	*	which will cause the looped sound to restart.
	*/
	void UpdateSound();

	static CFuncTrackTrain* Instance(CBaseEntity* pent);

	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DIRECTIONAL_USE; }

	void	OverrideReset() override;

	EHL_FIELD(Persisted)
	EHandle<CPathTrack> m_hPath;

	EHL_FIELD(Persisted)
	float m_length = 0;

	EHL_FIELD(Persisted)
	float m_height = 0;

	EHL_FIELD(Persisted)
	float m_speed = 0;

	EHL_FIELD(Persisted)
	float m_dir = 0;

	EHL_FIELD(Persisted)
	float m_startSpeed = 0;

	EHL_FIELD(Persisted)
	Vector m_controlMins;

	EHL_FIELD(Persisted)
	Vector m_controlMaxs;

	EHL_FIELD(Persisted)
	bool m_soundPlaying = false;

	EHL_FIELD(Persisted)
	int m_sounds = 0;

	EHL_FIELD(Persisted, Type=SoundName)
	string_t m_iszMovingSound = iStringNull;

	EHL_FIELD(Persisted)
	float m_flVolume = 0;

	EHL_FIELD(Persisted)
	float m_flBank = 0;

	EHL_FIELD(Persisted)
	float m_oldSpeed = 0;

private:
	unsigned short m_usAdjustPitch = 0;
};
