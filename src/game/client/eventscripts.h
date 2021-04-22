//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

struct cl_entity_t;
struct event_args_t;

// Some of these are HL/TFC specific?
/**
*	@brief Bullet shell casings
*/
void EV_EjectBrass(const Vector& origin, const Vector& elocity, float rotation, int model, int soundtype);

/**
*	@brief Figure out the height of the gun
*/
void EV_GetGunPosition(event_args_t* args, Vector& pos, const Vector& origin);

/**
*	@brief Determine where to eject shells from
*/
void EV_GetDefaultShellInfo(event_args_t* args,
	const Vector& origin, const Vector& velocity,
	Vector& ShellVelocity, Vector& ShellOrigin,
	const Vector& forward, const Vector& right, const Vector& up,
	float forwardScale, float upScale, float rightScale);

/**
*	@brief Is the entity == the local player
*/
bool EV_IsLocal(int idx);

/**
*	@brief Is the entity's index in the player range?
*/
bool EV_IsPlayer(int idx);
void EV_CreateTracer(Vector start, const Vector& end);

bool IsFirstPersonSpectator();
cl_entity_t* GetEntity(int idx);

/**
*	@brief Returns the current weapon/view model
*/
cl_entity_t* GetViewEntity();

/**
*	@brief Flag weapon/view model for muzzle flash
*/
void EV_MuzzleFlash();
