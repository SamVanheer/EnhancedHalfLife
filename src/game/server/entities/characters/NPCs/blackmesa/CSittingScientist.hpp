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

#include "CScientist.hpp"

/**
*	@brief kdb: changed from public CBaseMonster so he can speak
*/
class EHL_CLASS() CSittingScientist : public CScientist
{
public:
	void Spawn() override;
	void  Precache() override;

	/**
	*	@brief sit, do stuff
	*/
	void EXPORT SittingThink();

	/**
	*	@brief ID as a passive human
	*/
	int	Classify() override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	/**
	*	@brief prepare sitting scientist to answer a question
	*/
	void SetAnswerQuestion(CTalkMonster* pSpeaker) override;
	int FriendNumber(int arrayNumber) override;

	/**
	*	@brief ask question of nearby friend, or make statement
	*/
	int IdleSpeak();
	int m_baseSequence = 0;
	int m_headTurn = 0;
	float m_flResponseDelay = 0;
};
