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

#include "CPointEntity.hpp"
#include "CScriptedSentence.generated.hpp"

enum class SentenceAttenuation
{
	SmallRadius,
	MediumRadius,
	LargeRadius,
	PlayEverywhere
};

constexpr int SF_SENTENCE_ONCE = 0x0001;
constexpr int SF_SENTENCE_FOLLOWERS = 0x0002;	//!< only say if following player
constexpr int SF_SENTENCE_INTERRUPT = 0x0004;	//!< force talking except when dead
constexpr int SF_SENTENCE_CONCURRENT = 0x0008;	//!< allow other people to keep talking

class EHL_CLASS("EntityName": "scripted_sentence") CScriptedSentence : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Use(const UseInfo& info) override;
	void EXPORT FindThink();
	void EXPORT DelayThink();

	CBaseMonster* FindEntity();
	bool AcceptableSpeaker(CBaseMonster* pMonster);
	bool StartSentence(CBaseMonster* pTarget);


private:
	EHL_FIELD("Persisted": true)
	string_t m_iszSentence;	// string index for idle animation

	EHL_FIELD("Persisted": true)
	string_t m_iszEntity;		// entity that is wanted for this sentence

	EHL_FIELD("Persisted": true)
	float m_flRadius = 0;					// range to search

	EHL_FIELD("Persisted": true)
	float m_flDuration = 0;					// How long the sentence lasts

	EHL_FIELD("Persisted": true)
	float m_flRepeat = 0;					// repeat rate

	EHL_FIELD("Persisted": true)
	float m_flAttenuation = 0;

	EHL_FIELD("Persisted": true)
	float m_flVolume = 0;

	EHL_FIELD("Persisted": true)
	bool m_active = false;

	EHL_FIELD("Persisted": true)
	string_t m_iszListener;	// name of entity to look at while talking
};
