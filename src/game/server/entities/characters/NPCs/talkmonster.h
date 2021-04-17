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

#include "monsters.h"

//=========================================================
// Talking monster base class
// Used for scientists and barneys
//=========================================================

constexpr float TALKRANGE_MIN = 500.0;			// don't talk to anyone farther away than this

constexpr int TLK_STARE_DIST = 128;				// anyone closer than this and looking at me is probably staring at me.

constexpr int bit_saidDamageLight = 1 << 0;	// bits so we don't repeat key sentences
constexpr int bit_saidDamageMedium = 1 << 1;
constexpr int bit_saidDamageHeavy = 1 << 2;
constexpr int bit_saidHelloPlayer = 1 << 3;
constexpr int bit_saidWoundLight = 1 << 4;
constexpr int bit_saidWoundHeavy = 1 << 5;
constexpr int bit_saidHeard = 1 << 6;
constexpr int bit_saidSmelled = 1 << 7;

constexpr int TLK_CFRIENDS = 3;

enum TALKGROUPNAMES
{
	TLK_ANSWER = 0,
	TLK_QUESTION,
	TLK_IDLE,
	TLK_STARE,
	TLK_USE,
	TLK_UNUSE,
	TLK_STOP,
	TLK_NOSHOOT,
	TLK_HELLO,
	TLK_PHELLO,
	TLK_PIDLE,
	TLK_PQUESTION,
	TLK_PLHURT1,
	TLK_PLHURT2,
	TLK_PLHURT3,
	TLK_SMELL,
	TLK_WOUND,
	TLK_MORTAL,

	TLK_CGROUPS,					// MUST be last entry
};


enum
{
	SCHED_CANT_FOLLOW = LAST_COMMON_SCHEDULE + 1,
	SCHED_MOVE_AWAY,		// Try to get out of the player's way
	SCHED_MOVE_AWAY_FOLLOW,	// same, but follow afterward
	SCHED_MOVE_AWAY_FAIL,	// Turn back toward player

	LAST_TALKMONSTER_SCHEDULE,		// MUST be last
};

enum
{
	TASK_CANT_FOLLOW = LAST_COMMON_TASK + 1,
	TASK_MOVE_AWAY_PATH,
	TASK_WALK_PATH_FOR_UNITS,

	TASK_TLK_RESPOND,		// say my response
	TASK_TLK_SPEAK,			// question or remark
	TASK_TLK_HELLO,			// Try to say hello to player
	TASK_TLK_HEADRESET,		// reset head position
	TASK_TLK_STOPSHOOTING,	// tell player to stop shooting friend
	TASK_TLK_STARE,			// let the player know I know he's staring at me.
	TASK_TLK_LOOK_AT_CLIENT,// faces player if not moving and not talking and in idle.
	TASK_TLK_CLIENT_STARE,	// same as look at client, but says something if the player stares.
	TASK_TLK_EYECONTACT,	// maintain eyecontact with person who I'm talking to
	TASK_TLK_IDEALYAW,		// set ideal yaw to face who I'm talking to
	TASK_FACE_PLAYER,		// Face the player

	LAST_TALKMONSTER_TASK,			// MUST be last
};

/**
*	@brief Talking monster base class. Used for scientists and barneys
*/
class CTalkMonster : public CBaseMonster
{
public:
	void			TalkInit();
	CBaseEntity* FindNearestFriend(bool fPlayer);
	float			TargetDistance();
	void			StopTalking() { SentenceStop(); }

	// Base Monster functions
	void			Precache() override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	void			Touch(CBaseEntity* pOther) override;
	void			Killed(entvars_t* pevAttacker, int iGib) override;
	int				IRelationship(CBaseEntity* pTarget) override;
	bool CanPlaySentence(bool fDisregardState) override;
	void	PlaySentence(const char* pszSentence, float duration, float volume, float attenuation) override;
	void			PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, bool bConcurrent, CBaseEntity* pListener) override;
	void			KeyValue(KeyValueData* pkvd) override;

	// AI functions
	void			SetActivity(Activity newActivity) override;
	Schedule_t* GetScheduleOfType(int Type) override;
	void			StartTask(Task_t* pTask) override;
	void			RunTask(Task_t* pTask) override;
	void			HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void			PrescheduleThink() override;


	// Conversations / communication
	int				GetVoicePitch();
	void			IdleRespond();
	bool FIdleSpeak();
	bool FIdleStare();
	bool FIdleHello();
	void			IdleHeadTurn(Vector& vecFriend);
	bool FOkToSpeak();
	void			TrySmellTalk();
	CBaseEntity* EnumFriends(CBaseEntity* pentPrevious, int listNumber, bool bTrace);
	void			AlertFriends();
	void			ShutUpFriends();
	bool			IsTalking();
	void			Talk(float flDuration);
	// For following
	bool			CanFollow();
	bool			IsFollowing() { return m_hTargetEnt != nullptr && m_hTargetEnt->IsPlayer(); }
	void			StopFollowing(bool clearSchedule) override;
	void			StartFollowing(CBaseEntity* pLeader);
	virtual void	DeclineFollowing() {}
	void			LimitFollowers(CBaseEntity* pPlayer, int maxFollowers);

	void EXPORT		FollowerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	virtual void	SetAnswerQuestion(CTalkMonster* pSpeaker);
	virtual int		FriendNumber(int arrayNumber) { return arrayNumber; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];


	static const char* m_szFriends[TLK_CFRIENDS];		// array of friend names
	static float g_talkWaitTime;

	int			m_bitsSaid;						// set bits for sentences we don't want repeated
	int			m_nSpeak;						// number of times initiated talking
	int			m_voicePitch;					// pitch of voice for this head
	const char* m_szGrp[TLK_CGROUPS];			// sentence group names
	float		m_useTime;						// Don't allow +USE until this time
	string_t m_iszUse;							// Custom +USE sentence group (follow)
	string_t m_iszUnUse;						// Custom +USE sentence group (stop following)

	float		m_flLastSaidSmelled;// last time we talked about something that stinks
	float		m_flStopTalkTime;// when in the future that I'll be done saying this sentence.

	EHANDLE		m_hTalkTarget;	// who to look at while talking
	CUSTOM_SCHEDULES;
};


// Clients can push talkmonsters out of their way
constexpr int bits_COND_CLIENT_PUSH = bits_COND_SPECIAL1;
// Don't see a client right now.
constexpr int bits_COND_CLIENT_UNSEEN = bits_COND_SPECIAL2;
