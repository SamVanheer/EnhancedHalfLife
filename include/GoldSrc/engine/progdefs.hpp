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

#include "Platform.hpp"
#include "const.hpp"
#include "mathlib.hpp"

struct edict_t;

struct globalvars_t
{
	float		time = 0;
	float		frametime = 0;
	float		force_retouch = 0;
	string_t	mapname = iStringNull;
	string_t	startspot = iStringNull;
	float		deathmatch = 0;
	float		coop = 0;
	float		teamplay = 0;
	float		serverflags = 0;
	float		found_secrets = 0;
	Vector		v_forward;
	Vector		v_up;
	Vector		v_right;
	float		trace_allsolid = 0;
	float		trace_startsolid = 0;
	float		trace_fraction = 0;
	Vector		trace_endpos;
	Vector		trace_plane_normal;
	float		trace_plane_dist = 0;
	edict_t*	trace_ent = nullptr;
	float		trace_inopen = 0;
	float		trace_inwater = 0;
	int			trace_hitgroup = 0;
	int			trace_flags = 0;
	int			msg_entity = 0;
	int			cdAudioTrack = 0;
	int			maxClients = 0;
	int			maxEntities = 0;
	const char* pStringBase = "";

	void* pSaveData = nullptr;
	Vector		vecLandmarkOffset;
};

struct entvars_t
{
	string_t	classname = iStringNull;
	string_t	globalname = iStringNull;

	Vector		origin;
	Vector		oldorigin;
	Vector		velocity;
	Vector		basevelocity;
	Vector      clbasevelocity;  // Base velocity that was passed in to server physics so 
								 //  client can predict conveyors correctly.  Server zeroes it, so we need to store here, too.
	Vector		movedir;

	Vector		angles;			// Model angles
	Vector		avelocity;		// angle velocity (degrees per second)
	Vector		punchangle;		// auto-decaying view angle adjustment
	Vector		v_angle;		// Viewing angle (player only)

	// For parametric entities
	Vector		endpos;
	Vector		startpos;
	float		impacttime = 0;
	float		starttime = 0;

	FixAngleMode fixangle = FixAngleMode::None;
	float		idealpitch = 0;
	float		pitch_speed = 0;
	float		ideal_yaw = 0;
	float		yaw_speed = 0;

	int			modelindex = 0;
	string_t	model = iStringNull;

	string_t viewmodel = iStringNull;		// player's viewmodel
	string_t weaponmodel = iStringNull;		// what other players see

	Vector		absmin;		// BB max translated to world coord
	Vector		absmax;		// BB max translated to world coord
	Vector		mins;		// local BB min
	Vector		maxs;		// local BB max
	Vector		size;		// maxs - mins

	float		ltime = 0;
	float		nextthink = 0;

	Movetype movetype = Movetype::None;
	Solid solid = Solid::Not;

	int			skin = 0;
	int			body = 0;			// sub-model selection for studiomodels
	int 		effects = 0;

	float		gravity = 0;		// % of "normal" gravity
	float		friction = 0;		// inverse elasticity of Movetype::Bounce

	int			light_level = 0;

	int			sequence = 0;		// animation sequence
	int			gaitsequence = 0;	// movement animation sequence for player (0 for none)
	float		frame = 0;			// % playback position in animation sequences (0..255)
	float		animtime = 0;		// world time when frame was set
	float		framerate = 0;		// animation playback rate (-8x to 8x)
	byte		controller[4]{};	// bone controller setting (0..255)
	byte		blending[2]{};		// blending amount between sub-sequences (0..255)

	float		scale = 0;			// sprite rendering scale (0..255)

	RenderMode rendermode = RenderMode::Normal;
	float		renderamt = 0;
	Vector		rendercolor;
	RenderFX renderfx = RenderFX::None;

	float		health = 0;
	float		frags = 0;
	int			weapons = 0;  // bit mask for available weapons
	float		takedamage = 0;

	DeadFlag deadflag = DeadFlag::No;
	Vector		view_ofs;	// eye position

	int			button = 0;
	int			impulse = 0;

	edict_t* chain = nullptr;			// Entity pointer when linked into a linked list
	edict_t* dmg_inflictor = nullptr;
	edict_t* enemy = nullptr;
	edict_t* aiment = nullptr;			// entity pointer when MOVETYPE_FOLLOW
	edict_t* owner = nullptr;
	edict_t* groundentity = nullptr;

	int			spawnflags = 0;
	int			flags = 0;

	int			colormap = 0;			// lowbyte topcolor, highbyte bottomcolor
	int			team = 0;

	float		max_health = 0;
	float		teleport_time = 0;
	float		armortype = 0;
	float		armorvalue = 0;
	WaterLevel waterlevel = WaterLevel::Dry;
	Contents watertype = Contents::None;

	string_t	target = iStringNull;
	string_t	targetname = iStringNull;
	string_t	netname = iStringNull;
	string_t	message = iStringNull;

	float		dmg_take = 0;
	float		dmg_save = 0;
	float		dmg = 0;
	float		dmgtime = 0;

	string_t	noise = iStringNull;
	string_t	noise1 = iStringNull;
	string_t	noise2 = iStringNull;
	string_t	noise3 = iStringNull;

	float		speed = 0;
	float		air_finished = 0;
	float		pain_finished = 0;
	float		radsuit_finished = 0;

	edict_t* pContainingEntity = nullptr;

	int			playerclass = 0;
	float		maxspeed = 0;

	float		fov = 0;
	int			weaponanim = 0;

	int			pushmsec = 0;

	int			bInDuck = 0;
	int			flTimeStepSound = 0;
	int			flSwimTime= 0;
	int			flDuckTime= 0;
	int			iStepLeft = 0;
	float		flFallVelocity = 0;

	int			gamestate = 0;

	int			oldbuttons = 0;

	int			groupinfo = 0;

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
	edict_t* euser1 = nullptr;
	edict_t* euser2 = nullptr;
	edict_t* euser3 = nullptr;
	edict_t* euser4 = nullptr;
};
