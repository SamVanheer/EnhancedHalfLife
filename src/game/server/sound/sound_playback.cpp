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

#include "sound_playback.hpp"

void UTIL_EmitAmbientSound(CBaseEntity* entity, const Vector& vecOrigin, const char* samp, float vol, float attenuation, int fFlags, int pitch)
{
	auto edict = CBaseEntity::EdictOrNull(entity);

	if (samp)
	{
		if (*samp == '!')
		{
			char name[32];
			if (SENTENCEG_Lookup(samp, name, sizeof(name)) >= 0)
				g_engfuncs.pfnEmitAmbientSound(edict, vecOrigin, name, vol, attenuation, fFlags, pitch);
		}
		else
			g_engfuncs.pfnEmitAmbientSound(edict, vecOrigin, samp, vol, attenuation, fFlags, pitch);
	}
	else
	{
		if (entity)
		{
			ALERT(at_warning, "UTIL_EmitAmbientSound: NULL sample name passed for entity %s(%s)\n", entity->GetClassname(), entity->GetTarget());
		}
		else
		{
			ALERT(at_warning, "UTIL_EmitAmbientSound: NULL entity passed\n");
		}
	}
}

int SENTENCEG_PlayRndI(CBaseEntity* entity, int isentenceg,
	float volume, float attenuation, int pitch, int flags)
{
	if (!entity)
	{
		return -1;
	}

	if (!fSentencesInit)
		return -1;

	char name[64]{};
	const int ipick = USENTENCEG_Pick(isentenceg, name, sizeof(name));
	if (ipick > 0 && name)
		entity->EmitSound(SoundChannel::Voice, name, volume, attenuation, pitch, flags);
	return ipick;
}

int SENTENCEG_PlayRndSz(CBaseEntity* entity, const char* szgroupname,
	float volume, float attenuation, int pitch, int flags)
{
	if (!entity)
	{
		return -1;
	}

	if (!fSentencesInit)
		return -1;

	const int isentenceg = SENTENCEG_GetIndex(szgroupname);
	if (isentenceg < 0)
	{
		ALERT(at_console, "No such sentence group %s\n", szgroupname);
		return -1;
	}

	char name[64]{};
	const int ipick = USENTENCEG_Pick(isentenceg, name, sizeof(name));
	if (ipick >= 0 && name[0])
		entity->EmitSound(SoundChannel::Voice, name, volume, attenuation, pitch, flags);

	return ipick;
}

int SENTENCEG_PlaySequentialSz(CBaseEntity* entity, const char* szgroupname,
	float volume, float attenuation, int pitch, int ipick, int freset, int flags)
{
	if (!entity)
	{
		return -1;
	}

	if (!fSentencesInit)
		return -1;

	const int isentenceg = SENTENCEG_GetIndex(szgroupname);
	if (isentenceg < 0)
		return -1;

	char name[64]{};
	const int ipicknext = USENTENCEG_PickSequential(isentenceg, name, sizeof(name), ipick, freset);
	if (ipicknext >= 0 && name[0])
		entity->EmitSound(SoundChannel::Voice, name, volume, attenuation, pitch, flags);
	return ipicknext;
}

void SENTENCEG_Stop(CBaseEntity* entity, int isentenceg, int ipick)
{
	if (!entity)
	{
		return;
	}

	if (!fSentencesInit)
		return;

	if (isentenceg < 0 || ipick < 0)
		return;

	char buffer[64];
	snprintf(buffer, sizeof(buffer), "!%s%d", rgsentenceg[isentenceg].szgroupname, ipick);

	entity->StopSound(SoundChannel::Voice, buffer);
}

void EMIT_SOUND_DYN(CBaseEntity* entity, SoundChannel channel, const char* sample, float volume, float attenuation,
	int flags, int pitch)
{
	auto edict = CBaseEntity::EdictOrNull(entity);

	if (sample && *sample == '!')
	{
		char name[32];
		if (SENTENCEG_Lookup(sample, name, sizeof(name)) >= 0)
			g_engfuncs.pfnEmitSound(edict, channel, name, volume, attenuation, flags, pitch);
		else
			ALERT(at_aiconsole, "Unable to find %s in sentences.txt\n", sample);
	}
	else
		g_engfuncs.pfnEmitSound(edict, channel, sample, volume, attenuation, flags, pitch);
}

void EMIT_SOUND_SUIT(CBaseEntity* entity, const char* sample)
{
	int pitch = PITCH_NORM;

	const float fvol = CVAR_GET_FLOAT("suitvolume");
	if (RANDOM_LONG(0, 1))
		pitch = RANDOM_LONG(0, 6) + 98;

	if (fvol > 0.05)
		entity->EmitSound(SoundChannel::Static, sample, fvol, ATTN_NORM, pitch);
}

void EMIT_GROUPID_SUIT(CBaseEntity* entity, int isentenceg)
{
	int pitch = PITCH_NORM;

	const float fvol = CVAR_GET_FLOAT("suitvolume");
	if (RANDOM_LONG(0, 1))
		pitch = RANDOM_LONG(0, 6) + 98;

	if (fvol > 0.05)
		SENTENCEG_PlayRndI(entity, isentenceg, fvol, ATTN_NORM, pitch);
}

void EMIT_GROUPNAME_SUIT(CBaseEntity* entity, const char* groupname)
{
	int pitch = PITCH_NORM;

	const float fvol = CVAR_GET_FLOAT("suitvolume");
	if (RANDOM_LONG(0, 1))
		pitch = RANDOM_LONG(0, 6) + 98;

	if (fvol > 0.05)
		SENTENCEG_PlayRndSz(entity, groupname, fvol, ATTN_NORM, pitch);
}

float TEXTURETYPE_PlaySound(TraceResult* ptr, const Vector& vecSrc, const Vector& vecEnd, int iBulletType)
{
	// hit the world, try to play sound based on texture material type
	if (!g_pGameRules->PlayTextureSounds())
		return 0.0;

	CBaseEntity* pEntity = CBaseEntity::Instance(ptr->pHit);

	char chTextureType = 0;

	if (pEntity && pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
		// hit body
		chTextureType = CHAR_TEX_FLESH;
	else
	{
		// hit world

		// find texture under strike, get material type

		// get texture from entity or world (world is ent(0))
		auto target = pEntity ? pEntity : UTIL_GetWorld();
		const char* pTextureName = TRACE_TEXTURE(target->edict(), vecSrc, vecEnd);

		if (pTextureName)
		{
			// strip leading '-0' or '+0~' or '{' or '!'
			if (*pTextureName == '-' || *pTextureName == '+')
				pTextureName += 2;

			if (*pTextureName == '{' || *pTextureName == '!' || *pTextureName == '~' || *pTextureName == ' ')
				pTextureName++;
			// '}}'

			char szbuffer[64];
			safe_strcpy(szbuffer, pTextureName);

			// ALERT ( at_console, "texture hit: %s\n", szbuffer);

			// get texture type
			chTextureType = TEXTURETYPE_Find(szbuffer);
		}
	}

	float fvol;
	float fvolbar;
	const char* rgsz[4]{};
	int cnt;
	float fattn = ATTN_NORM;

	switch (chTextureType)
	{
	default:
	case CHAR_TEX_CONCRETE: fvol = 0.9;	fvolbar = 0.6;
		rgsz[0] = "player/pl_step1.wav";
		rgsz[1] = "player/pl_step2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_METAL: fvol = 0.9; fvolbar = 0.3;
		rgsz[0] = "player/pl_metal1.wav";
		rgsz[1] = "player/pl_metal2.wav";
		cnt = 2;
		break;
	case CHAR_TEX_DIRT:	fvol = 0.9; fvolbar = 0.1;
		rgsz[0] = "player/pl_dirt1.wav";
		rgsz[1] = "player/pl_dirt2.wav";
		rgsz[2] = "player/pl_dirt3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_VENT:	fvol = 0.5; fvolbar = 0.3;
		rgsz[0] = "player/pl_duct1.wav";
		rgsz[1] = "player/pl_duct1.wav";
		cnt = 2;
		break;
	case CHAR_TEX_GRATE: fvol = 0.9; fvolbar = 0.5;
		rgsz[0] = "player/pl_grate1.wav";
		rgsz[1] = "player/pl_grate4.wav";
		cnt = 2;
		break;
	case CHAR_TEX_TILE:	fvol = 0.8; fvolbar = 0.2;
		rgsz[0] = "player/pl_tile1.wav";
		rgsz[1] = "player/pl_tile3.wav";
		rgsz[2] = "player/pl_tile2.wav";
		rgsz[3] = "player/pl_tile4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_SLOSH: fvol = 0.9; fvolbar = 0.0;
		rgsz[0] = "player/pl_slosh1.wav";
		rgsz[1] = "player/pl_slosh3.wav";
		rgsz[2] = "player/pl_slosh2.wav";
		rgsz[3] = "player/pl_slosh4.wav";
		cnt = 4;
		break;
	case CHAR_TEX_WOOD: fvol = 0.9; fvolbar = 0.2;
		rgsz[0] = "debris/wood1.wav";
		rgsz[1] = "debris/wood2.wav";
		rgsz[2] = "debris/wood3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_GLASS:
	case CHAR_TEX_COMPUTER:
		fvol = 0.8; fvolbar = 0.2;
		rgsz[0] = "debris/glass1.wav";
		rgsz[1] = "debris/glass2.wav";
		rgsz[2] = "debris/glass3.wav";
		cnt = 3;
		break;
	case CHAR_TEX_FLESH:
		if (iBulletType == BULLET_PLAYER_CROWBAR)
			return 0.0; // crowbar already makes this sound
		fvol = 1.0;	fvolbar = 0.2;
		rgsz[0] = "weapons/bullet_hit1.wav";
		rgsz[1] = "weapons/bullet_hit2.wav";
		fattn = 1.0;
		cnt = 2;
		break;
	}

	// did we hit a breakable?

	if (pEntity && pEntity->ClassnameIs("func_breakable"))
	{
		// drop volumes, the object will already play a damaged sound
		fvol /= 1.5;
		fvolbar /= 2.0;
	}
	else if (chTextureType == CHAR_TEX_COMPUTER)
	{
		// play random spark if computer
		if (ptr->flFraction != 1.0 && RANDOM_LONG(0, 1))
		{
			UTIL_Sparks(ptr->vecEndPos);

			float flVolume = RANDOM_FLOAT(0.7, 1.0);//random volume range
			switch (RANDOM_LONG(0, 1))
			{
			case 0: UTIL_EmitAmbientSound(UTIL_GetWorld(), ptr->vecEndPos, "buttons/spark5.wav", flVolume, ATTN_NORM, 0, 100); break;
			case 1: UTIL_EmitAmbientSound(UTIL_GetWorld(), ptr->vecEndPos, "buttons/spark6.wav", flVolume, ATTN_NORM, 0, 100); break;
				// case 0: EmitSound(SoundChannel::Voice, "buttons/spark5.wav", flVolume); break;
				// case 1: EmitSound(SoundChannel::Voice, "buttons/spark6.wav", flVolume); break;
			}
		}
	}

	// play material hit sound
	UTIL_EmitAmbientSound(UTIL_GetWorld(), ptr->vecEndPos, rgsz[RANDOM_LONG(0, cnt - 1)], fvol, fattn, 0, 96 + RANDOM_LONG(0, 0xf));
	//m_pPlayer->EmitSound(SoundChannel::Weapon, rgsz[RANDOM_LONG(0,cnt-1)], fvol, ATTN_NORM, 96 + RANDOM_LONG(0,0xf));

	return fvolbar;
}
