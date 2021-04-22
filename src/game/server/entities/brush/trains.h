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
	void		Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	CPathTrack* ValidPath(CPathTrack* ppath, int testFlag);		// Returns ppath if enabled, nullptr otherwise
	void		Project(CPathTrack* pstart, CPathTrack* pend, Vector* origin, float dist);

	static CPathTrack* Instance(edict_t* pent);

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
	CPathTrack* m_pnext;
	CPathTrack* m_pprevious;
	CPathTrack* m_paltpath;
};

class CFuncTrackTrain : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;

	void Blocked(CBaseEntity* pOther) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void KeyValue(KeyValueData* pkvd) override;

	void EXPORT Next();
	void EXPORT Find();
	void EXPORT NearestPath();
	void EXPORT DeadEnd();

	void		NextThink(float thinkTime, bool alwaysThink);

	void SetTrack(CPathTrack* track) { m_ppath = track->Nearest(pev->origin); }
	void SetControls(entvars_t* pevControls);
	bool OnControls(entvars_t* pev) override;

	void StopSound();

	/**
	*	@brief update pitch based on speed, start sound if not playing
	*	NOTE: when train goes through transition, m_soundPlaying should go to false, 
	*	which will cause the looped sound to restart.
	*/
	void UpdateSound();

	static CFuncTrackTrain* Instance(edict_t* pent);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];
	int	ObjectCaps() override { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DIRECTIONAL_USE; }

	void	OverrideReset() override;

	CPathTrack* m_ppath;
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
