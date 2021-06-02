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

#include "CPointEntity.hpp"
#include "CPathTrack.generated.hpp"

// Spawnflag for CPathTrack
constexpr int SF_PATH_DISABLED = 0x00000001;
constexpr int SF_PATH_FIREONCE = 0x00000002;
constexpr int SF_PATH_ALTREVERSE = 0x00000004;
constexpr int SF_PATH_DISABLE_TRAIN = 0x00000008;
constexpr int SF_PATH_ALTERNATE = 0x00008000;

//#define PATH_SPARKLE_DEBUG		1	// This makes a particle effect around path_track entities for debugging
class EHL_CLASS(EntityName=path_track) CPathTrack : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void		Spawn() override;
	void		Activate() override;
	void		KeyValue(KeyValueData * pkvd) override;

	void		SetPrevious(CPathTrack * pprevious);
	void		Link();
	void		Use(const UseInfo & info) override;

	CPathTrack* ValidPath(CPathTrack * ppath, int testFlag);		// Returns ppath if enabled, nullptr otherwise
	void		Project(CPathTrack * pstart, CPathTrack * pend, Vector & origin, float dist);

	static CPathTrack* Instance(CBaseEntity * pent);

	CPathTrack* LookAhead(Vector & origin, float dist, int move);
	CPathTrack* Nearest(const Vector & origin);

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
