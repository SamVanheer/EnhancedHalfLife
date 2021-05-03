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
