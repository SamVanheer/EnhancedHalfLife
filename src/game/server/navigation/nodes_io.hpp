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

#pragma once

#include <cstdint>

#include "nodes.hpp"

constexpr std::int32_t GRAPH_VERSION = 17; //!< !!!increment this whever graph/node/link classes change, to obsolesce older disk files.

constexpr std::uint32_t DISKGRAPH_FLAG_ROUTINGCOMPLETE = 1 << 0;

/**
*	@brief Structure written to the node graph file on disk
*	@details The on-disk format of the graph is as follows:
*	std::int32_t Version
*	DiskGraph graph
*	CNode Nodes[graph.NodeCount]
*	CDiskLink LinkPool[graph.LinkCount]
*	DiskDistInfo DistanceInfo[graph.NodeCount]
*	std::int8_t RouteInfo[graph.RouteInfoSize]
*	std::int16_t HashLinks[graph.HashLinkCount]
*/
struct DiskGraph
{
	/**
	*	@brief Boolean flags for this graph
	*/
	std::uint32_t Flags;

	std::int32_t NodeCount;
	std::int32_t LinkCount;
	std::int32_t RouteInfoCount;

	std::int32_t RangeStart[3][CGraph::NUM_RANGES];
	std::int32_t RangeEnd[3][CGraph::NUM_RANGES];

	float RegionMin[3];
	float RegionMax[3];

	std::int32_t HashPrimes[CGraph::HashPrimesCount];

	std::int32_t HashLinkCount;
};
