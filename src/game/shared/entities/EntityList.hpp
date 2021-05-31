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

#include "ehandle.hpp"

class CBaseEntity;
class CBaseEntityFactory;

/**
*	@brief Manages the list of entities
*/
class CEntityList
{
public:
	CEntityList() = default;
	CEntityList(edict_t* entities);

	~CEntityList() = default;

	CEntityList(const CEntityList&) = delete;
	CEntityList& operator=(const CEntityList&) = delete;

	CEntityList(CEntityList&&) = default;
	CEntityList& operator=(CEntityList&&) = default;

	int GetEntityLimit() const;

	EHandle<CBaseEntity> GetEntityByIndex(int index) const;

	/**
	*	@brief Gets the first valid entity in the list
	*/
	EHandle<CBaseEntity> GetFirstEntity() const;

	/**
	*	@brief Gets the next valid entity in the list after the given entity
	*/
	EHandle<CBaseEntity> GetNextEntity(const EHandle<CBaseEntity>& startEntity) const;

	/**
	*	@brief Creates an entity by classname
	*/
	CBaseEntity* Create(const char* className);

	/**
	*	@brief Creates an entity by classname, and assigns the given edict to it
	*/
	CBaseEntity* Create(const char* className, edict_t* edict);

	/**
	*	@brief Destroys the entity. The associated edict is not freed
	*/
	void Destroy(CBaseEntity* entity);

private:
	EHandle<CBaseEntity> InternalGetEntityByIndex(int index) const;

	CBaseEntityFactory* GetFactory(const char* className);

	CBaseEntity* ConstructEntity(CBaseEntityFactory* factory, edict_t* edict);

private:
	edict_t* m_Entities = nullptr;
};

inline CEntityList g_EntityList;
