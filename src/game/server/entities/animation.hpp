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

#include "animationevent.hpp"

class CBaseEntity;
class Vector;

constexpr int ACTIVITY_NOT_AVAILABLE = -1;

bool IsSoundEvent(int eventNumber);

int LookupActivity(void* pmodel, int activity);
int LookupActivityHeaviest(void* pmodel, int activity);
int LookupSequence(void* pmodel, const char* label);
void GetSequenceInfo(void* pmodel, CBaseEntity* entity, float& flFrameRate, float& flGroundSpeed);
int GetSequenceFlags(void* pmodel, CBaseEntity* entity);
float SetController(void* pmodel, CBaseEntity* entity, int iController, float flValue);
float SetBlending(void* pmodel, CBaseEntity* entity, int iBlender, float flValue);
void GetEyePosition(void* pmodel, Vector& vecEyePosition);
void SequencePrecache(void* pmodel, const char* pSequenceName);
int FindTransition(void* pmodel, int iEndingAnim, int iGoalAnim, int& iDir);
void SetBodygroup(void* pmodel, CBaseEntity* entity, int iGroup, int iValue);
int GetBodygroup(void* pmodel, CBaseEntity* entity, int iGroup);

int GetAnimationEvent(void* pmodel, CBaseEntity* entity, AnimationEvent& animationEvent, float flStart, float flEnd, int index);
bool ExtractBbox(void* pmodel, int sequence, Vector& mins, Vector& maxs);
