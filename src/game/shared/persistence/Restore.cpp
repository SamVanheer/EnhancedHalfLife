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
#include "Restore.hpp"
#include "SerializerRegistry.hpp"
#include "ValueVisitor.hpp"
#include "metadata/MetaDataUtils.hpp"
#include "metadata/TypeReflection.hpp"

namespace persistence
{
struct RestoreValueVisitor : public persistence::ValueVisitor<RestoreValueVisitor>
{
	friend struct persistence::ValueVisitor<RestoreValueVisitor>;

	RestoreValueVisitor(const rttr::property& property, Restore& restore)
		: _property(property)
		, _restore(restore)
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

			view.set_value(i, viewElement);
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

		//ProcessValue(type, value);

		std::uint32_t raw{};

		_restore.BufferReadBytes(reinterpret_cast<std::byte*>(&raw), sizeof(raw));

		value = raw;
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
			serializer->Deserialize(_property, value, _restore);
		}
	}

private:
	const rttr::property _property;
	Restore& _restore;
};

void Restore::BufferReadBytes(std::byte* output, std::size_t size)
{
	ASSERT(m_data != nullptr);

	if (!m_data || IsEmpty())
	{
		return;
	}

	if ((static_cast<std::size_t>(m_data->size) + size) > static_cast<std::size_t>(m_data->bufferSize))
	{
		ALERT(at_error, "Restore overflow!");
		m_data->size = m_data->bufferSize;
		return;
	}

	if (output)
	{
		std::memcpy(output, m_data->pCurrentData, size);
	}

	m_data->pCurrentData += size;
	m_data->size += size;
}

std::string_view Restore::ReadClassName()
{
	const char* name = reinterpret_cast<const char*>(m_data->pCurrentData);

	const std::size_t nameLength = std::strlen(name);

	BufferReadBytes(nullptr, nameLength + 1);

	return {name, nameLength};
}

void Restore::ReadFields(const rttr::instance& instance)
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

	//Skip this, already done
	ReadClassName();

	ReadFieldsCore(instance, type);
}

void Restore::ReadFieldsCore(const rttr::instance& instance, const rttr::type& type)
{
	/*
	//Write base class first, then derived
	if (auto bases = type.get_base_classes(); bases.size() > 0)
	{
		ReadFieldsCore(instance, *bases.cbegin());
	}
	*/

	for (const auto& property : type.get_properties())
	{
		if (auto persistedMetaData = property.get_metadata(PersistedKey); persistedMetaData && persistedMetaData.to_bool())
		{
			const auto isGlobal = property.get_metadata(IsGlobalKey);

			if (ShouldRestoreGlobalFields() || !(isGlobal && isGlobal.to_bool()))
			{
				RestoreValueVisitor visitor{property, *this};

				auto value = property.get_value(instance);

				const auto position = m_data->size;
				visitor.Visit(value);
				const auto read = m_data->size - position;

				if (value.convert(property.get_type()))
				{
					property.set_value(instance, value);
				}
				else
				{
					//TODO
				}

				const auto declaringTypeName = RemoveTypePrefix(ToStringView(property.get_declaring_type().get_name()));
				const auto propertyName = ToStringView(property.get_name());
				const auto typeName = RemoveTypePrefix(ToStringView(property.get_type().get_name()));

				ALERT(at_console, "SaveRestore: Property \"%*s::%*s\" advanced %d bytes\n",
					static_cast<int>(declaringTypeName.length()), declaringTypeName.data(),
					static_cast<int>(propertyName.length()), propertyName.data(),
					read);
			}
		}
	}
}
}
