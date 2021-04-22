//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

/**
*	@file
*
*	Shared weapons common function prototypes
*/

#include "Exports.h"

struct edict_t;
struct local_state_t;

/**
*	@brief Log debug messages to file ( appends )
*/
void			COM_Log(const char* pszFile, const char* fmt, ...);
bool CL_IsDead();

float			UTIL_SharedRandomFloat(unsigned int seed, float low, float high);
int				UTIL_SharedRandomLong(unsigned int seed, int low, int high);

/**
*	@brief Retrieve current predicted weapon animation
*/
int				HUD_GetWeaponAnim();

/**
*	@brief Change weapon model animation
*/
void			HUD_SendWeaponAnim(int iAnim, int body, int force);

/**
*	@brief Play a sound, if we are seeing this command for the first time
*/
void			HUD_PlaySound(const char* sound, float volume);

/**
*	@brief Directly queue up an event on the client
*/
void			HUD_PlaybackEvent(int flags, const edict_t* pInvoker, unsigned short eventindex, float delay, const float* origin, const float* angles,
	float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);
void			HUD_SetMaxSpeed(const edict_t* ed, float speed);
int				stub_PrecacheModel(const char* s);
int				stub_PrecacheSound(const char* s);
unsigned short	stub_PrecacheEvent(int type, const char* s);
const char* stub_NameForFunction(uint32 function);
void			stub_SetModel(edict_t* e, const char* m);


extern cvar_t* cl_lw;

/**
*	@brief g_runfuncs is true if this is the first time we've predicted a particular movement/firing command.
*	If it is true, then we should play events/sounds etc., otherwise, we just will be updating state info, but not firing events
*/
inline bool g_runfuncs = false;
extern Vector v_angles;
extern Vector v_client_aimangles;
extern float g_lastFOV;
extern local_state_t* g_finalstate;
