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

#include <string_view>

class CTokenizer;
struct edict_t;

/**
*	@brief Parses the entity data string and creates instances of entities
*/
class EntityParser final
{
public:
	EntityParser(std::string_view entityData);
	~EntityParser() = default;

	EntityParser(const EntityParser&) = delete;
	EntityParser& operator=(const EntityParser&) = delete;

	void Parse();

private:
	std::string_view ParseEntity(std::string_view entityData, edict_t* ent);

	static std::string_view AdvancePastBlock(std::string_view entityData);

	static void FreeEntity(edict_t* ent);

private:
	std::string_view m_EntityData;
};