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
#include "CPointEntity.hpp"

#include "CPathTrack.generated.hpp"

// Trains
constexpr int SF_TRAIN_WAIT_RETRIGGER = 1;
constexpr int SF_TRAIN_START_ON = 4;		//!< Train is initially moving
constexpr int SF_TRAIN_PASSABLE = 8;		//!< Train is not solid -- used to make water trains

// Tracktrain spawn flags
constexpr int SF_TRACKTRAIN_NOPITCH = 0x0001;
constexpr int SF_TRACKTRAIN_NOCONTROL = 0x0002;
constexpr int SF_TRACKTRAIN_FORWARDONLY = 0x0004;
constexpr int SF_TRACKTRAIN_PASSABLE = 0x0008;

// Spawnflag for CPathTrack
constexpr int SF_PATH_DISABLED = 0x00000001;
constexpr int SF_PATH_FIREONCE = 0x00000002;
constexpr int SF_PATH_ALTREVERSE = 0x00000004;
constexpr int SF_PATH_DISABLE_TRAIN = 0x00000008;
constexpr int SF_PATH_ALTERNATE = 0x00008000;

// Spawnflags of CPathCorner
constexpr int SF_CORNER_WAITFORTRIG = 0x001;
constexpr int SF_CORNER_TELEPORT = 0x002;
constexpr int SF_CORNER_FIREONCE = 0x004;

//#define PATH_SPARKLE_DEBUG		1	// This makes a particle effect around path_track entities for debugging
class EHL_CLASS() CPathTrack : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void		Spawn() override;
	void		Activate() override;
	void		KeyValue(KeyValueData* pkvd) override;

	void		SetPrevious(CPathTrack* pprevious);
	void		Link();
	void		Use(const UseInfo& info) override;

	CPathTrack* ValidPath(CPathTrack* ppath, int testFlag);		// Returns ppath if enabled, nullptr otherwise
	void		Project(CPathTrack* pstart, CPathTrack* pend, Vector& origin, float dist);

	static CPathTrack* Instance(CBaseEntity* pent);

	CPathTrack* LookAhead(Vector& origin, float dist, int move);
	CPathTrack* Nearest(const Vector& origin);

	CPathTrack* GetNext();
	CPathTrack* GetPrevious();

#if PATH_SPARKLE_DEBUG
	void EXPORT Sparkle();
#endif

	EHL_FIELD(Persisted)
	float m_length = 0;

	EHL_FIELD(Persisted)
	string_t m_altName = iStringNull;

	EHL_FIELD(Persisted)
	EHandle<CPathTrack> m_hNext;

	EHL_FIELD(Persisted)
	EHandle<CPathTrack> m_hPrevious;

	EHL_FIELD(Persisted)
	EHandle<CPathTrack> m_hAltPath;
};

#include "CFuncTrackTrain.generated.hpp"

class EHL_CLASS() CFuncTrackTrain : public CBaseEntity
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

	EHL_FIELD(Persisted)
	float m_flVolume = 0;

	EHL_FIELD(Persisted)
	float m_flBank = 0;

	EHL_FIELD(Persisted)
	float m_oldSpeed = 0;

private:
	unsigned short m_usAdjustPitch = 0;
};
