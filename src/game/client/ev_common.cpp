/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

/**
*	@file
*
*	shared event functions
*/

#include "entity_state.h"
#include "cl_entity.h"

#include "r_efx.h"

#include "eventscripts.h"
#include "event_api.h"
#include "pm_shared.h"

bool IsFirstPersonSpectator()
{
	return g_iUser1 == OBS_IN_EYE || (g_iUser1 && (gHUD.m_Spectator.m_pip->value == INSET_IN_EYE));
}

cl_entity_t* GetEntity(int idx)
{
	return gEngfuncs.GetEntityByIndex(idx);
}

cl_entity_t* GetViewEntity()
{
	return gEngfuncs.GetViewModel();
}

void EV_CreateTracer(Vector start, const Vector& end)
{
	//start is modified by this function
	gEngfuncs.pEfxAPI->R_TracerEffect(start, end);
}

bool EV_IsPlayer(int idx)
{
	if (idx >= 1 && idx <= gEngfuncs.GetMaxClients())
		return true;

	return false;
}

bool EV_IsLocal(int idx)
{
	// check if we are in some way in first person spec mode
	if (IsFirstPersonSpectator())
		return (g_iUser2 == idx);
	else
		return gEngfuncs.pEventAPI->EV_IsLocal(idx - 1) != 0;
}

void EV_GetGunPosition(event_args_t* args, Vector& pos, const Vector& origin)
{
	const int idx = args->entindex;

	Vector view_ofs = VEC_VIEW;

	if (EV_IsPlayer(idx))
	{
		// in spec mode use entity viewheigh, not own
		if (EV_IsLocal(idx) && !IsFirstPersonSpectator())
		{
			// Grab predicted result for local player
			gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);
		}
		else if (args->ducking == 1)
		{
			view_ofs = VEC_DUCK_VIEW;
		}
	}

	pos = origin + view_ofs;
}

void EV_EjectBrass(const Vector& origin, const Vector& velocity, float rotation, int model, int soundtype)
{
	const Vector endpos{0, rotation, 0};
	gEngfuncs.pEfxAPI->R_TempModel(origin, velocity, endpos, 2.5, model, soundtype);
}

void EV_GetDefaultShellInfo(event_args_t* args,
	const Vector& origin, const Vector& velocity,
	Vector& ShellVelocity, Vector& ShellOrigin,
	const Vector& forward, const Vector& right, const Vector& up,
	float forwardScale, float upScale, float rightScale)
{
	const int idx = args->entindex;

	Vector view_ofs = VEC_VIEW;

	if (EV_IsPlayer(idx))
	{
		if (EV_IsLocal(idx))
		{
			gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);
		}
		else if (args->ducking == 1)
		{
			view_ofs = VEC_DUCK_VIEW;
		}
	}

	const float fR = gEngfuncs.pfnRandomFloat(50, 70);
	const float fU = gEngfuncs.pfnRandomFloat(100, 150);

	ShellVelocity = velocity + right * fR + up * fU + forward * 25;
	ShellOrigin = origin + view_ofs + up * upScale + forward * forwardScale + right * rightScale;
}

void EV_MuzzleFlash()
{
	// Add muzzle flash to current weapon model
	cl_entity_t* ent = GetViewEntity();
	if (!ent)
	{
		return;
	}

	// Or in the muzzle flash
	ent->curstate.effects |= EF_MUZZLEFLASH;
}
