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

#include "CTokenizer.hpp"
#include "CWorld.hpp"
#include "dll_functions.hpp"
#include "EntityParser.hpp"

extern "C" DLLEXPORT void worldspawn(entvars_t* pev)
{
	//If we're loading a save game we still need to do this for now
	//TODO: remove
	if (gpGlobals->pSaveData)
	{
		g_EntityList.Create("worldspawn", pev->pContainingEntity);
	}
	else
	{
		//Nothing. Must be provided so the engine doesn't free the already created worldspawn
	}
}

//TODO: remove this when save game handling is done server side
static CEntityFactory<CWorld> g_CWorldFactory{"worldspawn", "CWorld"};

EntityParser::EntityParser(std::string_view entityData)
	: m_EntityData(entityData)
{
}

void EntityParser::Parse()
{
	CTokenizer tokenizer{m_EntityData};

	//Silence compiler warning
	auto worldEdict = g_EntityList.GetEdictByIndex(0);

	if (!g_EntityList.IsValid() || !worldEdict)
	{
		ALERT(at_error, "Edicts array is NULL in EntityParser::Parse!\n");
		return;
	}

	auto deathmatch = CVAR_GET_POINTER("deathmatch");

	int inhibited = 0;
	edict_t* entity = nullptr;

	while (true)
	{
		if (!tokenizer.Next())
		{
			break;
		}

		if (tokenizer.GetToken()[0] != '{')
		{
			ALERT(at_warning, "EntityParser::Parse: found %s when expecting {", std::string{tokenizer.GetToken()}.c_str());
			SERVER_COMMAND("disconnect\n");
			return;
		}

		if (entity)
		{
			entity = g_engfuncs.pfnCreateEntity();
		}
		else
		{
			//Parsing worldspawn; entity already has keyvalues set by engine so don't memset
			entity = worldEdict;

			if (auto baseEntity = CBaseEntity::InstanceOrNull(entity); baseEntity)
			{
				g_EntityList.Destroy(baseEntity);
			}

			entity->v.pContainingEntity = entity;
		}

		tokenizer = CTokenizer{ParseEntity(tokenizer.GetCurrentData(), entity)};

		if (!entity->free)
		{
			if (deathmatch->value && entity->v.spawnflags & SF_NOTINDEATHMATCH)
			{
				++inhibited;
			}
			else if (!IsStringNull(entity->v.classname))
			{
				if ((DispatchSpawn(entity) >= 0 && !(entity->v.flags & FL_KILLME)) || entity->free)
				{
					continue;
				}
			}
			else
			{
				ALERT(at_console, "No classname for:\n");
			}

			g_engfuncs.pfnRemoveEntity(entity);
		}
	}

	ALERT(at_console, "%d entities inhibited\n", inhibited);
}

std::string_view EntityParser::ParseEntity(std::string_view entityData, edict_t* ent)
{
	if (g_EntityList.GetEdictByIndex(0) != ent)
	{
		memset(&ent->v, 0, sizeof(ent->v));
	}

	ent->v.pContainingEntity = ent;

	{
		std::string_view candidate;
		
		CTokenizer tokenizer{entityData};

		do
		{
			if (!tokenizer.Next() || tokenizer.GetToken()[0] == '}')
			{
				FreeEntity(ent);
				return tokenizer.GetCurrentData();
			}

			candidate = tokenizer.GetToken();

			tokenizer.Next();
		}
		while (tokenizer.HasToken() && candidate != "classname");

		char s1[256]{};

		safe_strcpy(s1, tokenizer.GetToken());

		KeyValueData kvd{};
		kvd.szClassName = nullptr;
		kvd.szKeyName = "classname";
		kvd.fHandled = false;
		kvd.szValue = s1;

		DispatchKeyValue(ent, &kvd);

		if (!kvd.fHandled)
		{
			ALERT(at_warning, "EntityParser::ParseEntity: couldn't set classname");
			FreeEntity(ent);
			return AdvancePastBlock(entityData);
		}
	}

	auto entity = g_EntityList.Create(STRING(ent->v.classname), ent);

	if (!entity)
	{
		//Failure to create is reported by the entity list
		FreeEntity(ent);
		return AdvancePastBlock(entityData);
	}

	char keyname[256]{};
	char valueBuffer[2048];

	CTokenizer tokenizer{entityData};

	while (true)
	{
		if (!tokenizer.Next())
		{
			ALERT(at_error, "EntityParser::ParseEntity: EOF without closing brace\n");
			FreeEntity(ent);
			return {};
		}

		if (tokenizer.GetToken() == "}")
		{
			break;
		}

		safe_strcpy(keyname, tokenizer.GetToken());

		const int keyLength = strlen(keyname);

		for (int i = keyLength - 1; i >= 0; --i)
		{
			if (keyname[i] != ' ')
			{
				break;
			}

			keyname[i] = '\0';
		}

		if (!tokenizer.Next())
		{
			ALERT(at_error, "EntityParser::ParseEntity: EOF without closing brace\n");
			FreeEntity(ent);
			return {};
		}

		safe_strcpy(valueBuffer, tokenizer.GetToken());

		if (valueBuffer[0] == '}')
		{
			ALERT(at_error, "EntityParser::ParseEntity: closing brace without data\n");
			FreeEntity(ent);
			return AdvancePastBlock(entityData);
		}

		const char* className = STRING(ent->v.classname);
		if (!className || strcmp(className, valueBuffer))
		{
			if (!strcmp(keyname, "angle"))
			{
				const float angle = atof(valueBuffer);

				if (angle >= 0)
				{
					snprintf(valueBuffer, sizeof(valueBuffer), "%f %f %f", ent->v.angles[0], angle, ent->v.angles[2]);
				}
				else if (static_cast<int>(angle) == -1)
				{
					snprintf(valueBuffer, sizeof(valueBuffer), "-90 0 0");
				}
				else
				{
					snprintf(valueBuffer, sizeof(valueBuffer), "90 0 0");
				}

				safe_strcpy(keyname, "angles");
			}

			KeyValueData kvd{};
			kvd.szKeyName = keyname;
			kvd.szValue = valueBuffer;
			kvd.szClassName = STRING(ent->v.classname);
			kvd.fHandled = false;

			DispatchKeyValue(ent, &kvd);
		}
	}

	return tokenizer.GetCurrentData();
}

std::string_view EntityParser::AdvancePastBlock(std::string_view entityData)
{
	CTokenizer tokenizer{entityData};

	while (tokenizer.Next())
	{
		if (tokenizer.GetToken() == "}")
		{
			return tokenizer.GetCurrentData();
		}
	}

	return {};
}

void EntityParser::FreeEntity(edict_t* ent)
{
	if (auto baseEntity = CBaseEntity::InstanceOrNull(ent); baseEntity)
	{
		g_EntityList.Destroy(baseEntity);
	}

	++ent->serialnumber;
	ent->free = true;
}
