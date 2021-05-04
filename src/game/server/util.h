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

#include <cstdint>

//
// Misc utility code
//
#include "activity.h"
#include "enginecallback.h"
#include "shared_utils.hpp"
#include "sound/materials.hpp"
#include "sound/sentences.hpp"
#include "sound/sound_playback.hpp"
#include "filesystem_shared.hpp"
#include "string_utils.hpp"

class CBaseEntity;
class CBasePlayer;
class CBasePlayerItem;

constexpr int TEAM_NAME_LENGTH = 16;

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
constexpr bool IsBitSet(const T& flBitVector, int bit)
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

inline int ENTINDEX(const edict_t* pEdict) { return (*g_engfuncs.pfnIndexOfEdict)(pEdict); }

// Testing the two types of "entity" for nullity
inline bool IsNullEnt(const edict_t* pent) { return pent == nullptr || ENTINDEX(pent) == 0; }

// Testing strings for nullity
constexpr string_t iStringNull = 0;
inline bool IsStringNull(string_t iString) { return iString == iStringNull; }

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
inline bool AreStringsEqual(const char* sz1, const char* sz2)
{
	return (strcmp(sz1, sz2) == 0);
}

// Misc. Prototypes
CBaseEntity* UTIL_EntityByIndex(int index);

//TODO: optimize this so it doesn't have to query every time
inline CBaseEntity* UTIL_GetWorld()
{
	return UTIL_EntityByIndex(0);
}

CBaseEntity* UTIL_FindEntityInSphere(CBaseEntity* pStartEntity, const Vector& vecCenter, float flRadius);
CBaseEntity* UTIL_FindEntityByString(CBaseEntity* pStartEntity, const char* szKeyword, const char* szValue);
CBaseEntity* UTIL_FindEntityByClassname(CBaseEntity* pStartEntity, const char* szName);
CBaseEntity* UTIL_FindEntityByTargetname(CBaseEntity* pStartEntity, const char* szName);

/**
*	@brief for doing a reverse lookup. Say you have a door, and want to find its button.
*/
CBaseEntity* UTIL_FindEntityByTarget(CBaseEntity* pStartEntity, const char* szName);

CBaseEntity* UTIL_FindEntityGeneric(const char* szName, const Vector& vecSrc, float flRadius);

CBaseEntity* UTIL_FindClientInPVS(CBaseEntity* pPVSEntity);

CBaseEntity* UTIL_CreateNamedEntity(string_t className);

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

void			UTIL_ParticleEffect(const Vector& vecOrigin, const Vector& vecDirection, std::uint32_t ulColor, std::uint32_t ulCount);

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
void			UTIL_ShowMessage(const char* pString, CBasePlayer* pPlayer);
void			UTIL_ShowMessageAll(const char* pString);
void			UTIL_ScreenFadeAll(const Vector& color, float fadeTime, float holdTime, int alpha, int flags);
void			UTIL_ScreenFade(CBasePlayer* pEntity, const Vector& color, float fadeTime, float fadeHold, int alpha, int flags);

enum class IgnoreMonsters
{
	No = 0,
	Yes = 1
};

enum class IgnoreGlass
{
	No = 0,
	Yes = 1
};

void			UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IgnoreMonsters igmon, CBaseEntity* pIgnore, TraceResult* ptr);
void			UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IgnoreMonsters igmon, IgnoreGlass ignoreGlass, CBaseEntity* pIgnore, TraceResult* ptr);

void			UTIL_TraceHull(const Vector& vecStart, const Vector& vecEnd, IgnoreMonsters igmon, Hull hullNumber, CBaseEntity* pIgnore, TraceResult* ptr);
TraceResult	UTIL_GetGlobalTrace();
void			UTIL_TraceModel(const Vector& vecStart, const Vector& vecEnd, Hull hullNumber, CBaseEntity* pModel, TraceResult* ptr);
void UTIL_TraceMonsterHull(CBaseEntity* pEntity, const Vector& vecStart, const Vector& vecEnd, IgnoreMonsters igmon, CBaseEntity* pIgnore, TraceResult* ptr);

Vector		UTIL_GetAimVector(CBaseEntity* entity, float flSpeed);
Contents UTIL_PointContents(const Vector& vec);

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
void UTIL_RemoveNow(CBaseEntity* pEntity);
bool			UTIL_IsValidEntity(CBaseEntity* pEntity);
bool			UTIL_TeamsMatch(const char* pTeamName1, const char* pTeamName2);

/**
*	@brief Search for water transition along a vertical line
*/
float		UTIL_WaterLevel(const Vector& position, float minz, float maxz);
void			UTIL_Bubbles(const Vector& mins, const Vector& maxs, int count);
void			UTIL_BubbleTrail(const Vector& from, const Vector& to, int count);

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
void ClientPrint(CBasePlayer* client, int msg_dest, const char* msg_name,
	const char* param1 = nullptr, const char* param2 = nullptr, const char* param3 = nullptr, const char* param4 = nullptr);

/**
*	@brief prints a message to the HUD say (chat)
*/
void			UTIL_SayText(const char* pText, CBasePlayer* pEntity);

/**
*	@copydoc UTIL_SayText(const char*, CBasePlayer*)
*/
void			UTIL_SayTextAll(const char* pText, CBasePlayer* pEntity);

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
void			UTIL_HudMessage(CBasePlayer* pEntity, const hudtextparms_t& textparms, const char* pMessage);

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
*	@details Sorta like IsInViewCone, but for nonmonsters.
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
void SetMovedir(CBaseEntity* pEntity);

/**
*	@brief calculates origin of a bmodel from absmin/size because all bmodel origins are 0 0 0
*/
Vector GetBrushModelOrigin(CBaseEntity* pBModel);

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
constexpr int LANGUAGE_GERMAN = 1;

extern DLL_GLOBAL int			g_Language;

constexpr int SND_SPAWNING = 1 << 8;		//!< duplicated in protocol.h we're spawing, used in some cases for ambients 
constexpr int SND_STOP = 1 << 5;			//!< duplicated in protocol.h stop sound
constexpr int SND_CHANGE_VOL = 1 << 6;		//!< duplicated in protocol.h change sound vol
constexpr int SND_CHANGE_PITCH = 1 << 7;	//!< duplicated in protocol.h change sound pitch

constexpr int SVC_TEMPENTITY = 23;
constexpr int SVC_INTERMISSION = 30;
constexpr int SVC_CDTRACK = 32;
constexpr int SVC_WEAPONANIM = 35;
constexpr int SVC_ROOMTYPE = 37;
constexpr int SVC_DIRECTOR = 51;

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
