/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#pragma once

#include "CBaseMonster.hpp"
#include "CGMan.generated.hpp"

/**
*	@brief misunderstood servant of the people
*/
class EHL_CLASS("EntityName": "monster_gman") CGMan : public CBaseMonster
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int  Classify() override;
	void HandleAnimEvent(AnimationEvent& event) override;
	int SoundMask() override;

	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;

	/**
	*	@brief Override all damage
	*/
	bool TakeDamage(const TakeDamageInfo& info) override;
	void TraceAttack(const TraceAttackInfo& info) override;

	void PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, bool bConcurrent, CBaseEntity* pListener) override;

	EHANDLE m_hPlayer;

	EHL_FIELD("Persisted": true)
	EHANDLE m_hTalkTarget;

	EHL_FIELD("Persisted": true, "Type": "Time")
	float m_flTalkTime = 0;
};
