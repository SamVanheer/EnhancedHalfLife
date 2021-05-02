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
/*

  Utility code.  Really not optional after all.

*/

#include <algorithm>
#include <string>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "shake.h"
#include "decals.hpp"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "UserMessages.h"

#include "CStringPool.hpp"

CStringPool g_StringPool;

string_t ALLOC_STRING(const char* str)
{
	return MAKE_STRING(g_StringPool.Allocate(str));
}

string_t ALLOC_ESCAPED_STRING(const char* str)
{
	if (!str)
	{
		ALERT(at_warning, "NULL string passed to ALLOC_ESCAPED_STRING\n");
		return MAKE_STRING("");
	}

	std::string converted{str};

	for (std::size_t index = 0; index < converted.length();)
	{
		if (converted[index] == '\\')
		{
			if (index + 1 >= converted.length())
			{
				ALERT(at_warning, "Incomplete escape character encountered in ALLOC_ESCAPED_STRING\n\tOriginal string: \"%s\"\n", str);
				break;
			}

			const char next = converted[index + 1];

			converted.erase(index, 1);

			//TODO: support all escape characters
			if (next == 'n')
			{
				converted[index] = '\n';
			}
		}

		++index;
	}

	return ALLOC_STRING(converted.c_str());
}

void ClearStringPool()
{
	g_StringPool.Clear();
}

float UTIL_WeaponTimeBase()
{
#if defined( CLIENT_WEAPONS )
	return 0.0;
#else
	return gpGlobals->time;
#endif
}

CBaseEntity* UTIL_FindEntityForward(CBaseEntity* pMe)
{
	TraceResult tr;

	UTIL_MakeVectors(pMe->pev->v_angle);
	UTIL_TraceLine(pMe->pev->origin + pMe->pev->view_ofs, pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * WORLD_SIZE, IgnoreMonsters::No, pMe->edict(), &tr);
	if (tr.flFraction != 1.0 && !IsNullEnt(tr.pHit))
	{
		CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
		return pHit;
	}
	return nullptr;
}

void UTIL_ParametricRocket(entvars_t* pev, Vector vecOrigin, Vector vecAngles, edict_t* owner)
{
	pev->startpos = vecOrigin;
	// Trace out line to end pos
	TraceResult tr;
	UTIL_MakeVectors(vecAngles);
	UTIL_TraceLine(pev->startpos, pev->startpos + gpGlobals->v_forward * WORLD_SIZE, IgnoreMonsters::Yes, owner, &tr);
	pev->endpos = tr.vecEndPos;

	// Now compute how long it will take based on current velocity
	Vector vecTravel = pev->endpos - pev->startpos;
	float travelTime = 0.0;
	if (pev->velocity.Length() > 0)
	{
		travelTime = vecTravel.Length() / pev->velocity.Length();
	}
	pev->starttime = gpGlobals->time;
	pev->impacttime = gpGlobals->time + travelTime;
}

int g_groupmask = 0;
int g_groupop = 0;

// Normal overrides
void UTIL_SetGroupTrace(int groupmask, int op)
{
	g_groupmask = groupmask;
	g_groupop = op;

	ENGINE_SETGROUPMASK(g_groupmask, g_groupop);
}

void UTIL_UnsetGroupTrace()
{
	g_groupmask = 0;
	g_groupop = 0;

	ENGINE_SETGROUPMASK(0, 0);
}

UTIL_GroupTrace::UTIL_GroupTrace(int groupmask, int op)
{
	m_oldgroupmask = g_groupmask;
	m_oldgroupop = g_groupop;

	g_groupmask = groupmask;
	g_groupop = op;

	ENGINE_SETGROUPMASK(g_groupmask, g_groupop);
}

UTIL_GroupTrace::~UTIL_GroupTrace()
{
	g_groupmask = m_oldgroupmask;
	g_groupop = m_oldgroupop;

	ENGINE_SETGROUPMASK(g_groupmask, g_groupop);
}

#ifdef	DEBUG
edict_t* DBG_EntOfVars(const entvars_t* pev)
{
	if (pev->pContainingEntity != nullptr)
		return pev->pContainingEntity;
	ALERT(at_console, "entvars_t pContainingEntity is NULL, calling into engine");
	edict_t* pent = (*g_engfuncs.pfnFindEntityByVars)((entvars_t*)pev);
	if (pent == nullptr)
		ALERT(at_console, "DAMN!  Even the engine couldn't FindEntityByVars!");
	((entvars_t*)pev)->pContainingEntity = pent;
	return pent;
}
#endif //DEBUG


#ifdef	DEBUG
void
DBG_AssertFunction(
	bool		fExpr,
	const char* szExpr,
	const char* szFile,
	int			szLine,
	const char* szMessage)
{
	if (fExpr)
		return;
	char szOut[512];
	if (szMessage != nullptr)
		snprintf(szOut, sizeof(szOut), "ASSERT FAILED:\n %s \n(%s@%d)\n%s", szExpr, szFile, szLine, szMessage);
	else
		snprintf(szOut, sizeof(szOut), "ASSERT FAILED:\n %s \n(%s@%d)", szExpr, szFile, szLine);
	ALERT(at_console, szOut);
}
#endif	// DEBUG

bool UTIL_GetNextBestWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pCurrentWeapon)
{
	return g_pGameRules->GetNextBestWeapon(pPlayer, pCurrentWeapon);
}

int UTIL_EntitiesInBox(CBaseEntity** pList, int listMax, const Vector& mins, const Vector& maxs, int flagMask)
{
	edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
	CBaseEntity* pEntity;
	int			count;

	count = 0;

	if (!pEdict)
		return count;

	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)	// Not in use
			continue;

		if (flagMask && !(pEdict->v.flags & flagMask))	// Does it meet the criteria?
			continue;

		if (mins.x > pEdict->v.absmax.x ||
			mins.y > pEdict->v.absmax.y ||
			mins.z > pEdict->v.absmax.z ||
			maxs.x < pEdict->v.absmin.x ||
			maxs.y < pEdict->v.absmin.y ||
			maxs.z < pEdict->v.absmin.z)
			continue;

		pEntity = CBaseEntity::Instance(pEdict);
		if (!pEntity)
			continue;

		pList[count] = pEntity;
		count++;

		if (count >= listMax)
			return count;
	}

	return count;
}


int UTIL_MonstersInSphere(CBaseEntity** pList, int listMax, const Vector& center, float radius)
{
	edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
	CBaseEntity* pEntity;
	int			count;
	float		distance, delta;

	count = 0;
	float radiusSquared = radius * radius;

	if (!pEdict)
		return count;

	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)	// Not in use
			continue;

		if (!(pEdict->v.flags & (FL_CLIENT | FL_MONSTER)))	// Not a client/monster ?
			continue;

		// Use origin for X & Y since they are centered for all monsters
		// Now X
		delta = center.x - pEdict->v.origin.x;//(pEdict->v.absmin.x + pEdict->v.absmax.x)*0.5;
		delta *= delta;

		if (delta > radiusSquared)
			continue;
		distance = delta;

		// Now Y
		delta = center.y - pEdict->v.origin.y;//(pEdict->v.absmin.y + pEdict->v.absmax.y)*0.5;
		delta *= delta;

		distance += delta;
		if (distance > radiusSquared)
			continue;

		// Now Z
		delta = center.z - (pEdict->v.absmin.z + pEdict->v.absmax.z) * 0.5;
		delta *= delta;

		distance += delta;
		if (distance > radiusSquared)
			continue;

		pEntity = CBaseEntity::Instance(pEdict);
		if (!pEntity)
			continue;

		pList[count] = pEntity;
		count++;

		if (count >= listMax)
			return count;
	}


	return count;
}

CBaseEntity* UTIL_EntityByIndex(int index)
{
	if (auto edict = INDEXENT(index); edict)
	{
		return reinterpret_cast<CBaseEntity*>(edict->pvPrivateData);
	}

	return nullptr;
}

CBaseEntity* UTIL_FindEntityInSphere(CBaseEntity* pStartEntity, const Vector& vecCenter, float flRadius)
{
	edict_t* pentEntity;

	if (pStartEntity)
		pentEntity = pStartEntity->edict();
	else
		pentEntity = nullptr;

	pentEntity = FIND_ENTITY_IN_SPHERE(pentEntity, vecCenter, flRadius);

	if (!IsNullEnt(pentEntity))
		return CBaseEntity::Instance(pentEntity);
	return nullptr;
}


CBaseEntity* UTIL_FindEntityByString(CBaseEntity* pStartEntity, const char* szKeyword, const char* szValue)
{
	edict_t* pentEntity;

	if (pStartEntity)
		pentEntity = pStartEntity->edict();
	else
		pentEntity = nullptr;

	pentEntity = FIND_ENTITY_BY_STRING(pentEntity, szKeyword, szValue);

	if (!IsNullEnt(pentEntity))
		return CBaseEntity::Instance(pentEntity);
	return nullptr;
}

CBaseEntity* UTIL_FindEntityByClassname(CBaseEntity* pStartEntity, const char* szName)
{
	return UTIL_FindEntityByString(pStartEntity, "classname", szName);
}

CBaseEntity* UTIL_FindEntityByTargetname(CBaseEntity* pStartEntity, const char* szName)
{
	return UTIL_FindEntityByString(pStartEntity, "targetname", szName);
}

CBaseEntity* UTIL_FindEntityByTarget(CBaseEntity* pStartEntity, const char* szName)
{
	return UTIL_FindEntityByString(pStartEntity, "target", szName);
}

CBaseEntity* UTIL_FindEntityGeneric(const char* szWhatever, Vector& vecSrc, float flRadius)
{
	CBaseEntity* pEntity = nullptr;

	pEntity = UTIL_FindEntityByTargetname(nullptr, szWhatever);
	if (pEntity)
		return pEntity;

	CBaseEntity* pSearch = nullptr;
	float flMaxDist2 = flRadius * flRadius;
	while ((pSearch = UTIL_FindEntityByClassname(pSearch, szWhatever)) != nullptr)
	{
		float flDist2 = (pSearch->pev->origin - vecSrc).Length();
		flDist2 = flDist2 * flDist2;
		if (flMaxDist2 > flDist2)
		{
			pEntity = pSearch;
			flMaxDist2 = flDist2;
		}
	}
	return pEntity;
}

CBaseEntity* UTIL_CreateNamedEntity(string_t className)
{
	auto edict = CREATE_NAMED_ENTITY(className);

	if (IsNullEnt(edict))
	{
		return nullptr;
	}

	auto entity = CBaseEntity::InstanceOrNull(edict);

	if (entity)
	{
		return entity;
	}

	//Cover this edge case with a seperate error message
	ALERT(at_error, "Couldn't create entity \"%s\" by name!\n", STRING(className));

	REMOVE_ENTITY(edict);

	return nullptr;
}

CBasePlayer* UTIL_PlayerByIndex(int playerIndex)
{
	if (playerIndex > 0 && playerIndex <= gpGlobals->maxClients)
	{
		edict_t* pPlayerEdict = INDEXENT(playerIndex);
		if (pPlayerEdict && !pPlayerEdict->free)
		{
			return static_cast<CBasePlayer*>(CBaseEntity::Instance(pPlayerEdict));
		}
	}

	return nullptr;
}

CBasePlayer* FindPlayerByName(const char* pTestName)
{
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		edict_t* pEdict = g_engfuncs.pfnPEntityOfEntIndex(i);
		if (pEdict)
		{
			CBaseEntity* pEnt = CBaseEntity::Instance(pEdict);
			if (pEnt && pEnt->IsPlayer())
			{
				const char* pNetName = STRING(pEnt->pev->netname);
				if (stricmp(pNetName, pTestName) == 0)
				{
					return (CBasePlayer*)pEnt;
				}
			}
		}
	}

	return nullptr;
}

void UTIL_MakeVectors(const Vector& vecAngles)
{
	AngleVectors(vecAngles, gpGlobals->v_forward, gpGlobals->v_right, gpGlobals->v_up);
}


void UTIL_MakeAimVectors(const Vector& vecAngles)
{
	Vector angles = vecAngles;
	angles[0] = -angles[0];
	AngleVectors(angles, gpGlobals->v_forward, gpGlobals->v_right, gpGlobals->v_up);
}

void UTIL_MakeInvVectors(const Vector& vec, globalvars_t* pgv)
{
	AngleVectors(vec, gpGlobals->v_forward, gpGlobals->v_right, gpGlobals->v_up);

	pgv->v_right = pgv->v_right * -1;

	std::swap(pgv->v_forward.y, pgv->v_right.x);
	std::swap(pgv->v_forward.z, pgv->v_up.x);
	std::swap(pgv->v_right.z, pgv->v_up.y);
}


void UTIL_EmitAmbientSound(edict_t* entity, const Vector& vecOrigin, const char* samp, float vol, float attenuation, int fFlags, int pitch)
{
	if (samp && *samp == '!')
	{
		char name[32];
		if (SENTENCEG_Lookup(samp, name, sizeof(name)) >= 0)
			EMIT_AMBIENT_SOUND(entity, vecOrigin, name, vol, attenuation, fFlags, pitch);
	}
	else
		EMIT_AMBIENT_SOUND(entity, vecOrigin, samp, vol, attenuation, fFlags, pitch);
}

static unsigned short FixedUnsigned16(float value, float scale)
{
	int output;

	output = value * scale;
	if (output < 0)
		output = 0;
	if (output > 0xFFFF)
		output = 0xFFFF;

	return (unsigned short)output;
}

static short FixedSigned16(float value, float scale)
{
	int output;

	output = value * scale;

	if (output > 32767)
		output = 32767;

	if (output < -32768)
		output = -32768;

	return (short)output;
}

void UTIL_ScreenShake(const Vector& center, float amplitude, float frequency, float duration, float radius)
{
	int			i;
	float		localAmplitude;
	ScreenShake	shake;

	shake.duration = FixedUnsigned16(duration, 1 << 12);		// 4.12 fixed
	shake.frequency = FixedUnsigned16(frequency, 1 << 8);	// 8.8 fixed

	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

		if (!pPlayer || !(pPlayer->pev->flags & FL_ONGROUND))	// Don't shake if not onground
			continue;

		localAmplitude = 0;

		if (radius <= 0)
			localAmplitude = amplitude;
		else
		{
			Vector delta = center - pPlayer->pev->origin;
			float distance = delta.Length();

			// Had to get rid of this falloff - it didn't work well
			if (distance < radius)
				localAmplitude = amplitude;//radius - distance;
		}
		if (localAmplitude)
		{
			shake.amplitude = FixedUnsigned16(localAmplitude, 1 << 12);		// 4.12 fixed

			MESSAGE_BEGIN(MessageDest::One, gmsgShake, nullptr, pPlayer->edict());		// use the magic #1 for "one client"

			WRITE_SHORT(shake.amplitude);				// shake amount
			WRITE_SHORT(shake.duration);				// shake lasts this long
			WRITE_SHORT(shake.frequency);				// shake noise frequency

			MESSAGE_END();
		}
	}
}



void UTIL_ScreenShakeAll(const Vector& center, float amplitude, float frequency, float duration)
{
	UTIL_ScreenShake(center, amplitude, frequency, duration, 0);
}


void UTIL_ScreenFadeBuild(ScreenFade& fade, const Vector& color, float fadeTime, float fadeHold, int alpha, int flags)
{
	fade.duration = FixedUnsigned16(fadeTime, 1 << 12);		// 4.12 fixed
	fade.holdTime = FixedUnsigned16(fadeHold, 1 << 12);		// 4.12 fixed
	fade.r = (int)color.x;
	fade.g = (int)color.y;
	fade.b = (int)color.z;
	fade.a = alpha;
	fade.fadeFlags = flags;
}


void UTIL_ScreenFadeWrite(const ScreenFade& fade, CBaseEntity* pEntity)
{
	if (!pEntity || !pEntity->IsNetClient())
		return;

	MESSAGE_BEGIN(MessageDest::One, gmsgFade, nullptr, pEntity->edict());		// use the magic #1 for "one client"

	WRITE_SHORT(fade.duration);		// fade lasts this long
	WRITE_SHORT(fade.holdTime);		// fade lasts this long
	WRITE_SHORT(fade.fadeFlags);		// fade type (in / out)
	WRITE_BYTE(fade.r);				// fade red
	WRITE_BYTE(fade.g);				// fade green
	WRITE_BYTE(fade.b);				// fade blue
	WRITE_BYTE(fade.a);				// fade blue

	MESSAGE_END();
}


void UTIL_ScreenFadeAll(const Vector& color, float fadeTime, float fadeHold, int alpha, int flags)
{
	int			i;
	ScreenFade	fade;


	UTIL_ScreenFadeBuild(fade, color, fadeTime, fadeHold, alpha, flags);

	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

		UTIL_ScreenFadeWrite(fade, pPlayer);
	}
}


void UTIL_ScreenFade(CBaseEntity* pEntity, const Vector& color, float fadeTime, float fadeHold, int alpha, int flags)
{
	ScreenFade	fade;

	UTIL_ScreenFadeBuild(fade, color, fadeTime, fadeHold, alpha, flags);
	UTIL_ScreenFadeWrite(fade, pEntity);
}


void UTIL_HudMessage(CBaseEntity* pEntity, const hudtextparms_t& textparms, const char* pMessage)
{
	if (!pEntity || !pEntity->IsNetClient())
		return;

	MESSAGE_BEGIN(MessageDest::One, SVC_TEMPENTITY, nullptr, pEntity->edict());
	WRITE_BYTE(TE_TEXTMESSAGE);
	WRITE_BYTE(textparms.channel & 0xFF);

	WRITE_SHORT(FixedSigned16(textparms.x, 1 << 13));
	WRITE_SHORT(FixedSigned16(textparms.y, 1 << 13));
	WRITE_BYTE(textparms.effect);

	WRITE_BYTE(textparms.r1);
	WRITE_BYTE(textparms.g1);
	WRITE_BYTE(textparms.b1);
	WRITE_BYTE(textparms.a1);

	WRITE_BYTE(textparms.r2);
	WRITE_BYTE(textparms.g2);
	WRITE_BYTE(textparms.b2);
	WRITE_BYTE(textparms.a2);

	WRITE_SHORT(FixedUnsigned16(textparms.fadeinTime, 1 << 8));
	WRITE_SHORT(FixedUnsigned16(textparms.fadeoutTime, 1 << 8));
	WRITE_SHORT(FixedUnsigned16(textparms.holdTime, 1 << 8));

	if (textparms.effect == 2)
		WRITE_SHORT(FixedUnsigned16(textparms.fxTime, 1 << 8));

	if (strlen(pMessage) < 512)
	{
		WRITE_STRING(pMessage);
	}
	else
	{
		char tmp[512];
		safe_strcpy(tmp, pMessage);
		WRITE_STRING(tmp);
	}
	MESSAGE_END();
}

void UTIL_HudMessageAll(const hudtextparms_t& textparms, const char* pMessage)
{
	int			i;

	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer)
			UTIL_HudMessage(pPlayer, textparms, pMessage);
	}
}

void UTIL_ClientPrintAll(int msg_dest, const char* msg_name, const char* param1, const char* param2, const char* param3, const char* param4)
{
	MESSAGE_BEGIN(MessageDest::All, gmsgTextMsg);
	WRITE_BYTE(msg_dest);
	WRITE_STRING(msg_name);

	if (param1)
		WRITE_STRING(param1);
	if (param2)
		WRITE_STRING(param2);
	if (param3)
		WRITE_STRING(param3);
	if (param4)
		WRITE_STRING(param4);

	MESSAGE_END();
}

void ClientPrint(entvars_t* client, int msg_dest, const char* msg_name, const char* param1, const char* param2, const char* param3, const char* param4)
{
	MESSAGE_BEGIN(MessageDest::One, gmsgTextMsg, nullptr, client);
	WRITE_BYTE(msg_dest);
	WRITE_STRING(msg_name);

	if (param1)
		WRITE_STRING(param1);
	if (param2)
		WRITE_STRING(param2);
	if (param3)
		WRITE_STRING(param3);
	if (param4)
		WRITE_STRING(param4);

	MESSAGE_END();
}

void UTIL_SayText(const char* pText, CBaseEntity* pEntity)
{
	if (!pEntity->IsNetClient())
		return;

	MESSAGE_BEGIN(MessageDest::One, gmsgSayText, nullptr, pEntity->edict());
	WRITE_BYTE(pEntity->entindex());
	WRITE_STRING(pText);
	MESSAGE_END();
}

void UTIL_SayTextAll(const char* pText, CBaseEntity* pEntity)
{
	MESSAGE_BEGIN(MessageDest::All, gmsgSayText, nullptr);
	WRITE_BYTE(pEntity->entindex());
	WRITE_STRING(pText);
	MESSAGE_END();
}


char* UTIL_dtos1(int d)
{
	static char buf[8];
	snprintf(buf, sizeof(buf), "%d", d);
	return buf;
}

char* UTIL_dtos2(int d)
{
	static char buf[8];
	snprintf(buf, sizeof(buf), "%d", d);
	return buf;
}

char* UTIL_dtos3(int d)
{
	static char buf[8];
	snprintf(buf, sizeof(buf), "%d", d);
	return buf;
}

char* UTIL_dtos4(int d)
{
	static char buf[8];
	snprintf(buf, sizeof(buf), "%d", d);
	return buf;
}

void UTIL_ShowMessage(const char* pString, CBaseEntity* pEntity)
{
	if (!pEntity || !pEntity->IsNetClient())
		return;

	MESSAGE_BEGIN(MessageDest::One, gmsgHudText, nullptr, pEntity->edict());
	WRITE_STRING(pString);
	MESSAGE_END();
}


void UTIL_ShowMessageAll(const char* pString)
{
	int		i;

	// loop through all players

	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer)
			UTIL_ShowMessage(pString, pPlayer);
	}
}

// Overloaded to add IGNORE_GLASS
void UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IgnoreMonsters igmon, IgnoreGlass ignoreGlass, edict_t* pentIgnore, TraceResult* ptr)
{
	TRACE_LINE(vecStart, vecEnd,
		(igmon == IgnoreMonsters::Yes ? TRACE_IGNORE_MONSTERS : TRACE_IGNORE_NOTHING)
		| (ignoreGlass == IgnoreGlass::Yes ? TRACE_IGNORE_GLASS : TRACE_IGNORE_NOTHING),
		pentIgnore, ptr);
}


void UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IgnoreMonsters igmon, edict_t* pentIgnore, TraceResult* ptr)
{
	TRACE_LINE(vecStart, vecEnd,
		(igmon == IgnoreMonsters::Yes ? TRACE_IGNORE_MONSTERS : TRACE_IGNORE_NOTHING),
		pentIgnore, ptr);
}


void UTIL_TraceHull(const Vector& vecStart, const Vector& vecEnd, IgnoreMonsters igmon, Hull hullNumber, edict_t* pentIgnore, TraceResult* ptr)
{
	TRACE_HULL(vecStart, vecEnd,
		(igmon == IgnoreMonsters::Yes ? TRACE_IGNORE_MONSTERS : TRACE_IGNORE_NOTHING),
		static_cast<int>(hullNumber), pentIgnore, ptr);
}

void UTIL_TraceModel(const Vector& vecStart, const Vector& vecEnd, Hull hullNumber, edict_t* pentModel, TraceResult* ptr)
{
	g_engfuncs.pfnTraceModel(vecStart, vecEnd, static_cast<int>(hullNumber), pentModel, ptr);
}

void UTIL_TraceMonsterHull(edict_t* pEdict, const Vector& vecStart, const Vector& vecEnd, IgnoreMonsters igmon, edict_t* pentIgnore, TraceResult* ptr)
{
	TRACE_MONSTER_HULL(pEdict, vecStart, vecEnd, igmon == IgnoreMonsters::Yes ? TRACE_IGNORE_MONSTERS : TRACE_IGNORE_NOTHING, pentIgnore, ptr);
}

TraceResult UTIL_GetGlobalTrace()
{
	TraceResult tr;

	tr.fAllSolid = gpGlobals->trace_allsolid;
	tr.fStartSolid = gpGlobals->trace_startsolid;
	tr.fInOpen = gpGlobals->trace_inopen;
	tr.fInWater = gpGlobals->trace_inwater;
	tr.flFraction = gpGlobals->trace_fraction;
	tr.flPlaneDist = gpGlobals->trace_plane_dist;
	tr.pHit = gpGlobals->trace_ent;
	tr.vecEndPos = gpGlobals->trace_endpos;
	tr.vecPlaneNormal = gpGlobals->trace_plane_normal;
	tr.iHitgroup = gpGlobals->trace_hitgroup;
	return tr;
}

void UTIL_ParticleEffect(const Vector& vecOrigin, const Vector& vecDirection, uint32 ulColor, uint32 ulCount)
{
	PARTICLE_EFFECT(vecOrigin, vecDirection, (float)ulColor, (float)ulCount);
}

char* UTIL_VarArgs(const char* format, ...)
{
	va_list		argptr;
	static char		string[1024];

	va_start(argptr, format);
	vsnprintf(string, sizeof(string), format, argptr);
	va_end(argptr);

	return string;
}

Vector UTIL_GetAimVector(edict_t* pent, float flSpeed)
{
	Vector tmp;
	GET_AIM_VECTOR(pent, flSpeed, tmp);
	return tmp;
}

bool UTIL_IsMasterTriggered(string_t sMaster, CBaseEntity* pActivator)
{
	if (!IsStringNull(sMaster))
	{
		if (CBaseEntity* pTarget = UTIL_FindEntityByTargetname(nullptr, STRING(sMaster)); !IsNullEnt(pTarget))
		{
			if (pTarget->ObjectCaps() & FCAP_MASTER)
				return pTarget->IsTriggered(pActivator);
		}

		ALERT(at_console, "Master was null or not a master!\n");
	}

	// if this isn't a master entity, just say yes.
	return true;
}

bool UTIL_ShouldShowBlood(int color)
{
	if (color != DONT_BLEED)
	{
		if (color == BLOOD_COLOR_RED)
		{
			if (CVAR_GET_FLOAT("violence_hblood") != 0)
				return true;
		}
		else
		{
			if (CVAR_GET_FLOAT("violence_ablood") != 0)
				return true;
		}
	}
	return false;
}

Contents UTIL_PointContents(const Vector& vec)
{
	return POINT_CONTENTS(vec);
}

void UTIL_BloodStream(const Vector& origin, const Vector& direction, int color, int amount)
{
	if (!UTIL_ShouldShowBlood(color))
		return;

	if (g_Language == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED)
		color = 0;


	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, origin);
	WRITE_BYTE(TE_BLOODSTREAM);
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_COORD(direction.x);
	WRITE_COORD(direction.y);
	WRITE_COORD(direction.z);
	WRITE_BYTE(color);
	WRITE_BYTE(std::min(amount, 255));
	MESSAGE_END();
}

void UTIL_BloodDrips(const Vector& origin, const Vector& direction, int color, int amount)
{
	if (!UTIL_ShouldShowBlood(color))
		return;

	if (color == DONT_BLEED || amount == 0)
		return;

	if (g_Language == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED)
		color = 0;

	if (g_pGameRules->IsMultiplayer())
	{
		// scale up blood effect in multiplayer for better visibility
		amount *= 2;
	}

	if (amount > 255)
		amount = 255;

	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, origin);
	WRITE_BYTE(TE_BLOODSPRITE);
	WRITE_COORD(origin.x);								// pos
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_SHORT(g_sModelIndexBloodSpray);				// initial sprite model
	WRITE_SHORT(g_sModelIndexBloodDrop);				// droplet sprite models
	WRITE_BYTE(color);								// color index into host_basepal
	WRITE_BYTE(std::min(std::max(3, amount / 10), 16));		// size
	MESSAGE_END();
}

Vector UTIL_RandomBloodVector()
{
	Vector direction;

	direction.x = RANDOM_FLOAT(-1, 1);
	direction.y = RANDOM_FLOAT(-1, 1);
	direction.z = RANDOM_FLOAT(0, 1);

	return direction;
}


void UTIL_BloodDecalTrace(TraceResult* pTrace, int bloodColor)
{
	if (UTIL_ShouldShowBlood(bloodColor))
	{
		if (bloodColor == BLOOD_COLOR_RED)
			UTIL_DecalTrace(pTrace, DECAL_BLOOD1 + RANDOM_LONG(0, 5));
		else
			UTIL_DecalTrace(pTrace, DECAL_YBLOOD1 + RANDOM_LONG(0, 5));
	}
}


void UTIL_DecalTrace(TraceResult* pTrace, int decalNumber)
{
	short entityIndex;
	int index;
	int message;

	if (decalNumber < 0)
		return;

	index = gDecals[decalNumber].index;

	if (index < 0)
		return;

	if (pTrace->flFraction == 1.0)
		return;

	// Only decal BSP models
	if (pTrace->pHit)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(pTrace->pHit);
		if (pEntity && !pEntity->IsBSPModel())
			return;
		entityIndex = ENTINDEX(pTrace->pHit);
	}
	else
		entityIndex = 0;

	message = TE_DECAL;
	if (entityIndex != 0)
	{
		if (index > 255)
		{
			message = TE_DECALHIGH;
			index -= 256;
		}
	}
	else
	{
		message = TE_WORLDDECAL;
		if (index > 255)
		{
			message = TE_WORLDDECALHIGH;
			index -= 256;
		}
	}

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(message);
	WRITE_COORD(pTrace->vecEndPos.x);
	WRITE_COORD(pTrace->vecEndPos.y);
	WRITE_COORD(pTrace->vecEndPos.z);
	WRITE_BYTE(index);
	if (entityIndex)
		WRITE_SHORT(entityIndex);
	MESSAGE_END();
}

void UTIL_PlayerDecalTrace(TraceResult* pTrace, int playernum, int decalNumber, bool bIsCustom)
{
	int index;

	if (!bIsCustom)
	{
		if (decalNumber < 0)
			return;

		index = gDecals[decalNumber].index;
		if (index < 0)
			return;
	}
	else
		index = decalNumber;

	if (pTrace->flFraction == 1.0)
		return;

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_PLAYERDECAL);
	WRITE_BYTE(playernum);
	WRITE_COORD(pTrace->vecEndPos.x);
	WRITE_COORD(pTrace->vecEndPos.y);
	WRITE_COORD(pTrace->vecEndPos.z);
	WRITE_SHORT((short)ENTINDEX(pTrace->pHit));
	WRITE_BYTE(index);
	MESSAGE_END();
}

void UTIL_GunshotDecalTrace(TraceResult* pTrace, int decalNumber)
{
	if (decalNumber < 0)
		return;

	int index = gDecals[decalNumber].index;
	if (index < 0)
		return;

	if (pTrace->flFraction == 1.0)
		return;

	MESSAGE_BEGIN(MessageDest::PAS, SVC_TEMPENTITY, pTrace->vecEndPos);
	WRITE_BYTE(TE_GUNSHOTDECAL);
	WRITE_COORD(pTrace->vecEndPos.x);
	WRITE_COORD(pTrace->vecEndPos.y);
	WRITE_COORD(pTrace->vecEndPos.z);
	WRITE_SHORT((short)ENTINDEX(pTrace->pHit));
	WRITE_BYTE(index);
	MESSAGE_END();
}


void UTIL_Sparks(const Vector& position)
{
	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, position);
	WRITE_BYTE(TE_SPARKS);
	WRITE_COORD(position.x);
	WRITE_COORD(position.y);
	WRITE_COORD(position.z);
	MESSAGE_END();
}


void UTIL_Ricochet(const Vector& position, float scale)
{
	MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, position);
	WRITE_BYTE(TE_ARMOR_RICOCHET);
	WRITE_COORD(position.x);
	WRITE_COORD(position.y);
	WRITE_COORD(position.z);
	WRITE_BYTE((int)(scale * 10));
	MESSAGE_END();
}


bool UTIL_TeamsMatch(const char* pTeamName1, const char* pTeamName2)
{
	// Everyone matches unless it's teamplay
	if (!g_pGameRules->IsTeamplay())
		return true;

	// Both on a team?
	if (*pTeamName1 != 0 && *pTeamName2 != 0)
	{
		if (!stricmp(pTeamName1, pTeamName2))	// Same Team?
			return true;
	}

	return false;
}

float UTIL_WaterLevel(const Vector& position, float minz, float maxz)
{
	Vector midUp = position;
	midUp.z = minz;

	if (UTIL_PointContents(midUp) != Contents::Water)
		return minz;

	midUp.z = maxz;
	if (UTIL_PointContents(midUp) == Contents::Water)
		return maxz;

	float diff = maxz - minz;
	while (diff > 1.0)
	{
		midUp.z = minz + diff / 2.0;
		if (UTIL_PointContents(midUp) == Contents::Water)
		{
			minz = midUp.z;
		}
		else
		{
			maxz = midUp.z;
		}
		diff = maxz - minz;
	}

	return midUp.z;
}

void UTIL_Bubbles(Vector mins, Vector maxs, int count)
{
	Vector mid = (mins + maxs) * 0.5;

	float flHeight = UTIL_WaterLevel(mid, mid.z, mid.z + 1024);
	flHeight = flHeight - mins.z;

	MESSAGE_BEGIN(MessageDest::PAS, SVC_TEMPENTITY, mid);
	WRITE_BYTE(TE_BUBBLES);
	WRITE_COORD(mins.x);	// mins
	WRITE_COORD(mins.y);
	WRITE_COORD(mins.z);
	WRITE_COORD(maxs.x);	// maxz
	WRITE_COORD(maxs.y);
	WRITE_COORD(maxs.z);
	WRITE_COORD(flHeight);			// height
	WRITE_SHORT(g_sModelIndexBubbles);
	WRITE_BYTE(count); // count
	WRITE_COORD(8); // speed
	MESSAGE_END();
}

void UTIL_BubbleTrail(Vector from, Vector to, int count)
{
	float flHeight = UTIL_WaterLevel(from, from.z, from.z + 256);
	flHeight = flHeight - from.z;

	if (flHeight < 8)
	{
		flHeight = UTIL_WaterLevel(to, to.z, to.z + 256);
		flHeight = flHeight - to.z;
		if (flHeight < 8)
			return;

		// UNDONE: do a ploink sound
		flHeight = flHeight + to.z - from.z;
	}

	if (count > 255)
		count = 255;

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BUBBLETRAIL);
	WRITE_COORD(from.x);	// mins
	WRITE_COORD(from.y);
	WRITE_COORD(from.z);
	WRITE_COORD(to.x);	// maxz
	WRITE_COORD(to.y);
	WRITE_COORD(to.z);
	WRITE_COORD(flHeight);			// height
	WRITE_SHORT(g_sModelIndexBubbles);
	WRITE_BYTE(count); // count
	WRITE_COORD(8); // speed
	MESSAGE_END();
}


void UTIL_Remove(CBaseEntity* pEntity)
{
	if (!pEntity)
		return;

	pEntity->UpdateOnRemove();
	pEntity->pev->flags |= FL_KILLME;
	pEntity->pev->targetname = iStringNull;
}


bool UTIL_IsValidEntity(edict_t* pent)
{
	if (!pent || pent->free || (pent->v.flags & FL_KILLME))
		return false;
	return true;
}


void UTIL_PrecacheOther(const char* szClassname)
{
	auto pEntity = UTIL_CreateNamedEntity(MAKE_STRING(szClassname));
	if (IsNullEnt(pEntity))
	{
		ALERT(at_console, "NULL Ent in UTIL_PrecacheOther\n");
		return;
	}

	pEntity->Precache();
	REMOVE_ENTITY(pEntity->edict());
}

void UTIL_LogPrintf(const char* fmt, ...)
{
	va_list			argptr;
	static char		string[1024];

	va_start(argptr, fmt);
	vsnprintf(string, sizeof(string), fmt, argptr);
	va_end(argptr);

	// Print to server console
	ALERT(at_logged, "%s", string);
}

float UTIL_DotPoints(const Vector& vecSrc, const Vector& vecCheck, const Vector& vecDir)
{
	Vector2D	vec2LOS;

	vec2LOS = (vecCheck - vecSrc).Make2D();
	vec2LOS = vec2LOS.Normalize();

	return DotProduct(vec2LOS, (vecDir.Make2D()));
}

void UTIL_StripToken(const char* pKey, char* pDest)
{
	int i = 0;

	while (pKey[i] && pKey[i] != '#')
	{
		pDest[i] = pKey[i];
		i++;
	}
	pDest[i] = 0;
}

void SetMovedir(CBaseEntity* pEntity)
{
	if (pEntity->pev->angles == Vector(0, -1, 0))
	{
		pEntity->pev->movedir = vec3_up;
	}
	else if (pEntity->pev->angles == Vector(0, -2, 0))
	{
		pEntity->pev->movedir = vec3_down;
	}
	else
	{
		UTIL_MakeVectors(pEntity->pev->angles);
		pEntity->pev->movedir = gpGlobals->v_forward;
	}

	pEntity->pev->angles = vec3_origin;
}

Vector GetBrushModelOrigin(CBaseEntity* pBModel)
{
	return pBModel->pev->absmin + (pBModel->pev->size * 0.5);
}

int UTIL_CountPlayers()
{
	int	num = 0;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* pEnt = UTIL_PlayerByIndex(i);

		if (pEnt)
		{
			++num;
		}
	}

	return num;
}

void SetObjectCollisionBox(entvars_t* pev)
{
	if ((pev->solid == Solid::BSP) &&
		(pev->angles.x || pev->angles.y || pev->angles.z))
	{
		// expand for rotation
		float max = 0;

		for (int i = 0; i < 3; i++)
		{
			float v = fabs(pev->mins[i]);
			if (v > max)
				max = v;
			v = fabs(pev->maxs[i]);
			if (v > max)
				max = v;
		}

		for (int i = 0; i < 3; i++)
		{
			pev->absmin[i] = pev->origin[i] - max;
			pev->absmax[i] = pev->origin[i] + max;
		}
	}
	else
	{
		pev->absmin = pev->origin + pev->mins;
		pev->absmax = pev->origin + pev->maxs;
	}

	pev->absmin.x -= 1;
	pev->absmin.y -= 1;
	pev->absmin.z -= 1;
	pev->absmax.x += 1;
	pev->absmax.y += 1;
	pev->absmax.z += 1;
}
