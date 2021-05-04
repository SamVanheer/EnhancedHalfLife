//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include <string_view>

#include "interface.h"

typedef void* FileHandle_t;
typedef int FileFindHandle_t;
typedef int WaitForResourcesHandle_t;

#ifndef FILESYSTEM_INTERNAL_H
enum class FileSystemSeek
{
	Head = 0,
	Current,
	Tail
};

enum
{
	FILESYSTEM_INVALID_FIND_HANDLE = -1
};

enum FileWarningLevel_t
{
	/**
	*	@brief Don't print anything
	*/
	FILESYSTEM_WARNING_QUIET = 0,

	/**
	*	@brief On shutdown, report names of files left unclosed
	*/
	FILESYSTEM_WARNING_REPORTUNCLOSED,

	/**
	*	@brief Report number of times a file was opened, closed
	*/
	FILESYSTEM_WARNING_REPORTUSAGE,

	/**
	*	@brief Report all open/close events to console ( !slow! )
	*/
	FILESYSTEM_WARNING_REPORTALLACCESSES
};

constexpr FileHandle_t FILESYSTEM_INVALID_HANDLE = static_cast<FileHandle_t>(0);
#endif

// turn off any windows defines
#undef GetCurrentDirectory

/**
*	@brief Main file system interface
*/
class IFileSystem : public IBaseInterface
{
public:
	// Mount and unmount the filesystem
	virtual void			Mount() = 0;
	virtual void			Unmount() = 0;

	/**
	*	@brief Remove all search paths (including write path?)
	*/
	virtual void			RemoveAllSearchPaths() = 0;

	/**
	*	@brief Add paths in priority order (mod dir, game dir, ....)
	*	@details If one or more .pak files are in the specified directory,
	*	then they are added after the file system path
	*	If the path is the relative path to a .bsp file,
	*	then any previous .bsp file override is cleared and the current .bsp is searched for an embedded PAK file
	*	 and this file becomes the highest priority search path
	*	( i.e., it's looked at first even before the mod's file system path ).
	*/
	virtual void			AddSearchPath(const char* pPath, const char* pathID) = 0;
	virtual bool			RemoveSearchPath(const char* pPath) = 0;

	virtual void			RemoveFile(const char* pRelativePath, const char* pathID) = 0;

	virtual void			CreateDirHierarchy(const char* path, const char* pathID) = 0;

	// File I/O and info
	virtual bool			FileExists(const char* pFileName) = 0;
	virtual bool			IsDirectory(const char* pFileName) = 0;

	/**
	*	@brief opens a file
	*	@param pathID if nullptr, all paths will be searched for the file
	*/
	virtual FileHandle_t	Open(const char* pFileName, const char* pOptions, const char* pathID = 0L) = 0;

	virtual void			Close(FileHandle_t file) = 0;

	virtual void			Seek(FileHandle_t file, int pos, FileSystemSeek seekType) = 0;
	virtual unsigned int	Tell(FileHandle_t file) = 0;

	virtual unsigned int	Size(FileHandle_t file) = 0;
	virtual unsigned int	Size(const char* pFileName) = 0;

	virtual long			GetFileTime(const char* pFileName) = 0;
	virtual void			FileTimeToString(char* pStrip, int maxCharsIncludingTerminator, long fileTime) = 0;

	virtual bool			IsOk(FileHandle_t file) = 0;

	virtual void			Flush(FileHandle_t file) = 0;
	virtual bool			EndOfFile(FileHandle_t file) = 0;

	virtual int				Read(void* pOutput, int size, FileHandle_t file) = 0;
	virtual int				Write(void const* pInput, int size, FileHandle_t file) = 0;
	virtual char* ReadLine(char* pOutput, int maxChars, FileHandle_t file) = 0;
	virtual int				FPrintf(FileHandle_t file, const char* pFormat, ...) = 0;

	// Read buffers are obsolete and don't work
	virtual void* GetReadBuffer(FileHandle_t file, int* outBufferSize, bool failIfNotInCache) = 0;
	virtual void            ReleaseReadBuffer(FileHandle_t file, void* readBuffer) = 0;

	// FindFirst/FindNext
	virtual const char* FindFirst(const char* pWildCard, FileFindHandle_t* pHandle, const char* pathID = 0L) = 0;
	virtual const char* FindNext(FileFindHandle_t handle) = 0;
	virtual bool			FindIsDirectory(FileFindHandle_t handle) = 0;
	virtual void			FindClose(FileFindHandle_t handle) = 0;

	virtual void			GetLocalCopy(const char* pFileName) = 0;

	virtual const char* GetLocalPath(const char* pFileName, char* pLocalPath, int localPathBufferSize) = 0;

	/**
	*	@brief Note: This is sort of a secondary feature; but it's really useful to have it here
	*/
	virtual char* ParseFile(char* pFileBytes, char* pToken, bool* pWasQuoted) = 0;

	/**
	*	@brief Returns true on success ( based on current list of search paths, otherwise false if it can't be resolved )
	*/
	virtual bool			FullPathToRelativePath(const char* pFullpath, char* pRelative) = 0;

	/**
	*	@brief Gets the current working directory
	*/
	virtual bool			GetCurrentDirectory(char* pDirectory, int maxlen) = 0;

	/**
	*	@brief Dump to printf/OutputDebugString the list of files that have not been closed
	*/
	virtual void			PrintOpenedFiles() = 0;

	virtual void			SetWarningFunc(void (*pfnWarning)(const char* fmt, ...)) = 0;
	virtual void			SetWarningLevel(FileWarningLevel_t level) = 0;

	virtual void			LogLevelLoadStarted(const char* name) = 0;
	virtual void			LogLevelLoadFinished(const char* name) = 0;
	virtual int				HintResourceNeed(const char* hintlist, int forgetEverything) = 0;
	virtual int				PauseResourcePreloading() = 0;
	virtual int				ResumeResourcePreloading() = 0;
	virtual int				SetVBuf(FileHandle_t stream, char* buffer, int mode, long size) = 0;
	virtual void			GetInterfaceVersion(char* p, int maxlen) = 0;
	virtual bool			IsFileImmediatelyAvailable(const char* pFileName) = 0;

	//Resource waiting is obsolete and does nothing
	virtual WaitForResourcesHandle_t WaitForResources(const char* resourcelist) = 0;
	virtual bool			GetWaitForResourcesProgress(WaitForResourcesHandle_t handle, float* progress, bool* complete) = 0;
	virtual void			CancelWaitForResources(WaitForResourcesHandle_t handle) = 0;

	/**
	*	@brief Always returns true
	*/
	virtual bool			IsAppReadyForOfflinePlay(int appID) = 0;

	/**
	*	@brief interface for custom pack files > 4Gb
	*/
	virtual bool			AddPackFile(const char* fullpath, const char* pathID) = 0;

	/**
	*	@brief Don't use this
	*/
	virtual FileHandle_t	OpenFromCacheForRead(const char* pFileName, const char* pOptions, const char* pathID = 0L) = 0;

	virtual void			AddSearchPathNoWrite(const char* pPath, const char* pathID) = 0;
};

// Steam3/Src compat
using IBaseFileSystem = IFileSystem;

constexpr std::string_view FILESYSTEM_INTERFACE_VERSION{"VFileSystem009"};
