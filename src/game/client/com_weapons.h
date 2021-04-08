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

void			COM_Log( const char *pszFile, const char *fmt, ...);
int				CL_IsDead();

float			UTIL_SharedRandomFloat( unsigned int seed, float low, float high );
int				UTIL_SharedRandomLong( unsigned int seed, int low, int high );

int				HUD_GetWeaponAnim();
void			HUD_SendWeaponAnim( int iAnim, int body, int force );
void			HUD_PlaySound( const char *sound, float volume );
void			HUD_PlaybackEvent( int flags, const edict_t* pInvoker, unsigned short eventindex, float delay, const float *origin, const float *angles,
	float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
void			HUD_SetMaxSpeed( const edict_t* ed, float speed );
int				stub_PrecacheModel( const char* s );
int				stub_PrecacheSound( const char* s );
unsigned short	stub_PrecacheEvent( int type, const char *s );
const char		*stub_NameForFunction	( uint32 function );
void			stub_SetModel			( edict_t* e, const char *m );


extern cvar_t *cl_lw;

extern int g_runfuncs;
extern Vector v_angles;
extern Vector v_client_aimangles;
extern float g_lastFOV;
extern local_state_t *g_finalstate;
