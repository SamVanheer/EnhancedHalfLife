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
#include <unordered_map>

class CBaseEntity;

/**
*	@brief Base class for entity factories
*/
class CBaseEntityFactory
{
protected:
	CBaseEntityFactory(const char* mapClassName, const char* internalClassName, const char* canonicalMapClassName = nullptr);

public:
	virtual ~CBaseEntityFactory();

	const char* GetMapClassName() const { return m_MapClassName; }

	const char* GetInternalClassName() const { return m_InternalClassName; }

	/**
	*	@brief The name that maps refer to this entity by. On construction the name will be changed to the actual classname
	*/
	const char* GetCanonicalMapClassName() const { return m_CanonicalMapClassName; }

	virtual CBaseEntity* CreateInstance() = 0;

	virtual void DestroyInstance(CBaseEntity* entity) = 0;

private:
	const char* const m_MapClassName;
	const char* const m_InternalClassName;
	const char* const m_CanonicalMapClassName;
};

/**
*	@brief Implementation of the factory interface
*	Handles creation and destruction of specific entity classes
*/
template<typename TEntity>
class CEntityFactory final : public CBaseEntityFactory
{
public:
	CEntityFactory(const char* mapClassName, const char* internalClassName, const char* canonicalMapClassName = nullptr)
		: CBaseEntityFactory(mapClassName, internalClassName, canonicalMapClassName)
	{
	}

	CBaseEntity* CreateInstance() override { return new TEntity(); }

	void DestroyInstance(CBaseEntity* entity) override
	{
		delete entity;
	}
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

	void Add(CBaseEntityFactory* factory);

	void Remove(CBaseEntityFactory* factory);

	CBaseEntityFactory* Find(const char* mapClassName) const;

	auto begin() const { return m_Entities.begin(); }

	auto end() const { return m_Entities.end(); }

private:
	std::unordered_map<std::string_view, CBaseEntityFactory*> m_Entities;
};

CEntityDictionary& GetEntityDictionary();
