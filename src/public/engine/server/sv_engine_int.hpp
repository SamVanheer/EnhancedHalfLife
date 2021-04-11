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

//
// Defines entity interface between engine and DLLs.
// This header file included by engine files and DLL files.
//
// Before including this header, DLLs must:
//		include progdefs.h
// This is conveniently done for them in extdll.h
//

#include "archtypes.h"     // DAL
#include <stdio.h>
#include "cvardef.h"
#include "Sequence.h"
#include "crc.h"

struct cvar_t;
struct delta_t;
struct entity_state_t;
struct entvars_t;

enum ALERT_TYPE
{
	at_notice,
	at_console,		// same as at_notice, but forces a ConPrintf, not a message box
	at_aiconsole,	// same as at_console, but only shown if developer level is 2!
	at_warning,
	at_error,
	at_logged		// Server print to console ( only in multiplayer games ).
};

// 4-22-98  JOHN: added for use in pfnClientPrintf
enum PRINT_TYPE
{
	print_console,
	print_center,
	print_chat,
};

// For integrity checking of content on clients
enum FORCE_TYPE
{
	force_exactfile,					// File on client must exactly match server's file
	force_model_samebounds,				// For model files only, the geometry must fit in the same bbox
	force_model_specifybounds,			// For model files only, the geometry must fit in the specified bbox
	force_model_specifybounds_if_avail,	// For Steam model files only, the geometry must fit in the specified bbox (if the file is available)
};

constexpr int TRACE_IGNORE_NOTHING = 0;
constexpr int TRACE_IGNORE_MONSTERS = 1 << 0;
constexpr int TRACE_IGNORE_GLASS = 1 << 8;

// Returned by TraceLine
struct TraceResult
{
	int		fAllSolid;			// if true, plane is not valid
	int		fStartSolid;		// if true, the initial point was in a solid area
	int		fInOpen;
	int		fInWater;
	float	flFraction;			// time completed, 1.0 = didn't hit anything
	Vector	vecEndPos;			// final position
	float	flPlaneDist;
	Vector	vecPlaneNormal;		// surface normal at impact
	edict_t* pHit;				// entity the surface is on
	int		iHitgroup;			// 0 == generic, non zero is specific body part
};

// Engine hands this to DLLs for functionality callbacks
// ONLY ADD NEW FUNCTIONS TO THE END OF THIS STRUCT.
struct enginefuncs_t
{
	int			(*pfnPrecacheModel)			(const char* s);
	int			(*pfnPrecacheSound)			(const char* s);
	void		(*pfnSetModel)				(edict_t* e, const char* m);
	int			(*pfnModelIndex)			(const char* m);
	int			(*pfnModelFrames)			(int modelIndex);
	void		(*pfnSetSize)				(edict_t* e, const float* rgflMin, const float* rgflMax);
	void		(*pfnChangeLevel)			(const char* s1, const char* s2);
	void		(*pfnGetSpawnParms)			(edict_t* ent);
	void		(*pfnSaveSpawnParms)		(edict_t* ent);
	float		(*pfnVecToYaw)				(const float* rgflVector);
	void		(*pfnVecToAngles)			(const float* rgflVectorIn, float* rgflVectorOut);
	void		(*pfnMoveToOrigin)			(edict_t* ent, const float* pflGoal, float dist, int iMoveType);
	void		(*pfnChangeYaw)				(edict_t* ent);
	void		(*pfnChangePitch)			(edict_t* ent);
	edict_t* (*pfnFindEntityByString)	(edict_t* pEdictStartSearchAfter, const char* pszField, const char* pszValue);
	int			(*pfnGetEntityIllum)		(edict_t* pEnt);
	edict_t* (*pfnFindEntityInSphere)	(edict_t* pEdictStartSearchAfter, const float* org, float rad);
	edict_t* (*pfnFindClientInPVS)		(edict_t* pEdict);
	edict_t* (*pfnEntitiesInPVS)			(edict_t* pplayer);
	void		(*pfnMakeVectors)			(const float* rgflVector);
	void		(*pfnAngleVectors)			(const float* rgflVector, float* forward, float* right, float* up);
	edict_t* (*pfnCreateEntity)			();
	void		(*pfnRemoveEntity)			(edict_t* e);
	edict_t* (*pfnCreateNamedEntity)		(string_t className);
	void		(*pfnMakeStatic)			(edict_t* ent);
	int			(*pfnEntIsOnFloor)			(edict_t* e);
	int			(*pfnDropToFloor)			(edict_t* e);
	int			(*pfnWalkMove)				(edict_t* ent, float yaw, float dist, int iMode);
	void		(*pfnSetOrigin)				(edict_t* e, const float* rgflOrigin);
	void		(*pfnEmitSound)				(edict_t* entity, int channel, const char* sample, /*int*/float volume, float attenuation, int fFlags, int pitch);
	void		(*pfnEmitAmbientSound)		(edict_t* entity, const float* pos, const char* samp, float vol, float attenuation, int fFlags, int pitch);
	void		(*pfnTraceLine)				(const float* v1, const float* v2, int fNoMonsters, edict_t* pentToSkip, TraceResult* ptr);
	void		(*pfnTraceToss)				(edict_t* pent, edict_t* pentToIgnore, TraceResult* ptr);
	int			(*pfnTraceMonsterHull)		(edict_t* pEdict, const float* v1, const float* v2, int fNoMonsters, edict_t* pentToSkip, TraceResult* ptr);
	void		(*pfnTraceHull)				(const float* v1, const float* v2, int fNoMonsters, int hullNumber, edict_t* pentToSkip, TraceResult* ptr);
	void		(*pfnTraceModel)			(const float* v1, const float* v2, int hullNumber, edict_t* pent, TraceResult* ptr);
	const char* (*pfnTraceTexture)			(edict_t* pTextureEntity, const float* v1, const float* v2);
	void		(*pfnTraceSphere)			(const float* v1, const float* v2, int fNoMonsters, float radius, edict_t* pentToSkip, TraceResult* ptr);
	void		(*pfnGetAimVector)			(edict_t* ent, float speed, float* rgflReturn);
	void		(*pfnServerCommand)			(const char* str);
	void		(*pfnServerExecute)			();
	void		(*pfnClientCommand)			(edict_t* pEdict, const char* szFmt, ...);
	void		(*pfnParticleEffect)		(const float* org, const float* dir, float color, float count);
	void		(*pfnLightStyle)			(int style, const char* val);
	int			(*pfnDecalIndex)			(const char* name);
	int			(*pfnPointContents)			(const float* rgflVector);
	void		(*pfnMessageBegin)			(int msg_dest, int msg_type, const float* pOrigin, edict_t* ed);
	void		(*pfnMessageEnd)			();
	void		(*pfnWriteByte)				(int iValue);
	void		(*pfnWriteChar)				(int iValue);
	void		(*pfnWriteShort)			(int iValue);
	void		(*pfnWriteLong)				(int iValue);
	void		(*pfnWriteAngle)			(float flValue);
	void		(*pfnWriteCoord)			(float flValue);
	void		(*pfnWriteString)			(const char* sz);
	void		(*pfnWriteEntity)			(int iValue);
	void		(*pfnCVarRegister)			(cvar_t* pCvar);
	float		(*pfnCVarGetFloat)			(const char* szVarName);
	const char* (*pfnCVarGetString)			(const char* szVarName);
	void		(*pfnCVarSetFloat)			(const char* szVarName, float flValue);
	void		(*pfnCVarSetString)			(const char* szVarName, const char* szValue);
	void		(*pfnAlertMessage)			(ALERT_TYPE atype, const char* szFmt, ...);
	void		(*pfnEngineFprintf)			(void* pfile, const char* szFmt, ...);
	void* (*pfnPvAllocEntPrivateData)	(edict_t* pEdict, int32 cb);
	void* (*pfnPvEntPrivateData)		(edict_t* pEdict);
	void		(*pfnFreeEntPrivateData)	(edict_t* pEdict);
	const char* (*pfnSzFromIndex)			(int iString);
	int			(*pfnAllocString)			(const char* szValue);
	entvars_t* (*pfnGetVarsOfEnt)			(edict_t* pEdict);
	edict_t* (*pfnPEntityOfEntOffset)	(int iEntOffset);
	int			(*pfnEntOffsetOfPEntity)	(const edict_t* pEdict);
	int			(*pfnIndexOfEdict)			(const edict_t* pEdict);
	edict_t* (*pfnPEntityOfEntIndex)		(int iEntIndex);
	edict_t* (*pfnFindEntityByVars)		(entvars_t* pvars);
	void* (*pfnGetModelPtr)			(edict_t* pEdict);
	int			(*pfnRegUserMsg)			(const char* pszName, int iSize);
	void		(*pfnAnimationAutomove)		(const edict_t* pEdict, float flTime);
	void		(*pfnGetBonePosition)		(const edict_t* pEdict, int iBone, float* rgflOrigin, float* rgflAngles);
	uint32(*pfnFunctionFromName)	(const char* pName);
	const char* (*pfnNameForFunction)		(uint32 function);
	void		(*pfnClientPrintf)			(edict_t* pEdict, PRINT_TYPE ptype, const char* szMsg); // JOHN: engine callbacks so game DLL can print messages to individual clients
	void		(*pfnServerPrint)			(const char* szMsg);
	const char* (*pfnCmd_Args)				();		// these 3 added 
	const char* (*pfnCmd_Argv)				(int argc);	// so game DLL can easily 
	int			(*pfnCmd_Argc)				();		// access client 'cmd' strings
	void		(*pfnGetAttachment)			(const edict_t* pEdict, int iAttachment, float* rgflOrigin, float* rgflAngles);
	void		(*pfnCRC32_Init)			(CRC32_t* pulCRC);
	void        (*pfnCRC32_ProcessBuffer)   (CRC32_t* pulCRC, void* p, int len);
	void		(*pfnCRC32_ProcessByte)     (CRC32_t* pulCRC, unsigned char ch);
	CRC32_t(*pfnCRC32_Final)			(CRC32_t pulCRC);
	int32(*pfnRandomLong)			(int32  lLow, int32  lHigh);
	float		(*pfnRandomFloat)			(float flLow, float flHigh);
	void		(*pfnSetView)				(const edict_t* pClient, const edict_t* pViewent);
	float		(*pfnTime)					();
	void		(*pfnCrosshairAngle)		(const edict_t* pClient, float pitch, float yaw);
	byte* (*pfnLoadFileForMe)         (const char* filename, int* pLength);
	void        (*pfnFreeFile)              (void* buffer);
	void        (*pfnEndSection)            (const char* pszSectionName); // trigger_endsection
	int 		(*pfnCompareFileTime)       (const char* filename1, const char* filename2, int* iCompare);
	void        (*pfnGetGameDir)            (char* szGetGameDir);
	void		(*pfnCvar_RegisterVariable) (cvar_t* variable);
	void        (*pfnFadeClientVolume)      (const edict_t* pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds);
	void        (*pfnSetClientMaxspeed)     (const edict_t* pEdict, float fNewMaxspeed);
	edict_t* (*pfnCreateFakeClient)		(const char* netname);	// returns nullptr if fake client can't be created
	void		(*pfnRunPlayerMove)			(edict_t* fakeclient, const float* viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec);
	int			(*pfnNumberOfEntities)		();
	char* (*pfnGetInfoKeyBuffer)		(edict_t* e);	// passing in nullptr gets the serverinfo
	char* (*pfnInfoKeyValue)			(char* infobuffer, const char* key);
	void		(*pfnSetKeyValue)			(char* infobuffer, const char* key, const char* value);
	void		(*pfnSetClientKeyValue)		(int clientIndex, char* infobuffer, const char* key, const char* value);
	int			(*pfnIsMapValid)			(const char* filename);
	void		(*pfnStaticDecal)			(const float* origin, int decalIndex, int entityIndex, int modelIndex);
	int			(*pfnPrecacheGeneric)		(const char* s);
	int			(*pfnGetPlayerUserId)		(edict_t* e); // returns the server assigned userid for this player.  useful for logging frags, etc.  returns -1 if the edict couldn't be found in the list of clients
	void		(*pfnBuildSoundMsg)			(edict_t* entity, int channel, const char* sample, /*int*/float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float* pOrigin, edict_t* ed);
	int			(*pfnIsDedicatedServer)		();// is this a dedicated server?
	cvar_t* (*pfnCVarGetPointer)		(const char* szVarName);
	unsigned int (*pfnGetPlayerWONId)		(edict_t* e); // returns the server assigned WONid for this player.  useful for logging frags, etc.  returns -1 if the edict couldn't be found in the list of clients

	// YWB 8/1/99 TFF Physics additions
	void		(*pfnInfo_RemoveKey)		(char* s, const char* key);
	const char* (*pfnGetPhysicsKeyValue)	(const edict_t* pClient, const char* key);
	void		(*pfnSetPhysicsKeyValue)	(const edict_t* pClient, const char* key, const char* value);
	const char* (*pfnGetPhysicsInfoString)	(const edict_t* pClient);
	unsigned short (*pfnPrecacheEvent)		(int type, const char* psz);
	void		(*pfnPlaybackEvent)			(int flags, const edict_t* pInvoker, unsigned short eventindex, float delay, const float* origin, const float* angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);

	unsigned char* (*pfnSetFatPVS)			(float* org);
	unsigned char* (*pfnSetFatPAS)			(float* org);

	int			(*pfnCheckVisibility)		(const edict_t* entity, unsigned char* pset);

	void		(*pfnDeltaSetField)			(delta_t* pFields, const char* fieldname);
	void		(*pfnDeltaUnsetField)		(delta_t* pFields, const char* fieldname);
	void		(*pfnDeltaAddEncoder)		(const char* name, void (*conditionalencode)(delta_t* pFields, const unsigned char* from, const unsigned char* to));
	int			(*pfnGetCurrentPlayer)		();
	int			(*pfnCanSkipPlayer)			(const edict_t* player);
	int			(*pfnDeltaFindField)		(delta_t* pFields, const char* fieldname);
	void		(*pfnDeltaSetFieldByIndex)	(delta_t* pFields, int fieldNumber);
	void		(*pfnDeltaUnsetFieldByIndex)(delta_t* pFields, int fieldNumber);

	void		(*pfnSetGroupMask)			(int mask, int op);

	int			(*pfnCreateInstancedBaseline) (int classname, entity_state_t* baseline);
	void		(*pfnCvar_DirectSet)		(cvar_t* var, const char* value);

	// Forces the client and server to be running with the same version of the specified file
	//  ( e.g., a player model ).
	// Calling this has no effect in single player
	void		(*pfnForceUnmodified)		(FORCE_TYPE type, float* mins, float* maxs, const char* filename);

	void		(*pfnGetPlayerStats)		(const edict_t* pClient, int* ping, int* packet_loss);

	void		(*pfnAddServerCommand)		(const char* cmd_name, void (*function) ());

	// For voice communications, set which clients hear eachother.
	// NOTE: these functions take player entity indices (starting at 1).
	qboolean(*pfnVoice_GetClientListening)(int iReceiver, int iSender);
	qboolean(*pfnVoice_SetClientListening)(int iReceiver, int iSender, qboolean bListen);

	const char* (*pfnGetPlayerAuthId)		(edict_t* e);

	// PSV: Added for CZ training map
//	const char *(*pfnKeyNameForBinding)		( const char* pBinding );

	sequenceEntry* (*pfnSequenceGet)			(const char* fileName, const char* entryName);
	sentenceEntry* (*pfnSequencePickSentence)	(const char* groupName, int pickMethod, int* picked);

	// LH: Give access to filesize via filesystem
	int			(*pfnGetFileSize)			(const char* filename);

	unsigned int (*pfnGetApproxWavePlayLen) (const char* filepath);
	// MDC: Added for CZ career-mode
	int			(*pfnIsCareerMatch)			();

	// BGC: return the number of characters of the localized string referenced by using "label"
	int			(*pfnGetLocalizedStringLength)(const char* label);

	// BGC: added to facilitate persistent storage of tutor message decay values for
	// different career game profiles.  Also needs to persist regardless of mp.dll being
	// destroyed and recreated.
	void (*pfnRegisterTutorMessageShown)(int mid);
	int (*pfnGetTimesTutorMessageShown)(int mid);
	void (*ProcessTutorMessageDecayBuffer)(int* buffer, int bufferLength);
	void (*ConstructTutorMessageDecayBuffer)(int* buffer, int bufferLength);
	void (*ResetTutorMessageDecayData)();

	void (*pfnQueryClientCvarValue)(const edict_t* player, const char* cvarName);
	void (*pfnQueryClientCvarValue2)(const edict_t* player, const char* cvarName, int requestID);
	int (*pfnCheckParm)(const char* pchCmdLineToken, char** ppnext);
	edict_t* (*pfnPEntityOfEntIndexAllEntities)(int iEntIndex);
};

// Passed to pfnKeyValue
struct KeyValueData
{
	const char* szClassName;	// in: entity classname
	const char* szKeyName;		// in: name of key
	const char* szValue;		// in: value of key
	int32		fHandled;		// out: DLL sets to true if key-value pair was understood
};

struct LEVELLIST
{
	char		mapName[32];
	char		landmarkName[32];
	edict_t* pentLandmark;
	Vector		vecLandmarkOrigin;
};

constexpr int MAX_LEVEL_CONNECTIONS = 16;		// These are encoded in the lower 16bits of ENTITYTABLE->flags

struct ENTITYTABLE
{
	int			id;				// Ordinal ID of this entity (used for entity <--> pointer conversions)
	edict_t* pent;			// Pointer to the in-game entity

	int			location;		// Offset from the base data of this entity
	int			size;			// Byte size of this entity's data
	int			flags;			// This could be a short -- bit mask of transitions that this entity is in the PVS of
	string_t	classname;		// entity class name
};

constexpr int FENTTABLE_PLAYER = 0x80000000;
constexpr int FENTTABLE_REMOVED = 0x40000000;
constexpr int FENTTABLE_MOVEABLE = 0x20000000;
constexpr int FENTTABLE_GLOBAL = 0x10000000;

struct SAVERESTOREDATA
{
	char* pBaseData;		// Start of all entity save data
	char* pCurrentData;	// Current buffer pointer for sequential access
	int			size;			// Current data size
	int			bufferSize;		// Total space for data
	int			tokenSize;		// Size of the linear list of tokens
	int			tokenCount;		// Number of elements in the pTokens table
	char** pTokens;		// Hash table of entity strings (sparse)
	int			currentIndex;	// Holds a global entity table ID
	int			tableCount;		// Number of elements in the entity table
	int			connectionCount;// Number of elements in the levelList[]
	ENTITYTABLE* pTable;		// Array of ENTITYTABLE elements (1 for each entity)
	LEVELLIST	levelList[MAX_LEVEL_CONNECTIONS];		// List of connections from this level

	// smooth transition
	int			fUseLandmark;
	char		szLandmarkName[20];// landmark we'll spawn near in next level
	Vector		vecLandmarkOffset;// for landmark transitions
	float		time;
	char		szCurrentMapName[32];	// To check global entities

};

enum FIELDTYPE
{
	FIELD_FLOAT = 0,		// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_ENTITY,			// An entity offset (EOFFSET)
	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EVARS,			// EVARS *
	FIELD_EDICT,			// edict_t *, or edict_t *  (same thing)
	FIELD_VECTOR,			// Any vector
	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_POINTER,			// Arbitrary data pointer... to be removed, use an array of FIELD_CHARACTER
	FIELD_INTEGER,			// Any integer or enum
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)

	FIELD_TYPECOUNT,		// MUST BE LAST
};

//TODO: probably not needed anymore
#if !defined(offsetof)  && !defined(GNUC)
#define offsetof(s,m)	(size_t)&(((s *)0)->m)
#endif

#define _FIELD(type,name,fieldtype,count,flags)		{ fieldtype, #name, static_cast<int>(offsetof(type, name)), count, flags }
#define DEFINE_FIELD(type,name,fieldtype)			_FIELD(type, name, fieldtype, 1, 0)
#define DEFINE_ARRAY(type,name,fieldtype,count)		_FIELD(type, name, fieldtype, count, 0)
#define DEFINE_ENTITY_FIELD(name,fieldtype)			_FIELD(entvars_t, name, fieldtype, 1, 0 )
#define DEFINE_ENTITY_GLOBAL_FIELD(name,fieldtype)	_FIELD(entvars_t, name, fieldtype, 1, FTYPEDESC_GLOBAL )
#define DEFINE_GLOBAL_FIELD(type,name,fieldtype)	_FIELD(type, name, fieldtype, 1, FTYPEDESC_GLOBAL )


constexpr int FTYPEDESC_GLOBAL = 0x0001;		// This field is masked for global entity save/restore

struct TYPEDESCRIPTION
{
	FIELDTYPE		fieldType;
	const char* fieldName;
	int				fieldOffset;
	short			fieldSize;
	short			flags;
};
