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

#include <cassert>

#include "Platform.hpp"

#include "mathlib.hpp"
#include "cdll_dll.hpp"
#include "const.hpp"
#include "usercmd.hpp"
#include "pm_defs.hpp"
#include "pm_shared.hpp"
#include "pm_movevars.hpp"
#include "pm_debug.hpp"
#include "sound/materials.hpp"
#include "com_model.hpp"
#include "string_utils.hpp"
#include "view_utils.hpp"

#pragma warning( disable : 4305 )
// double to float warning
#pragma warning(disable : 4244)

// Ducking time
constexpr double TIME_TO_DUCK = 0.4;
constexpr int STUCK_MOVEUP = 1;
constexpr int STUCK_MOVEDOWN = -1;
constexpr double STOP_EPSILON = 0.1;

enum class StepType
{
	Concrete = 0,	//!< default step sound
	Metal,			//!< metal floor
	Dirt,			//!< dirt, sand, rock
	Vent,			//!< ventillation duct
	Grate,			//!< metal grating
	Tile,			//!< floor tiles
	Slosh,			//!< shallow liquid puddle
	Wade,			//!< wading in liquid
	Ladder			//!< climbing ladder
};

constexpr Vector current_table[] =
{
	vec3_forward, vec3_right, vec3_backward,
	vec3_left, vec3_up, vec3_down
};

static bool pm_shared_initialized = false;

static Vector rgv3tStuckTable[54];
static int rgStuckLast[MAX_CLIENTS][2];

bool g_onladder = false;

void PM_PlayStepSound(StepType step, float fvol)
{
	pmove->iStepLeft = pmove->iStepLeft == 0;

	if (!pmove->runfuncs)
	{
		return;
	}

	int irand = pmove->RandomLong(0, 1) + (pmove->iStepLeft * 2);

	// FIXME mp_footsteps needs to be a movevar
	if (pmove->multiplayer && !pmove->movevars->footsteps)
		return;

	Vector hvel = pmove->velocity;
	hvel[2] = 0.0;

	if (pmove->multiplayer && (!g_onladder && hvel.Length() <= 220))
		return;

	// irand - 0,1 for right foot, 2,3 for left foot
	// used to alternate left and right foot
	// FIXME, move to player state

	switch (step)
	{
	default:
	case StepType::Concrete:
		switch (irand)
		{
			// right foot
		case 0:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_step1.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 1:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_step3.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
			// left foot
		case 2:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_step2.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 3:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_step4.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		}
		break;
	case StepType::Metal:
		switch (irand)
		{
			// right foot
		case 0:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_metal1.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 1:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_metal3.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
			// left foot
		case 2:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_metal2.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 3:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_metal4.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		}
		break;
	case StepType::Dirt:
		switch (irand)
		{
			// right foot
		case 0:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_dirt1.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 1:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_dirt3.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
			// left foot
		case 2:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_dirt2.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 3:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_dirt4.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		}
		break;
	case StepType::Vent:
		switch (irand)
		{
			// right foot
		case 0:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_duct1.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 1:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_duct3.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
			// left foot
		case 2:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_duct2.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 3:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_duct4.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		}
		break;
	case StepType::Grate:
		switch (irand)
		{
			// right foot
		case 0:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_grate1.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 1:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_grate3.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
			// left foot
		case 2:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_grate2.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 3:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_grate4.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		}
		break;
	case StepType::Tile:
		if (!pmove->RandomLong(0, 4))
			irand = 4;
		switch (irand)
		{
			// right foot
		case 0:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_tile1.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 1:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_tile3.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
			// left foot
		case 2:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_tile2.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 3:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_tile4.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 4: pmove->PM_PlaySound(SoundChannel::Body, "player/pl_tile5.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		}
		break;
	case StepType::Slosh:
		switch (irand)
		{
			// right foot
		case 0:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_slosh1.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 1:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_slosh3.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
			// left foot
		case 2:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_slosh2.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 3:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_slosh4.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		}
		break;
	case StepType::Wade:
	{
		static int iSkipStep = 0;
		if (iSkipStep == 0)
		{
			iSkipStep++;
			break;
		}

		if (iSkipStep++ == 3)
		{
			iSkipStep = 0;
		}

		switch (irand)
		{
			// right foot
		case 0:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_wade1.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 1:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_wade2.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
			// left foot
		case 2:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_wade3.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 3:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_wade4.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		}
		break;
	}
	case StepType::Ladder:
		switch (irand)
		{
			// right foot
		case 0:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_ladder1.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 1:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_ladder3.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
			// left foot
		case 2:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_ladder2.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		case 3:	pmove->PM_PlaySound(SoundChannel::Body, "player/pl_ladder4.wav", fvol, ATTN_NORM, 0, PITCH_NORM);	break;
		}
		break;
	}
}

StepType PM_MapTextureTypeStepType(char chTextureType)
{
	switch (chTextureType)
	{
	default:
	case CHAR_TEX_CONCRETE:	return StepType::Concrete;
	case CHAR_TEX_METAL: return StepType::Metal;
	case CHAR_TEX_DIRT: return StepType::Dirt;
	case CHAR_TEX_VENT: return StepType::Vent;
	case CHAR_TEX_GRATE: return StepType::Grate;
	case CHAR_TEX_TILE: return StepType::Tile;
	case CHAR_TEX_SLOSH: return StepType::Slosh;
	}
}

/*
====================
PM_CatagorizeTextureType

Determine texture info for the texture we are standing on.
====================
*/
void PM_CatagorizeTextureType()
{
	const Vector start = pmove->origin;
	Vector end = pmove->origin;

	// Straight down
	end[2] -= 64;

	// Fill in default values, just in case.
	pmove->sztexturename[0] = '\0';
	pmove->chtexturetype = CHAR_TEX_CONCRETE;

	const char* pTextureName = pmove->PM_TraceTexture(pmove->onground, start, end);
	if (!pTextureName)
		return;

	// strip leading '-0' or '+0~' or '{' or '!'
	if (*pTextureName == '-' || *pTextureName == '+')
		pTextureName += 2;

	if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
		pTextureName++;
	// '}}'

	safe_strcpy(pmove->sztexturename, pTextureName);
	pmove->sztexturename[CBTEXTURENAMEMAX - 1] = 0;

	// get texture type
	pmove->chtexturetype = TEXTURETYPE_Find(pmove->sztexturename);
}

void PM_UpdateStepSound()
{
	if (pmove->flTimeStepSound > 0)
		return;

	if (pmove->flags & FL_FROZEN)
		return;

	PM_CatagorizeTextureType();

	const float speed = pmove->velocity.Length();

	// determine if we are on a ladder
	const bool fLadder = (pmove->movetype == Movetype::Fly);// IsOnLadder();

	// UNDONE: need defined numbers for run, walk, crouch, crouch run velocities!!!!	
	float velrun;
	float velwalk;
	float flduck;

	if ((pmove->flags & FL_DUCKING) || fLadder)
	{
		velwalk = 60;		// These constants should be based on cl_movespeedkey * cl_forwardspeed somehow
		velrun = 80;		// UNDONE: Move walking to server
		flduck = 100;
	}
	else
	{
		velwalk = 120;
		velrun = 210;
		flduck = 0;
	}

	// If we're on a ladder or on the ground, and we're moving fast enough,
	//  play step sound.  Also, if pmove->flTimeStepSound is zero, get the new
	//  sound right away - we just started moving in new level.
	if ((fLadder || (pmove->onground != -1)) &&
		(pmove->velocity.Length() > 0.0) &&
		(speed >= velwalk || !pmove->flTimeStepSound))
	{
		const bool fWalking = speed < velrun;

		const Vector center = pmove->origin;
		Vector knee = pmove->origin;
		Vector feet = pmove->origin;

		const float height = pmove->player_maxs[pmove->usehull][2] - pmove->player_mins[pmove->usehull][2];

		knee[2] = pmove->origin[2] - 0.3 * height;
		feet[2] = pmove->origin[2] - 0.5 * height;

		// find out what we're stepping in or on...
		StepType step;
		float fvol;

		if (fLadder)
		{
			step = StepType::Ladder;
			fvol = 0.35;
			pmove->flTimeStepSound = 350;
		}
		else if (pmove->PM_PointContents(knee, nullptr) == Contents::Water)
		{
			step = StepType::Wade;
			fvol = 0.65;
			pmove->flTimeStepSound = 600;
		}
		else if (pmove->PM_PointContents(feet, nullptr) == Contents::Water)
		{
			step = StepType::Slosh;
			fvol = fWalking ? 0.2 : 0.5;
			pmove->flTimeStepSound = fWalking ? 400 : 300;
		}
		else
		{
			// find texture under player, if different from current texture, 
			// get material type
			step = PM_MapTextureTypeStepType(pmove->chtexturetype);

			switch (pmove->chtexturetype)
			{
			default:
			case CHAR_TEX_CONCRETE:
				fvol = fWalking ? 0.2 : 0.5;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;

			case CHAR_TEX_METAL:
				fvol = fWalking ? 0.2 : 0.5;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;

			case CHAR_TEX_DIRT:
				fvol = fWalking ? 0.25 : 0.55;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;

			case CHAR_TEX_VENT:
				fvol = fWalking ? 0.4 : 0.7;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;

			case CHAR_TEX_GRATE:
				fvol = fWalking ? 0.2 : 0.5;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;

			case CHAR_TEX_TILE:
				fvol = fWalking ? 0.2 : 0.5;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;

			case CHAR_TEX_SLOSH:
				fvol = fWalking ? 0.2 : 0.5;
				pmove->flTimeStepSound = fWalking ? 400 : 300;
				break;
			}
		}

		pmove->flTimeStepSound += flduck; // slower step time if ducking

		// play the sound
		// 35% volume if ducking
		if (pmove->flags & FL_DUCKING)
		{
			fvol *= 0.35;
		}

		PM_PlayStepSound(step, fvol);
	}
}

/*
================
PM_AddToTouched

Add's the trace result to touch list, if contact is not already in list.
================
*/
bool PM_AddToTouched(pmtrace_t tr, const Vector& impactvelocity)
{
	int i;

	for (i = 0; i < pmove->numtouch; i++)
	{
		if (pmove->touchindex[i].ent == tr.ent)
			break;
	}
	if (i != pmove->numtouch)  // Already in list.
		return false;

	tr.deltavelocity = impactvelocity;

	//TODO: this doesn't protect against out of bounds access!
	if (pmove->numtouch >= MAX_PHYSENTS)
		pmove->Con_DPrintf("Too many entities were touched!\n");

	pmove->touchindex[pmove->numtouch++] = tr;
	return true;
}

/*
================
PM_CheckVelocity

See if the player has a bogus velocity value.
================
*/
void PM_CheckVelocity()
{
	//
	// bound velocity
	//
	for (int i = 0; i < 3; i++)
	{
		// See if it's bogus.
		if (IS_NAN(pmove->velocity[i]))
		{
			pmove->Con_Printf("PM  Got a NaN velocity %i\n", i);
			pmove->velocity[i] = 0;
		}
		if (IS_NAN(pmove->origin[i]))
		{
			pmove->Con_Printf("PM  Got a NaN origin on %i\n", i);
			pmove->origin[i] = 0;
		}

		// Bound it.
		if (pmove->velocity[i] > pmove->movevars->maxvelocity)
		{
			pmove->Con_DPrintf("PM  Got a velocity too high on %i\n", i);
			pmove->velocity[i] = pmove->movevars->maxvelocity;
		}
		else if (pmove->velocity[i] < -pmove->movevars->maxvelocity)
		{
			pmove->Con_DPrintf("PM  Got a velocity too low on %i\n", i);
			pmove->velocity[i] = -pmove->movevars->maxvelocity;
		}
	}
}

/*
==================
PM_ClipVelocity

Slide off of the impacting object
returns the blocked flags:
0x01 == floor
0x02 == step / wall
==================
*/
int PM_ClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce)
{
	const float angle = normal[2];

	int blocked = 0x00;            // Assume unblocked.
	if (angle > 0)      // If the plane that is blocking us has a positive z component, then assume it's a floor.
		blocked |= 0x01;		// 
	if (!angle)         // If the plane has no Z, it is vertical (wall/step)
		blocked |= 0x02;		// 

	// Determine how far along plane to slide based on incoming direction.
	// Scale by overbounce factor.
	const float backoff = DotProduct(in, normal) * overbounce;

	for (int i = 0; i < 3; i++)
	{
		const float change = normal[i] * backoff;
		out[i] = in[i] - change;
		// If out velocity is too small, zero it out.
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}

	// Return blocking flags.
	return blocked;
}

void PM_AddCorrectGravity()
{
	if (pmove->waterjumptime)
		return;

	const float ent_gravity = pmove->gravity ? pmove->gravity : 1.0;

	// Add gravity so they'll be in the correct position during movement
	// yes, this 0.5 looks wrong, but it's not.  
	pmove->velocity[2] -= (ent_gravity * pmove->movevars->gravity * 0.5f * pmove->frametime);
	pmove->velocity[2] += pmove->basevelocity[2] * pmove->frametime;
	pmove->basevelocity[2] = 0;

	PM_CheckVelocity();
}


void PM_FixupGravityVelocity()
{
	if (pmove->waterjumptime)
		return;

	const float ent_gravity = pmove->gravity ? pmove->gravity : 1.0;

	// Get the correct velocity for the end of the dt 
	pmove->velocity[2] -= (ent_gravity * pmove->movevars->gravity * pmove->frametime * 0.5f);

	PM_CheckVelocity();
}

/*
============
PM_FlyMove

The basic solid body movement clip that slides along multiple planes
============
*/
int PM_FlyMove()
{
	Vector		planes[MAX_CLIP_PLANES];

	int numbumps = 4;           // Bump up to four times

	int blocked = 0;           // Assume not blocked
	int numplanes = 0;           //  and not sliding along any planes
	Vector original_velocity = pmove->velocity;  // Store original velocity
	const Vector primal_velocity = pmove->velocity;

	float allFraction = 0;
	float time_left = pmove->frametime;   // Total time for this movement operation.

	for (int bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		if (!pmove->velocity[0] && !pmove->velocity[1] && !pmove->velocity[2])
			break;

		// Assume we can move all the way from the current origin to the
		//  end point.
		const Vector end = pmove->origin + time_left * pmove->velocity;

		// See if we can make it from origin to end point.
		const pmtrace_t trace = pmove->PM_PlayerTrace(pmove->origin, end, PM_NORMAL, -1);

		allFraction += trace.fraction;
		// If we started in a solid object, or we were in solid space
		//  the whole way, zero out our velocity and return that we
		//  are blocked by floor and wall.
		if (trace.allsolid)
		{	// entity is trapped in another solid
			pmove->velocity = vec3_origin;
			//Con_DPrintf("Trapped 4\n");
			return 4;
		}

		// If we moved some portion of the total distance, then
		//  copy the end position into the pmove->origin and 
		//  zero the plane counter.
		if (trace.fraction > 0)
		{	// actually covered some distance
			pmove->origin = trace.endpos;
			original_velocity = pmove->velocity;
			numplanes = 0;
		}

		// If we covered the entire distance, we are done
		//  and can return.
		if (trace.fraction == 1)
			break;		// moved the entire distance

	   //if (!trace.ent)
	   //	Sys_Error ("PM_PlayerTrace: !trace.ent");

	   // Save entity that blocked us (since fraction was < 1.0)
	   //  for contact
	   // Add it if it's not already in the list!!!
		PM_AddToTouched(trace, pmove->velocity);

		// If the plane we hit has a high z component in the normal, then
		//  it's probably a floor
		if (trace.plane.normal[2] > 0.7)
		{
			blocked |= 1;		// floor
		}
		// If the plane has a zero z component in the normal, then it's a 
		//  step or wall
		if (!trace.plane.normal[2])
		{
			blocked |= 2;		// step / wall
			//Con_DPrintf("Blocked by %i\n", trace.ent);
		}

		// Reduce amount of pmove->frametime left by total time left * fraction
		//  that we covered.
		time_left -= time_left * trace.fraction;

		// Did we run out of planes to clip against?
		if (numplanes >= MAX_CLIP_PLANES)
		{	// this shouldn't really happen
			//  Stop our movement if so.
			pmove->velocity = vec3_origin;
			//Con_DPrintf("Too many planes 4\n");

			break;
		}

		// Set up next clipping plane
		planes[numplanes] = trace.plane.normal;
		numplanes++;
		//

		// modify original_velocity so it parallels all of the clip planes
		//
		if (pmove->movetype == Movetype::Walk &&
			((pmove->onground == -1) || (pmove->friction != 1)))	// relfect player velocity
		{
			Vector new_velocity;

			for (int i = 0; i < numplanes; i++)
			{
				if (planes[i][2] > 0.7)
				{// floor or slope
					PM_ClipVelocity(original_velocity, planes[i], new_velocity, 1);
					original_velocity = new_velocity;
				}
				else
					PM_ClipVelocity(original_velocity, planes[i], new_velocity, 1.0f + pmove->movevars->bounce * (1 - pmove->friction));
			}

			pmove->velocity = new_velocity;
			original_velocity = new_velocity;
		}
		else
		{
			int i;
			for (i = 0; i < numplanes; i++)
			{
				PM_ClipVelocity(
					original_velocity,
					planes[i],
					pmove->velocity,
					1);
				int j;
				for (j = 0; j < numplanes; j++)
					if (j != i)
					{
						// Are we now moving against this plane?
						if (DotProduct(pmove->velocity, planes[j]) < 0)
							break;	// not ok
					}
				if (j == numplanes)  // Didn't have to clip, so we're ok
					break;
			}

			// Did we go all the way through plane set
			if (i != numplanes)
			{	// go along this plane
				// pmove->velocity is set in clipping call, no need to set again.
				;
			}
			else
			{	// go along the crease
				if (numplanes != 2)
				{
					//Con_Printf ("clip velocity, numplanes == %i\n",numplanes);
					pmove->velocity = vec3_origin;
					//Con_DPrintf("Trapped 4\n");

					break;
				}
				const Vector dir = CrossProduct(planes[0], planes[1]);
				const float d = DotProduct(dir, pmove->velocity);
				pmove->velocity = dir * d;
			}

			//
			// if original velocity is against the original velocity, stop dead
			// to avoid tiny occilations in sloping corners
			//
			if (DotProduct(pmove->velocity, primal_velocity) <= 0)
			{
				//Con_DPrintf("Back\n");
				pmove->velocity = vec3_origin;
				break;
			}
		}
	}

	if (allFraction == 0)
	{
		pmove->velocity = vec3_origin;
		//Con_DPrintf( "Don't stick\n" );
	}

	return blocked;
}

/*
==============
PM_Accelerate
==============
*/
void PM_Accelerate(const Vector& wishdir, float wishspeed, float accel)
{
	// Dead player's don't accelerate
	if (pmove->dead)
		return;

	// If waterjumping, don't accelerate
	if (pmove->waterjumptime)
		return;

	// See if we are changing direction a bit
	const float currentspeed = DotProduct(pmove->velocity, wishdir);

	// Reduce wishspeed by the amount of veer.
	const float addspeed = wishspeed - currentspeed;

	// If not going to add any speed, done.
	if (addspeed <= 0)
		return;

	// Determine amount of accleration.
	float accelspeed = accel * pmove->frametime * wishspeed * pmove->friction;

	// Cap at addspeed
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	// Adjust velocity.
	pmove->velocity = pmove->velocity + accelspeed * wishdir;
}

/*
=====================
PM_WalkMove

Only used by players.  Moves along the ground when player is a MOVETYPE_WALK.
======================
*/
void PM_WalkMove()
{
	// Copy movement amounts
	const float fmove = pmove->cmd.forwardmove;
	const float smove = pmove->cmd.sidemove;

	// Zero out z components of movement vectors
	pmove->forward[2] = 0;
	pmove->right[2] = 0;

	VectorNormalize(pmove->forward);  // Normalize remainder of vectors.
	VectorNormalize(pmove->right);    // 

	Vector wishvel;
	for (int i = 0; i < 2; i++)       // Determine x and y parts of velocity
		wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;

	wishvel[2] = 0;             // Zero out z part of velocity

	Vector wishdir = wishvel;   // Determine maginitude of speed of move
	float wishspeed = VectorNormalize(wishdir);

	//
	// Clamp to server defined max speed
	//
	if (wishspeed > pmove->maxspeed)
	{
		wishvel = wishvel * (pmove->maxspeed / wishspeed);
		wishspeed = pmove->maxspeed;
	}

	// Set pmove velocity
	pmove->velocity[2] = 0;
	PM_Accelerate(wishdir, wishspeed, pmove->movevars->accelerate);
	pmove->velocity[2] = 0;

	// Add in any base velocity to the current velocity.
	pmove->velocity = pmove->velocity + pmove->basevelocity;

	const float spd = pmove->velocity.Length();

	if (spd < 1.0f)
	{
		pmove->velocity = vec3_origin;
		return;
	}

	// If we are not moving, do nothing
	//if (!pmove->velocity[0] && !pmove->velocity[1] && !pmove->velocity[2])
	//	return;

	const int oldonground = pmove->onground;

	// first try just moving to the destination	
	Vector dest
	{
		pmove->origin[0] + pmove->velocity[0] * pmove->frametime,
		pmove->origin[1] + pmove->velocity[1] * pmove->frametime,
		pmove->origin[2]
	};

	// first try moving directly to the next spot
	const Vector start = dest;
	pmtrace_t trace = pmove->PM_PlayerTrace(pmove->origin, dest, PM_NORMAL, -1);
	// If we made it all the way, then copy trace end
	//  as new player position.
	if (trace.fraction == 1)
	{
		pmove->origin = trace.endpos;
		return;
	}

	if (oldonground == -1 &&   // Don't walk up stairs if not on ground.
		pmove->waterlevel == WaterLevel::Dry)
		return;

	if (pmove->waterjumptime)         // If we are jumping out of water, don't do anything more.
		return;

	// Try sliding forward both on ground and up 16 pixels
	//  take the move that goes farthest
	const Vector original = pmove->origin;       // Save out original pos &
	const Vector originalvel = pmove->velocity;  //  velocity.

	// Slide move
	int clip = PM_FlyMove();

	// Copy the results out
	const Vector down = pmove->origin;
	const Vector downvel = pmove->velocity;

	// Reset original values.
	pmove->origin = original;
	pmove->velocity = originalvel;

	// Start out up one stair height
	dest = pmove->origin;
	dest[2] += pmove->movevars->stepsize;

	trace = pmove->PM_PlayerTrace(pmove->origin, dest, PM_NORMAL, -1);
	// If we started okay and made it part of the way at least,
	//  copy the results to the movement start position and then
	//  run another move try.
	if (!trace.startsolid && !trace.allsolid)
	{
		pmove->origin = trace.endpos;
	}

	// slide move the rest of the way.
	clip = PM_FlyMove();

	// Now try going back down from the end point
	//  press down the stepheight
	dest = pmove->origin;
	dest[2] -= pmove->movevars->stepsize;

	trace = pmove->PM_PlayerTrace(pmove->origin, dest, PM_NORMAL, -1);

	bool useDown = false;

	// If we are not on the ground any more then
	//  use the original movement attempt
	if (trace.plane.normal[2] < 0.7)
	{
		useDown = true;
	}
	else
	{
		// If the trace ended up in empty space, copy the end
		//  over to the origin.
		if (!trace.startsolid && !trace.allsolid)
		{
			pmove->origin = trace.endpos;
		}
		// Copy this origion to up.
		pmove->up = pmove->origin;

		// decide which one went farther
		const float downdist = (down[0] - original[0]) * (down[0] - original[0])
			+ (down[1] - original[1]) * (down[1] - original[1]);
		const float updist = (pmove->up[0] - original[0]) * (pmove->up[0] - original[0])
			+ (pmove->up[1] - original[1]) * (pmove->up[1] - original[1]);

		useDown = downdist > updist;
	}

	if (useDown)
	{
		pmove->origin = down;
		pmove->velocity = downvel;
	}
	else // copy z value from slide move
		pmove->velocity[2] = downvel[2];

}

/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
void PM_Friction()
{
	// If we are in water jump cycle, don't apply friction
	if (pmove->waterjumptime)
		return;

	// Get velocity
	const Vector& vel = pmove->velocity;

	// Calculate speed
	const float speed = vel.Length();

	// If too slow, return
	if (speed < 0.1f)
	{
		return;
	}

	float drop = 0;

	// apply ground friction
	if (pmove->onground != -1)  // On an entity that is the ground
	{
		const Vector start
		{
			pmove->origin[0] + vel[0] / speed * 16,
			pmove->origin[1] + vel[1] / speed * 16,
			pmove->origin[2] + pmove->player_mins[pmove->usehull][2]
		};

		const Vector stop
		{
			start[0],
			start[1],
			start[2] - 34
		};

		const pmtrace_t trace = pmove->PM_PlayerTrace(start, stop, PM_NORMAL, -1);

		float friction;
		if (trace.fraction == 1.0)
			friction = pmove->movevars->friction * pmove->movevars->edgefriction;
		else
			friction = pmove->movevars->friction;

		// Grab friction value.
		//friction = pmove->movevars->friction;      

		friction *= pmove->friction;  // player friction?

		// Bleed off some speed, but if we have less than the bleed
		//  threshhold, bleed the theshold amount.
		const float control = (speed < pmove->movevars->stopspeed) ?
			pmove->movevars->stopspeed : speed;
		// Add the amount to t'he drop amount.
		drop += control * friction * pmove->frametime;
	}

	// apply water friction
	//	if (pmove->waterlevel != WaterLevel::Dry)
	//		drop += speed * pmove->movevars->waterfriction * waterlevel * pmove->frametime;

	// scale the velocity
	float newspeed = speed - drop;
	if (newspeed < 0)
		newspeed = 0;

	// Determine proportion of old speed we are using.
	newspeed /= speed;

	// Adjust velocity according to proportion.
	pmove->velocity = vel * newspeed;
}

void PM_AirAccelerate(const Vector& wishdir, float wishspeed, float accel)
{
	if (pmove->dead)
		return;
	if (pmove->waterjumptime)
		return;

	// Cap speed
	//wishspd = VectorNormalize (pmove->wishveloc);

	const float wishspd = std::min(30.0f, wishspeed);

	// Determine veer amount
	const float currentspeed = DotProduct(pmove->velocity, wishdir);
	// See how much to add
	const float addspeed = wishspd - currentspeed;
	// If not adding any, done.
	if (addspeed <= 0)
		return;
	// Determine acceleration speed after acceleration

	float accelspeed = accel * wishspeed * pmove->frametime * pmove->friction;
	// Cap it
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	// Adjust pmove vel.
	pmove->velocity = pmove->velocity + accelspeed * wishdir;
}

/*
===================
PM_WaterMove

===================
*/
void PM_WaterMove()
{
	//
	// user intentions
	//
	Vector wishvel = pmove->forward * pmove->cmd.forwardmove + pmove->right * pmove->cmd.sidemove;

	// Sinking after no other movement occurs
	if (!pmove->cmd.forwardmove && !pmove->cmd.sidemove && !pmove->cmd.upmove)
		wishvel[2] -= 60;		// drift towards bottom
	else  // Go straight up by upmove amount.
		wishvel[2] += pmove->cmd.upmove;

	// Copy it over and determine speed
	Vector wishdir = wishvel;
	float wishspeed = VectorNormalize(wishdir);

	// Cap speed.
	if (wishspeed > pmove->maxspeed)
	{
		wishvel = wishvel * (pmove->maxspeed / wishspeed);
		wishspeed = pmove->maxspeed;
	}
	// Slow us down a bit.
	wishspeed *= 0.8;

	pmove->velocity = pmove->velocity + pmove->basevelocity;
	// Water friction
	Vector temp = pmove->velocity;
	const float speed = VectorNormalize(temp);
	float newspeed;
	if (speed)
	{
		newspeed = speed - pmove->frametime * speed * pmove->movevars->friction * pmove->friction;

		if (newspeed < 0)
			newspeed = 0;
		pmove->velocity = pmove->velocity * (newspeed / speed);
	}
	else
		newspeed = 0;

	//
	// water acceleration
	//
	if (wishspeed < 0.1f)
	{
		return;
	}

	const float addspeed = wishspeed - newspeed;
	if (addspeed > 0)
	{
		VectorNormalize(wishvel);
		float accelspeed = pmove->movevars->accelerate * wishspeed * pmove->frametime * pmove->friction;
		if (accelspeed > addspeed)
			accelspeed = addspeed;

		pmove->velocity = pmove->velocity + accelspeed * wishvel;
	}

	// Now move
	// assume it is a stair or a slope, so press down from stepheight above
	const Vector dest = pmove->origin + pmove->frametime * pmove->velocity;
	Vector start = dest;
	start[2] += pmove->movevars->stepsize + 1;
	const pmtrace_t trace = pmove->PM_PlayerTrace(start, dest, PM_NORMAL, -1);
	if (!trace.startsolid && !trace.allsolid)	// FIXME: check steep slope?
	{	// walked up the step, so just keep result and exit
		pmove->origin = trace.endpos;
		return;
	}

	// Try moving straight along out normal path.
	PM_FlyMove();
}


/*
===================
PM_AirMove

===================
*/
void PM_AirMove()
{
	// Copy movement amounts
	const float fmove = pmove->cmd.forwardmove;
	const float smove = pmove->cmd.sidemove;

	// Zero out z components of movement vectors
	pmove->forward[2] = 0;
	pmove->right[2] = 0;
	// Renormalize
	VectorNormalize(pmove->forward);
	VectorNormalize(pmove->right);

	// Determine x and y parts of velocity
	Vector wishvel;
	for (int i = 0; i < 2; i++)
	{
		wishvel[i] = pmove->forward[i] * fmove + pmove->right[i] * smove;
	}
	// Zero out z part of velocity
	wishvel[2] = 0;

	// Determine maginitude of speed of move
	Vector wishdir = wishvel;
	float wishspeed = VectorNormalize(wishdir);

	// Clamp to server defined max speed
	if (wishspeed > pmove->maxspeed)
	{
		wishvel = wishvel * (pmove->maxspeed / wishspeed);
		wishspeed = pmove->maxspeed;
	}

	PM_AirAccelerate(wishdir, wishspeed, pmove->movevars->airaccelerate);

	// Add in any base velocity to the current velocity.
	pmove->velocity = pmove->velocity + pmove->basevelocity;

	PM_FlyMove();
}

bool PM_InWater()
{
	return (pmove->waterlevel > WaterLevel::Feet);
}

/*
=============
PM_CheckWater

Sets pmove->waterlevel and pmove->watertype values.
=============
*/
bool PM_CheckWater()
{
	// Pick a spot just above the players feet.
	Vector point
	{
		pmove->origin[0] + (pmove->player_mins[pmove->usehull][0] + pmove->player_maxs[pmove->usehull][0]) * 0.5f,
		pmove->origin[1] + (pmove->player_mins[pmove->usehull][1] + pmove->player_maxs[pmove->usehull][1]) * 0.5f,
		pmove->origin[2] + pmove->player_mins[pmove->usehull][2] + 1
	};

	// Assume that we are not in water at all.
	pmove->waterlevel = WaterLevel::Dry;
	pmove->watertype = Contents::Empty;

	// Grab point contents.
	Contents truecont;
	Contents cont = pmove->PM_PointContents(point, &truecont);
	// Are we under water? (not solid and not empty?)
	if (cont <= Contents::Water && cont > Contents::Translucent)
	{
		// Set water type
		pmove->watertype = cont;

		// We are at least at level one
		pmove->waterlevel = WaterLevel::Feet;

		const float height = (pmove->player_mins[pmove->usehull][2] + pmove->player_maxs[pmove->usehull][2]);
		const float heightover2 = height * 0.5;

		// Now check a point that is at the player hull midpoint.
		point[2] = pmove->origin[2] + heightover2;
		cont = pmove->PM_PointContents(point, nullptr);
		// If that point is also under water...
		if (cont <= Contents::Water && cont > Contents::Translucent)
		{
			// Set a higher water level.
			pmove->waterlevel = WaterLevel::Waist;

			// Now check the eye position.  (view_ofs is relative to the origin)
			point[2] = pmove->origin[2] + pmove->view_ofs[2];

			cont = pmove->PM_PointContents(point, nullptr);
			if (cont <= Contents::Water && cont > Contents::Translucent)
				pmove->waterlevel = WaterLevel::Head;  // In over our eyes
		}

		// Adjust velocity based on water current, if any.
		if ((truecont <= Contents::Current0) &&
			(truecont >= Contents::CurrentDown))
		{
			// The deeper we are, the stronger the current.
			pmove->basevelocity = pmove->basevelocity
				+ (50.0 * static_cast<int>(pmove->waterlevel)) * current_table[static_cast<int>(Contents::Current0) - static_cast<int>(truecont)];
		}
	}

	return pmove->waterlevel > WaterLevel::Feet;
}

/*
=============
PM_CatagorizePosition
=============
*/
void PM_CatagorizePosition()
{
	// if the player hull point one unit down is solid, the player
	// is on ground

	// see if standing on something solid	

		// Doing this before we move may introduce a potential latency in water detection, but
		// doing it after can get us stuck on the bottom in water if the amount we move up
		// is less than the 1 pixel 'threshold' we're about to snap to.	Also, we'll call
		// this several times per frame, so we really need to avoid sticking to the bottom of
		// water on each call, and the converse case will correct itself if called twice.
	PM_CheckWater();

	const Vector point
	{
		pmove->origin[0],
		pmove->origin[1],
		pmove->origin[2] - 2
	};

	if (pmove->velocity[2] > 180)   // Shooting up really fast.  Definitely not on ground.
	{
		pmove->onground = -1;
	}
	else
	{
		// Try and move down.
		const pmtrace_t tr = pmove->PM_PlayerTrace(pmove->origin, point, PM_NORMAL, -1);
		// If we hit a steep plane, we are not on ground
		if (tr.plane.normal[2] < 0.7)
			pmove->onground = -1;	// too steep
		else
			pmove->onground = tr.ent;  // Otherwise, point to index of ent under us.

		// If we are on something...
		if (pmove->onground != -1)
		{
			// Then we are not in water jump sequence
			pmove->waterjumptime = 0;
			// If we could make the move, drop us down that 1 pixel
			if (pmove->waterlevel < WaterLevel::Waist && !tr.startsolid && !tr.allsolid)
				pmove->origin = tr.endpos;
		}

		// Standing on an entity other than the world
		if (tr.ent > 0)   // So signal that we are touching something.
		{
			PM_AddToTouched(tr, pmove->velocity);
		}
	}
}

/*
=================
PM_GetRandomStuckOffsets

When a player is stuck, it's costly to try and unstick them
Grab a test offset for the player based on a passed in index
=================
*/
int PM_GetRandomStuckOffsets(int nIndex, int server, Vector& offset)
{
	// Last time we did a full
	const int idx = rgStuckLast[nIndex][server]++;

	offset = rgv3tStuckTable[idx % ArraySize(rgv3tStuckTable)];

	return (idx % ArraySize(rgv3tStuckTable));
}

void PM_ResetStuckOffsets(int nIndex, int server)
{
	rgStuckLast[nIndex][server] = 0;
}

/*
=================
NudgePosition

If pmove->origin is in a solid position,
try nudging slightly on all axis to
allow for the cut precision of the net coordinates
=================
*/
constexpr double PM_CHECKSTUCK_MINTIME = 0.05;  // Don't check again too quickly.

bool PM_CheckStuck()
{
	static float rgStuckCheckTime[MAX_CLIENTS][2]{}; // Last time we did a full

	// If position is okay, exit
	pmtrace_t traceresult;
	int hitent = pmove->PM_TestPlayerPosition(pmove->origin, &traceresult);
	if (hitent == -1)
	{
		PM_ResetStuckOffsets(pmove->player_index, pmove->server);
		return false;
	}

	Vector base = pmove->origin;

	// 
	// Deal with precision error in network.
	// 
	if (!pmove->server)
	{
		// World or BSP model
		if ((hitent == 0) ||
			(pmove->physents[hitent].model != nullptr))
		{
			std::size_t nReps = 0;
			PM_ResetStuckOffsets(pmove->player_index, pmove->server);
			do
			{
				Vector offset;
				const int i = PM_GetRandomStuckOffsets(pmove->player_index, pmove->server, offset);

				const Vector test = base + offset;
				if (pmove->PM_TestPlayerPosition(test, &traceresult) == -1)
				{
					PM_ResetStuckOffsets(pmove->player_index, pmove->server);

					pmove->origin = test;
					return false;
				}
				nReps++;
			}
			while (nReps < ArraySize(rgv3tStuckTable));
		}
	}

	// Only an issue on the client.

	int idx;
	if (pmove->server)
		idx = 0;
	else
		idx = 1;

	const float fTime = pmove->Sys_FloatTime();
	// Too soon?
	if (rgStuckCheckTime[pmove->player_index][idx] >=
		(fTime - PM_CHECKSTUCK_MINTIME))
	{
		return true;
	}
	rgStuckCheckTime[pmove->player_index][idx] = fTime;

	pmove->PM_StuckTouch(hitent, &traceresult);

	Vector offset;
	const int i = PM_GetRandomStuckOffsets(pmove->player_index, pmove->server, offset);

	Vector test = base + offset;
	if ((hitent = pmove->PM_TestPlayerPosition(test, nullptr)) == -1)
	{
		//Con_DPrintf("Nudged\n");

		PM_ResetStuckOffsets(pmove->player_index, pmove->server);

		if (i >= 27)
			pmove->origin = test;

		return false;
	}

	// If player is flailing while stuck in another player ( should never happen ), then see
	//  if we can't "unstick" them forceably.
	if (pmove->cmd.buttons & (IN_JUMP | IN_DUCK | IN_ATTACK) && (pmove->physents[hitent].player != 0))
	{
		constexpr float xystep = 8.0;
		constexpr float zstep = 18.0;
		constexpr float xyminmax = xystep;
		constexpr float zminmax = 4 * zstep;

		for (float z = 0; z <= zminmax; z += zstep)
		{
			for (float x = -xyminmax; x <= xyminmax; x += xystep)
			{
				for (float y = -xyminmax; y <= xyminmax; y += xystep)
				{
					test = base + Vector(x, y, z);

					if (pmove->PM_TestPlayerPosition(test, nullptr) == -1)
					{
						pmove->origin = test;
						return false;
					}
				}
			}
		}
	}

	//pmove->origin = base;

	return true;
}

/*
===============
PM_SpectatorMove
===============
*/
void PM_SpectatorMove()
{
	// this routine keeps track of the spectators psoition
	// there a two different main move types : track player or moce freely (OBS_ROAMING)
	// doesn't need excate track position, only to generate PVS, so just copy
	// targets position and real view position is calculated on client (saves server CPU)

	if (pmove->iuser1 == OBS_ROAMING)
	{

#ifdef CLIENT_DLL
		// jump only in roaming mode
		if (iJumpSpectator)
		{
			pmove->origin = vJumpOrigin;
			pmove->angles = vJumpAngles;
			pmove->velocity = vec3_origin;
			iJumpSpectator = false;
			return;
		}
#endif
		// Move around in normal spectator method

		const float speed = pmove->velocity.Length();
		if (speed < 1)
		{
			pmove->velocity = vec3_origin;
		}
		else
		{
			float drop = 0;

			const float friction = pmove->movevars->friction * 1.5;	// extra friction
			const float control = speed < pmove->movevars->stopspeed ? pmove->movevars->stopspeed : speed;
			drop += control * friction * pmove->frametime;

			// scale the velocity
			float newspeed = speed - drop;
			if (newspeed < 0)
				newspeed = 0;
			newspeed /= speed;

			pmove->velocity = pmove->velocity * newspeed;
		}

		// accelerate
		const float fmove = pmove->cmd.forwardmove;
		const float smove = pmove->cmd.sidemove;

		VectorNormalize(pmove->forward);
		VectorNormalize(pmove->right);

		Vector wishvel = pmove->forward * fmove + pmove->right * smove;
		wishvel[2] += pmove->cmd.upmove;

		Vector wishdir = wishvel;
		float wishspeed = VectorNormalize(wishdir);

		//
		// clamp to server defined max speed
		//
		if (wishspeed > pmove->movevars->spectatormaxspeed)
		{
			wishvel = wishvel * (pmove->movevars->spectatormaxspeed / wishspeed);
			wishspeed = pmove->movevars->spectatormaxspeed;
		}

		const float currentspeed = DotProduct(pmove->velocity, wishdir);
		const float addspeed = wishspeed - currentspeed;
		if (addspeed <= 0)
			return;

		float accelspeed = pmove->movevars->accelerate * pmove->frametime * wishspeed;
		if (accelspeed > addspeed)
			accelspeed = addspeed;

		pmove->velocity = pmove->velocity + accelspeed * wishdir;

		// move
		pmove->origin = pmove->origin + pmove->frametime * pmove->velocity;
	}
	else
	{
		// all other modes just track some kind of target, so spectator PVS = target PVS

		int target;

		// no valid target ?
		if (pmove->iuser2 <= 0)
			return;

		// Find the client this player's targeting
		for (target = 0; target < pmove->numphysent; target++)
		{
			if (pmove->physents[target].info == pmove->iuser2)
				break;
		}

		if (target == pmove->numphysent)
			return;

		// use targets position as own origin for PVS
		pmove->angles = pmove->physents[target].angles;
		pmove->origin = pmove->physents[target].origin;

		// no velocity
		pmove->velocity = vec3_origin;
	}
}

void PM_FixPlayerCrouchStuck(int direction)
{
	int hitent = pmove->PM_TestPlayerPosition(pmove->origin, nullptr);
	if (hitent == -1)
		return;

	const Vector test = pmove->origin;
	for (int i = 0; i < 36; i++)
	{
		pmove->origin[2] += direction;
		hitent = pmove->PM_TestPlayerPosition(pmove->origin, nullptr);
		if (hitent == -1)
			return;
	}

	pmove->origin = test; // Failed
}

void PM_UnDuck()
{
	Vector newOrigin = pmove->origin;

	if (pmove->onground != -1)
	{
		newOrigin = newOrigin + (pmove->player_mins[1] - pmove->player_mins[0]);
	}

	pmtrace_t trace = pmove->PM_PlayerTrace(newOrigin, newOrigin, PM_NORMAL, -1);

	if (!trace.startsolid)
	{
		pmove->usehull = static_cast<int>(PlayerHull::Standing);

		// Oh, no, changing hulls stuck us into something, try unsticking downward first.
		trace = pmove->PM_PlayerTrace(newOrigin, newOrigin, PM_NORMAL, -1);
		if (trace.startsolid)
		{
			// See if we are stuck?  If so, stay ducked with the duck hull until we have a clear spot
			//Con_Printf( "unstick got stuck\n" );
			pmove->usehull = static_cast<int>(PlayerHull::Crouched);
			return;
		}

		pmove->flags &= ~FL_DUCKING;
		pmove->bInDuck = false;
		pmove->view_ofs = VEC_VIEW;
		pmove->flDuckTime = 0;

		pmove->origin = newOrigin;

		// Recatagorize position since ducking can change origin
		PM_CatagorizePosition();
	}
}

void PM_Duck()
{
	const int buttonsChanged = (pmove->oldbuttons ^ pmove->cmd.buttons);	// These buttons have changed this frame
	const int nButtonPressed = buttonsChanged & pmove->cmd.buttons;		// The changed ones still down are "pressed"

	const bool duckchange = (buttonsChanged & IN_DUCK) != 0;
	const bool duckpressed = (nButtonPressed & IN_DUCK) != 0;

	if (pmove->cmd.buttons & IN_DUCK)
	{
		pmove->oldbuttons |= IN_DUCK;
	}
	else
	{
		pmove->oldbuttons &= ~IN_DUCK;
	}

	// Prevent ducking if the iuser3 variable is set
	if (pmove->iuser3 || pmove->dead)
	{
		// Try to unduck
		if (pmove->flags & FL_DUCKING)
		{
			PM_UnDuck();
		}
		return;
	}

	if (pmove->flags & FL_DUCKING)
	{
		pmove->cmd.forwardmove *= PLAYER_DUCKING_MULTIPLIER;
		pmove->cmd.sidemove *= PLAYER_DUCKING_MULTIPLIER;
		pmove->cmd.upmove *= PLAYER_DUCKING_MULTIPLIER;
	}

	if ((pmove->cmd.buttons & IN_DUCK) || (pmove->bInDuck) || (pmove->flags & FL_DUCKING))
	{
		if (pmove->cmd.buttons & IN_DUCK)
		{
			if ((nButtonPressed & IN_DUCK) && !(pmove->flags & FL_DUCKING))
			{
				// Use 1 second so super long jump will work
				pmove->flDuckTime = 1000;
				pmove->bInDuck = true;
			}

			const float time = std::max(0.0, (1.0 - (float)pmove->flDuckTime / 1000.0));

			if (pmove->bInDuck)
			{
				// Finish ducking immediately if duck time is over or not on ground
				if (((float)pmove->flDuckTime / 1000.0 <= (1.0 - TIME_TO_DUCK)) ||
					(pmove->onground == -1))
				{
					pmove->usehull = static_cast<int>(PlayerHull::Crouched);
					pmove->view_ofs = VEC_DUCK_VIEW;
					pmove->flags |= FL_DUCKING;
					pmove->bInDuck = false;

					// HACKHACK - Fudge for collision bug - no time to fix this properly
					if (pmove->onground != -1)
					{
						pmove->origin = pmove->origin - (pmove->player_mins[1] - pmove->player_mins[0]);
						// See if we are stuck?
						PM_FixPlayerCrouchStuck(STUCK_MOVEUP);

						// Recatagorize position since ducking can change origin
						PM_CatagorizePosition();
					}
				}
				else
				{
					float fMore = (VEC_DUCK_HULL_MIN[2] - VEC_HULL_MIN[2]);

					// Calc parametric time
					const float duckFraction = UTIL_SplineFraction(time, (1.0 / TIME_TO_DUCK));
					pmove->view_ofs[2] = ((VEC_DUCK_VIEW[2] - fMore) * duckFraction) + (VEC_VIEW[2] * (1 - duckFraction));
				}
			}
		}
		else
		{
			// Try to unduck
			PM_UnDuck();
		}
	}
}

void PM_LadderMove(physent_t* pLadder)
{
	if (pmove->movetype == Movetype::Noclip)
		return;

	Vector modelmins, modelmaxs;
	pmove->PM_GetModelBounds(pLadder->model, modelmins, modelmaxs);

	const Vector ladderCenter = (modelmins + modelmaxs) * 0.5;

	pmove->movetype = Movetype::Fly;

	// On ladder, convert movement to be relative to the ladder

	Vector floor = pmove->origin;
	floor[2] += pmove->player_mins[pmove->usehull][2] - 1;

	const bool onFloor = pmove->PM_PointContents(floor, nullptr) == Contents::Solid;

	pmove->gravity = 0;
	trace_t trace;
	pmove->PM_TraceModel(pLadder, pmove->origin, ladderCenter, &trace);
	if (trace.fraction != 1.0)
	{
		float flSpeed = MAX_CLIMB_SPEED;

		// they shouldn't be able to move faster than their maxspeed
		if (flSpeed > pmove->maxspeed)
		{
			flSpeed = pmove->maxspeed;
		}

		Vector vpn, v_right;
		AngleVectors(pmove->angles, &vpn, &v_right, nullptr);

		if (pmove->flags & FL_DUCKING)
		{
			flSpeed *= PLAYER_DUCKING_MULTIPLIER;
		}

		float forward = 0;
		float right = 0;

		if (pmove->cmd.buttons & IN_BACK)
		{
			forward -= flSpeed;
		}
		if (pmove->cmd.buttons & IN_FORWARD)
		{
			forward += flSpeed;
		}
		if (pmove->cmd.buttons & IN_MOVELEFT)
		{
			right -= flSpeed;
		}
		if (pmove->cmd.buttons & IN_MOVERIGHT)
		{
			right += flSpeed;
		}

		if (pmove->cmd.buttons & IN_JUMP)
		{
			pmove->movetype = Movetype::Walk;
			pmove->velocity = trace.plane.normal * 270;
		}
		else
		{
			if (forward != 0 || right != 0)
			{
				//ALERT(at_console, "pev %.2f %.2f %.2f - ",
				//	GetAbsVelocity().x, GetAbsVelocity().y, GetAbsVelocity().z);
				// Calculate player's intended velocity
				const Vector velocity = vpn * forward + v_right * right;

				// Perpendicular in the ladder plane
				Vector perp = CrossProduct(vec3_up, trace.plane.normal);
				VectorNormalize(perp);

				// decompose velocity into ladder plane
				const float normal = DotProduct(velocity, trace.plane.normal);
				// This is the velocity into the face of the ladder
				const Vector cross = trace.plane.normal * normal;

				// This is the player's additional velocity
				const Vector lateral = velocity - cross;

				// This turns the velocity into the face of the ladder into velocity that
				// is roughly vertically perpendicular to the face of the ladder.
				// NOTE: It IS possible to face up and move down or face down and move up
				// because the velocity is a sum of the directional velocity and the converted
				// velocity through the face of the ladder -- by design.
				const Vector tmp = CrossProduct(trace.plane.normal, perp);
				pmove->velocity = lateral + tmp * -normal;
				if (onFloor && normal > 0)	// On ground moving away from the ladder
				{
					pmove->velocity = pmove->velocity + MAX_CLIMB_SPEED * trace.plane.normal;
				}
				//pmove->velocity = lateral - (CrossProduct( trace.vecPlaneNormal, perp ) * normal);
			}
			else
			{
				pmove->velocity = vec3_origin;
			}
		}
	}
}

physent_t* PM_Ladder()
{
	for (int i = 0; i < pmove->nummoveent; i++)
	{
		physent_t* pe = &pmove->moveents[i];

		if (pe->model && (modtype_t)pmove->PM_GetModelType(pe->model) == mod_brush && pe->skin == static_cast<int>(Contents::Ladder))
		{
			Vector test;
			hull_t* hull = pmove->PM_HullForBsp(pe, test);
			const int num = hull->firstclipnode;

			// Offset the test point appropriately for this hull.
			test = pmove->origin - test;

			// Test the player's hull for intersection with this model
			if (pmove->PM_HullPointContents(hull, num, test) == Contents::Empty)
				continue;

			return pe;
		}
	}

	return nullptr;
}



void PM_WaterJump()
{
	if (pmove->waterjumptime > 10000)
	{
		pmove->waterjumptime = 10000;
	}

	if (!pmove->waterjumptime)
		return;

	pmove->waterjumptime -= pmove->cmd.msec;
	if (pmove->waterjumptime < 0 ||
		pmove->waterlevel == WaterLevel::Dry)
	{
		pmove->waterjumptime = 0;
		pmove->flags &= ~FL_WATERJUMP;
	}

	pmove->velocity[0] = pmove->movedir[0];
	pmove->velocity[1] = pmove->movedir[1];
}

/*
============
PM_AddGravity

============
*/
void PM_AddGravity()
{
	const float ent_gravity = pmove->gravity ? pmove->gravity : 1.0;

	// Add gravity incorrectly
	pmove->velocity[2] -= (ent_gravity * pmove->movevars->gravity * pmove->frametime);
	pmove->velocity[2] += pmove->basevelocity[2] * pmove->frametime;
	pmove->basevelocity[2] = 0;
	PM_CheckVelocity();
}
/*
============
PM_PushEntity

Does not change the entities velocity at all
============
*/
pmtrace_t PM_PushEntity(const Vector& push)
{
	const Vector end = pmove->origin + push;

	const pmtrace_t trace = pmove->PM_PlayerTrace(pmove->origin, end, PM_NORMAL, -1);

	pmove->origin = trace.endpos;

	// So we can run impact function afterwards.
	if (trace.fraction < 1.0 &&
		!trace.allsolid)
	{
		PM_AddToTouched(trace, pmove->velocity);
	}

	return trace;
}

/*
============
PM_Physics_Toss()

Dead player flying through air., e.g.
============
*/
void PM_Physics_Toss()
{
	PM_CheckWater();

	if (pmove->velocity[2] > 0)
		pmove->onground = -1;

	// If on ground and not moving, return.
	if (pmove->onground != -1)
	{
		if (pmove->basevelocity == vec3_origin &&
			pmove->velocity == vec3_origin)
			return;
	}

	PM_CheckVelocity();

	// add gravity
	if (pmove->movetype != Movetype::Fly &&
		pmove->movetype != Movetype::BounceMissile &&
		pmove->movetype != Movetype::FlyMissile)
		PM_AddGravity();

	// move origin
		// Base velocity is not properly accounted for since this entity will move again after the bounce without
		// taking it into account
	pmove->velocity = pmove->velocity + pmove->basevelocity;

	PM_CheckVelocity();
	Vector move = pmove->velocity * pmove->frametime;
	pmove->velocity = pmove->velocity - pmove->basevelocity;

	pmtrace_t trace = PM_PushEntity(move);	// Should this clear basevelocity

	PM_CheckVelocity();

	if (trace.allsolid)
	{
		// entity is trapped in another solid
		pmove->onground = trace.ent;
		pmove->velocity = vec3_origin;
		return;
	}

	if (trace.fraction == 1)
	{
		PM_CheckWater();
		return;
	}

	float backoff;
	if (pmove->movetype == Movetype::Bounce)
		backoff = 2.0 - pmove->friction;
	else if (pmove->movetype == Movetype::BounceMissile)
		backoff = 2.0;
	else
		backoff = 1;

	PM_ClipVelocity(pmove->velocity, trace.plane.normal, pmove->velocity, backoff);

	// stop if on ground
	if (trace.plane.normal[2] > 0.7)
	{
		const Vector base = vec3_origin;
		if (pmove->velocity[2] < pmove->movevars->gravity * pmove->frametime)
		{
			// we're rolling on the ground, add static friction.
			pmove->onground = trace.ent;
			pmove->velocity[2] = 0;
		}

		const float vel = DotProduct(pmove->velocity, pmove->velocity);

		// Con_DPrintf("%f %f: %.0f %.0f %.0f\n", vel, trace.fraction, ent->velocity[0], ent->velocity[1], ent->velocity[2] );

		if (vel < (30 * 30) || (pmove->movetype != Movetype::Bounce && pmove->movetype != Movetype::BounceMissile))
		{
			pmove->onground = trace.ent;
			pmove->velocity = vec3_origin;
		}
		else
		{
			move = pmove->velocity * ((1.0 - trace.fraction) * pmove->frametime * 0.9);
			trace = PM_PushEntity(move);
		}
		//TODO: base is always zero so what is this for?
		pmove->velocity = pmove->velocity - base;
	}

	// check for in water
	PM_CheckWater();
}

/*
====================
PM_NoClip

====================
*/
void PM_NoClip()
{
	// Copy movement amounts
	const float fmove = pmove->cmd.forwardmove;
	const float smove = pmove->cmd.sidemove;

	VectorNormalize(pmove->forward);
	VectorNormalize(pmove->right);

	// Determine x and y parts of velocity
	Vector wishvel = pmove->forward * fmove + pmove->right * smove;
	wishvel[2] += pmove->cmd.upmove;

	pmove->origin = pmove->origin + pmove->frametime * wishvel;

	// Zero out the velocity so that we don't accumulate a huge downward velocity from
	//  gravity, etc.
	pmove->velocity = vec3_origin;
}

//-----------------------------------------------------------------------------
// Purpose: Corrects bunny jumping ( where player initiates a bunny jump before other
//  movement logic runs, thus making onground == -1 thus making PM_Friction get skipped and
//  running PM_AirMove, which doesn't crop velocity to maxspeed like the ground / other
//  movement logic does.
//-----------------------------------------------------------------------------
void PM_PreventMegaBunnyJumping()
{
	// Speed at which bunny jumping is limited
	const float maxscaledspeed = BUNNYJUMP_MAX_SPEED_FACTOR * pmove->maxspeed;

	// Don't divide by zero
	if (maxscaledspeed <= 0.0f)
		return;

	// Current player speed
	const float spd = pmove->velocity.Length();

	if (spd <= maxscaledspeed)
		return;

	// If we have to crop, apply this cropping fraction to velocity
	const float fraction = (maxscaledspeed / spd) * 0.65; //Returns the modifier for the velocity

	pmove->velocity = pmove->velocity * fraction; //Crop it down!.
}

/*
=============
PM_Jump
=============
*/
void PM_Jump()
{
	if (pmove->dead)
	{
		pmove->oldbuttons |= IN_JUMP;	// don't jump again until released
		return;
	}

	// See if we are waterjumping.  If so, decrement count and return.
	if (pmove->waterjumptime)
	{
		pmove->waterjumptime -= pmove->cmd.msec;
		if (pmove->waterjumptime < 0)
		{
			pmove->waterjumptime = 0;
		}
		return;
	}

	// If we are in the water most of the way...
	if (pmove->waterlevel >= WaterLevel::Waist)
	{	// swimming, not jumping
		pmove->onground = -1;

		if (pmove->watertype == Contents::Water)    // We move up a certain amount
			pmove->velocity[2] = 100;
		else if (pmove->watertype == Contents::Slime)
			pmove->velocity[2] = 80;
		else  // LAVA
			pmove->velocity[2] = 50;

		// play swiming sound
		if (pmove->flSwimTime <= 0)
		{
			// Don't play sound again for 1 second
			pmove->flSwimTime = 1000;
			switch (pmove->RandomLong(0, 3))
			{
			case 0:
				pmove->PM_PlaySound(SoundChannel::Body, "player/pl_wade1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
				break;
			case 1:
				pmove->PM_PlaySound(SoundChannel::Body, "player/pl_wade2.wav", 1, ATTN_NORM, 0, PITCH_NORM);
				break;
			case 2:
				pmove->PM_PlaySound(SoundChannel::Body, "player/pl_wade3.wav", 1, ATTN_NORM, 0, PITCH_NORM);
				break;
			case 3:
				pmove->PM_PlaySound(SoundChannel::Body, "player/pl_wade4.wav", 1, ATTN_NORM, 0, PITCH_NORM);
				break;
			}
		}

		return;
	}

	// No more effect
	if (pmove->onground == -1)
	{
		// Flag that we jumped.
		// HACK HACK HACK
		// Remove this when the game .dll no longer does physics code!!!!
		pmove->oldbuttons |= IN_JUMP;	// don't jump again until released
		return;		// in air, so no effect
	}

	if (pmove->oldbuttons & IN_JUMP)
		return;		// don't pogo stick

	// In the air now.
	pmove->onground = -1;

	PM_PreventMegaBunnyJumping();

	PM_PlayStepSound(PM_MapTextureTypeStepType(pmove->chtexturetype), 1.0);

	// See if user can super long jump?
	const bool cansuperjump = atoi(pmove->PM_Info_ValueForKey(pmove->physinfo, "slj")) == 1;

	// Acclerate upward
	// If we are ducking...
	if ((pmove->bInDuck) || (pmove->flags & FL_DUCKING))
	{
		// Adjust for super long jump module
		// UNDONE -- note this should be based on forward angles, not current velocity.
		if (cansuperjump &&
			(pmove->cmd.buttons & IN_DUCK) &&
			(pmove->flDuckTime > 0) &&
			pmove->velocity.Length() > 50)
		{
			pmove->punchangle[0] = -5;

			for (int i = 0; i < 2; i++)
			{
				pmove->velocity[i] = pmove->forward[i] * PLAYER_LONGJUMP_SPEED * 1.6;
			}

			pmove->velocity[2] = sqrt(2 * 800 * 56.0f);
		}
		else
		{
			pmove->velocity[2] = sqrt(2 * 800 * 45.0f);
		}
	}
	else
	{
		pmove->velocity[2] = sqrt(2 * 800 * 45.0f);
	}

	// Decay it for simulation
	PM_FixupGravityVelocity();

	// Flag that we jumped.
	pmove->oldbuttons |= IN_JUMP;	// don't jump again until released
}

/*
=============
PM_CheckWaterJump
=============
*/
void PM_CheckWaterJump()
{
	// Already water jumping.
	if (pmove->waterjumptime)
		return;

	// Don't hop out if we just jumped in
	if (pmove->velocity[2] < -180)
		return; // only hop out if we are moving up

	// See if we are backing up
	Vector flatvelocity
	{
		pmove->velocity[0],
		pmove->velocity[1],
		0
	};

	// Must be moving
	const float curspeed = VectorNormalize(flatvelocity);

	// see if near an edge
	Vector flatforward
	{
		pmove->forward[0],
		pmove->forward[1],
		0
	};
	VectorNormalize(flatforward);

	// Are we backing into water from steps or something?  If so, don't pop forward
	if (curspeed != 0.0 && (DotProduct(flatvelocity, flatforward) < 0.0))
		return;

	Vector vecStart = pmove->origin;
	vecStart[2] += PLAYER_WATERJUMP_HEIGHT;

	Vector vecEnd = vecStart + 24 * flatforward;

	// Trace, this trace should use the point sized collision hull
	const int savehull = pmove->usehull;
	pmove->usehull = static_cast<int>(PlayerHull::Point);
	pmtrace_t tr = pmove->PM_PlayerTrace(vecStart, vecEnd, PM_NORMAL, -1);
	if (tr.fraction < 1.0 && fabs(tr.plane.normal[2]) < 0.1f)  // Facing a near vertical wall?
	{
		vecStart[2] += pmove->player_maxs[savehull][2] - PLAYER_WATERJUMP_HEIGHT;
		vecEnd = vecStart + 24 * flatforward;
		pmove->movedir = -50 * tr.plane.normal;

		tr = pmove->PM_PlayerTrace(vecStart, vecEnd, PM_NORMAL, -1);
		if (tr.fraction == 1.0)
		{
			pmove->waterjumptime = 2000;
			pmove->velocity[2] = 225;
			pmove->oldbuttons |= IN_JUMP;
			pmove->flags |= FL_WATERJUMP;
		}
	}

	// Reset the collision hull
	pmove->usehull = savehull;
}

void PM_CheckFalling()
{
	if (pmove->onground != -1 &&
		!pmove->dead &&
		pmove->flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD)
	{
		float fvol = 0.5;

		if (pmove->waterlevel > WaterLevel::Dry)
		{
		}
		else if (pmove->flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED)
		{
			// NOTE:  In the original game dll , there were no breaks after these cases, causing the first one to 
			// cascade into the second
			//switch ( RandomLong(0,1) )
			//{
			//case 0:
				//pmove->PM_PlaySound( SoundChannel::Voice, "player/pl_fallpain2.wav", 1, ATTN_NORM, 0, PITCH_NORM );
				//break;
			//case 1:
			pmove->PM_PlaySound(SoundChannel::Voice, "player/pl_fallpain3.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			//	break;
			//}
			fvol = 1.0;
		}
		else if (pmove->flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED / 2)
		{
			fvol = 0.85;
		}
		else if (pmove->flFallVelocity < PLAYER_MIN_BOUNCE_SPEED)
		{
			fvol = 0;
		}

		if (fvol > 0.0)
		{
			// Play landing step right away
			pmove->flTimeStepSound = 0;

			PM_UpdateStepSound();

			// play step sound for current texture
			PM_PlayStepSound(PM_MapTextureTypeStepType(pmove->chtexturetype), fvol);

			// Knock the screen around a little bit, temporary effect
			pmove->punchangle[2] = pmove->flFallVelocity * 0.013;	// punch z axis

			if (pmove->punchangle[0] > 8)
			{
				pmove->punchangle[0] = 8;
			}
		}
	}

	if (pmove->onground != -1)
	{
		pmove->flFallVelocity = 0;
	}
}

/*
=================
PM_PlayWaterSounds

=================
*/
void PM_PlayWaterSounds()
{
	// Did we enter or leave water?
	if ((pmove->oldwaterlevel == WaterLevel::Dry && pmove->waterlevel != WaterLevel::Dry) ||
		(pmove->oldwaterlevel != WaterLevel::Dry && pmove->waterlevel == WaterLevel::Dry))
	{
		switch (pmove->RandomLong(0, 3))
		{
		case 0:
			pmove->PM_PlaySound(SoundChannel::Body, "player/pl_wade1.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			break;
		case 1:
			pmove->PM_PlaySound(SoundChannel::Body, "player/pl_wade2.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			break;
		case 2:
			pmove->PM_PlaySound(SoundChannel::Body, "player/pl_wade3.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			break;
		case 3:
			pmove->PM_PlaySound(SoundChannel::Body, "player/pl_wade4.wav", 1, ATTN_NORM, 0, PITCH_NORM);
			break;
		}
	}
}

/*
==============
PM_CheckParamters

==============
*/
void PM_CheckParamters()
{
	const float spd = Vector(pmove->cmd.forwardmove, pmove->cmd.sidemove, pmove->cmd.upmove).Length();

	const float maxspeed = pmove->clientmaxspeed; //atof( pmove->PM_Info_ValueForKey( pmove->physinfo, "maxspd" ) );
	if (maxspeed != 0.0)
	{
		pmove->maxspeed = std::min(maxspeed, pmove->maxspeed);
	}

	if ((spd != 0.0) &&
		(spd > pmove->maxspeed))
	{
		const float fRatio = pmove->maxspeed / spd;
		pmove->cmd.forwardmove *= fRatio;
		pmove->cmd.sidemove *= fRatio;
		pmove->cmd.upmove *= fRatio;
	}

	if (pmove->flags & FL_FROZEN ||
		pmove->flags & FL_ONTRAIN ||
		pmove->dead)
	{
		pmove->cmd.forwardmove = 0;
		pmove->cmd.sidemove = 0;
		pmove->cmd.upmove = 0;
	}

	UTIL_DropPunchAngle(pmove->frametime, pmove->punchangle);

	// Take angles from command.
	if (!pmove->dead)
	{
		const Vector v_angle = pmove->cmd.viewangles + pmove->punchangle;

		// Set up view angles.
		pmove->angles[ROLL] = UTIL_CalcRoll(v_angle, pmove->velocity, pmove->movevars->rollangle, pmove->movevars->rollspeed) * 4;
		pmove->angles[PITCH] = v_angle[PITCH];
		pmove->angles[YAW] = v_angle[YAW];
	}
	else
	{
		pmove->angles = pmove->oldangles;
	}

	// Set dead player view_offset
	if (pmove->dead)
	{
		pmove->view_ofs = VEC_DEAD_VIEW;
	}

	// Adjust client view angles to match values used on server.
	if (pmove->angles[YAW] > 180.0f)
	{
		pmove->angles[YAW] -= 360.0f;
	}

}

void PM_ReduceTimers()
{
	if (pmove->flTimeStepSound > 0)
	{
		pmove->flTimeStepSound -= pmove->cmd.msec;
		if (pmove->flTimeStepSound < 0)
		{
			pmove->flTimeStepSound = 0;
		}
	}
	if (pmove->flDuckTime > 0)
	{
		pmove->flDuckTime -= pmove->cmd.msec;
		if (pmove->flDuckTime < 0)
		{
			pmove->flDuckTime = 0;
		}
	}
	if (pmove->flSwimTime > 0)
	{
		pmove->flSwimTime -= pmove->cmd.msec;
		if (pmove->flSwimTime < 0)
		{
			pmove->flSwimTime = 0;
		}
	}
}

/*
=============
PlayerMove

Returns with origin, angles, and velocity modified in place.

Numtouch and touchindex[] will be set if any of the physents
were contacted during the move.
=============
*/
void PM_PlayerMove(bool server)
{
	// Are we running server code?
	pmove->server = server;

	// Adjust speeds etc.
	PM_CheckParamters();

	// Assume we don't touch anything
	pmove->numtouch = 0;

	// # of msec to apply movement
	pmove->frametime = pmove->cmd.msec * 0.001;

	PM_ReduceTimers();

	// Convert view angles to vectors
	AngleVectors(pmove->angles, &pmove->forward, &pmove->right, &pmove->up);

	// PM_ShowClipBox();

	// Special handling for spectator and observers. (iuser1 is set if the player's in observer mode)
	if (pmove->spectator || pmove->iuser1 > 0)
	{
		PM_SpectatorMove();
		PM_CatagorizePosition();
		return;
	}

	// Always try and unstick us unless we are in NOCLIP mode
	if (pmove->movetype != Movetype::Noclip && pmove->movetype != Movetype::None)
	{
		if (PM_CheckStuck())
		{
			return;  // Can't move, we're stuck
		}
	}

	// Now that we are "unstuck", see where we are ( waterlevel and type, pmove->onground ).
	PM_CatagorizePosition();

	// Store off the starting water level
	pmove->oldwaterlevel = pmove->waterlevel;

	// If we are not on ground, store off how fast we are moving down
	if (pmove->onground == -1)
	{
		pmove->flFallVelocity = -pmove->velocity[2];
	}

	g_onladder = false;
	physent_t* pLadder = nullptr;
	// Don't run ladder code if dead or on a train
	if (!pmove->dead && !(pmove->flags & FL_ONTRAIN))
	{
		pLadder = PM_Ladder();
		if (pLadder)
		{
			g_onladder = true;
		}
	}

	PM_UpdateStepSound();

	PM_Duck();

	// Don't run ladder code if dead or on a train
	if (!pmove->dead && !(pmove->flags & FL_ONTRAIN))
	{
		if (pLadder)
		{
			PM_LadderMove(pLadder);
		}
		else if (pmove->movetype != Movetype::Walk &&
			pmove->movetype != Movetype::Noclip)
		{
			// Clear ladder stuff unless player is noclipping
			//  it will be set immediately again next frame if necessary
			pmove->movetype = Movetype::Walk;
		}
	}

	// Slow down, I'm pulling it! (a box maybe) but only when I'm standing on ground
	if ((pmove->onground != -1) && (pmove->cmd.buttons & IN_USE))
	{
		pmove->velocity = pmove->velocity * 0.3;
	}

	// Handle movement
	switch (pmove->movetype)
	{
	default:
		pmove->Con_DPrintf("Bogus pmove player movetype %i on (%i) 0=cl 1=sv\n", pmove->movetype, pmove->server);
		break;

	case Movetype::None:
		break;

	case Movetype::Noclip:
		PM_NoClip();
		break;

	case Movetype::Toss:
	case Movetype::Bounce:
		PM_Physics_Toss();
		break;

	case Movetype::Fly:

		PM_CheckWater();

		// Was jump button pressed?
		// If so, set velocity to 270 away from ladder.  This is currently wrong.
		// Also, set MOVE_TYPE to walk, too.
		if (pmove->cmd.buttons & IN_JUMP)
		{
			if (!pLadder)
			{
				PM_Jump();
			}
		}
		else
		{
			pmove->oldbuttons &= ~IN_JUMP;
		}

		// Perform the move accounting for any base velocity.
		pmove->velocity = pmove->velocity + pmove->basevelocity;
		PM_FlyMove();
		pmove->velocity = pmove->velocity - pmove->basevelocity;
		break;

	case Movetype::Walk:
		if (!PM_InWater())
		{
			PM_AddCorrectGravity();
		}

		// If we are leaping out of the water, just update the counters.
		if (pmove->waterjumptime)
		{
			PM_WaterJump();
			PM_FlyMove();

			// Make sure waterlevel is set correctly
			PM_CheckWater();
			return;
		}

		// If we are swimming in the water, see if we are nudging against a place we can jump up out
		//  of, and, if so, start out jump.  Otherwise, if we are not moving up, then reset jump timer to 0
		if (pmove->waterlevel >= WaterLevel::Waist)
		{
			if (pmove->waterlevel == WaterLevel::Waist)
			{
				PM_CheckWaterJump();
			}

			// If we are falling again, then we must not trying to jump out of water any more.
			if (pmove->velocity[2] < 0 && pmove->waterjumptime)
			{
				pmove->waterjumptime = 0;
			}

			// Was jump button pressed?
			if (pmove->cmd.buttons & IN_JUMP)
			{
				PM_Jump();
			}
			else
			{
				pmove->oldbuttons &= ~IN_JUMP;
			}

			// Perform regular water movement
			PM_WaterMove();

			pmove->velocity = pmove->velocity - pmove->basevelocity;

			// Get a final position
			PM_CatagorizePosition();
		}
		else

			// Not underwater
		{
			// Was jump button pressed?
			if (pmove->cmd.buttons & IN_JUMP)
			{
				if (!pLadder)
				{
					PM_Jump();
				}
			}
			else
			{
				pmove->oldbuttons &= ~IN_JUMP;
			}

			// Fricion is handled before we add in any base velocity. That way, if we are on a conveyor, 
			//  we don't slow when standing still, relative to the conveyor.
			if (pmove->onground != -1)
			{
				pmove->velocity[2] = 0.0;
				PM_Friction();
			}

			// Make sure velocity is valid.
			PM_CheckVelocity();

			// Are we on ground now
			if (pmove->onground != -1)
			{
				PM_WalkMove();
			}
			else
			{
				PM_AirMove();  // Take into account movement when in air.
			}

			// Set final flags.
			PM_CatagorizePosition();

			// Now pull the base velocity back out.
			// Base velocity is set if you are on a moving object, like
			//  a conveyor (or maybe another monster?)
			pmove->velocity = pmove->velocity - pmove->basevelocity;

			// Make sure velocity is valid.
			PM_CheckVelocity();

			// Add any remaining gravitational component.
			if (!PM_InWater())
			{
				PM_FixupGravityVelocity();
			}

			// If we are on ground, no downward velocity.
			if (pmove->onground != -1)
			{
				pmove->velocity[2] = 0;
			}

			// See if we landed on the ground with enough force to play
			//  a landing sound.
			PM_CheckFalling();
		}

		// Did we enter or leave the water?
		PM_PlayWaterSounds();
		break;
	}
}

void PM_CreateStuckTable()
{
	memset(rgv3tStuckTable, 0, sizeof(rgv3tStuckTable));

	int idx = 0;
	// Little Moves.
	float x = 0;
	float y = 0;
	float z = 0;
	// Z moves
	for (z = -0.125; z <= 0.125; z += 0.125)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	x = z = 0;
	// Y moves
	for (y = -0.125; y <= 0.125; y += 0.125)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	y = z = 0;
	// X moves
	for (x = -0.125; x <= 0.125; x += 0.125)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	// Remaining multi axis nudges.
	for (x = -0.125; x <= 0.125; x += 0.250)
	{
		for (y = -0.125; y <= 0.125; y += 0.250)
		{
			for (z = -0.125; z <= 0.125; z += 0.250)
			{
				rgv3tStuckTable[idx][0] = x;
				rgv3tStuckTable[idx][1] = y;
				rgv3tStuckTable[idx][2] = z;
				idx++;
			}
		}
	}

	// Big Moves.
	x = y = 0;

	const Vector zi{0.0f, 1.0f, 6.0f};

	for (int i = 0; i < 3; i++)
	{
		// Z moves
		z = zi[i];
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	x = z = 0;

	// Y moves
	for (y = -2.0f; y <= 2.0f; y += 2.0)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}
	y = z = 0;
	// X moves
	for (x = -2.0f; x <= 2.0f; x += 2.0f)
	{
		rgv3tStuckTable[idx][0] = x;
		rgv3tStuckTable[idx][1] = y;
		rgv3tStuckTable[idx][2] = z;
		idx++;
	}

	// Remaining multi axis nudges.
	for (int i = 0; i < 3; i++)
	{
		z = zi[i];

		for (x = -2.0f; x <= 2.0f; x += 2.0f)
		{
			for (y = -2.0f; y <= 2.0f; y += 2.0)
			{
				rgv3tStuckTable[idx][0] = x;
				rgv3tStuckTable[idx][1] = y;
				rgv3tStuckTable[idx][2] = z;
				idx++;
			}
		}
	}
}



/*
This modume implements the shared player physics code between any particular game and
the engine.  The same PM_Move routine is built into the game .dll and the client .dll and is
invoked by each side as appropriate.  There should be no distinction, internally, between server
and client.  This will ensure that prediction behaves appropriately.
*/

void PM_Move(playermove_t* ppmove, int server)
{
	assert(pm_shared_initialized);

	pmove = ppmove;

	PM_PlayerMove(server != 0);

	if (pmove->onground != -1)
	{
		pmove->flags |= FL_ONGROUND;
	}
	else
	{
		pmove->flags &= ~FL_ONGROUND;
	}

	// In single player, reset friction after each movement to FrictionModifier Triggers work still.
	if (!pmove->multiplayer && (pmove->movetype == Movetype::Walk))
	{
		pmove->friction = 1.0f;
	}
}

int PM_GetVisEntInfo(int ent)
{
	if (ent >= 0 && ent <= pmove->numvisent)
	{
		return pmove->visents[ent].info;
	}
	return -1;
}

int PM_GetPhysEntInfo(int ent)
{
	if (ent >= 0 && ent <= pmove->numphysent)
	{
		return pmove->physents[ent].info;
	}
	return -1;
}

void PM_Init(playermove_t* ppmove)
{
	assert(!pm_shared_initialized);

	pmove = ppmove;

	PM_CreateStuckTable();
	TEXTURETYPE_Init();

	pm_shared_initialized = true;
}
