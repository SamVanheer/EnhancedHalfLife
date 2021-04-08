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

#pragma once

struct playermove_t;

void PM_Init(playermove_t* ppmove );
void PM_Move (playermove_t* ppmove, int server );
char PM_FindTextureType( const char *name );

int PM_GetVisEntInfo(int ent);
int PM_GetPhysEntInfo(int ent);

// Spectator Movement modes (stored in pev->iuser1, so the physics code can get at them)
constexpr int OBS_NONE = 0;
constexpr int OBS_CHASE_LOCKED = 1;
constexpr int OBS_CHASE_FREE = 2;
constexpr int OBS_ROAMING = 3;
constexpr int OBS_IN_EYE = 4;
constexpr int OBS_MAP_FREE = 5;
constexpr int OBS_MAP_CHASE = 6;

inline playermove_t* pmove = nullptr;

#ifdef CLIENT_DLL
// Spectator Mode
inline bool		iJumpSpectator;
inline Vector	vJumpOrigin;
inline Vector	vJumpAngles;
#endif
