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

/**
*	@file
*
*	Constants shared by the engine and dlls
*	This header file included by engine files and DLL files.
*	Most came from server.h
*/

/**
*	@brief Maximum value that a coordinate can have before it's considered out of the map.
*
*	Applies to both positive and negative values ([-WORLD_BOUNDARY, WORLD_BOUNDARY])
*/
constexpr float WORLD_BOUNDARY = 4096;

/**
*	@brief Maximum size of the world on any given axis
*/
constexpr float WORLD_SIZE = WORLD_BOUNDARY * 2;

// edict->flags
constexpr int FL_FLY = 1 << 0;				//!< Changes the SV_Movestep() behavior to not need to be on ground
constexpr int FL_SWIM = 1 << 1;				//!< Changes the SV_Movestep() behavior to not need to be on ground (but stay in water)
constexpr int FL_CONVEYOR = 1 << 2;
constexpr int FL_CLIENT = 1 << 3;
constexpr int FL_INWATER = 1 << 4;
constexpr int FL_MONSTER = 1 << 5;
constexpr int FL_GODMODE = 1 << 6;
constexpr int FL_NOTARGET = 1 << 7;
constexpr int FL_SKIPLOCALHOST = 1 << 8;	//!< Don't send entity to local host, it's predicting this entity itself
constexpr int FL_ONGROUND = 1 << 9;			//!< At rest / on the ground
constexpr int FL_PARTIALGROUND = 1 << 10;	//!< not all corners are valid
constexpr int FL_WATERJUMP = 1 << 11;		//!< player jumping out of water
constexpr int FL_FROZEN = 1 << 12;			//!< Player is frozen for 3rd person camera
constexpr int FL_FAKECLIENT = 1 << 13;		//!< JAC: fake client, simulated server side; don't send network messages to them
constexpr int FL_DUCKING = 1 << 14;			//!< Player flag -- Player is fully crouched
constexpr int FL_FLOAT = 1 << 15;			//!< Apply floating force to this entity when in water
constexpr int FL_GRAPHED = 1 << 16;			//!< worldgraph has this ent listed as something that blocks a connection

// UNDONE: Do we need these?
constexpr int FL_IMMUNE_WATER = 1 << 17;
constexpr int FL_IMMUNE_SLIME = 1 << 18;
constexpr int FL_IMMUNE_LAVA = 1 << 19;

constexpr int FL_PROXY = 1 << 20;			//!< This is a spectator proxy
constexpr int FL_ALWAYSTHINK = 1 << 21;		//!< Brush model flag -- call think every frame regardless of nextthink - ltime (for constantly changing velocity/path)
constexpr int FL_BASEVELOCITY = 1 << 22;	//!< Base velocity has been applied this frame (used to convert base velocity into momentum)
constexpr int FL_MONSTERCLIP = 1 << 23;		//!< Only collide in with monsters who have FL_MONSTERCLIP set
constexpr int FL_ONTRAIN = 1 << 24;			//!< Player is _controlling_ a train, so movement commands should be ignored on client during prediction.
constexpr int FL_WORLDBRUSH = 1 << 25;		//!< Not moveable/removeable brush entity (really part of the world, but represented as an entity for transparency or something)
constexpr int FL_SPECTATOR = 1 << 26;		//!< This client is a spectator, don't run touch functions, etc.
constexpr int FL_CUSTOMENTITY = 1 << 29;	//!< This is a custom entity
constexpr int FL_KILLME = 1 << 30;			//!< This entity is marked for death -- This allows the engine to kill ents at the appropriate time
constexpr int FL_DORMANT = 1 << 31;			//!< Entity is dormant, no updates to client

// Goes into globalvars_t.trace_flags
constexpr int FTRACE_SIMPLEBOX = 1 << 0;	//!< Traceline with a simple box

/**
*	@brief walkmove modes
*/
enum class WalkMoveMode
{
	Normal = 0,		//!< normal walkmove
	WorldOnly = 1,	//!< doesn't hit ANY entities, no matter what the solid type
	CheckOnly = 2	//!< move, but don't touch triggers
};

/**
*	@brief edict->movetype values
*/
enum class Movetype
{
	None = 0,				//!< never moves
	//AngleNoclip = 1,
	//AngleClip = 2,
	Walk = 3,				//!< Player only - moving on the ground
	Step = 4,				//!< gravity, special edge handling -- monsters use this
	Fly = 5,				//!< No gravity, but still collides with stuff
	Toss = 6,				//!< gravity/collisions
	Push = 7,				//!< no clip to world, push and crush
	Noclip = 8,				//!< No gravity, no collisions, still do velocity/avelocity
	FlyMissile = 9,			//!< extra size to monsters
	Bounce = 10,			//!< Just like Toss, but reflect velocity when contacting surfaces
	BounceMissile = 11,		//!< bounce w/o gravity
	Follow = 12,			//!< track movement of aiment
	PushStep = 13,			//!< BSP model that needs physics/world collisions (uses nearest hull for world collision)
};

/**
*	@brief edict->solid values
*	NOTE: Some movetypes will cause collisions independent of Solid::Not/Solid::Trigger when the entity moves
*	SOLID only effects OTHER entities colliding with this one when they move - UGH!
*/
enum class Solid
{
	Not = 0,		//!< no interaction with other objects
	Trigger = 1,	//!< touch on edge, but not blocking
	BBox = 2,		//!< touch on edge, block
	SlideBox = 3,	//!< touch on edge, but not an onground
	BSP = 4,		//!< bsp clip, touch on edge, block
};

/**
*	@brief edict->deadflag values
*/
enum class DeadFlag
{
	No = 0,				//!< alive
	Dying = 1,			//!< playing death animation or still falling off of a ledge waiting to hit ground
	Dead = 2,			//!< dead. lying still.
	Respawnable = 3,
	DiscardBody = 4,
	DeaderThanDead = 5,	//!< Used by client weapons code. Never set
};

enum class DamageMode
{
	No = 0,
	Yes,
	Aim
};

// entity effects
constexpr int EF_BRIGHTFIELD = 1;			//!< swirling cloud of particles
constexpr int EF_MUZZLEFLASH = 2;			//!< single frame ELIGHT on entity attachment 0
constexpr int EF_BRIGHTLIGHT = 4;			//!< DLIGHT centered at entity origin
constexpr int EF_DIMLIGHT = 8;				//!< player flashlight
constexpr int EF_INVLIGHT = 16;				//!< get lighting from ceiling
constexpr int EF_NOINTERP = 32;				//!< don't interpolate the next frame
constexpr int EF_LIGHT = 64;				//!< rocket flare glow sprite
constexpr int EF_NODRAW = 128;				//!< don't draw entity
constexpr int EF_NIGHTVISION = 256;			//!< player nightvision
constexpr int EF_SNIPERLASER = 512;			//!< sniper laser effect
constexpr int EF_FIBERCAMERA = 1024;		//!< fiber camera

// entity flags
constexpr int EFLAG_SLERP = 1;				//!< do studio interpolation of this entity

enum class WaterLevel
{
	Dry = 0,	//!< not in water
	Feet,		//!< feet in water
	Waist,		//!< waist in water
	Head		//!< head in water
};

enum class Hull
{
	Point = 0,
	Human = 1,
	Large = 2,
	Head = 3
};

/**
*	@brief Differs from ::Hull, used in physics code
*/
enum class PlayerHull
{
	Standing = 0,
	Crouched = 1,
	Point = 2
};

//
// temp entity events
//
constexpr int TE_BEAMPOINTS = 0;		// beam effect between two points
// coord coord coord (start position) 
// coord coord coord (end position) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

constexpr int TE_BEAMENTPOINT = 1;		// beam effect between point and entity
// short (start entity) 
// coord coord coord (end position) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

constexpr int TE_GUNSHOT = 2;		// particle effect plus ricochet sound
// coord coord coord (position) 

constexpr int TE_EXPLOSION = 3;		// additive sprite, 2 dynamic lights, flickering particles, explosion sound, move vertically 8 pps
// coord coord coord (position) 
// short (sprite index)
// byte (scale in 0.1's)
// byte (framerate)
// byte (flags)
//
// The Explosion effect has some flags to control performance/aesthetic features:
constexpr int TE_EXPLFLAG_NONE = 0;			//!< all flags clear makes default Half-Life explosion
constexpr int TE_EXPLFLAG_NOADDITIVE = 1;	//!< sprite will be drawn opaque (ensure that the sprite you send is a non-additive sprite)
constexpr int TE_EXPLFLAG_NODLIGHTS = 2;	//!< do not render dynamic lights
constexpr int TE_EXPLFLAG_NOSOUND = 4;		//!< do not play client explosion sound
constexpr int TE_EXPLFLAG_NOPARTICLES = 8;	//!< do not draw particles

constexpr int TE_TAREXPLOSION = 4;		// Quake1 "tarbaby" explosion with sound
// coord coord coord (position) 

constexpr int TE_SMOKE = 5;		// alphablend sprite, move vertically 30 pps
// coord coord coord (position) 
// short (sprite index)
// byte (scale in 0.1's)
// byte (framerate)

constexpr int TE_TRACER = 6;		// tracer effect from point to point
// coord, coord, coord (start) 
// coord, coord, coord (end)

constexpr int TE_LIGHTNING = 7;		// TE_BEAMPOINTS with simplified parameters
// coord, coord, coord (start) 
// coord, coord, coord (end) 
// byte (life in 0.1's) 
// byte (width in 0.1's) 
// byte (amplitude in 0.01's)
// short (sprite model index)

constexpr int TE_BEAMENTS = 8;
// short (start entity) 
// short (end entity) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

constexpr int TE_SPARKS = 9;		// 8 random tracers with gravity, ricochet sprite
// coord coord coord (position) 

constexpr int TE_LAVASPLASH = 10;		// Quake1 lava splash
// coord coord coord (position) 

constexpr int TE_TELEPORT = 11;		// Quake1 teleport splash
// coord coord coord (position) 

constexpr int TE_EXPLOSION2 = 12;		// Quake1 colormaped (base palette) particle explosion with sound
// coord coord coord (position) 
// byte (starting color)
// byte (num colors)

constexpr int TE_BSPDECAL = 13;		// Decal from the .BSP file 
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// short (texture index of precached decal texture name)
// short (entity index)
// [optional - only included if previous short is non-zero (not the world)] short (index of model of above entity)

constexpr int TE_IMPLOSION = 14;		// tracers moving toward a point
// coord, coord, coord (position)
// byte (radius)
// byte (count)
// byte (life in 0.1's) 

constexpr int TE_SPRITETRAIL = 15;		// line of moving glow sprites with gravity, fadeout, and collisions
// coord, coord, coord (start) 
// coord, coord, coord (end) 
// short (sprite index)
// byte (count)
// byte (life in 0.1's) 
// byte (scale in 0.1's) 
// byte (velocity along vector in 10's)
// byte (randomness of velocity in 10's)

constexpr int TE_BEAM = 16;		// obsolete

constexpr int TE_SPRITE = 17;		// additive sprite, plays 1 cycle
// coord, coord, coord (position) 
// short (sprite index) 
// byte (scale in 0.1's) 
// byte (brightness)

constexpr int TE_BEAMSPRITE = 18;		// A beam with a sprite at the end
// coord, coord, coord (start position) 
// coord, coord, coord (end position) 
// short (beam sprite index) 
// short (end sprite index) 

constexpr int TE_BEAMTORUS = 19;		// screen aligned beam ring, expands to max radius over lifetime
// coord coord coord (center position) 
// coord coord coord (axis and radius) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

constexpr int TE_BEAMDISK = 20;		// disk that expands to max radius over lifetime
// coord coord coord (center position) 
// coord coord coord (axis and radius) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

constexpr int TE_BEAMCYLINDER = 21;		// cylinder that expands to max radius over lifetime
// coord coord coord (center position) 
// coord coord coord (axis and radius) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

constexpr int TE_BEAMFOLLOW = 22;		// create a line of decaying beam segments until entity stops moving
// short (entity:attachment to follow)
// short (sprite index)
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte,byte,byte (color)
// byte (brightness)

constexpr int TE_GLOWSPRITE = 23;
// coord, coord, coord (pos) short (model index) byte (scale / 10)

constexpr int TE_BEAMRING = 24;		// connect a beam ring to two entities
// short (start entity) 
// short (end entity) 
// short (sprite index) 
// byte (starting frame) 
// byte (frame rate in 0.1's) 
// byte (life in 0.1's) 
// byte (line width in 0.1's) 
// byte (noise amplitude in 0.01's) 
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

constexpr int TE_STREAK_SPLASH = 25;		// oriented shower of tracers
// coord coord coord (start position) 
// coord coord coord (direction vector) 
// byte (color)
// short (count)
// short (base speed)
// short (ramdon velocity)

constexpr int TE_BEAMHOSE = 26;		// obsolete

constexpr int TE_DLIGHT = 27;		// dynamic light, effect world, minor entity effect
// coord, coord, coord (pos) 
// byte (radius in 10's) 
// byte byte byte (color)
// byte (brightness)
// byte (life in 10's)
// byte (decay rate in 10's)

constexpr int TE_ELIGHT = 28;		// point entity light, no world effect
// short (entity:attachment to follow)
// coord coord coord (initial position) 
// coord (radius)
// byte byte byte (color)
// byte (life in 0.1's)
// coord (decay rate)

constexpr int TE_TEXTMESSAGE = 29;
// short 1.2.13 x (-1 = center)
// short 1.2.13 y (-1 = center)
// byte Effect 0 = fade in/fade out
			// 1 is flickery credits
			// 2 is write out (training room)

// 4 bytes r,g,b,a color1	(text color)
// 4 bytes r,g,b,a color2	(effect color)
// ushort 8.8 fadein time
// ushort 8.8  fadeout time
// ushort 8.8 hold time
// optional ushort 8.8 fxtime	(time the highlight lags behing the leading text in effect 2)
// string text message		(512 chars max sz string)
constexpr int TE_LINE = 30;
// coord, coord, coord		startpos
// coord, coord, coord		endpos
// short life in 0.1 s
// 3 bytes r, g, b

constexpr int TE_BOX = 31;
// coord, coord, coord		boxmins
// coord, coord, coord		boxmaxs
// short life in 0.1 s
// 3 bytes r, g, b

constexpr int TE_KILLBEAM = 99;		// kill all beams attached to entity
// short (entity)

constexpr int TE_LARGEFUNNEL = 100;
// coord coord coord (funnel position)
// short (sprite index) 
// short (flags) 

constexpr int	TE_BLOODSTREAM = 101;		// particle spray
// coord coord coord (start position)
// coord coord coord (spray vector)
// byte (color)
// byte (speed)

constexpr int	TE_SHOWLINE = 102;		// line of particles every 5 units, dies in 30 seconds
// coord coord coord (start position)
// coord coord coord (end position)

constexpr int TE_BLOOD = 103;		// particle spray
// coord coord coord (start position)
// coord coord coord (spray vector)
// byte (color)
// byte (speed)

constexpr int TE_DECAL = 104;	// Decal applied to a brush entity (not the world)
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// byte (texture index of precached decal texture name)
// short (entity index)

constexpr int TE_FIZZ = 105;		// create alpha sprites inside of entity, float upwards
// short (entity)
// short (sprite index)
// byte (density)

constexpr int TE_MODEL = 106;		// create a moving model that bounces and makes a sound when it hits
// coord, coord, coord (position) 
// coord, coord, coord (velocity)
// angle (initial yaw)
// short (model index)
// byte (bounce sound type)
// byte (life in 0.1's)

constexpr int TE_EXPLODEMODEL = 107;		// spherical shower of models, picks from set
// coord, coord, coord (origin)
// coord (velocity)
// short (model index)
// short (count)
// byte (life in 0.1's)

constexpr int TE_BREAKMODEL = 108;		// box of models or sprites
// coord, coord, coord (position)
// coord, coord, coord (size)
// coord, coord, coord (velocity)
// byte (random velocity in 10's)
// short (sprite or model index)
// byte (count)
// byte (life in 0.1 secs)
// byte (flags)

constexpr int TE_GUNSHOTDECAL = 109;		// decal and ricochet sound
// coord, coord, coord (position)
// short (entity index???)
// byte (decal???)

constexpr int TE_SPRITE_SPRAY = 110;		// spay of alpha sprites
// coord, coord, coord (position)
// coord, coord, coord (velocity)
// short (sprite index)
// byte (count)
// byte (speed)
// byte (noise)

constexpr int TE_ARMOR_RICOCHET = 111;		// quick spark sprite, client ricochet sound. 
// coord, coord, coord (position)
// byte (scale in 0.1's)

constexpr int TE_PLAYERDECAL = 112;		// ???
// byte (playerindex)
// coord, coord, coord (position)
// short (entity???)
// byte (decal number???)
// [optional] short (model index???)

constexpr int TE_BUBBLES = 113;		// create alpha sprites inside of box, float upwards
// coord, coord, coord (min start position)
// coord, coord, coord (max start position)
// coord (float height)
// short (model index)
// byte (count)
// coord (speed)

constexpr int TE_BUBBLETRAIL = 114;		// create alpha sprites along a line, float upwards
// coord, coord, coord (min start position)
// coord, coord, coord (max start position)
// coord (float height)
// short (model index)
// byte (count)
// coord (speed)

constexpr int TE_BLOODSPRITE = 115;		// spray of opaque sprite1's that fall, single sprite2 for 1..2 secs (this is a high-priority tent)
// coord, coord, coord (position)
// short (sprite1 index)
// short (sprite2 index)
// byte (color)
// byte (scale)

constexpr int TE_WORLDDECAL = 116;		// Decal applied to the world brush
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// byte (texture index of precached decal texture name)

constexpr int TE_WORLDDECALHIGH = 117;		// Decal (with texture index > 256) applied to world brush
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// byte (texture index of precached decal texture name - 256)

constexpr int TE_DECALHIGH = 118;		// Same as TE_DECAL, but the texture index was greater than 256
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// byte (texture index of precached decal texture name - 256)
// short (entity index)

constexpr int TE_PROJECTILE = 119;		// Makes a projectile (like a nail) (this is a high-priority tent)
// coord, coord, coord (position)
// coord, coord, coord (velocity)
// short (modelindex)
// byte (life)
// byte (owner)  projectile won't collide with owner (if owner == 0, projectile will hit any client).

constexpr int TE_SPRAY = 120;		// Throws a shower of sprites or models
// coord, coord, coord (position)
// coord, coord, coord (direction)
// short (modelindex)
// byte (count)
// byte (speed)
// byte (noise)
// byte (rendermode)

constexpr int TE_PLAYERSPRITES = 121;		// sprites emit from a player's bounding box (ONLY use for players!)
// byte (playernum)
// short (sprite modelindex)
// byte (count)
// byte (variance) (0 = no variance in size) (10 = 10% variance in size)

constexpr int TE_PARTICLEBURST = 122;		// very similar to lavasplash.
// coord (origin)
// short (radius)
// byte (particle color)
// byte (duration * 10) (will be randomized a bit)

constexpr int TE_FIREFIELD = 123;		// makes a field of fire.
// coord (origin)
// short (radius) (fire is made in a square around origin. -radius, -radius to radius, radius)
// short (modelindex)
// byte (count)
// byte (flags)
// byte (duration (in seconds) * 10) (will be randomized a bit)
//
// to keep network traffic low, this message has associated flags that fit into a byte:
constexpr int TEFIRE_FLAG_ALLFLOAT = 1;		//!< all sprites will drift upwards as they animate
constexpr int TEFIRE_FLAG_SOMEFLOAT = 2;	//!< some of the sprites will drift upwards. (50% chance)
constexpr int TEFIRE_FLAG_LOOP = 4;			//!< if set, sprite plays at 15 fps, otherwise plays at whatever rate stretches the animation over the sprite's duration.
constexpr int TEFIRE_FLAG_ALPHA = 8;		//!< if set, sprite is rendered alpha blended at 50% else, opaque
constexpr int TEFIRE_FLAG_PLANAR = 16;		//!< if set, all fire sprites have same initial Z instead of randomly filling a cube. 
constexpr int TEFIRE_FLAG_ADDITIVE = 32;	//!< if set, sprite is rendered non-opaque with additive

constexpr int TE_PLAYERATTACHMENT = 124; // attaches a TENT to a player (this is a high-priority tent)
// byte (entity index of player)
// coord (vertical offset) ( attachment origin.z = player origin.z + vertical offset )
// short (model index)
// short (life * 10 );

constexpr int TE_KILLPLAYERATTACHMENTS = 125; // will expire all TENTS attached to a player.
// byte (entity index of player)

constexpr int TE_MULTIGUNSHOT = 126; // much more compact shotgun message
// This message is used to make a client approximate a 'spray' of gunfire.
// Any weapon that fires more than one bullet per frame and fires in a bit of a spread is
// a good candidate for MULTIGUNSHOT use. (shotguns)
//
// NOTE: This effect makes the client do traces for each bullet, these client traces ignore
//		 entities that have studio models.Traces are 4096 long.
//
// coord (origin)
// coord (origin)
// coord (origin)
// coord (direction)
// coord (direction)
// coord (direction)
// coord (x noise * 100)
// coord (y noise * 100)
// byte (count)
// byte (bullethole decal texture index)

constexpr int TE_USERTRACER = 127; // larger message than the standard tracer, but allows some customization.
// coord (origin)
// coord (origin)
// coord (origin)
// coord (velocity)
// coord (velocity)
// coord (velocity)
// byte ( life * 10 )
// byte ( color ) this is an index into an array of color vectors in the engine. (0 - )
// byte ( length * 10 )

enum class MessageDest
{
	Broadcast = 0,			//!< unreliable to all
	One = 1,				//!< reliable to one (msg_entity)
	All = 2,				//!< reliable to all
	Init = 3,				//!< write to the init string
	PVS = 4,				//!< Ents in PVS of org
	PAS = 5,				//!< Ents in PAS of org
	PVSReliable = 6,		//!< Reliable to PVS
	PASReliable = 7,		//!< Reliable to PAS
	OneUnreliable = 8,		//!< Send to one client, but don't put in reliable stream, put in unreliable datagram ( could be dropped )
	Spectator = 9,			//!< Sends to all spectator proxies
};

/**
*	@brief contents of a spot in the world
*/
enum class Contents
{
	None = 0,
	Empty = -1,
	Solid = -2,
	Water = -3,
	Slime = -4,
	Lava = -5,
	Sky = -6,
	//These additional contents constants are defined in bspfile.h
	Origin = -7,		//!< removed at csg time
	Clip = -8,			//!< changed to contents_solid
	Current0 = -9,
	Current90 = -10,
	Current180 = -11,
	Current270 = -12,
	CurrentUp = -13,
	CurrentDown = -14,

	Translucent = -15,
	Ladder = -16,

	FlyField = -17,
	GravityFlyField = -18,
	Fog = -19,
};

enum class SoundChannel
{
	Auto = 0,
	Weapon = 1,
	Voice = 2,
	Item = 3,
	Body = 4,
	Stream = 5,				//!< allocate stream channel from the static or dynamic area
	Static = 6,				//!< allocate channel from the static area 
	NetworkVoiceBase = 7,	//!< voice data coming across the network
	NetworkVoiceEnd = 500,	//!< network voice data reserves slots (NetworkVoiceBase through NetworkVoiceEnd).
	Bot = 501,				//!< channel used for bot chatter.
};

// attenuation values
constexpr float ATTN_NONE = 0;
constexpr float ATTN_NORM = 0.8;
constexpr float ATTN_IDLE = 2;
constexpr float ATTN_STATIC = 1.25;

// pitch values
constexpr int PITCH_NORM = 100;				//!< non-pitch shifted
constexpr int PITCH_LOW = 95;				//!< other values are possible - 0-255, where 255 is very high
constexpr int PITCH_HIGH = 120;

// volume values
constexpr float VOL_NORM = 1.0;

/**
*	@brief view angle update types for fixangle
*/
enum class FixAngleMode
{
	None = 0,	//!< nothing
	Absolute,	//!< force view angles
	Relative	//!< add avelocity
};

// buttons
#include "in_buttons.h"

// Break Model Defines

constexpr int BREAK_TYPEMASK = 0x4F;
constexpr int BREAK_GLASS = 0x01;
constexpr int BREAK_METAL = 0x02;
constexpr int BREAK_FLESH = 0x04;
constexpr int BREAK_WOOD = 0x08;

constexpr int BREAK_SMOKE = 0x10;
constexpr int BREAK_TRANS = 0x20;
constexpr int BREAK_CONCRETE = 0x40;
constexpr int BREAK_2 = 0x80;

// Colliding temp entity sounds

constexpr int BOUNCE_GLASS = BREAK_GLASS;
constexpr int BOUNCE_METAL = BREAK_METAL;
constexpr int BOUNCE_FLESH = BREAK_FLESH;
constexpr int BOUNCE_WOOD = BREAK_WOOD;
constexpr int BOUNCE_SHRAP = 0x10;
constexpr int BOUNCE_SHELL = 0x20;
constexpr int BOUNCE_CONCRETE = BREAK_CONCRETE;
constexpr int BOUNCE_SHOTSHELL = 0x80;

// Temp entity bounce sound types
constexpr int TE_BOUNCE_NULL = 0;
constexpr int TE_BOUNCE_SHELL = 1;
constexpr int TE_BOUNCE_SHOTSHELL = 2;

// Rendering constants
enum class RenderMode
{
	Normal,			//!< src
	TransColor,		//!< c*a+dest*(1-a)
	TransTexture,	//!< src*a+dest*(1-a)
	Glow,			//!< src*a+dest -- No Z buffer checks
	TransAlpha,		//!< src*srca+dest*(1-srca)
	TransAdd,		//!< src*a+dest
};

enum class RenderFX
{
	None = 0,
	PulseSlow,
	PulseFast,
	PulseSlowWide,
	PulseFastWide,
	FadeSlow,
	FadeFast,
	SolidSlow,
	SolidFast,
	StrobeSlow,
	StrobeFast,
	StrobeFaster,
	FlickerSlow,
	FlickerFast,
	NoDissipation,
	Distort,			//!< Distort/scale/translate flicker
	Hologram,			//!< kRenderFxDistort + distance fade
	DeadPlayer,			//!< kRenderAmt is the player index
	Explode,			//!< Scale up really big!
	GlowShell,			//!< Glowing Shell
	ClampMinScale,		//!< Keep this sprite from getting very small (SPRITES only!)
	LightMultiplier,	//!< CTM !!!CZERO added to tell the studiorender that the value in iuser2 is a lightmultiplier
};

struct color24
{
	byte r = 0, g = 0, b = 0;
};

struct colorVec
{
	unsigned r, g, b, a;
};

struct PackedColorVec
{
	unsigned short r, g, b, a;
};

struct link_t
{
	link_t* prev = nullptr, * next = nullptr;
};

struct edict_t;

struct plane_t
{
	Vector	normal{};
	float	dist = 0;
};

struct trace_t
{
	qboolean	allsolid = false;	// if true, plane is not valid
	qboolean	startsolid = false;	// if true, the initial point was in a solid area
	qboolean	inopen = false, inwater = false;
	float	fraction = 0;			// time completed, 1.0 = didn't hit anything
	Vector	endpos{};				// final position
	plane_t	plane;					// surface normal at impact
	edict_t* ent = nullptr;			// entity the surface is on
	int		hitgroup = 0;			// 0 == generic, non zero is specific body part
};
