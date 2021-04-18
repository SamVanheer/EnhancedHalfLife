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

#include "extdll.h"
#include "util.h"
#include "studio.h"
#include "activity.h"
#include "animation.h"
#include "scriptevent.h"

#pragma warning( disable : 4244 )

bool ExtractBbox(void* pmodel, int sequence, Vector& mins, Vector& maxs)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr)
		return false;

	mstudioseqdesc_t* pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex);

	mins = pseqdesc[sequence].bbmin;
	maxs = pseqdesc[sequence].bbmax;

	return true;
}

int LookupActivity(void* pmodel, entvars_t* pev, int activity)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr)
		return 0;

	mstudioseqdesc_t* pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex);

	int weighttotal = 0;
	int seq = ACTIVITY_NOT_AVAILABLE;
	for (int i = 0; i < pstudiohdr->numseq; i++)
	{
		if (pseqdesc[i].activity == activity)
		{
			weighttotal += pseqdesc[i].actweight;
			if (!weighttotal || RANDOM_LONG(0, weighttotal - 1) < pseqdesc[i].actweight)
				seq = i;
		}
	}

	return seq;
}

int LookupActivityHeaviest(void* pmodel, entvars_t* pev, int activity)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr)
		return 0;

	mstudioseqdesc_t* pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex);

	int weight = 0;
	int seq = ACTIVITY_NOT_AVAILABLE;
	for (int i = 0; i < pstudiohdr->numseq; i++)
	{
		if (pseqdesc[i].activity == activity)
		{
			if (pseqdesc[i].actweight > weight)
			{
				weight = pseqdesc[i].actweight;
				seq = i;
			}
		}
	}

	return seq;
}

void GetEyePosition(void* pmodel, Vector& vecEyePosition)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;

	if (!pstudiohdr)
	{
		ALERT(at_console, "GetEyePosition() Can't get pstudiohdr ptr!\n");
		return;
	}

	vecEyePosition = pstudiohdr->eyeposition;
}

int LookupSequence(void* pmodel, const char* label)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr)
		return 0;

	mstudioseqdesc_t* pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex);

	for (int i = 0; i < pstudiohdr->numseq; i++)
	{
		if (stricmp(pseqdesc[i].label, label) == 0)
			return i;
	}

	return -1;
}

bool IsSoundEvent(int eventNumber)
{
	if (eventNumber == SCRIPT_EVENT_SOUND || eventNumber == SCRIPT_EVENT_SOUND_VOICE)
		return true;
	return false;
}

void SequencePrecache(void* pmodel, const char* pSequenceName)
{
	int index = LookupSequence(pmodel, pSequenceName);
	if (index >= 0)
	{
		studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;
		if (!pstudiohdr || index >= pstudiohdr->numseq)
			return;

		mstudioseqdesc_t* pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex) + index;
		mstudioevent_t* pevent = (mstudioevent_t*)((byte*)pstudiohdr + pseqdesc->eventindex);

		for (int i = 0; i < pseqdesc->numevents; i++)
		{
			// Don't send client-side events to the server AI
			if (pevent[i].event >= EVENT_CLIENT)
				continue;

			// UNDONE: Add a callback to check to see if a sound is precached yet and don't allocate a copy
			// of it's name if it is.
			if (IsSoundEvent(pevent[i].event))
			{
				if (!strlen(pevent[i].options))
				{
					ALERT(at_error, "Bad sound event %d in sequence %s :: %s (sound is \"%s\")\n", pevent[i].event, pstudiohdr->name, pSequenceName, pevent[i].options);
				}

				PRECACHE_SOUND(STRING(ALLOC_STRING(pevent[i].options)));
			}
		}
	}
}

void GetSequenceInfo(void* pmodel, entvars_t* pev, float& flFrameRate, float& flGroundSpeed)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr)
		return;

	if (pev->sequence >= pstudiohdr->numseq)
	{
		flFrameRate = 0.0;
		flGroundSpeed = 0.0;
		return;
	}

	mstudioseqdesc_t* pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	if (pseqdesc->numframes > 1)
	{
		flFrameRate = 256 * pseqdesc->fps / (pseqdesc->numframes - 1);
		flGroundSpeed = pseqdesc->linearmovement.Length();
		flGroundSpeed = flGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
	}
	else
	{
		flFrameRate = 256.0;
		flGroundSpeed = 0.0;
	}
}


int GetSequenceFlags(void* pmodel, entvars_t* pev)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr || pev->sequence >= pstudiohdr->numseq)
		return 0;

	mstudioseqdesc_t* pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	return pseqdesc->flags;
}


int GetAnimationEvent(void* pmodel, entvars_t* pev, MonsterEvent_t& monsterEvent, float flStart, float flEnd, int index)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr || pev->sequence >= pstudiohdr->numseq)
		return 0;

	mstudioseqdesc_t* pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;
	mstudioevent_t* pevent = (mstudioevent_t*)((byte*)pstudiohdr + pseqdesc->eventindex);

	if (pseqdesc->numevents == 0 || index > pseqdesc->numevents)
		return 0;

	if (pseqdesc->numframes > 1)
	{
		flStart *= (pseqdesc->numframes - 1) / 256.0;
		flEnd *= (pseqdesc->numframes - 1) / 256.0;
	}
	else
	{
		flStart = 0;
		flEnd = 1.0;
	}

	for (; index < pseqdesc->numevents; index++)
	{
		// Don't send client-side events to the server AI
		if (pevent[index].event >= EVENT_CLIENT)
			continue;

		if ((pevent[index].frame >= flStart && pevent[index].frame < flEnd) ||
			((pseqdesc->flags & STUDIO_LOOPING) && flEnd >= pseqdesc->numframes - 1 && pevent[index].frame < flEnd - pseqdesc->numframes + 1))
		{
			monsterEvent.event = pevent[index].event;
			monsterEvent.options = pevent[index].options;
			return index + 1;
		}
	}
	return 0;
}

float SetController(void* pmodel, entvars_t* pev, int iController, float flValue)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr)
		return flValue;

	mstudiobonecontroller_t* pbonecontroller = (mstudiobonecontroller_t*)((byte*)pstudiohdr + pstudiohdr->bonecontrollerindex);

	// find first controller that matches the index
	int i;
	for (i = 0; i < pstudiohdr->numbonecontrollers; i++, pbonecontroller++)
	{
		if (pbonecontroller->index == iController)
			break;
	}
	if (i >= pstudiohdr->numbonecontrollers)
		return flValue;

	// wrap 0..360 if it's a rotational controller

	if (pbonecontroller->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pbonecontroller->end < pbonecontroller->start)
			flValue = -flValue;

		// does the controller not wrap?
		if (pbonecontroller->start + 359.0 >= pbonecontroller->end)
		{
			if (flValue > ((pbonecontroller->start + pbonecontroller->end) / 2.0) + 180)
				flValue = flValue - 360;
			if (flValue < ((pbonecontroller->start + pbonecontroller->end) / 2.0) - 180)
				flValue = flValue + 360;
		}
		else
		{
			if (flValue > 360)
				flValue = flValue - (int)(flValue / 360.0) * 360.0;
			else if (flValue < 0)
				flValue = flValue + (int)((flValue / -360.0) + 1) * 360.0;
		}
	}

	int setting = 255 * (flValue - pbonecontroller->start) / (pbonecontroller->end - pbonecontroller->start);

	setting = std::clamp(setting, 0, 255);

	pev->controller[iController] = setting;

	return setting * (1.0 / 255.0) * (pbonecontroller->end - pbonecontroller->start) + pbonecontroller->start;
}


float SetBlending(void* pmodel, entvars_t* pev, int iBlender, float flValue)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr)
		return flValue;

	mstudioseqdesc_t* pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;

	if (pseqdesc->blendtype[iBlender] == 0)
		return flValue;

	if (pseqdesc->blendtype[iBlender] & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pseqdesc->blendend[iBlender] < pseqdesc->blendstart[iBlender])
			flValue = -flValue;

		// does the controller not wrap?
		if (pseqdesc->blendstart[iBlender] + 359.0 >= pseqdesc->blendend[iBlender])
		{
			if (flValue > ((pseqdesc->blendstart[iBlender] + pseqdesc->blendend[iBlender]) / 2.0) + 180)
				flValue = flValue - 360;
			if (flValue < ((pseqdesc->blendstart[iBlender] + pseqdesc->blendend[iBlender]) / 2.0) - 180)
				flValue = flValue + 360;
		}
	}

	int setting = 255 * (flValue - pseqdesc->blendstart[iBlender]) / (pseqdesc->blendend[iBlender] - pseqdesc->blendstart[iBlender]);

	setting = std::clamp(setting, 0, 255);

	pev->blending[iBlender] = setting;

	return setting * (1.0 / 255.0) * (pseqdesc->blendend[iBlender] - pseqdesc->blendstart[iBlender]) + pseqdesc->blendstart[iBlender];
}




int FindTransition(void* pmodel, int iEndingAnim, int iGoalAnim, int& iDir)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr)
		return iGoalAnim;

	mstudioseqdesc_t* pseqdesc;
	pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex);

	// bail if we're going to or from a node 0
	if (pseqdesc[iEndingAnim].entrynode == 0 || pseqdesc[iGoalAnim].entrynode == 0)
	{
		return iGoalAnim;
	}

	int	iEndNode;

	// ALERT( at_console, "from %d to %d: ", pEndNode->iEndNode, pGoalNode->iStartNode );

	if (iDir > 0)
	{
		iEndNode = pseqdesc[iEndingAnim].exitnode;
	}
	else
	{
		iEndNode = pseqdesc[iEndingAnim].entrynode;
	}

	if (iEndNode == pseqdesc[iGoalAnim].entrynode)
	{
		iDir = 1;
		return iGoalAnim;
	}

	byte* pTransition = ((byte*)pstudiohdr + pstudiohdr->transitionindex);

	const int iInternNode = pTransition[(iEndNode - 1) * pstudiohdr->numtransitions + (pseqdesc[iGoalAnim].entrynode - 1)];

	if (iInternNode == 0)
		return iGoalAnim;

	// look for someone going
	for (int i = 0; i < pstudiohdr->numseq; i++)
	{
		if (pseqdesc[i].entrynode == iEndNode && pseqdesc[i].exitnode == iInternNode)
		{
			iDir = 1;
			return i;
		}
		if (pseqdesc[i].nodeflags)
		{
			if (pseqdesc[i].exitnode == iEndNode && pseqdesc[i].entrynode == iInternNode)
			{
				iDir = -1;
				return i;
			}
		}
	}

	ALERT(at_console, "error in transition graph");
	return iGoalAnim;
}

void SetBodygroup(void* pmodel, entvars_t* pev, int iGroup, int iValue)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr)
		return;

	if (iGroup > pstudiohdr->numbodyparts)
		return;

	mstudiobodyparts_t* pbodypart = (mstudiobodyparts_t*)((byte*)pstudiohdr + pstudiohdr->bodypartindex) + iGroup;

	if (iValue >= pbodypart->nummodels)
		return;

	const int iCurrent = (pev->body / pbodypart->base) % pbodypart->nummodels;

	pev->body = (pev->body - (iCurrent * pbodypart->base) + (iValue * pbodypart->base));
}

int GetBodygroup(void* pmodel, entvars_t* pev, int iGroup)
{
	studiohdr_t* pstudiohdr = (studiohdr_t*)pmodel;
	if (!pstudiohdr)
		return 0;

	if (iGroup > pstudiohdr->numbodyparts)
		return 0;

	mstudiobodyparts_t* pbodypart = (mstudiobodyparts_t*)((byte*)pstudiohdr + pstudiohdr->bodypartindex) + iGroup;

	if (pbodypart->nummodels <= 1)
		return 0;

	const int iCurrent = (pev->body / pbodypart->base) % pbodypart->nummodels;

	return iCurrent;
}
