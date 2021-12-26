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

/**
*	@file
*
*	Serializers for primitive and commonly used types
*
*	Serialization is handled by 3 types of structs:
*	@li Binary serializers: these handle the serialization to and from binary buffers
*	@li Scalar and sequential serializers: these handle the serialization of scalar and sequential types
*	@li Serializer factories: these handle the creation of serializers for a certain type, and can select one based on property-specific information
*/

#include <cassert>
#include <cstring>
#include <memory>

#include <rttr/instance.h>
#include <rttr/property.h>
#include <rttr/type.h>
#include <rttr/variant.h>

#include "cbase.hpp"
#include "ISerializer.hpp"
#include "Platform.hpp"
#include "Restore.hpp"
#include "Save.hpp"
#include "shared_utils.hpp"
#include "vector.hpp"

#include "metadata/TypeReflection.hpp"

namespace persistence
{
namespace detail
{
struct BinarySerializerHelpers
{
	static FieldType GetFieldType(const rttr::property& property)
	{
		if (const auto type = property.get_metadata(TypeKey); type)
		{
			assert(type.is_type<FieldType>());

			bool ok = false;
			const auto typeValue = type.convert<FieldType>(&ok);

			assert(ok && "Field type should be an enum value of type FieldType");

			if (ok)
			{
				return typeValue;
			}
		}

		return FieldType::Default;
	}

	static void AssertFieldTypeIsDefault(const rttr::property& property)
	{
		assert(GetFieldType(property) == FieldType::Default);
	}

	template<typename T>
	static void SerializeValue(const T& value, Save& save)
	{
		save.BufferData(reinterpret_cast<const std::byte*>(&value), sizeof(T));
	}

	template<typename T>
	static void DeserializeValue(T& value, Restore& restore)
	{
		restore.BufferReadBytes(reinterpret_cast<std::byte*>(&value), sizeof(T));
	}
};
}

/**
*	@brief Template class that handles the serialization of types using bitwise serialization
*/
template<typename T>
struct ArithmeticBinarySerializer
{
	static void Serialize(const rttr::property& property, const T& data, Save& save)
	{
		detail::BinarySerializerHelpers::AssertFieldTypeIsDefault(property);
		detail::BinarySerializerHelpers::SerializeValue(data, save);
	}

	static void Deserialize(const rttr::property& property, T& data, Restore& restore)
	{
		detail::BinarySerializerHelpers::AssertFieldTypeIsDefault(property);
		detail::BinarySerializerHelpers::DeserializeValue(data, restore);
	}
};

/**
*	@brief Specialization to serialize boolean values as bytes
*/
template<>
struct ArithmeticBinarySerializer<bool>
{
	static void Serialize(const rttr::property& property, const bool& data, Save& save)
	{
		ArithmeticBinarySerializer<std::uint8_t>::Serialize(property, data ? 1 : 0, save);
	}

	static void Deserialize(const rttr::property& property, bool& data, Restore& restore)
	{
		std::uint8_t byteValue{};

		ArithmeticBinarySerializer<std::uint8_t>::Deserialize(property, byteValue, restore);

		data = byteValue != 0;
	}
};

/**
*	@brief Specialization to handle time values
*/
template<>
struct ArithmeticBinarySerializer<float>
{
	// Always encode time as a delta from the current time so it can be re-based if loaded in a new level
	// Times of 0 are never written to the file, so they will be restored as 0, not a relative time

	static void Serialize(const rttr::property& property, const float& data, Save& save)
	{
		const auto type = detail::BinarySerializerHelpers::GetFieldType(property);

		float value = data;

		switch (type)
		{
		case FieldType::Default: break;
		case FieldType::Time: value -= save.GetTimeOffset(); break;
		default: assert(false && "Invalid field type for float"); break;
		}

		return detail::BinarySerializerHelpers::SerializeValue(value, save);
	}

	static void Deserialize(const rttr::property& property, float& data, Restore& restore)
	{
		detail::BinarySerializerHelpers::DeserializeValue(data, restore);

		const auto type = detail::BinarySerializerHelpers::GetFieldType(property);

		switch (type)
		{
		case FieldType::Default: break;
		case FieldType::Time: data += restore.GetTimeOffset(); break;
		default: assert(false && "Invalid field type for float"); break;
		}
	}
};

/**
*	@brief Serializes vectors as absolute coordinates/directions or as positions relative to the landmark
*/
struct VectorBinarySerializer
{
	static void Serialize(const rttr::property& property, const Vector& data, Save& save)
	{
		const auto type = detail::BinarySerializerHelpers::GetFieldType(property);

		Vector value = data;

		switch (type)
		{
		case FieldType::Default: break;
		case FieldType::Position: value = value - save.GetLandmarkOffset(); break;
		default: assert(false && "Invalid field type for Vector"); break;
		}

		detail::BinarySerializerHelpers::SerializeValue(value, save);
	}

	static void Deserialize(const rttr::property& property, Vector& data, Restore& restore)
	{
		detail::BinarySerializerHelpers::DeserializeValue(data, restore);

		const auto type = detail::BinarySerializerHelpers::GetFieldType(property);

		switch (type)
		{
		case FieldType::Default: break;
		case FieldType::Position: data = data + restore.GetLandmarkOffset(); break;
		default: assert(false && "Invalid field type for Vector"); break;
		}
	}
};

/**
*	@brief Serializes string offsets as null-terminated strings, precaches models and sounds on load if required
*/
struct StringOffsetBinarySerializer
{
	static void Serialize(const rttr::property& property, const string_t& data, Save& save)
	{
		const auto type = detail::BinarySerializerHelpers::GetFieldType(property);

		switch (type)
		{
		case FieldType::Default:
			[[fallthrough]];

		case FieldType::ModelName:
			[[fallthrough]];

		case FieldType::SoundName: break;

		default: assert(false && "Invalid field type for string_t"); break;
		}

		const char* stringValue = STRING(data);

		if (!IsStringNull(data))
		{
			if (!strcmp(stringValue, "ad"))
			{
				int x = 10;
			}
		}

		save.BufferData(reinterpret_cast<const std::byte*>(stringValue), std::strlen(stringValue) + 1);
	}

	static void Deserialize(const rttr::property& property, string_t& data, Restore& restore)
	{
		auto stringValue = reinterpret_cast<const char*>(restore.GetCurrentData());

		const std::size_t length = strlen(stringValue);

		restore.BufferReadBytes(nullptr, length + 1);

		if (length > 0)
		{
			data = ALLOC_STRING(stringValue);
		}
		else
		{
			data = string_t::Null;
		}

		const auto type = detail::BinarySerializerHelpers::GetFieldType(property);

		switch (type)
		{
		case FieldType::Default: break;

		case FieldType::ModelName:
		{
			if (restore.ShouldPrecache())
			{
				if (!IsStringNull(data))
				{
					if (!strcmp(stringValue, ""))
					{
						int x = 10;
					}
					PRECACHE_MODEL(STRING(data));
				}
			}
			break;
		}

		case FieldType::SoundName:
		{
			if (restore.ShouldPrecache())
			{
				if (!IsStringNull(data))
				{
					PRECACHE_SOUND(STRING(data));
				}
			}
			break;
		}

		default: assert(false && "Invalid field type for string_t"); break;
		}
	}
};

struct EHandleBinarySerializer
{
	static void Serialize(const rttr::property& property, const BaseHandle& data, Save& save)
	{
		detail::BinarySerializerHelpers::AssertFieldTypeIsDefault(property);

		auto entity = data.UnsafeGet();
		const int index = save.EntityIndex(entity);
		detail::BinarySerializerHelpers::SerializeValue(index, save);
	}

	static void Deserialize(const rttr::property& property, BaseHandle& data, Restore& restore)
	{
		detail::BinarySerializerHelpers::AssertFieldTypeIsDefault(property);

		int index{};

		detail::BinarySerializerHelpers::DeserializeValue(index, restore);

		data.UnsafeSet(CBaseEntity::InstanceOrNull(restore.EntityFromIndex(index)));
	}
};

struct EdictBinarySerializer
{
	static void Serialize(const rttr::property& property, const edict_t* const& data, Save& save)
	{
		detail::BinarySerializerHelpers::AssertFieldTypeIsDefault(property);

		const int index = save.EntityIndex(data);
		detail::BinarySerializerHelpers::SerializeValue(index, save);
	}

	static void Deserialize(const rttr::property& property, edict_t*& data, Restore& restore)
	{
		detail::BinarySerializerHelpers::AssertFieldTypeIsDefault(property);

		int index{};

		detail::BinarySerializerHelpers::DeserializeValue(index, restore);

		data = restore.EntityFromIndex(index);
	}
};

/**
*	@brief Serializes single variables of the given type
*/
template<typename T, typename TSerializer>
struct ScalarVariableSerializer : public ISerializer
{
	void Serialize(const rttr::property& property, const rttr::variant& value, Save& save) const override
	{
		TSerializer::Serialize(property, value.get_value<T>(), save);
	}

	void Deserialize(const rttr::property& property, rttr::variant& value, Restore& restore) const override
	{
		T primitive{};

		TSerializer::Deserialize(property, primitive, restore);

		value = primitive;
	}
};

/**
*	@brief Serializes entvars pointers
*/
struct EntvarsSerializer : public ISerializer
{
	void Serialize(const rttr::property& property, const rttr::variant& value, Save& save) const override
	{
		save.WriteFields(value, "ENTVARS");
	}

	void Deserialize(const rttr::property& property, rttr::variant& value, Restore& restore) const override
	{
		restore.ReadFields(value);
	}
};

void RegisterSerializers();
}
