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

#include <cassert>

#include "Platform.h"
#include "filesystem_shared.hpp"
#include "materials.hpp"
#include "shared_utils.hpp"

static bool fTextureTypeInit = false;

int gcTextures = 0;
char grgszTextureName[CTEXTURESMAX][CBTEXTURENAMEMAX];	// texture names
char grgchTextureType[CTEXTURESMAX];						// parallel array of texture types

void TEXTURETYPE_SwapTextures(int i, int j)
{
	char szTemp[CBTEXTURENAMEMAX];

	strcpy(szTemp, grgszTextureName[i]);
	const char chTemp = grgchTextureType[i];

	strcpy(grgszTextureName[i], grgszTextureName[j]);
	grgchTextureType[i] = grgchTextureType[j];

	strcpy(grgszTextureName[j], szTemp);
	grgchTextureType[j] = chTemp;
}

void TEXTURETYPE_SortTextures()
{
	// Bubble sort, yuck, but this only occurs at startup and it's only 512 elements...
	//
	for (int i = 0; i < gcTextures; i++)
	{
		for (int j = i + 1; j < gcTextures; j++)
		{
			if (stricmp(grgszTextureName[i], grgszTextureName[j]) > 0)
			{
				// Swap
				//
				TEXTURETYPE_SwapTextures(i, j);
			}
		}
	}
}

void TEXTURETYPE_Init()
{
	char buffer[512];
	int i, j;
	int filePos = 0;

	if (fTextureTypeInit)
		return;

	memset(&(grgszTextureName[0][0]), 0, CTEXTURESMAX * CBTEXTURENAMEMAX);
	memset(grgchTextureType, 0, CTEXTURESMAX);

	gcTextures = 0;
	memset(buffer, 0, 512);

	auto [fileBuffer, size] = FileSystem_LoadFileIntoBuffer("sound/materials.txt");

	byte* pMemFile = fileBuffer.get();
	//TODO: really large files could cause problems here due to the narrowing conversion
	const int fileSize = size;
	if (!pMemFile)
		return;

	// for each line in the file...
	while (memfgets(pMemFile, fileSize, filePos, buffer, 511) != nullptr && (gcTextures < CTEXTURESMAX))
	{
		// skip whitespace
		i = 0;
		while (buffer[i] && isspace(buffer[i]))
			i++;

		if (!buffer[i])
			continue;

		// skip comment lines
		if (buffer[i] == '/' || !isalpha(buffer[i]))
			continue;

		// get texture type
		grgchTextureType[gcTextures] = toupper(buffer[i++]);

		// skip whitespace
		while (buffer[i] && isspace(buffer[i]))
			i++;

		if (!buffer[i])
			continue;

		// get sentence name
		j = i;
		while (buffer[j] && !isspace(buffer[j]))
			j++;

		if (!buffer[j])
			continue;

		// null-terminate name and save in sentences array
		j = V_min(j, CBTEXTURENAMEMAX - 1 + i);
		buffer[j] = 0;
		strcpy(&(grgszTextureName[gcTextures++][0]), &(buffer[i]));
	}

	TEXTURETYPE_SortTextures();

	fTextureTypeInit = true;
}

char TEXTURETYPE_Find(const char* name)
{
	assert(fTextureTypeInit);

	int left = 0;
	int right = gcTextures - 1;

	while (left <= right)
	{
		const int pivot = (left + right) / 2;

		const int val = strnicmp(name, grgszTextureName[pivot], CBTEXTURENAMEMAX - 1);
		if (val == 0)
		{
			return grgchTextureType[pivot];
		}
		else if (val > 0)
		{
			left = pivot + 1;
		}
		else if (val < 0)
		{
			right = pivot - 1;
		}
	}

	return CHAR_TEX_CONCRETE;
}