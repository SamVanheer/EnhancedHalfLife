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

class CScriptedSentence : public CBaseToggle
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Use(const UseInfo& info) override;
	void EXPORT FindThink();
	void EXPORT DelayThink();
	int	 ObjectCaps() override { return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	CBaseMonster* FindEntity();
	bool AcceptableSpeaker(CBaseMonster* pMonster);
	bool StartSentence(CBaseMonster* pTarget);


private:
	string_t m_iszSentence = iStringNull;	// string index for idle animation
	string_t m_iszEntity = iStringNull;		// entity that is wanted for this sentence
	float m_flRadius = 0;					// range to search
	float m_flDuration = 0;					// How long the sentence lasts
	float m_flRepeat = 0;					// repeat rate
	float m_flAttenuation = 0;
	float m_flVolume = 0;
	bool m_active = false;
	string_t m_iszListener = iStringNull;	// name of entity to look at while talking
};
