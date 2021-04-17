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

/*
==============================================================================

STUDIO MODELS

Studio models are position independent, so the cache manager can move them.
==============================================================================
*/


constexpr int MAXSTUDIOTRIANGLES = 20000;	// TODO: tune this
constexpr int MAXSTUDIOVERTS = 2048;		// TODO: tune this
constexpr int MAXSTUDIOSEQUENCES = 2048;	// total animation sequences -- KSH incremented
constexpr int MAXSTUDIOSKINS = 100;			// total textures
constexpr int MAXSTUDIOSRCBONES = 512;		// bones allowed at source movement
constexpr int MAXSTUDIOBONES = 128;			// total bones actually used
constexpr int MAXSTUDIOMODELS = 32;			// sub-models per model
constexpr int MAXSTUDIOBODYPARTS = 32;
constexpr int MAXSTUDIOGROUPS = 16;
constexpr int MAXSTUDIOANIMATIONS = 2048;
constexpr int MAXSTUDIOMESHES = 256;
constexpr int MAXSTUDIOEVENTS = 1024;
constexpr int MAXSTUDIOPIVOTS = 256;
constexpr int MAXSTUDIOCONTROLLERS = 8;

struct studiohdr_t
{
	int					id;
	int					version;

	char				name[64];
	int					length;

	Vector				eyeposition;	// ideal eye position
	Vector				min;			// ideal movement hull size
	Vector				max;

	Vector				bbmin;			// clipping bounding box
	Vector				bbmax;

	int					flags;

	int					numbones;			// bones
	int					boneindex;

	int					numbonecontrollers;		// bone controllers
	int					bonecontrollerindex;

	int					numhitboxes;			// complex bounding boxes
	int					hitboxindex;

	int					numseq;				// animation sequences
	int					seqindex;

	int					numseqgroups;		// demand loaded sequences
	int					seqgroupindex;

	int					numtextures;		// raw textures
	int					textureindex;
	int					texturedataindex;

	int					numskinref;			// replaceable textures
	int					numskinfamilies;
	int					skinindex;

	int					numbodyparts;
	int					bodypartindex;

	int					numattachments;		// queryable attachable points
	int					attachmentindex;

	int					soundtable;
	int					soundindex;
	int					soundgroups;
	int					soundgroupindex;

	int					numtransitions;		// animation node to animation node transition graph
	int					transitionindex;
};

// header for demand loaded sequence group data
struct studioseqhdr_t
{
	int					id;
	int					version;

	char				name[64];
	int					length;
};

// bones
struct mstudiobone_t
{
	char				name[32];	// bone name for symbolic links
	int		 			parent;		// parent bone
	int					flags;		// ??
	int					bonecontroller[6];	// bone controller index, -1 == none
	float				value[6];	// default DoF values
	float				scale[6];   // scale for delta DoF values
};


// bone controllers
struct mstudiobonecontroller_t
{
	int					bone;	// -1 == 0
	int					type;	// X, Y, Z, XR, YR, ZR, M
	float				start;
	float				end;
	int					rest;	// byte index value at rest
	int					index;	// 0-3 user set controller, 4 mouth
};

// intersection boxes
struct mstudiobbox_t
{
	int					bone;
	int					group;			// intersection group
	Vector				bbmin;		// bounding box
	Vector				bbmax;
};

//
// demand loaded sequence groups
//
struct mstudioseqgroup_t
{
	char				label[32];	// textual name
	char				name[64];	// file name
	int32				unused1;    // was "cache"  - index pointer
	int					unused2;    // was "data" -  hack for group 0
};

// sequence descriptions
struct mstudioseqdesc_t
{
	char				label[32];	// sequence label

	float				fps;		// frames per second	
	int					flags;		// looping/non-looping flags

	int					activity;
	int					actweight;

	int					numevents;
	int					eventindex;

	int					numframes;	// number of frames per sequence

	int					numpivots;	// number of foot pivots
	int					pivotindex;

	int					motiontype;
	int					motionbone;
	Vector				linearmovement;
	int					automoveposindex;
	int					automoveangleindex;

	Vector				bbmin;		// per sequence bounding box
	Vector				bbmax;

	int					numblends;
	int					animindex;		// mstudioanim_t pointer relative to start of sequence group data
										// [blend][bone][X, Y, Z, XR, YR, ZR]

	int					blendtype[2];	// X, Y, Z, XR, YR, ZR
	float				blendstart[2];	// starting value
	float				blendend[2];	// ending value
	int					blendparent;

	int					seqgroup;		// sequence group for demand loading

	int					entrynode;		// transition node at entry
	int					exitnode;		// transition node at exit
	int					nodeflags;		// transition rules

	int					nextseq;		// auto advancing sequences
};

struct mstudioevent_t
{
	int 				frame;
	int					event;
	int					type;
	char				options[64];
};

// pivots
struct mstudiopivot_t
{
	Vector				org;	// pivot point
	int					start;
	int					end;
};

// attachment
struct mstudioattachment_t
{
	char				name[32];
	int					type;
	int					bone;
	Vector				org;	// attachment point
	Vector				vectors[3];
};

struct mstudioanim_t
{
	unsigned short	offset[6];
};

// animation frames
union mstudioanimvalue_t
{
	struct
	{
		byte	valid;
		byte	total;
	} num;
	short		value;
};



// body part index
struct mstudiobodyparts_t
{
	char				name[64];
	int					nummodels;
	int					base;
	int					modelindex; // index into models array
};



// skin info
struct mstudiotexture_t
{
	char					name[64];
	int						flags;
	int						width;
	int						height;
	int						index;
};


// skin families
// short	index[skinfamilies][skinref]

// studio models
struct mstudiomodel_t
{
	char				name[64];

	int					type;

	float				boundingradius;

	int					nummesh;
	int					meshindex;

	int					numverts;		// number of unique vertices
	int					vertinfoindex;	// vertex bone info
	int					vertindex;		// vertex Vector
	int					numnorms;		// number of unique surface normals
	int					norminfoindex;	// normal bone info
	int					normindex;		// normal Vector

	int					numgroups;		// deformation groups
	int					groupindex;
};


// Vector	boundingbox[model][bone][2];	// complex intersection info


// meshes
struct mstudiomesh_t
{
	int					numtris;
	int					triindex;
	int					skinref;
	int					numnorms;		// per mesh normals
	int					normindex;		// normal Vector
};

// triangles
#if 0
struct mstudiotrivert_t
{
	short				vertindex;		// index into vertex array
	short				normindex;		// index into normal array
	short				s, t;			// s,t position on skin
};
#endif

// lighting options
constexpr int STUDIO_NF_FLATSHADE = 0x0001;
constexpr int STUDIO_NF_CHROME = 0x0002;
constexpr int STUDIO_NF_FULLBRIGHT = 0x0004;
constexpr int STUDIO_NF_NOMIPS = 0x0008;
constexpr int STUDIO_NF_ALPHA = 0x0010;
constexpr int STUDIO_NF_ADDITIVE = 0x0020;
constexpr int STUDIO_NF_MASKED = 0x0040;

// motion flags
constexpr int STUDIO_X = 0x0001;
constexpr int STUDIO_Y = 0x0002;
constexpr int STUDIO_Z = 0x0004;
constexpr int STUDIO_XR = 0x0008;
constexpr int STUDIO_YR = 0x0010;
constexpr int STUDIO_ZR = 0x0020;
constexpr int STUDIO_LX = 0x0040;
constexpr int STUDIO_LY = 0x0080;
constexpr int STUDIO_LZ = 0x0100;
constexpr int STUDIO_AX = 0x0200;
constexpr int STUDIO_AY = 0x0400;
constexpr int STUDIO_AZ = 0x0800;
constexpr int STUDIO_AXR = 0x1000;
constexpr int STUDIO_AYR = 0x2000;
constexpr int STUDIO_AZR = 0x4000;
constexpr int STUDIO_TYPES = 0x7FFF;
constexpr int STUDIO_RLOOP = 0x8000;	// controller that wraps shortest distance

// sequence flags
constexpr int STUDIO_LOOPING = 0x0001;

// bone flags
constexpr int STUDIO_HAS_NORMALS = 0x0001;
constexpr int STUDIO_HAS_VERTICES = 0x0002;
constexpr int STUDIO_HAS_BBOX = 0x0004;
constexpr int STUDIO_HAS_CHROME = 0x0008;	// if any of the textures have chrome on them

#define RAD_TO_STUDIO		(32768.0/M_PI)
#define STUDIO_TO_RAD		(M_PI/32768.0)
