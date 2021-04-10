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

#include "extdll.h"
#include "util.h"

#include "materials.hpp"
#include "shared_utils.hpp"

bool fTextureTypeInit = false;

int gcTextures = 0;
char grgszTextureName[CTEXTURESMAX][CBTEXTURENAMEMAX];	// texture names
char grgchTextureType[CTEXTURESMAX];						// parallel array of texture types

// open materials.txt,  get size, alloc space, 
// save in array.  Only works first time called, 
// ignored on subsequent calls.

void TEXTURETYPE_Init()
{
	char buffer[512];
	int i, j;
	byte* pMemFile;
	int fileSize, filePos = 0;

	if (fTextureTypeInit)
		return;

	memset(&(grgszTextureName[0][0]), 0, CTEXTURESMAX * CBTEXTURENAMEMAX);
	memset(grgchTextureType, 0, CTEXTURESMAX);

	gcTextures = 0;
	memset(buffer, 0, 512);

	pMemFile = g_engfuncs.pfnLoadFileForMe("sound/materials.txt", &fileSize);
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

	g_engfuncs.pfnFreeFile(pMemFile);

	fTextureTypeInit = true;
}

// given texture name, find texture type
// if not found, return type 'concrete'

// NOTE: this routine should ONLY be called if the 
// current texture under the player changes!

char TEXTURETYPE_Find(char* name)
{
	// CONSIDER: pre-sort texture names and perform faster binary search here

	for (int i = 0; i < gcTextures; i++)
	{
		if (!strnicmp(name, &(grgszTextureName[i][0]), CBTEXTURENAMEMAX - 1))
			return (grgchTextureType[i]);
	}

	return CHAR_TEX_CONCRETE;
}
