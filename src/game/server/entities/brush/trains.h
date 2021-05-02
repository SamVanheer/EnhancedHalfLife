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
class CPathTrack : public CPointEntity
{
public:
	void		Spawn() override;
	void		Activate() override;
	void		KeyValue(KeyValueData* pkvd) override;

	void		SetPrevious(CPathTrack* pprevious);
	void		Link();
	void		Use(const UseInfo& info) override;

	CPathTrack* ValidPath(CPathTrack* ppath, int testFlag);		// Returns ppath if enabled, nullptr otherwise
	void		Project(CPathTrack* pstart, CPathTrack* pend, Vector* origin, float dist);

	static CPathTrack* Instance(CBaseEntity* pent);

	CPathTrack* LookAhead(Vector* origin, float dist, int move);
	CPathTrack* Nearest(Vector origin);

	CPathTrack* GetNext();
	CPathTrack* GetPrevious();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];
#if PATH_SPARKLE_DEBUG
	void EXPORT Sparkle();
#endif

	float		m_length;
	string_t	m_altName;
	EHandle<CPathTrack> m_hNext;
	EHandle<CPathTrack> m_hPrevious;
	EHandle<CPathTrack> m_hAltPath;
};

class CFuncTrackTrain : public CBaseEntity
{
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

	void SetTrack(CPathTrack* track) { m_hPath = track->Nearest(pev->origin); }
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

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];
	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DIRECTIONAL_USE; }

	void	OverrideReset() override;

	EHandle<CPathTrack> m_hPath;
	float		m_length;
	float		m_height;
	float		m_speed;
	float		m_dir;
	float		m_startSpeed;
	Vector		m_controlMins;
	Vector		m_controlMaxs;
	bool m_soundPlaying;
	int			m_sounds;
	float		m_flVolume;
	float		m_flBank;
	float		m_oldSpeed;

private:
	unsigned short m_usAdjustPitch;
};
