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

#include <cstddef>

#include "SaveRestoreBuffer.hpp"

namespace persistence
{
struct ISerializer;

class Save : public SaveRestoreBuffer
{
public:
	using SaveRestoreBuffer::SaveRestoreBuffer;

	bool DataEmpty(const std::byte* data, std::size_t size) const;

	void BufferData(const std::byte* data, std::size_t size);
	void BufferHeader(const char* name, std::size_t size);
	void BufferField(const char* name, std::size_t size, const std::byte* data);

	void WriteFields(const rttr::instance& instance, const char* className);

private:
	void WriteFieldsCore(const rttr::instance& instance, const rttr::type& type);
};
}
