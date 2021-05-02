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
*	Shared weapons common/shared functions
*/

#include "com_weapons.h"

#include "entity_state.h"
#include "r_efx.h"

/**
*	@brief During our weapon prediction processing,
*	we'll need to reference some data that is part of the final state passed into the postthink functionality.
*	We'll set this pointer and then reset it to nullptr as appropriate
*/
local_state_t* g_finalstate = nullptr;

/**
*	@brief remember the current animation for the view model, in case we get out of sync with server.
*/
static int g_currentanim;

void HUD_SendWeaponAnim(int iAnim, int body, bool force)
{
	// Don't actually change it.
	if (!g_runfuncs && !force)
		return;

	g_currentanim = iAnim;

	// Tell animation system new info
	gEngfuncs.pfnWeaponAnim(iAnim, body);
}

int HUD_GetWeaponAnim()
{
	return g_currentanim;
}

void HUD_PlaySound(const char* sound, float volume)
{
	if (!g_runfuncs || !g_finalstate)
		return;

	gEngfuncs.pfnPlaySoundByNameAtLocation(sound, volume, g_finalstate->playerstate.origin);
}

void HUD_PlaybackEvent(int flags, const edict_t* pInvoker, unsigned short eventindex, float delay,
	const float* origin, const float* angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2)
{
	if (!g_runfuncs || !g_finalstate)
		return;

	// Weapon prediction events are assumed to occur at the player's origin
	const Vector org = g_finalstate->playerstate.origin;
	const Vector ang = v_client_aimangles;
	gEngfuncs.pfnPlaybackEvent(flags, pInvoker, eventindex, delay, org, ang, fparam1, fparam2, iparam1, iparam2, bparam1, bparam2);
}

void HUD_SetMaxSpeed(const edict_t* ed, float speed)
{
}

float UTIL_WeaponTimeBase()
{
	return 0.0;
}

//stub functions for such things as precaching.
//So we don't have to modify weapons code that is compiled into both gameand client.dlls.
int				stub_PrecacheModel(const char* s) { return 0; }
int				stub_PrecacheSound(const char* s) { return 0; }
unsigned short	stub_PrecacheEvent(int type, const char* s) { return 0; }
const char* stub_NameForFunction(uint32 function) { return "func"; }
void			stub_SetModel(edict_t* e, const char* m) {}
