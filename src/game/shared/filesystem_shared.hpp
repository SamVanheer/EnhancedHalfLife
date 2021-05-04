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
#include <filesystem>
#include <memory>
#include <tuple>

#include "FileSystem.h"

inline IFileSystem* g_pFileSystem = nullptr;

std::filesystem::path FileSystem_GetGameDirectory();

bool FileSystem_LoadFileSystem();
void FileSystem_FreeFileSystem();

std::tuple<std::unique_ptr<byte[]>, std::size_t> FileSystem_LoadFileIntoBuffer(const char* filename);

class FSFile
{
public:
	FSFile() = default;
	FSFile(const char* filename, const char* options, const char* pathID = nullptr);
	~FSFile();

	constexpr bool IsOpen() const { return _handle != FILESYSTEM_INVALID_HANDLE; }

	bool Open(const char* filename, const char* options, const char* pathID = nullptr);
	void Close();

	void Seek(int pos, FileSystemSeek seekType);

	int Read(void* dest, int size);

	int Write(const void* input, int size);

	template<typename... Args>
	int Printf(const char* format, Args&&... args)
	{
		return g_pFileSystem->FPrintf(_handle, format, std::forward<Args>(args)...);
	}

	constexpr operator bool() const { return IsOpen(); }

private:
	FileHandle_t _handle = FILESYSTEM_INVALID_HANDLE;
};

inline FSFile::FSFile(const char* filename, const char* options, const char* pathID)
{
	Open(filename, options, pathID);
}

inline FSFile::~FSFile()
{
	Close();
}

inline bool FSFile::Open(const char* filename, const char* options, const char* pathID)
{
	Close();

	_handle = g_pFileSystem->Open(filename, options, pathID);

	return IsOpen();
}

inline void FSFile::Close()
{
	if (IsOpen())
	{
		g_pFileSystem->Close(_handle);
		_handle = FILESYSTEM_INVALID_HANDLE;
	}
}

inline void FSFile::Seek(int pos, FileSystemSeek seekType)
{
	if (IsOpen())
	{
		g_pFileSystem->Seek(_handle, pos, seekType);
	}
}

inline int FSFile::Read(void* dest, int size)
{
	return g_pFileSystem->Read(dest, size, _handle);
}

inline int FSFile::Write(const void* input, int size)
{
	return g_pFileSystem->Write(input, size, _handle);
}
