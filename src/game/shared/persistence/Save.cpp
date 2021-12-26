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
#include "ISerializer.hpp"
#include "Save.hpp"
#include "SerializerRegistry.hpp"
#include "ValueVisitor.hpp"
#include "metadata/MetaDataUtils.hpp"
#include "metadata/TypeReflection.hpp"

namespace persistence
{
struct SaveValueVisitor : public persistence::ValueVisitor<SaveValueVisitor>
{
	friend struct persistence::ValueVisitor<SaveValueVisitor>;

	SaveValueVisitor(const rttr::property& property, Save& save)
		: _property(property)
		, _save(save)
	{
	}

protected:
	void DefaultVisit(rttr::variant& value)
	{
		const auto declaringTypeName = RemoveTypePrefix(ToStringView(_property.get_declaring_type().get_name()));
		const auto propertyName = ToStringView(_property.get_name());
		const auto typeName = RemoveTypePrefix(ToStringView(_property.get_type().get_name()));

		ALERT(at_console, "SaveRestore: Property \"%*s::%*s\" has unsupported type \"%*s\"\n",
			static_cast<int>(declaringTypeName.length()), declaringTypeName.data(),
			static_cast<int>(propertyName.length()), propertyName.data(),
			static_cast<int>(typeName.length()), typeName.data());
	}

	void VisitSequentialContainer(rttr::variant& value)
	{
		auto view = value.create_sequential_view();

		for (std::size_t i = 0; i < view.get_size(); ++i)
		{
			auto viewElement = view.get_value(i);

			VisitValid(viewElement);

			//TODO:
			/*
			if constexpr (IsWrite)
			{
				view.set_value(viewElement);
			}
			*/
		}
	}

	void VisitArithmetic(rttr::variant& value)
	{
		ProcessValue(_property.get_type(), value);
	}

	void VisitEnumeration(rttr::variant& value)
	{
		auto type = _property.get_type();

		auto enumeration = type.get_enumeration();

		//TODO: need to serialize enums in a way that uses value names, not actual values
		type = enumeration.get_underlying_type();

		//TODO: register enums so they can be used here
		//ProcessValue(type, value);

		auto raw = value.to_uint32();

		_save.BufferData(reinterpret_cast<const std::byte*>(&raw), sizeof(raw));
	}

	void VisitClass(rttr::variant& value)
	{
		ProcessValue(_property.get_type(), value);
	}

	void VisitPointer(rttr::variant& value)
	{
		//See if we have a pointer serializer registered for this type
		ProcessValue(_property.get_type(), value);
	}

private:
	void ProcessValue(const rttr::type& type, rttr::variant& value)
	{
		if (auto serializer = SerializerRegistry::GetInstance().GetSerializer(type); serializer)
		{
			serializer->Serialize(_property, value, _save);
		}
	}

private:
	const rttr::property _property;
	Save& _save;
};

bool Save::DataEmpty(const std::byte* data, std::size_t size) const
{
	for (std::size_t i = 0; i < size; ++i)
	{
		if (static_cast<unsigned int>(data[i]) != 0)
		{
			return false;
		}
	}

	return true;
}

void Save::BufferData(const std::byte* data, std::size_t size)
{
	if (!m_data)
	{
		return;
	}

	if (static_cast<std::size_t>(m_data->size) + size > static_cast<std::size_t>(m_data->bufferSize))
	{
		ALERT(at_error, "Save/Restore overflow!");
		m_data->size = m_data->bufferSize;
		return;
	}

	memcpy(m_data->pCurrentData, data, size);
	m_data->pCurrentData += size;
	m_data->size += size;
}

void Save::BufferHeader(const char* name, std::size_t size)
{
	const auto hashvalue = TokenHash(name);

	if (size > (1 << (sizeof(short) * 8)))
	{
		ALERT(at_error, "Save::BufferHeader() size parameter exceeds 'short'!");
	}

	const auto sizeData = static_cast<unsigned short>(size);

	BufferData(reinterpret_cast<const std::byte*>(&sizeData), sizeof(unsigned short));
	BufferData(reinterpret_cast<const std::byte*>(&hashvalue), sizeof(unsigned short));
}

void Save::BufferField(const char* name, std::size_t size, const std::byte* data)
{
	BufferHeader(name, size);
	BufferData(data, size);
}

void Save::WriteFields(const rttr::instance& instance, const char* className)
{
	if (!instance)
	{
		return;
	}

	auto type = instance.get_type();

	//For object instances this will be a shared_ptr wrapper, so we have to get the underlying type
	//The type could be a pointer to a shared_ptr, so get the raw type first
	type = type.get_raw_type();

	if (type.is_wrapper())
	{
		type = type.get_wrapped_type();
	}

	//Could be a pointer type, so get the actual type
	type = type.get_raw_type();

	const std::size_t nameLength = std::strlen(className) + 1;

	BufferData(reinterpret_cast<const std::byte*>(className), nameLength);

	WriteFieldsCore(instance, type);
}

void Save::WriteFieldsCore(const rttr::instance& instance, const rttr::type& type)
{
	/*
	//Write base class first, then derived
	if (auto bases = type.get_base_classes(); bases.size() > 0)
	{
		WriteFieldsCore(instance, *bases.cbegin());
	}
	*/

	for (const auto& property : type.get_properties())
	{
		if (auto persistedMetaData = property.get_metadata(PersistedKey); persistedMetaData && persistedMetaData.to_bool())
		{
			SaveValueVisitor visitor{property, *this};

			auto value = property.get_value(instance);

			const auto position = m_data->size;
			visitor.Visit(value);
			const auto written = m_data->size - position;

			const auto declaringTypeName = RemoveTypePrefix(ToStringView(property.get_declaring_type().get_name()));
			const auto propertyName = ToStringView(property.get_name());
			const auto typeName = RemoveTypePrefix(ToStringView(property.get_type().get_name()));

			ALERT(at_console, "SaveRestore: Property \"%*s::%*s\" advanced %d bytes\n",
				static_cast<int>(declaringTypeName.length()), declaringTypeName.data(),
				static_cast<int>(propertyName.length()), propertyName.data(),
				written);
		}
	}
}
}
