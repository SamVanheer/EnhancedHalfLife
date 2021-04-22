/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#include "steam/steamtypes.h"

//
// Misc utility code
//
#include "activity.h"
#include "enginecallback.h"
#include "materials.hpp"
#include "filesystem_shared.hpp"
#include "string_utils.hpp"

class CBaseEntity;
class CBasePlayer;
class CBasePlayerItem;

constexpr int TEAM_NAME_LENGTH = 16;

inline void MESSAGE_BEGIN(int msg_dest, int msg_type, const float* pOrigin, entvars_t* ent);

extern globalvars_t* gpGlobals;

inline const char* STRING(string_t offset)
{
	return gpGlobals->pStringBase + static_cast<unsigned int>(offset);
}

/**
*	@brief Use this instead of ALLOC_STRING on constant strings
*/
inline string_t MAKE_STRING(const char* str)
{
	return reinterpret_cast<uint64>(str) - reinterpret_cast<uint64>(STRING(0));
}

string_t ALLOC_STRING(const char* str);

/**
*	@brief Version of ALLOC_STRING that parses and converts escape characters
*/
string_t ALLOC_ESCAPED_STRING(const char* str);

void ClearStringPool();

inline edict_t* FIND_ENTITY_BY_CLASSNAME(edict_t* entStart, const char* pszName)
{
	return FIND_ENTITY_BY_STRING(entStart, "classname", pszName);
}

inline edict_t* FIND_ENTITY_BY_TARGETNAME(edict_t* entStart, const char* pszName)
{
	return FIND_ENTITY_BY_STRING(entStart, "targetname", pszName);
}

/**
*	@brief for doing a reverse lookup. Say you have a door, and want to find its button.
*/
inline edict_t* FIND_ENTITY_BY_TARGET(edict_t* entStart, const char* pszName)
{
	return FIND_ENTITY_BY_STRING(entStart, "target", pszName);
}

// Keeps clutter down a bit, when using a float as a bit-vector
template<typename T>
constexpr T& SetBits(T& flBitVector, int bits)
{
	flBitVector = ((int)flBitVector) | bits;
	return flBitVector;
}

template<typename T>
constexpr T& ClearBits(T& flBitVector, int bits)
{
	flBitVector = ((int)flBitVector) & ~bits;
	return flBitVector;
}

template<typename T>
constexpr bool FBitSet(const T& flBitVector, int bit)
{
	return (((int)flBitVector) & bit) != 0;
}

// Makes these more explicit, and easier to find
#define DLL_GLOBAL

/**
*	@brief This is the glue that hooks .MAP entity class names to our CPP classes
*	The _declspec forces them to be exported by name so we can do a lookup with GetProcAddress()
*	The function is used to intialize / allocate the object for the entity
*/
#define LINK_ENTITY_TO_CLASS(mapClassName,DLLClassName) \
	extern "C" DLLEXPORT void mapClassName( entvars_t *pev ); \
	void mapClassName( entvars_t *pev ) { GetClassPtr( (DLLClassName *)pev ); }


//
// Conversion among the two types of "entity", including identity-conversions.
//
#ifdef DEBUG
edict_t* DBG_EntOfVars(const entvars_t* pev);
inline edict_t* ENT(const entvars_t* pev) { return DBG_EntOfVars(pev); }
#else
inline edict_t* ENT(const entvars_t* pev) { return pev->pContainingEntity; }
#endif
inline edict_t* ENT(edict_t* pent) { return pent; }
inline entvars_t* VARS(entvars_t* pev) { return pev; }

inline entvars_t* VARS(edict_t* pent)
{
	if (!pent)
		return nullptr;

	return &pent->v;
}

inline int ENTINDEX(const edict_t* pEdict) { return (*g_engfuncs.pfnIndexOfEdict)(pEdict); }
inline int ENTINDEX(entvars_t* pev) { return ENTINDEX(ENT(pev)); }
inline edict_t* INDEXENT(int iEdictNum) { return (*g_engfuncs.pfnPEntityOfEntIndex)(iEdictNum); }
inline void MESSAGE_BEGIN(int msg_dest, int msg_type, const float* pOrigin, entvars_t* ent) {
	(*g_engfuncs.pfnMessageBegin)(msg_dest, msg_type, pOrigin, ENT(ent));
}

inline entvars_t* INDEXVARS(int iEdictNum) { return VARS(INDEXENT(iEdictNum)); }

// Testing the two types of "entity" for nullity
inline bool FNullEnt(const edict_t* pent) { return pent == nullptr || ENTINDEX(pent) == 0; }
inline bool FNullEnt(entvars_t* pev) { return pev == nullptr || ENTINDEX(pev) == 0; }

// Testing strings for nullity
constexpr string_t iStringNull = 0;
inline bool FStringNull(string_t iString) { return iString == iStringNull; }

// Dot products for view cone checking
constexpr float VIEW_FIELD_FULL = -1.0;			//!< +-180 degrees
constexpr float VIEW_FIELD_WIDE = -0.7;			//!< +-135 degrees 0.1 // +-85 degrees, used for full FOV checks 
constexpr float VIEW_FIELD_NARROW = 0.7;		//!< +-45 degrees, more narrow check used to set up ranged attacks
constexpr float VIEW_FIELD_ULTRA_NARROW = 0.9;	//!< +-25 degrees, more narrow check used to set up ranged attacks

// All monsters need this data
constexpr byte DONT_BLEED = -1;
constexpr byte BLOOD_COLOR_RED = (byte)247;
constexpr byte BLOOD_COLOR_YELLOW = (byte)195;
constexpr byte BLOOD_COLOR_GREEN = BLOOD_COLOR_YELLOW;

enum class NPCState
{
	None = 0,
	Idle,
	Combat,
	Alert,
	Hunt,
	Prone,
	Script,
	PlayDead,
	Dead
};

constexpr std::size_t NPCStatesCount = static_cast<std::size_t>(NPCState::Dead) + 1;


/**
*	@brief Things that toggle (buttons/triggers/doors) need this
*/
enum class ToggleState
{
	AtTop,
	AtBottom,
	GoingUp,
	GoingDown
};

// Misc useful
inline bool FStrEq(const char* sz1, const char* sz2)
{
	return (strcmp(sz1, sz2) == 0);
}
inline bool FClassnameIs(edict_t* pent, const char* szClassname)
{
	return FStrEq(STRING(VARS(pent)->classname), szClassname);
}
inline bool FClassnameIs(entvars_t* pev, const char* szClassname)
{
	return FStrEq(STRING(pev->classname), szClassname);
}

// Misc. Prototypes
void			UTIL_SetSize(entvars_t* pev, const Vector& vecMin, const Vector& vecMax);

CBaseEntity* UTIL_FindEntityInSphere(CBaseEntity* pStartEntity, const Vector& vecCenter, float flRadius);
CBaseEntity* UTIL_FindEntityByString(CBaseEntity* pStartEntity, const char* szKeyword, const char* szValue);
CBaseEntity* UTIL_FindEntityByClassname(CBaseEntity* pStartEntity, const char* szName);
CBaseEntity* UTIL_FindEntityByTargetname(CBaseEntity* pStartEntity, const char* szName);
CBaseEntity* UTIL_FindEntityGeneric(const char* szName, Vector& vecSrc, float flRadius);

/**
*	@brief Returns a CBasePlayer pointer to a player by index
*
*	Only returns if the player is spawned and connected, otherwise returns nullptr
*	Index is 1 based
*/
CBasePlayer* UTIL_PlayerByIndex(int playerIndex);

/**
*	@brief Find a player with a case-insensitive name search.
*/
CBasePlayer* FindPlayerByName(const char* pTestName);

#define UTIL_EntitiesInPVS(pent)			(*g_engfuncs.pfnEntitiesInPVS)(pent)
void			UTIL_MakeVectors(const Vector& vecAngles);

/**
*	@brief Pass in an array of pointers and an array size, it fills the array and returns the number inserted
*/
int			UTIL_MonstersInSphere(CBaseEntity** pList, int listMax, const Vector& center, float radius);
int			UTIL_EntitiesInBox(CBaseEntity** pList, int listMax, const Vector& mins, const Vector& maxs, int flagMask);

/**
*	@brief like MakeVectors, but assumes pitch isn't inverted
*/
void			UTIL_MakeAimVectors(const Vector& vecAngles);
void			UTIL_MakeInvVectors(const Vector& vec, globalvars_t* pgv);

void			UTIL_SetOrigin(entvars_t* pev, const Vector& vecOrigin);
void			UTIL_EmitAmbientSound(edict_t* entity, const Vector& vecOrigin, const char* samp, float vol, float attenuation, int fFlags, int pitch);
void			UTIL_ParticleEffect(const Vector& vecOrigin, const Vector& vecDirection, uint32 ulColor, uint32 ulCount);

/**
*	@brief Shake the screen of all clients within radius
* 
*	@details radius == 0, shake all clients
*	UNDONE: Allow caller to shake clients not ONGROUND?
*	UNDONE: Fix falloff model (disabled)?
*	UNDONE: Affect user controls?
*/
void			UTIL_ScreenShake(const Vector& center, float amplitude, float frequency, float duration, float radius);
void			UTIL_ScreenShakeAll(const Vector& center, float amplitude, float frequency, float duration);
void			UTIL_ShowMessage(const char* pString, CBaseEntity* pPlayer);
void			UTIL_ShowMessageAll(const char* pString);
void			UTIL_ScreenFadeAll(const Vector& color, float fadeTime, float holdTime, int alpha, int flags);
void			UTIL_ScreenFade(CBaseEntity* pEntity, const Vector& color, float fadeTime, float fadeHold, int alpha, int flags);

enum IGNORE_MONSTERS { ignore_monsters = 1, dont_ignore_monsters = 0, missile = 2 };
enum IGNORE_GLASS { ignore_glass = 1, dont_ignore_glass = 0 };
void			UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IGNORE_MONSTERS igmon, edict_t* pentIgnore, TraceResult* ptr);
void			UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t* pentIgnore, TraceResult* ptr);
enum { point_hull = 0, human_hull = 1, large_hull = 2, head_hull = 3 };
void			UTIL_TraceHull(const Vector& vecStart, const Vector& vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t* pentIgnore, TraceResult* ptr);
TraceResult	UTIL_GetGlobalTrace();
void			UTIL_TraceModel(const Vector& vecStart, const Vector& vecEnd, int hullNumber, edict_t* pentModel, TraceResult* ptr);
Vector		UTIL_GetAimVector(edict_t* pent, float flSpeed);
int			UTIL_PointContents(const Vector& vec);

bool UTIL_IsMasterTriggered(string_t sMaster, CBaseEntity* pActivator);
void			UTIL_BloodStream(const Vector& origin, const Vector& direction, int color, int amount);
void			UTIL_BloodDrips(const Vector& origin, const Vector& direction, int color, int amount);
Vector		UTIL_RandomBloodVector();
bool			UTIL_ShouldShowBlood(int bloodColor);
void			UTIL_BloodDecalTrace(TraceResult* pTrace, int bloodColor);
void			UTIL_DecalTrace(TraceResult* pTrace, int decalNumber);

/**
*	@brief A player is trying to apply his custom decal for the spray can.
*	Tell connected clients to display it, or use the default spray can decal if the custom can't be loaded.
*/
void			UTIL_PlayerDecalTrace(TraceResult* pTrace, int playernum, int decalNumber, bool bIsCustom);
void			UTIL_GunshotDecalTrace(TraceResult* pTrace, int decalNumber);
void			UTIL_Sparks(const Vector& position);
void			UTIL_Ricochet(const Vector& position, float scale);

char* UTIL_VarArgs(const char* format, ...);
void			UTIL_Remove(CBaseEntity* pEntity);
bool			UTIL_IsValidEntity(edict_t* pent);
bool			UTIL_TeamsMatch(const char* pTeamName1, const char* pTeamName2);

/**
*	@brief Search for water transition along a vertical line
*/
float		UTIL_WaterLevel(const Vector& position, float minz, float maxz);
void			UTIL_Bubbles(Vector mins, Vector maxs, int count);
void			UTIL_BubbleTrail(Vector from, Vector to, int count);

/**
*	@brief allows precaching of other entities
*/
void			UTIL_PrecacheOther(const char* szClassname);

/**
*	@brief prints a message to each client
*/
void			UTIL_ClientPrintAll(int msg_dest, const char* msg_name,
	const char* param1 = nullptr, const char* param2 = nullptr, const char* param3 = nullptr, const char* param4 = nullptr);
inline void			UTIL_CenterPrintAll(const char* msg_name,
	const char* param1 = nullptr, const char* param2 = nullptr, const char* param3 = nullptr, const char* param4 = nullptr)
{
	UTIL_ClientPrintAll(HUD_PRINTCENTER, msg_name, param1, param2, param3, param4);
}

bool UTIL_GetNextBestWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pCurrentWeapon);

/**
*	@brief prints messages through the HUD
*/
void ClientPrint(entvars_t* client, int msg_dest, const char* msg_name,
	const char* param1 = nullptr, const char* param2 = nullptr, const char* param3 = nullptr, const char* param4 = nullptr);

/**
*	@brief prints a message to the HUD say (chat)
*/
void			UTIL_SayText(const char* pText, CBaseEntity* pEntity);

/**
*	@copydoc UTIL_SayText(const char*, CBaseEntity*)
*/
void			UTIL_SayTextAll(const char* pText, CBaseEntity* pEntity);


struct hudtextparms_t
{
	float		x;
	float		y;
	int			effect;
	byte		r1, g1, b1, a1;
	byte		r2, g2, b2, a2;
	float		fadeinTime;
	float		fadeoutTime;
	float		holdTime;
	float		fxTime;
	int			channel;
};

// prints as transparent 'title' to the HUD
/**
*	@brief prints as transparent 'title' to the HUD
*/
void			UTIL_HudMessageAll(const hudtextparms_t& textparms, const char* pMessage);
/**
*	@copydoc UTIL_HudMessageAll(const hudtextparms_t&, const char*)
*/
void			UTIL_HudMessage(CBaseEntity* pEntity, const hudtextparms_t& textparms, const char* pMessage);

char* UTIL_dtos1(int d);	//!< for handy use with ClientPrint params
char* UTIL_dtos2(int d);	//!< @copydoc UTIL_dtos1(int)
char* UTIL_dtos3(int d);	//!< @copydoc UTIL_dtos1(int)
char* UTIL_dtos4(int d);	//!< @copydoc UTIL_dtos1(int)

/**
*	@brief Writes message to console with timestamp and FragLog header.
*/
void			UTIL_LogPrintf(const char* fmt, ...);

/**
*	@brief returns the dot product of a line from src to check and vecdir.
*	@details Sorta like FInViewCone, but for nonmonsters.
*/
float UTIL_DotPoints(const Vector& vecSrc, const Vector& vecCheck, const Vector& vecDir);

/**
*	@brief for redundant keynames. Strips trailing #<number> added by Hammer from keys
*/
void UTIL_StripToken(const char* pKey, char* pDest);

// Misc functions
/**
*	@brief QuakeEd only writes a single float for angles (bad idea), so up and down are just constant angles.
*/
void SetMovedir(entvars_t* pev);

/**
*	@brief calculates origin of a bmodel from absmin/size because all bmodel origins are 0 0 0
*/
Vector VecBModelOrigin(entvars_t* pevBModel);

//
// How did I ever live without ASSERT?
//
#ifdef	DEBUG
void DBG_AssertFunction(bool fExpr, const char* szExpr, const char* szFile, int szLine, const char* szMessage);
#define ASSERT(f)		DBG_AssertFunction(f, #f, __FILE__, __LINE__, nullptr)
#define ASSERTSZ(f, sz)	DBG_AssertFunction(f, #f, __FILE__, __LINE__, sz)
#else	// !DEBUG
#define ASSERT(f)
#define ASSERTSZ(f, sz)
#endif	// !DEBUG

//
// Constants that were used only by QC (maybe not used at all now)
//
// Un-comment only as needed
//
constexpr int LANGUAGE_ENGLISH = 0;
constexpr int LANGUAGE_GERMAN = 1;
constexpr int LANGUAGE_FRENCH = 2;
constexpr int LANGUAGE_BRITISH = 3;

extern DLL_GLOBAL int			g_Language;

constexpr int AMBIENT_SOUND_STATIC = 0;	//!< medium radius attenuation
constexpr int AMBIENT_SOUND_EVERYWHERE = 1;
constexpr int AMBIENT_SOUND_SMALLRADIUS = 2;
constexpr int AMBIENT_SOUND_MEDIUMRADIUS = 4;
constexpr int AMBIENT_SOUND_LARGERADIUS = 8;
constexpr int AMBIENT_SOUND_START_SILENT = 16;
constexpr int AMBIENT_SOUND_NOT_LOOPING = 32;

constexpr int SPEAKER_START_SILENT = 1;	//!< wait for trigger 'on' to start announcements

constexpr int SND_SPAWNING = 1 << 8;		//!< duplicated in protocol.h we're spawing, used in some cases for ambients 
constexpr int SND_STOP = 1 << 5;			//!< duplicated in protocol.h stop sound
constexpr int SND_CHANGE_VOL = 1 << 6;		//!< duplicated in protocol.h change sound vol
constexpr int SND_CHANGE_PITCH = 1 << 7;	//!< duplicated in protocol.h change sound pitch

constexpr int LFO_SQUARE = 1;
constexpr int LFO_TRIANGLE = 2;
constexpr int LFO_RANDOM = 3;

// func_rotating
constexpr int SF_BRUSH_ROTATE_Y_AXIS = 0;
constexpr int SF_BRUSH_ROTATE_INSTANT = 1;
constexpr int SF_BRUSH_ROTATE_BACKWARDS = 2;
constexpr int SF_BRUSH_ROTATE_Z_AXIS = 4;
constexpr int SF_BRUSH_ROTATE_X_AXIS = 8;
constexpr int SF_PENDULUM_AUTO_RETURN = 16;
constexpr int SF_PENDULUM_PASSABLE = 32;


constexpr int SF_BRUSH_ROTATE_SMALLRADIUS = 128;
constexpr int SF_BRUSH_ROTATE_MEDIUMRADIUS = 256;
constexpr int SF_BRUSH_ROTATE_LARGERADIUS = 512;

constexpr int SVC_TEMPENTITY = 23;
constexpr int SVC_INTERMISSION = 30;
constexpr int SVC_CDTRACK = 32;
constexpr int SVC_WEAPONANIM = 35;
constexpr int SVC_ROOMTYPE = 37;
constexpr int SVC_DIRECTOR = 51;



// triggers
constexpr int SF_TRIGGER_ALLOWMONSTERS = 1;	//!< monsters allowed to fire this trigger
constexpr int SF_TRIGGER_NOCLIENTS = 2;		//!< players not allowed to fire this trigger
constexpr int SF_TRIGGER_PUSHABLES = 4;		//!< only pushables can fire this trigger

// func breakable
constexpr int SF_BREAK_TRIGGER_ONLY = 1;	//!< may only be broken by trigger
constexpr int SF_BREAK_TOUCH = 2;			//!< can be 'crashed through' by running player (plate glass)
constexpr int SF_BREAK_PRESSURE = 4;		//!< can be broken by a player standing on it
constexpr int SF_BREAK_CROWBAR = 256;		//!< instant break if hit with crowbar

// func_pushable (it's also func_breakable, so don't collide with those flags)
constexpr int SF_PUSH_BREAKABLE = 128;

constexpr int SF_LIGHT_START_OFF = 1;

constexpr int SPAWNFLAG_NOMESSAGE = 1;
constexpr int SPAWNFLAG_NOTOUCH = 1;

constexpr int SF_TRIG_PUSH_ONCE = 1;


// Sound Utilities

// sentence groups
constexpr int CBSENTENCENAME_MAX = 16;
/**
*	@brief max number of sentences in game. NOTE: this must match CVOXFILESENTENCEMAX in engine\sound.h!!!
*/
constexpr int CVOXFILESENTENCEMAX = 1536;

extern char gszallsentencenames[CVOXFILESENTENCEMAX][CBSENTENCENAME_MAX];
extern int gcallsentences;

/**
*	@brief pick a random sentence from rootname0 to rootnameX.
*	picks from the rgsentenceg[isentenceg] least recently used, modifies lru array. returns the sentencename.
*	@details note, lru must be seeded with 0-n randomized sentence numbers, with the rest of the lru filled with -1.
*	The first integer in the lru is actually the size of the list.
*	@return ipick, the ordinal of the picked sentence within the group.
*/
int USENTENCEG_Pick(int isentenceg, char* szfound, std::size_t foundSize);

/**
*	@brief ignore lru. pick next sentence from sentence group.
*	Go in order until we hit the last sentence, then repeat list if freset is true.
*	@param freset if false, then repeat last sentence.
*	@param ipick requested sentence ordinal.  
*	@return ipick 'next' is returned. -1 indicates an error.
*/
int USENTENCEG_PickSequential(int isentenceg, char* szfound, std::size_t foundSize, int ipick, int freset);

/**
*	@brief randomize list of sentence name indices
*/
void USENTENCEG_InitLRU(unsigned char* plru, int count);

/**
*	@brief open sentences.txt, scan for groups, build rgsentenceg
*	Should be called from world spawn, only works on the first call and is ignored subsequently.
*/
void SENTENCEG_Init();

/**
*	@brief for this entity, for the given sentence within the sentence group, stop the sentence.
*/
void SENTENCEG_Stop(CBaseEntity* entity, int isentenceg, int ipick);

/**
*	@brief given sentence group index, play random sentence for given entity.
*	@return ipick - which sentence was picked to play from the group.
*	@details Ipick is only needed if you plan on stopping the sound before playback is done (see SENTENCEG_Stop).
*/
int SENTENCEG_PlayRndI(CBaseEntity* entity, int isentenceg, float volume, float attenuation, int pitch, int flags = 0);

/**
*	@brief same as SENTENCEG_PlayRndI, but takes sentence group name instead of index
*/
int SENTENCEG_PlayRndSz(CBaseEntity* entity, const char* szrootname, float volume, float attenuation, int pitch, int flags = 0);

/**
*	@brief play sentences in sequential order from sentence group. Reset after last sentence.
*/
int SENTENCEG_PlaySequentialSz(CBaseEntity* entity, const char* szrootname, float volume, float attenuation, int pitch, int ipick, int freset, int flags = 0);

/**
*	@brief Given sentence group rootname (name without number suffix), get sentence group index (isentenceg).
*	@return -1 if no such name.
*/
int SENTENCEG_GetIndex(const char* szrootname);

/**
*	@brief convert sentence (sample) name to !sentencenum
*	@return !sentencenum
*/
int SENTENCEG_Lookup(const char* sample, char* sentencenum, std::size_t sentencenumSize);

/**
*	@brief play a strike sound based on the texture that was hit by the attack traceline.
*	VecSrc/VecEnd are the original traceline endpoints used by the attacker
*	@param iBulletType the type of bullet that hit the texture.
*	@return volume of strike instrument (crowbar) to play
*/
float TEXTURETYPE_PlaySound(TraceResult* ptr, Vector vecSrc, Vector vecEnd, int iBulletType);

/**
*	@brief use EMIT_SOUND_DYN to set the pitch of a sound.
*
*	Pitch of 100 is no pitch shift. Pitch > 100 up to 255 is a higher pitch, pitch < 100 down to 1 is a lower pitch.
*	150 to 70 is the realistic range.
*	EMIT_SOUND_DYN with pitch != 100 should be used sparingly,
*	as it's not quite as fast as EMIT_SOUND (the pitchshift mixer is not native coded).
*/
void EMIT_SOUND_DYN(edict_t* entity, int channel, const char* sample, float volume, float attenuation,
	int flags, int pitch);

/**
*	@brief play a specific sentence over the HEV suit speaker - just pass player entity, and !sentencename
*/
void EMIT_SOUND_SUIT(CBaseEntity* entity, const char* sample);

/**
*	@brief play a sentence, randomly selected from the passed in group id, over the HEV suit speaker
*/
void EMIT_GROUPID_SUIT(CBaseEntity* entity, int isentenceg);

/**
*	@brief play a sentence, randomly selected from the passed in groupname
*/
void EMIT_GROUPNAME_SUIT(CBaseEntity* entity, const char* groupname);

template<std::size_t Size>
void PRECACHE_SOUND_ARRAY(const char* (&a)[Size])
{
	for (std::size_t i = 0; i < ArraySize(a); i++)
	{
		PRECACHE_SOUND(a[i]);
	}
}

template<std::size_t Size>
const char* RANDOM_SOUND_ARRAY(const char* (&array)[Size])
{
	return array[RANDOM_LONG(0, ArraySize(array) - 1)];
}

inline void PLAYBACK_EVENT(int flags, edict_t* invoker, unsigned short index)
{
	PLAYBACK_EVENT_FULL(flags, invoker, index, 0, vec3_origin, vec3_origin, 0.0, 0.0, 0, 0, 0, 0);
}

inline void PLAYBACK_EVENT_DELAY(int flags, edict_t* invoker, unsigned short index, float delay)
{
	PLAYBACK_EVENT_FULL(flags, invoker, index, delay, vec3_origin, vec3_origin, 0.0, 0.0, 0, 0, 0, 0);
}

constexpr int GROUP_OP_AND = 0;
constexpr int GROUP_OP_NAND = 1;

extern int g_groupmask;
extern int g_groupop;

/**
*	@brief Smart version, it'll clean itself up when it pops off stack
*/
class UTIL_GroupTrace
{
public:
	UTIL_GroupTrace(int groupmask, int op);
	~UTIL_GroupTrace();

private:
	int m_oldgroupmask, m_oldgroupop;
};

void UTIL_SetGroupTrace(int groupmask, int op);
void UTIL_UnsetGroupTrace();

int UTIL_SharedRandomLong(unsigned int seed, int low, int high);
float UTIL_SharedRandomFloat(unsigned int seed, float low, float high);

/**
*	@brief Always 0.0 on client, even if not predicting weapons ( won't get called in that case )
*/
float UTIL_WeaponTimeBase();

CBaseEntity* UTIL_FindEntityForward(CBaseEntity* pMe);

void EntvarsKeyvalue(entvars_t* pev, KeyValueData* pkvd);

/**
*	@brief Determine the current # of active players on the server for map cycling logic
*/
int UTIL_CountPlayers();

constexpr bool UTIL_IsInWorld(const Vector& point)
{
	return point.x > -WORLD_BOUNDARY && point.y > -WORLD_BOUNDARY && point.z > -WORLD_BOUNDARY
		&& point.x < WORLD_BOUNDARY&& point.y < WORLD_BOUNDARY&& point.z < WORLD_BOUNDARY;
}

/**
*	@brief Initialize absmin & absmax to the appropriate box
*/
void SetObjectCollisionBox(entvars_t* pev);
