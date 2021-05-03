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

#include <string>

#include "Platform.h"
#include "shared_utils.hpp"
#include "cbase.h"
#include "player.h"
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

char* memfgets(byte* pMemFile, std::size_t fileSize, std::size_t& filePos, char* pBuffer, std::size_t bufferSize)
{
	// Bullet-proofing
	if (!pMemFile || !pBuffer)
		return nullptr;

	if (filePos >= fileSize)
		return nullptr;

	std::size_t i = filePos;
	std::size_t last = fileSize;

	// fgets always NULL terminates, so only read bufferSize-1 characters
	if (last - filePos > (bufferSize - 1))
		last = filePos + (bufferSize - 1);

	bool stop = false;

	// Stop at the next newline (inclusive) or end of buffer
	while (i < last && !stop)
	{
		if (pMemFile[i] == '\n')
			stop = true;
		++i;
	}


	// If we actually advanced the pointer, copy it over
	if (i != filePos)
	{
		// We read in size bytes
		std::size_t size = i - filePos;
		// copy it out
		memcpy(pBuffer, pMemFile + filePos, sizeof(byte) * size);

		// If the buffer isn't full, terminate (this is always true)
		if (size < bufferSize)
			pBuffer[size] = 0;

		// Update file pointer
		filePos = i;
		return pBuffer;
	}

	// No data read, bail
	return nullptr;
}

static unsigned int glSeed = 0;

static constexpr unsigned int seed_table[256] =
{
	28985, 27138, 26457, 9451, 17764, 10909, 28790, 8716, 6361, 4853, 17798, 21977, 19643, 20662, 10834, 20103,
	27067, 28634, 18623, 25849, 8576, 26234, 23887, 18228, 32587, 4836, 3306, 1811, 3035, 24559, 18399, 315,
	26766, 907, 24102, 12370, 9674, 2972, 10472, 16492, 22683, 11529, 27968, 30406, 13213, 2319, 23620, 16823,
	10013, 23772, 21567, 1251, 19579, 20313, 18241, 30130, 8402, 20807, 27354, 7169, 21211, 17293, 5410, 19223,
	10255, 22480, 27388, 9946, 15628, 24389, 17308, 2370, 9530, 31683, 25927, 23567, 11694, 26397, 32602, 15031,
	18255, 17582, 1422, 28835, 23607, 12597, 20602, 10138, 5212, 1252, 10074, 23166, 19823, 31667, 5902, 24630,
	18948, 14330, 14950, 8939, 23540, 21311, 22428, 22391, 3583, 29004, 30498, 18714, 4278, 2437, 22430, 3439,
	28313, 23161, 25396, 13471, 19324, 15287, 2563, 18901, 13103, 16867, 9714, 14322, 15197, 26889, 19372, 26241,
	31925, 14640, 11497, 8941, 10056, 6451, 28656, 10737, 13874, 17356, 8281, 25937, 1661, 4850, 7448, 12744,
	21826, 5477, 10167, 16705, 26897, 8839, 30947, 27978, 27283, 24685, 32298, 3525, 12398, 28726, 9475, 10208,
	617, 13467, 22287, 2376, 6097, 26312, 2974, 9114, 21787, 28010, 4725, 15387, 3274, 10762, 31695, 17320,
	18324, 12441, 16801, 27376, 22464, 7500, 5666, 18144, 15314, 31914, 31627, 6495, 5226, 31203, 2331, 4668,
	12650, 18275, 351, 7268, 31319, 30119, 7600, 2905, 13826, 11343, 13053, 15583, 30055, 31093, 5067, 761,
	9685, 11070, 21369, 27155, 3663, 26542, 20169, 12161, 15411, 30401, 7580, 31784, 8985, 29367, 20989, 14203,
	29694, 21167, 10337, 1706, 28578, 887, 3373, 19477, 14382, 675, 7033, 15111, 26138, 12252, 30996, 21409,
	25678, 18555, 13256, 23316, 22407, 16727, 991, 9236, 5373, 29402, 6117, 15241, 27715, 19291, 19888, 19847
};

static unsigned int U_Random()
{
	glSeed *= 69069;
	glSeed += seed_table[glSeed & 0xff];

	return (++glSeed & 0x0fffffff);
}

static constexpr void U_Srand(unsigned int seed)
{
	glSeed = seed_table[seed & 0xff];
}

int UTIL_SharedRandomLong(unsigned int seed, int low, int high)
{
	U_Srand((int)seed + low + high);

	const unsigned int range = high - low + 1;
	if (!(range - 1))
	{
		return low;
	}
	else
	{
		const int rnum = U_Random();

		const int offset = rnum % range;

		return (low + offset);
	}
}

float UTIL_SharedRandomFloat(unsigned int seed, float low, float high)
{
	U_Srand((int)seed + *(int*)&low + *(int*)&high);

	U_Random();
	U_Random();

	const unsigned int range = high - low;
	if (!range)
	{
		return low;
	}
	else
	{
		const int tensixrand = U_Random() & 65535;

		const float offset = (float)tensixrand / 65536.0;

		return (low + offset * range);
	}
}

void MESSAGE_BEGIN(MessageDest msg_dest, int msg_type, const float* pOrigin, CBasePlayer* pPlayer)
{
	//TODO: reliable versions of this don't seem to use the origin parameter properly in the engine
	if ((msg_dest == MessageDest::PVS
		|| msg_dest == MessageDest::PAS
		|| msg_dest == MessageDest::PVSReliable
		|| msg_dest == MessageDest::PASReliable)
		&& !pOrigin)
	{
		ALERT(at_warning, "No origin provided for position-based message!\n");
	}

	g_engfuncs.pfnMessageBegin(msg_dest, msg_type, pOrigin, CBaseEntity::EdictOrNull(pPlayer));
}

void UTIL_PlaybackEvent(int flags, CBaseEntity* invoker, unsigned short eventIndex, const EventPlaybackArgs& args)
{
	PLAYBACK_EVENT_FULL(flags, CBaseEntity::EdictOrNull(invoker), eventIndex, args.delay,
		args.origin, args.angles, args.fparam1, args.fparam2, args.iparam1, args.iparam2, args.bparam1, args.bparam2);
}
