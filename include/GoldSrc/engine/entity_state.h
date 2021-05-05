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

#include "pm_info.h"
#include "weaponinfo.h"

// For entityType below
constexpr int ENTITY_NORMAL = 1 << 0;
constexpr int ENTITY_BEAM = 1 << 1;

/**
*	@brief Entity state is used for the baseline and for delta compression of a packet of entities that is sent to a client.
*/
struct entity_state_t
{
	// Fields which are filled in by routines outside of delta compression
	int			entityType = 0;
	// Index into cl_entities array for this entity.
	int			number = 0;
	float		msg_time = 0;

	// Message number last time the player/entity state was updated.
	int			messagenum = 0;

	// Fields which can be transitted and reconstructed over the network stream
	Vector		origin;
	Vector		angles;

	int			modelindex = 0;
	int			sequence = 0;
	float		frame = 0;
	int			colormap = 0;
	short		skin = 0;
	short		solid = 0;

	Solid GetSolid() const { return static_cast<Solid>(solid); }

	int			effects = 0;
	float		scale = 0;

	byte		eflags = 0;

	// Render information
	RenderMode rendermode = RenderMode::Normal;
	int			renderamt = 0;
	color24		rendercolor;
	RenderFX renderfx = RenderFX::None;

	Movetype movetype = Movetype::None;
	float		animtime = 0;
	float		framerate = 0;
	int			body = 0;
	byte		controller[4]{};
	byte		blending[4]{};
	Vector		velocity;

	// Send bbox down to client for use during prediction.
	Vector		mins;
	Vector		maxs;

	int			aiment = 0;
	// If owned by a player, the index of that player ( for projectiles ).
	int			owner = 0;

	// Friction, for prediction.
	float		friction = 0;
	// Gravity multiplier
	float		gravity = 0;

	// PLAYER SPECIFIC
	int			team = 0;
	int			playerclass = 0;
	int			health = 0;
	qboolean	spectator = false;
	int         weaponmodel = 0;
	int			gaitsequence = 0;
	// If standing on conveyor, e.g.
	Vector		basevelocity;
	// Use the crouched hull, or the regular player hull.
	int			usehull = 0;
	// Latched buttons last time state updated.
	int			oldbuttons = 0;
	// -1 = in air, else pmove entity number
	int			onground = 0;
	int			iStepLeft = 0;
	// How fast we are falling
	float		flFallVelocity = 0;

	float		fov = 0;
	int			weaponanim = 0;

	// Parametric movement overrides
	Vector				startpos;
	Vector				endpos;
	float				impacttime = 0;
	float				starttime = 0;

	// For mods
	int			iuser1 = 0;
	int			iuser2 = 0;
	int			iuser3 = 0;
	int			iuser4 = 0;
	float		fuser1 = 0;
	float		fuser2 = 0;
	float		fuser3 = 0;
	float		fuser4 = 0;
	Vector		vuser1;
	Vector		vuser2;
	Vector		vuser3;
	Vector		vuser4;
};

struct clientdata_t
{
	Vector				origin;
	Vector				velocity;

	string_t			viewmodel;
	Vector				punchangle;
	int					flags;
	WaterLevel waterlevel;
	Contents watertype;
	Vector				view_ofs;
	float				health;

	int					bInDuck;

	int					weapons; // remove?

	int					flTimeStepSound;
	int					flDuckTime;
	int					flSwimTime;
	int					waterjumptime;

	float				maxspeed;

	float				fov;
	int					weaponanim;

	int					m_iId;
	int					ammo_shells;
	int					ammo_nails;
	int					ammo_cells;
	int					ammo_rockets;
	float				m_flNextAttack;

	int					tfstate;

	int					pushmsec;

	DeadFlag deadflag;

	char				physinfo[MAX_PHYSINFO_STRING];

	// For mods
	int					iuser1;
	int					iuser2;
	int					iuser3;
	int					iuser4;
	float				fuser1;
	float				fuser2;
	float				fuser3;
	float				fuser4;
	Vector				vuser1;
	Vector				vuser2;
	Vector				vuser3;
	Vector				vuser4;
};

struct local_state_t
{
	entity_state_t playerstate;
	clientdata_t   client;
	weapon_data_t  weapondata[64];
};
