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

#include "cbase.hpp"

#include "EntityDictionary.hpp"
#include "metadata/TypeReflection.hpp"

CEntityFactory::CEntityFactory(rttr::type type, const std::string& mapClassName, const std::string& internalClassName,
	const std::optional<std::string> aliasMapClassName)
	: m_type(type)
	, m_mapClassName(mapClassName)
	, m_internalClassName(internalClassName)
	, m_aliasMapClassName(aliasMapClassName.value_or(mapClassName))
{
	ASSERTSZ(!mapClassName.empty(), "Entity map classname must be valid");
	ASSERTSZ(!internalClassName.empty(), "Entity internal classname must be valid");
	ASSERTSZ(!aliasMapClassName.has_value() || !aliasMapClassName.value().empty(), "Alias entity map classname must be either null or valid");
}

CEntityFactory::~CEntityFactory() = default;

CBaseEntity* CEntityFactory::CreateInstance()
{
	auto instance = m_type.create();
	auto entity = instance.get_value<CBaseEntity*>();

	//Entity owns itself for its own lifetime
	entity->SetInstance(std::move(instance));

	return entity;
}

void CEntityFactory::DestroyInstance(CBaseEntity* entity)
{
	rttr::variant instance{entity->GetInstance()};

	//Clear the instance so the entity can be destroyed
	entity->SetInstance(rttr::variant{});

	m_type.destroy(instance);
}

CEntityFactory* CEntityDictionary::Find(const char* mapClassName) const
{
	if (!mapClassName)
	{
		return nullptr;
	}

	if (auto it = m_entities.find(mapClassName); it != m_entities.end())
	{
		return it->second.get();
	}

	return nullptr;
}

bool CEntityDictionary::RegisterEntityTypes()
{
	auto baseEntityType = rttr::type::get_by_name("CBaseEntity");

	if (!baseEntityType)
	{
		ALERT(at_error, "Could not find Reflection type for CBaseEntity\n");
		return false;
	}

	auto stringType = rttr::type::get<std::string>();

	if (!stringType)
	{
		ALERT(at_error, "Could not find Reflection type for std::string\n");
		return false;
	}

	const rttr::variant nameKey{EntityNameKey};
	const rttr::variant aliasesKey{EntityNameAliasesKey};

	//Create a copy of the types list
	//This is necessary because create_sequential_view internally creates new types, which invalidates the iterators from the returned list
	const std::vector<rttr::type> types
	{
		[]()
		{
			const auto types = rttr::type::get_types();
			return std::vector<rttr::type>{types.begin(), types.end()};
		}()
	};

	for (const rttr::type& type : types)
	{
		//Only consider entity classes
		if (type.is_derived_from(baseEntityType))
		{
			auto entityName = type.get_metadata(nameKey);

			if (entityName)
			{
				const std::string canonicalName{entityName.to_string()};
				const std::string internalName{type.get_name()};

				Add(std::make_unique<CEntityFactory>(type, canonicalName, internalName));

				const auto aliases = type.get_metadata(aliasesKey);

				if (aliases)
				{
					if (!aliases.is_sequential_container())
					{
						ALERT(at_error, "Entity name aliases for type \"%s\" (%s) is not a list of names\n",
							internalName.c_str(), canonicalName.c_str());
					}
					else
					{
						auto view = aliases.create_sequential_view();

						if (view.get_value_type() != stringType)
						{
							ALERT(at_error, "Entity name aliases for type \"%s\" (%s) is not a list of strings\n",
								internalName.c_str(), canonicalName.c_str());
						}
						else
						{
							for (auto alias : view)
							{
								Add(std::make_unique<CEntityFactory>(type, canonicalName, internalName, alias.to_string()));
							}
						}
					}
				}
			}
		}
	}

	CreateSortedEntitiesList();

	return true;
}

void CEntityDictionary::Add(std::unique_ptr<CEntityFactory>&& factory)
{
	if (!factory)
	{
		ALERT(at_error, "CEntityDictionary::Add: tried to add NULL entity factory\n");
		return;
	}

	const std::string_view mapClassName{factory->GetAliasMapClassName()};

	if (auto it = m_entities.find(mapClassName); it != m_entities.end())
	{
		ALERT(at_error, "CEntityDictionary::Add: an entity class with name \"%s\" (%s) has already been added\n",
			it->second->GetMapClassName(), it->second->GetInternalClassName());
		return;
	}

	m_entities.emplace(mapClassName, std::move(factory));
}

void CEntityDictionary::Remove(CEntityFactory* factory)
{
	if (!factory)
	{
		ALERT(at_error, "CEntityDictionary::Remove: tried to remove NULL entity factory\n");
		return;
	}

	m_entities.erase(factory->GetAliasMapClassName());
}

void CEntityDictionary::CreateSortedEntitiesList()
{
	m_sortedEntities.clear();

	m_sortedEntities.reserve(m_entities.size());

	std::transform(m_entities.begin(), m_entities.end(), std::back_inserter(m_sortedEntities), [](const auto& factory)
		{
			return factory.second.get();
		});

	std::sort(m_sortedEntities.begin(), m_sortedEntities.end(), [](const auto& lhs, const auto& rhs)
		{
			return lhs->GetAliasMapClassName().compare(rhs->GetAliasMapClassName()) < 0;
		});
}
