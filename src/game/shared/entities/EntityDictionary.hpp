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

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <rttr/type.h>

class CBaseEntity;

/**
*	@brief Handles creation and destruction of specific entity classes, maps aliases to canonical names
*/
class CEntityFactory
{
public:
	CEntityFactory(rttr::type type, const std::string& mapClassName, const std::string& internalClassName, const std::optional<std::string> aliasMapClassName = {});
	~CEntityFactory();

	const std::string& GetMapClassName() const { return m_mapClassName; }

	const std::string& GetInternalClassName() const { return m_internalClassName; }

	/**
	*	@brief The name that maps refer to this entity by. On construction the name will be changed to the actual classname
	*/
	const std::string& GetAliasMapClassName() const { return m_aliasMapClassName; }

	CBaseEntity* CreateInstance();

	void DestroyInstance(CBaseEntity* entity);

private:
	rttr::type m_type;
	const std::string m_mapClassName;
	const std::string m_internalClassName;
	const std::string m_aliasMapClassName;
};

/**
*	@brief Dictionary of all entity classes
*/
class CEntityDictionary final
{
public:
	CEntityDictionary() = default;
	~CEntityDictionary() = default;

	CEntityDictionary(const CEntityDictionary&) = delete;
	CEntityDictionary& operator=(const CEntityDictionary&) = delete;

	const std::vector<CEntityFactory*>& GetSortedEntityFactories() const { return m_sortedEntities; }

	std::size_t GetClassCount() const { return m_sortedEntities.size(); }

	CEntityFactory* Find(const char* mapClassName) const;

	auto begin() const { return m_sortedEntities.begin(); }

	auto end() const { return m_sortedEntities.end(); }

	bool RegisterEntityTypes();

private:
	void Add(std::unique_ptr<CEntityFactory>&& factory);

	void Remove(CEntityFactory* factory);

	void Clear()
	{
		m_entities.clear();
		m_sortedEntities.clear();
	}

	void CreateSortedEntitiesList();

private:
	std::unordered_map<std::string_view, std::unique_ptr<CEntityFactory>> m_entities;

	std::vector<CEntityFactory*> m_sortedEntities;
};

inline CEntityDictionary g_EntityDictionary;
