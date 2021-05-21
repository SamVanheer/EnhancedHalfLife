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

#include "CSittingScientist.hpp"

LINK_ENTITY_TO_CLASS(monster_sitting_scientist, CSittingScientist);

// animation sequence aliases 
enum SITTING_ANIM
{
	SITTING_ANIM_sitlookleft,
	SITTING_ANIM_sitlookright,
	SITTING_ANIM_sitscared,
	SITTING_ANIM_sitting2,
	SITTING_ANIM_sitting3
};

void CSittingScientist::Spawn()
{
	PRECACHE_MODEL("models/scientist.mdl");
	SetModel("models/scientist.mdl");
	Precache();
	InitBoneControllers();

	SetSize(Vector(-14, -14, 0), Vector(14, 14, 36));

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Step);
	pev->effects = 0;
	pev->health = 50;

	m_bloodColor = BLOOD_COLOR_RED;
	m_flFieldOfView = VIEW_FIELD_WIDE; // indicates the width of this monster's forward view cone ( as a dotproduct result )

	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD;

	SetBits(pev->spawnflags, SF_MONSTER_PREDISASTER); // predisaster only!

	if (pev->body == -1)
	{// -1 chooses a random head
		pev->body = RANDOM_LONG(0, NUM_SCIENTIST_HEADS - 1);// pick a head, any head
	}

	//Remap old bodies to new ones
	SetBodygroup(SCIENTIST_BODYGROUP_HEAD, pev->body);

	// Luther is black, make his hands black
	if (GetBodygroup(SCIENTIST_BODYGROUP_HEAD) == SCIENTIST_HEAD_LUTHER)
		pev->skin = 1;

	m_baseSequence = LookupSequence("sitlookleft");
	pev->sequence = m_baseSequence + RANDOM_LONG(0, 4);
	ResetSequenceInfo();

	SetThink(&CSittingScientist::SittingThink);
	pev->nextthink = gpGlobals->time + 0.1;

	DROP_TO_FLOOR(edict());
}

void CSittingScientist::Precache()
{
	m_baseSequence = LookupSequence("sitlookleft");
	TalkInit();
}

int	CSittingScientist::Classify()
{
	return CLASS_HUMAN_PASSIVE;
}

int CSittingScientist::FriendNumber(int arrayNumber)
{
	static constexpr int array[3] = {2, 1, 0};
	if (arrayNumber < 3)
		return array[arrayNumber];
	return arrayNumber;
}

void CSittingScientist::SittingThink()
{
	StudioFrameAdvance();

	// try to greet player
	if (IdleHello())
	{
		CBaseEntity* pent = FindNearestFriend(true);
		if (pent)
		{
			float yaw = VecToYaw(pent->GetAbsOrigin() - GetAbsOrigin()) - GetAbsAngles().y;

			if (yaw > 180) yaw -= 360;
			if (yaw < -180) yaw += 360;

			if (yaw > 0)
				pev->sequence = m_baseSequence + SITTING_ANIM_sitlookleft;
			else
				pev->sequence = m_baseSequence + SITTING_ANIM_sitlookright;

			ResetSequenceInfo();
			pev->frame = 0;
			SetBoneController(0, 0);
		}
	}
	else if (m_fSequenceFinished)
	{
		int i = RANDOM_LONG(0, 99);
		m_headTurn = 0;

		if (m_flResponseDelay && gpGlobals->time > m_flResponseDelay)
		{
			// respond to question
			IdleRespond();
			pev->sequence = m_baseSequence + SITTING_ANIM_sitscared;
			m_flResponseDelay = 0;
		}
		else if (i < 30)
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitting3;

			// turn towards player or nearest friend and speak

			CBaseEntity* pent;
			if (!IsBitSet(m_bitsSaid, bit_saidHelloPlayer))
				pent = FindNearestFriend(true);
			else
				pent = FindNearestFriend(false);

			if (!IdleSpeak() || !pent)
			{
				m_headTurn = RANDOM_LONG(0, 8) * 10 - 40;
				pev->sequence = m_baseSequence + SITTING_ANIM_sitting3;
			}
			else
			{
				// only turn head if we spoke
				float yaw = VecToYaw(pent->GetAbsOrigin() - GetAbsOrigin()) - GetAbsAngles().y;

				//TODO: this keeps showing up everywhere
				if (yaw > 180) yaw -= 360;
				if (yaw < -180) yaw += 360;

				if (yaw > 0)
					pev->sequence = m_baseSequence + SITTING_ANIM_sitlookleft;
				else
					pev->sequence = m_baseSequence + SITTING_ANIM_sitlookright;

				//ALERT(at_console, "sitting speak\n");
			}
		}
		else if (i < 60)
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitting3;
			m_headTurn = RANDOM_LONG(0, 8) * 10 - 40;
			if (RANDOM_LONG(0, 99) < 5)
			{
				//ALERT(at_console, "sitting speak2\n");
				IdleSpeak();
			}
		}
		else if (i < 80)
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitting2;
		}
		else if (i < 100)
		{
			pev->sequence = m_baseSequence + SITTING_ANIM_sitscared;
		}

		ResetSequenceInfo();
		pev->frame = 0;
		SetBoneController(0, m_headTurn);
	}
	pev->nextthink = gpGlobals->time + 0.1;
}

void CSittingScientist::SetAnswerQuestion(CTalkMonster* pSpeaker)
{
	m_flResponseDelay = gpGlobals->time + RANDOM_FLOAT(3, 4);
	m_hTalkTarget = (CBaseMonster*)pSpeaker;
}

int CSittingScientist::IdleSpeak()
{
	// try to start a conversation, or make statement
	if (!OkToSpeak())
		return false;

	// set global min delay for next conversation
	CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(4.8, 5.2);

	const int pitch = GetVoicePitch();

	// if there is a friend nearby to speak to, play sentence, set friend's response time, return

	// try to talk to any standing or sitting scientists nearby
	if (CBaseEntity* pentFriend = FindNearestFriend(false); pentFriend && RANDOM_LONG(0, 1))
	{
		CTalkMonster* pTalkMonster = GetClassPtr((CTalkMonster*)pentFriend->pev);
		pTalkMonster->SetAnswerQuestion(this);

		IdleHeadTurn(pentFriend->GetAbsOrigin());
		SENTENCEG_PlayRndSz(this, m_szGrp[TLK_PQUESTION], 1.0, ATTN_IDLE, pitch);
		// set global min delay for next conversation
		CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(4.8, 5.2);
		return true;
	}

	// otherwise, play an idle statement
	if (RANDOM_LONG(0, 1))
	{
		SENTENCEG_PlayRndSz(this, m_szGrp[TLK_PIDLE], 1.0, ATTN_IDLE, pitch);
		// set global min delay for next conversation
		CTalkMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(4.8, 5.2);
		return true;
	}

	// never spoke
	CTalkMonster::g_talkWaitTime = 0;
	return false;
}
