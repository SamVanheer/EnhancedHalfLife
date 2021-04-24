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

#include <algorithm>
#include <filesystem>
#include <memory>
#include <new>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "nodes_io.hpp"
#include "BinaryReader.hpp"

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

bool InternalLoadGraphFromDisk(const std::unique_ptr<byte[]>& fileBuffer, std::size_t size, CGraph& graph)
{
	BinaryReader reader{fileBuffer.get(), size};

	const std::int32_t version = reader.ReadInt32();

	if (version != GRAPH_VERSION)
	{
		// This file was written by a different build of the dll!
		//
		ALERT(at_aiconsole, "**ERROR** Graph version is %d, expected %d\n", static_cast<int>(version), GRAPH_VERSION);
		return false;
	}

	DiskGraph diskGraph{};

	if (reader.Read(reinterpret_cast<byte*>(&diskGraph), sizeof(diskGraph)) != sizeof(diskGraph))
	{
		return false;
	}

	//Validate graph data
	if (diskGraph.NodeCount < 0
		|| diskGraph.LinkCount < 0
		|| diskGraph.RouteInfoCount < 0
		|| diskGraph.HashLinkCount < 0)
	{
		return false;
	}

	//TODO: these ranges could potentially be just out of the valid range due to floatin point errors, so maybe disable this check?
	for (std::size_t i = 0; i < CGraph::NUM_RANGES; ++i)
	{
		for (std::size_t j = 0; j < 3; ++j)
		{
			if (diskGraph.RangeStart[j][i] < NODE_RANGE_MIN || diskGraph.RangeStart[j][i] > NODE_RANGE_MAX
				|| diskGraph.RangeEnd[j][i] < NODE_RANGE_MIN || diskGraph.RangeEnd[j][i] > NODE_RANGE_MAX)
			{
				return false;
			}
		}
	}

	for (std::size_t j = 0; j < 3; ++j)
	{
		if (diskGraph.RegionMin[j] > diskGraph.RegionMax[j])
		{
			return false;
		}
	}

	//Load arrays
	auto nodes = std::make_unique<CNode[]>(diskGraph.NodeCount);
	reader.ReadArray(nodes.get(), diskGraph.NodeCount);
	
	auto links = std::make_unique<CLink[]>(diskGraph.LinkCount);
	{
		auto diskLinks = std::make_unique<CDiskLink[]>(diskGraph.LinkCount);
		reader.ReadArray(diskLinks.get(), diskGraph.LinkCount);

		std::transform(diskLinks.get(), diskLinks.get() + diskGraph.LinkCount, links.get(), [](const auto& diskLink)
			{
				CLink link{diskLink};
				link.m_pLinkEnt = nullptr;
				return link;
			});
	}

	auto distanceInfo = std::make_unique<DIST_INFO[]>(diskGraph.NodeCount);
	{
		auto diskDistanceInfo = std::make_unique<DiskDistInfo[]>(diskGraph.NodeCount);
		reader.ReadArray(diskDistanceInfo.get(), diskGraph.NodeCount);

		std::transform(diskDistanceInfo.get(), diskDistanceInfo.get() + diskGraph.NodeCount, distanceInfo.get(), [](const auto& diskDistInfo)
			{
				DIST_INFO distInfo{diskDistInfo};
				distInfo.m_CheckedEvent = 0;
				return distInfo;
			});
	}

	auto routeInfo = std::make_unique<std::int8_t[]>(diskGraph.RouteInfoCount);
	reader.ReadArray(routeInfo.get(), diskGraph.RouteInfoCount);

	auto hashLinks = std::make_unique<std::int16_t[]>(diskGraph.HashLinkCount);
	reader.ReadArray(hashLinks.get(), diskGraph.HashLinkCount);

	if (reader.GetOffset() < reader.GetSizeInBytes())
	{
		ALERT(at_aiconsole, "***WARNING***:Node graph was longer than expected by %zu bytes.!\n", (reader.GetSizeInBytes() - reader.GetOffset()));
	}

	//Now copy the data over
	graph.m_fRoutingComplete = (diskGraph.Flags & DISKGRAPH_FLAG_ROUTINGCOMPLETE) != 0;

	graph.m_cNodes = diskGraph.NodeCount;
	graph.m_cLinks = diskGraph.LinkCount;
	graph.m_nRouteInfo = diskGraph.RouteInfoCount;

	for (std::size_t i = 0; i < CGraph::NUM_RANGES; ++i)
	{
		for (std::size_t j = 0; j < 3; ++j)
		{
			graph.m_RangeStart[j][i] = diskGraph.RangeStart[j][i];
			graph.m_RangeEnd[j][i] = diskGraph.RangeEnd[j][i];
		}
	}

	for (std::size_t j = 0; j < 3; ++j)
	{
		graph.m_RegionMin[j] = diskGraph.RegionMin[j];
		graph.m_RegionMax[j] = diskGraph.RegionMax[j];
	}

	for (std::size_t i = 0; i < CGraph::HashPrimesCount; ++i)
	{
		graph.m_HashPrimes[i] = diskGraph.HashPrimes[i];
	}

	graph.m_nHashLinks = diskGraph.HashLinkCount;

	graph.m_pNodes = std::move(nodes);
	graph.m_pLinkPool = std::move(links);
	graph.m_pRouteInfo = std::move(routeInfo);
	graph.m_di = std::move(distanceInfo);
	graph.m_pHashLinks = std::move(hashLinks);

	// Set the graph present flag, clear the pointers set flag
	graph.m_fGraphPresent = true;
	graph.m_fGraphPointersSet = false;

	return true;
}

bool CGraph::LoadGraph(const char* szMapName)
{
	InitGraph();

	// make sure the directories have been made
	std::filesystem::path dirName = std::filesystem::path{"maps"} / "graphs";
	g_pFileSystem->CreateDirHierarchy(dirName.string().c_str(), "GAMECONFIG");

	auto filename = dirName / szMapName;
	filename += ".nod";

	auto [fileBuffer, size] = FileSystem_LoadFileIntoBuffer(filename.string().c_str());

	if (!fileBuffer)
	{
		return false;
	}

	try
	{
		return InternalLoadGraphFromDisk(fileBuffer, size, *this);
	}
	catch (const std::exception& e)
	{
		ALERT(at_aiconsole, "Error reading graph: %s\n", e.what());
		return false;
	}
}

bool CGraph::SaveGraph(const char* szMapName)
{
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

	if (!file)
	{
		ALERT(at_aiconsole, "Couldn't Create: %s\n", filenameString.c_str());
		return false;
	}

	ALERT(at_aiconsole, "Created: %s\n", filenameString.c_str());

	// write the version
	const std::int32_t iVersion = GRAPH_VERSION;
	file.Write(&iVersion, sizeof(std::int32_t));

	// write the DiskGraph struct
	DiskGraph diskGraph{};

	if (m_fRoutingComplete)
	{
		diskGraph.Flags |= DISKGRAPH_FLAG_ROUTINGCOMPLETE;
	}

	diskGraph.NodeCount = m_cNodes;
	diskGraph.LinkCount = m_cLinks;
	diskGraph.RouteInfoCount = m_nRouteInfo;

	for (std::size_t i = 0; i < CGraph::NUM_RANGES; ++i)
	{
		for (std::size_t j = 0; j < 3; ++j)
		{
			diskGraph.RangeStart[j][i] = m_RangeStart[j][i];
			diskGraph.RangeEnd[j][i] = m_RangeEnd[j][i];
		}
	}

	for (std::size_t j = 0; j < 3; ++j)
	{
		diskGraph.RegionMin[j] = m_RegionMin[j];
		diskGraph.RegionMax[j] = m_RegionMax[j];
	}

	for (std::size_t i = 0; i < CGraph::HashPrimesCount; ++i)
	{
		diskGraph.HashPrimes[i] = m_HashPrimes[i];
	}

	diskGraph.HashLinkCount = m_nHashLinks;

	file.Write(&diskGraph, sizeof(diskGraph));

	file.Write(m_pNodes.get(), sizeof(CNode) * m_cNodes);

	{
		// write the links
		//Don't write the entity pointer
		auto diskLinks = std::make_unique<CDiskLink[]>(m_cLinks);

		std::transform(m_pLinkPool.get(), m_pLinkPool.get() + m_cLinks, diskLinks.get(), [](const auto& link)
			{
				return link;
			});

		file.Write(diskLinks.get(), sizeof(CDiskLink) * m_cLinks);
	}

	{
		auto diskDistInfo = std::make_unique<DiskDistInfo[]>(m_cNodes);

		std::transform(m_di.get(), m_di.get() + m_cNodes, diskDistInfo.get(), [](const auto& distInfo)
			{
				return distInfo;
			});

		file.Write(diskDistInfo.get(), sizeof(DiskDistInfo) * m_cNodes);
	}

	if (m_pRouteInfo && m_nRouteInfo)
	{
		file.Write(m_pRouteInfo.get(), sizeof(std::int8_t) * m_nRouteInfo);
	}

	if (m_pHashLinks && m_nHashLinks)
	{
		file.Write(m_pHashLinks.get(), sizeof(std::int16_t) * m_nHashLinks);
	}

	return true;
}
