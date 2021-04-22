/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#include <filesystem>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "nodes_io.hpp"

bool CGraph::CheckNODFile(const char* szMapName)
{
	std::filesystem::path bspFilename = std::filesystem::path{"maps"} / szMapName;
	bspFilename += ".bsp";

	std::filesystem::path graphFilename = std::filesystem::path{"maps"} / "graphs" / szMapName;
	graphFilename += ".nod";

	bool retValue = true;

	int iCompare;
	if (COMPARE_FILE_TIME(bspFilename.string().c_str(), graphFilename.string().c_str(), &iCompare))
	{
		if (iCompare > 0)
		{// BSP file is newer.
			ALERT(at_aiconsole, ".NOD File will be updated\n\n");
			retValue = false;
		}
	}
	else
	{
		retValue = false;
	}

	return retValue;
}

bool CGraph::FLoadGraph(const char* szMapName)
{
	//TODO: rewrite graph loading
	int		iVersion;

	// make sure the directories have been made
	std::filesystem::path dirName = std::filesystem::path{"maps"} / "graphs";
	g_pFileSystem->CreateDirHierarchy(dirName.string().c_str(), "GAMECONFIG");

	auto filename = dirName / szMapName;
	filename += ".nod";

	auto [fileBuffer, size] = FileSystem_LoadFileIntoBuffer(filename.string().c_str());

	byte* pMemFile = fileBuffer.get();

	if (!fileBuffer)
	{
		return false;
	}
	else
	{
		// Read the graph version number
		//
		if (size < sizeof(int)) goto ShortFile;
		size -= sizeof(int);
		memcpy(&iVersion, pMemFile, sizeof(int));
		pMemFile += sizeof(int);

		if (iVersion != GRAPH_VERSION)
		{
			// This file was written by a different build of the dll!
			//
			ALERT(at_aiconsole, "**ERROR** Graph version is %d, expected %d\n", iVersion, GRAPH_VERSION);
			goto ShortFile;
		}

		// Read the graph class
		//
		if (size < sizeof(CGraph)) goto ShortFile;
		size -= sizeof(CGraph);
		memcpy(this, pMemFile, sizeof(CGraph));
		pMemFile += sizeof(CGraph);

		// Set the pointers to zero, just in case we run out of memory.
		//
		m_pNodes = nullptr;
		m_pLinkPool = nullptr;
		m_di = nullptr;
		m_pRouteInfo = nullptr;
		m_pHashLinks = nullptr;


		// Malloc for the nodes
		//
		m_pNodes = (CNode*)calloc(sizeof(CNode), m_cNodes);

		if (!m_pNodes)
		{
			ALERT(at_aiconsole, "**ERROR**\nCouldn't malloc %d nodes!\n", m_cNodes);
			goto NoMemory;
		}

		// Read in all the nodes
		//
		if (size < (sizeof(CNode) * m_cNodes)) goto ShortFile;
		size -= sizeof(CNode) * m_cNodes;
		memcpy(m_pNodes, pMemFile, sizeof(CNode) * m_cNodes);
		pMemFile += sizeof(CNode) * m_cNodes;


		// Malloc for the link pool
		//
		m_pLinkPool = (CLink*)calloc(sizeof(CLink), m_cLinks);

		if (!m_pLinkPool)
		{
			ALERT(at_aiconsole, "**ERROR**\nCouldn't malloc %d link!\n", m_cLinks);
			goto NoMemory;
		}

		// Read in all the links
		//
		if (size < (sizeof(CLink) * m_cLinks)) goto ShortFile;
		size -= sizeof(CLink) * m_cLinks;
		memcpy(m_pLinkPool, pMemFile, sizeof(CLink) * m_cLinks);
		pMemFile += sizeof(CLink) * m_cLinks;

		// Malloc for the sorting info.
		//
		m_di = (DIST_INFO*)calloc(sizeof(DIST_INFO), m_cNodes);
		if (!m_di)
		{
			ALERT(at_aiconsole, "***ERROR**\nCouldn't malloc %d entries sorting nodes!\n", m_cNodes);
			goto NoMemory;
		}

		// Read it in.
		//
		if (size < (sizeof(DIST_INFO) * m_cNodes)) goto ShortFile;
		size -= sizeof(DIST_INFO) * m_cNodes;
		memcpy(m_di, pMemFile, sizeof(DIST_INFO) * m_cNodes);
		pMemFile += sizeof(DIST_INFO) * m_cNodes;

		// Malloc for the routing info.
		//
		m_fRoutingComplete = false;
		m_pRouteInfo = (std::int8_t*)calloc(sizeof(std::int8_t), m_nRouteInfo);
		if (!m_pRouteInfo)
		{
			ALERT(at_aiconsole, "***ERROR**\nCounldn't malloc %d route bytes!\n", m_nRouteInfo);
			goto NoMemory;
		}
		m_CheckedCounter = 0;
		for (int i = 0; i < m_cNodes; i++)
		{
			m_di[i].m_CheckedEvent = 0;
		}

		// Read in the route information.
		//
		if (size < (sizeof(std::int8_t) * m_nRouteInfo)) goto ShortFile;
		size -= sizeof(std::int8_t) * m_nRouteInfo;
		memcpy(m_pRouteInfo, pMemFile, sizeof(std::int8_t) * m_nRouteInfo);
		pMemFile += sizeof(std::int8_t) * m_nRouteInfo;
		m_fRoutingComplete = true;

		// malloc for the hash links
		//
		m_pHashLinks = (short*)calloc(sizeof(short), m_nHashLinks);
		if (!m_pHashLinks)
		{
			ALERT(at_aiconsole, "***ERROR**\nCounldn't malloc %d hash link bytes!\n", m_nHashLinks);
			goto NoMemory;
		}

		// Read in the hash link information
		//
		if (size < (sizeof(short) * m_nHashLinks)) goto ShortFile;
		size -= sizeof(short) * m_nHashLinks;
		memcpy(m_pHashLinks, pMemFile, sizeof(short) * m_nHashLinks);
		pMemFile += sizeof(short) * m_nHashLinks;

		// Set the graph present flag, clear the pointers set flag
		//
		m_fGraphPresent = true;
		m_fGraphPointersSet = false;

		if (size != 0)
		{
			ALERT(at_aiconsole, "***WARNING***:Node graph was longer than expected by %zu bytes.!\n", size);
		}

		return true;
	}

ShortFile:
NoMemory:
	return false;
}

bool CGraph::FSaveGraph(const char* szMapName)
{

	int		iVersion = GRAPH_VERSION;

	if (!m_fGraphPresent || !m_fGraphPointersSet)
	{// protect us in the case that the node graph isn't available or built
		ALERT(at_aiconsole, "Graph not ready!\n");
		return false;
	}

	// make sure directories have been made
	std::filesystem::path dirName = std::filesystem::path{"maps"} / "graphs";
	g_pFileSystem->CreateDirHierarchy(dirName.string().c_str(), "GAMECONFIG");

	auto filename = dirName / szMapName;
	filename += ".nod";

	const auto filenameString = filename.string();

	FSFile file{filenameString.c_str(), "wb", "GAMECONFIG"};

	ALERT(at_aiconsole, "Created: %s\n", filenameString.c_str());

	if (!file)
	{// couldn't create
		ALERT(at_aiconsole, "Couldn't Create: %s\n", filenameString.c_str());
		return false;
	}

	// write the version
	file.Write(&iVersion, sizeof(int));

	// write the CGraph class
	file.Write(this, sizeof(CGraph));

	// write the nodes
	file.Write(m_pNodes, sizeof(CNode) * m_cNodes);

	// write the links
	file.Write(m_pLinkPool, sizeof(CLink) * m_cLinks);

	file.Write(m_di, sizeof(DIST_INFO) * m_cNodes);

	// Write the route info.
	//
	if (m_pRouteInfo && m_nRouteInfo)
	{
		file.Write(m_pRouteInfo, sizeof(std::int8_t) * m_nRouteInfo);
	}

	if (m_pHashLinks && m_nHashLinks)
	{
		file.Write(m_pHashLinks, sizeof(short) * m_nHashLinks);
	}

	return true;
}
