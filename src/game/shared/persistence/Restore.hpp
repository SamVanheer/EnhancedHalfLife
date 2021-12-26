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

#include <rttr/instance.h>
#include <rttr/property.h>
#include <rttr/type.h>

#include "SaveRestoreBuffer.hpp"

namespace persistence
{
struct ISerializer;

class Restore : public SaveRestoreBuffer
{
public:
	using SaveRestoreBuffer::SaveRestoreBuffer;

	bool ShouldPrecache() const { return _precache; }

	void SetPrecacheMode(bool precache)
	{
		_precache = precache;
	}

	bool ShouldRestoreGlobalFields() const { return _restoreGlobalFields; }

	void SetRestoreGlobalFields(bool value)
	{
		_restoreGlobalFields = value;
	}

	std::byte* GetCurrentData() const { return reinterpret_cast<std::byte*>(m_data->pCurrentData); }

	bool IsEmpty() const { return (m_data == nullptr) || ((m_data->pCurrentData - m_data->pBaseData) >= m_data->bufferSize); }

	void BufferReadBytes(std::byte* output, std::size_t size);

	std::string_view ReadClassName();

	void ReadFields(const rttr::instance& instance);

private:
	void ReadFieldsCore(const rttr::instance& instance, const rttr::type& type);

private:
	bool _precache = true;
	bool _restoreGlobalFields = true;
};
}
