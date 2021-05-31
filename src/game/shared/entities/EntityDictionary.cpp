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

CBaseEntityFactory::CBaseEntityFactory(const char* mapClassName, const char* internalClassName, const char* canonicalMapClassName)
	: m_MapClassName(mapClassName)
	, m_InternalClassName(internalClassName)
	, m_CanonicalMapClassName(canonicalMapClassName ? canonicalMapClassName : mapClassName)
{
	ASSERTSZ(mapClassName && *mapClassName, "Entity map classname must be valid");
	ASSERTSZ(internalClassName && *internalClassName, "Entity internal classname must be valid");
	ASSERTSZ(!canonicalMapClassName || *canonicalMapClassName, "Canoonical entity map classname must be either null or valid");

	GetEntityDictionary().Add(this);
}

CBaseEntityFactory::~CBaseEntityFactory()
{
	GetEntityDictionary().Remove(this);
}

void CEntityDictionary::Add(CBaseEntityFactory* factory)
{
	if (!factory)
	{
		ALERT(at_error, "CEntityDictionary::Add: tried to add NULL entity factory\n");
		return;
	}

	const std::string_view mapClassName{factory->GetCanonicalMapClassName()};

	if (auto it = m_Entities.find(mapClassName); it != m_Entities.end())
	{
		ALERT(at_error, "CEntityDictionary::Add: an entity class with name \"%s\" (%s) has already been added\n",
			it->second->GetMapClassName(), it->second->GetInternalClassName());
		return;
	}

	m_Entities.emplace(mapClassName, factory);
}

void CEntityDictionary::Remove(CBaseEntityFactory* factory)
{
	if (!factory)
	{
		ALERT(at_error, "CEntityDictionary::Remove: tried to remove NULL entity factory\n");
		return;
	}

	m_Entities.erase(factory->GetCanonicalMapClassName());
}

CBaseEntityFactory* CEntityDictionary::Find(const char* mapClassName) const
{
	if (!mapClassName)
	{
		return nullptr;
	}

	if (auto it = m_Entities.find(mapClassName); it != m_Entities.end())
	{
		return it->second;
	}

	return nullptr;
}

CEntityDictionary& GetEntityDictionary()
{
	//Ensures initialization on first use
	//TODO: rework entity registration so this isn't needed anymore
	static CEntityDictionary entityDictionary;

	return entityDictionary;
}
