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

#include "Platform.h"
#include "shared_utils.hpp"

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
