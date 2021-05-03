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

#include <cstddef>

class CBaseEntity;
class CBasePlayer;
struct globalvars_t;

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

char* memfgets(byte* pMemFile, std::size_t fileSize, std::size_t& filePos, char* pBuffer, std::size_t bufferSize);

int UTIL_SharedRandomLong(unsigned int seed, int low, int high);
float UTIL_SharedRandomFloat(unsigned int seed, float low, float high);

struct EventPlaybackArgs
{
	const float delay = 0;

	const Vector origin;
	const Vector angles;

	const float fparam1 = 0;
	const float fparam2 = 0;

	const int iparam1 = 0;
	const int iparam2 = 0;

	const bool bparam1 = false;
	const bool bparam2 = false;
};

void MESSAGE_BEGIN(MessageDest msg_dest, int msg_type, const float* pOrigin, CBasePlayer* pPlayer);

inline void MESSAGE_BEGIN(MessageDest msg_dest, int msg_type)
{
	MESSAGE_BEGIN(msg_dest, msg_type, nullptr, nullptr);
}

inline void MESSAGE_BEGIN(MessageDest msg_dest, int msg_type, const Vector& origin)
{
	MESSAGE_BEGIN(msg_dest, msg_type, origin, nullptr);
}

inline void MESSAGE_BEGIN(MessageDest msg_dest, int msg_type, CBasePlayer* pPlayer)
{
	MESSAGE_BEGIN(msg_dest, msg_type, nullptr, pPlayer);
}

void UTIL_PlaybackEvent(int flags, CBaseEntity* invoker, unsigned short eventIndex, const EventPlaybackArgs& args = {});
