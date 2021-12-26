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

#include "cbase.hpp"
#include "EntityDictionary.hpp"
#include "EntityList.hpp"

CEntityList::CEntityList(edict_t* entities)
	: m_Entities(entities)
{
	ASSERTSZ(m_Entities, "Entities list must be non-null");
}

int CEntityList::GetEntityLimit() const
{
	return gpGlobals->maxEntities;
}

edict_t* CEntityList::GetEdictByIndex(int index) const
{
	if (index < 0 || index >= GetEntityLimit())
	{
		return nullptr;
	}

	return m_Entities + index;
}

EHandle<CBaseEntity> CEntityList::GetEntityByIndex(int index) const
{
	if (index < 0 || index >= GetEntityLimit())
	{
		return {};
	}

	return InternalGetEntityByIndex(index);
}

EHandle<CBaseEntity> CEntityList::InternalGetEntityByIndex(int index) const
{
	ASSERTSZ(m_Entities, "Entities list not initialized");
	ASSERT(index >= 0 && index < GetEntityLimit());

	auto edict = m_Entities + index;

	//Player entities can have a valid CBaseEntity instance while still being marked free
	if (edict->free)
	{
		return nullptr;
	}

	auto entity = CBaseEntity::InstanceOrNull(edict);

	return entity;
}

EHandle<CBaseEntity> CEntityList::GetFirstEntity() const
{
	return GetNextEntity({});
}

EHandle<CBaseEntity> CEntityList::GetNextEntity(const EHandle<CBaseEntity>& startEntity) const
{
	for (int index = startEntity ? startEntity->entindex() + 1 : 0; index < GetEntityLimit(); ++index)
	{
		if (auto handle = InternalGetEntityByIndex(index); handle)
		{
			return handle;
		}
	}

	return {};
}

CBaseEntity* CEntityList::Create(const char* className)
{
	if (!className)
	{
		ALERT(at_error, "CEntityList::Create: NULL classname\n");
		return nullptr;
	}

	if (auto factory = GetFactory(className); factory)
	{
		//TODO: avoid using this engine function
		auto edict = g_engfuncs.pfnCreateEntity();

		return ConstructEntity(factory, edict);
	}

	return nullptr;
}

CBaseEntity* CEntityList::Create(const char* className, edict_t* edict)
{
	if (!className)
	{
		ALERT(at_error, "CEntityList::Create: NULL classname\n");
		return nullptr;
	}

	if (!edict)
	{
		ALERT(at_error, "CEntityList::Create: NULL edict\n");
		return nullptr;
	}

	if (auto factory = GetFactory(className); factory)
	{
		return ConstructEntity(factory, edict);
	}

	return nullptr;
}

CEntityFactory* CEntityList::GetFactory(const char* className)
{
	if (auto factory = g_EntityDictionary.Find(className); factory)
	{
		return factory;
	}

	ALERT(at_error, "CEntityList::Create: no factory for \"%s\", can't create entity\n", className);

	return nullptr;
}

CBaseEntity* CEntityList::ConstructEntity(CEntityFactory* factory, edict_t* edict)
{
	auto entity = factory->CreateInstance();

	entity->pev = &edict->v;
	edict->pvPrivateData = entity;

	//Use the factory's string to ensure it lives long enough
	entity->pev->classname = MAKE_STRING(factory->GetMapClassName().c_str());

	entity->Construct();

	return entity;
}

void CEntityList::Destroy(CBaseEntity* entity)
{
	//TODO: this is designed to be called from OnFreeEntPrivateData. Rework this to free the edict as well at some point
	if (!entity)
	{
		ALERT(at_error, "CEntityList::Destroy: NULL entity\n");
		return;
	}

	auto className = STRING(entity->pev->classname);

	auto edict = entity->edict();

	entity->Destroy();

	if (auto factory = g_EntityDictionary.Find(className); factory)
	{
		factory->DestroyInstance(entity);
	}
	else
	{
		//Should only happen if somebody messed with the classname
		ALERT(at_error, "CEntityList::Destroy: no factory for \"%s\"\n", className);
	}

	if (edict)
	{
		edict->pvPrivateData = nullptr;
	}
	else
	{
		ALERT(at_error, "CEntityList::Destroy: NULL edict\n");
	}
}
