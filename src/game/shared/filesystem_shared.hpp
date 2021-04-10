/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
#include <memory>
#include <tuple>

#include "FileSystem.h"

inline IFileSystem* g_pFileSystem = nullptr;

bool FileSystem_LoadFileSystem();
void FileSystem_FreeFileSystem();

std::tuple<std::unique_ptr<byte[]>, std::size_t> FileSystem_LoadFileIntoBuffer(const char* filename);
