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

#include <cassert>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <rttr/type.h>

#include "ISerializer.hpp"

namespace persistence
{
/**
*	@brief Registry of serializers for specific types
*/
class SerializerRegistry final
{
public:
	~SerializerRegistry() = default;

	static SerializerRegistry& GetInstance() { return _instance; }

	ISerializer* GetSerializer(rttr::type type) const
	{
		if (auto it = _typeSerializers.find(type.get_id()); it != _typeSerializers.end())
		{
			return it->second.get();
		}

		if (type.is_class() && type.get_base_classes().size() > 0)
		{
			//TODO: profile this
			for (const auto& candidate : _derivedClassSerializers)
			{
				//Use the passed in type for this
				if (std::get<0>(candidate).is_base_of(type))
				{
					return std::get<1>(candidate);
				}
			}
		}

		return nullptr;
	}

	template<typename TArithmetic>
	void AddArithmeticSerializer(std::unique_ptr<ISerializer>&& serializer)
	{
		auto type = rttr::type::get<TArithmetic>();

		assert(type);

		if (!type)
		{
			return;
		}

		assert(type.is_arithmetic());

		if (!type.is_arithmetic())
		{
			return;
		}

		AddSerializerCore(type, std::move(serializer), false);
	}

	/**
	*	@brief Adds a serializer for a class or struct
	*	@param useForDerivedClasses If true this serializer will be used to serialize and deserialize classes deriving from @p TClass
	*		(unless there is a serializer registered for a derived class)
	*/
	template<typename TClass>
	void AddClassSerializer(std::unique_ptr<ISerializer>&& serializer, bool useForDerivedClasses = false)
	{
		auto type = rttr::type::get<TClass>();

		assert(type);

		if (!type)
		{
			return;
		}

		const bool isAcceptableType = type.is_class() || (type.is_pointer() && type.get_raw_type().is_class());

		assert(isAcceptableType);

		if (!isAcceptableType)
		{
			return;
		}

		AddSerializerCore(type, std::move(serializer), useForDerivedClasses);
	}

private:
	SerializerRegistry() = default;

	void AddSerializerCore(const rttr::type& type, std::unique_ptr<ISerializer>&& serializer, bool useForDerivedClasses)
	{
		assert(serializer);

		if (!serializer)
		{
			return;
		}

		assert(!_typeSerializers.contains(type.get_id()));

		if (!_typeSerializers.contains(type.get_id()))
		{
			auto result = _typeSerializers.emplace(type.get_id(), std::move(serializer));

			if (useForDerivedClasses)
			{
				_derivedClassSerializers.emplace_back(type, result.first->second.get());
			}
		}
	}

private:
	static SerializerRegistry _instance;

	std::unordered_map<rttr::type::type_id, std::unique_ptr<ISerializer>> _typeSerializers;

	std::vector<std::tuple<rttr::type, ISerializer*>> _derivedClassSerializers;
};

inline SerializerRegistry SerializerRegistry::_instance{};
}
