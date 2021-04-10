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

#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

#include "Platform.h"
#include "filesystem_shared.hpp"
#include "materials.hpp"
#include "shared_utils.hpp"

struct Material
{
	std::array<char, CBTEXTURENAMEMAX> Name;
	char Type = CHAR_TEX_CONCRETE;
};

static bool g_TextureTypeInit = false;

static std::vector<Material> g_Materials;

void TEXTURETYPE_Init()
{
	if (g_TextureTypeInit)
		return;

	g_Materials.clear();

	auto [fileBuffer, size] = FileSystem_LoadFileIntoBuffer("sound/materials.txt");

	byte* pMemFile = fileBuffer.get();
	//TODO: really large files could cause problems here due to the narrowing conversion
	const int fileSize = size;

	if (!pMemFile)
		return;

	char buffer[512]{};

	// for each line in the file...
	for (int i, j, filePos = 0; memfgets(pMemFile, fileSize, filePos, buffer, 511) != nullptr;)
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

		Material material;

		// get texture type
		material.Type = toupper(buffer[i++]);

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
		buffer[j] = '\0';
		strncpy(material.Name.data(), &(buffer[i]), material.Name.size() - 1);
		material.Name.back() = '\0';

		g_Materials.push_back(std::move(material));
	}

	std::sort(g_Materials.begin(), g_Materials.end(), [](const Material& lhs, const Material& rhs)
		{
			return stricmp(lhs.Name.data(), rhs.Name.data()) < 0;
		}
	);

	g_TextureTypeInit = true;
}

char TEXTURETYPE_Find(const char* name)
{
	assert(g_TextureTypeInit);

	auto result = std::lower_bound(g_Materials.begin(), g_Materials.end(), name, [](const Material& material, const char* textureName)
		{
			return strnicmp(material.Name.data(), textureName, CBTEXTURENAMEMAX - 1) < 0;
		});

	if (result != g_Materials.end())
	{
		return result->Type;
	}

	return CHAR_TEX_CONCRETE;
}
