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

#include "CCorpse.hpp"
#include "CTestHull.hpp"
#include "dll_functions.hpp"
#include "game.h"
#include "nodes.h"

LINK_ENTITY_TO_CLASS(testhull, CTestHull);

void CTestHull::Spawn(CBaseEntity* pMasterNode)
{
	SetModel("models/player.mdl");
	SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Step);
	pev->effects = 0;
	pev->health = 50;
	pev->yaw_speed = 8;

	if (WorldGraph.m_fGraphPresent)
	{// graph loaded from disk, so we don't need the test hull
		SetThink(&CTestHull::SUB_Remove);
		pev->nextthink = gpGlobals->time;
	}
	else
	{
		SetThink(&CTestHull::DropDelay);
		pev->nextthink = gpGlobals->time + 1;
	}

	// Make this invisible
	// UNDONE: Shouldn't we just use EF_NODRAW?  This doesn't need to go to the client.
	SetRenderMode(RenderMode::TransTexture);
	SetRenderAmount(0);
}

void CTestHull::DropDelay()
{
	//	UTIL_CenterPrintAll( "Node Graph out of Date. Rebuilding..." );

	SetAbsOrigin(WorldGraph.m_pNodes[0].m_vecOrigin);

	SetThink(&CTestHull::CallBuildNodeGraph);

	pev->nextthink = gpGlobals->time + 1;
}

void CTestHull::ShowBadNode()
{
	SetMovetype(Movetype::Fly);
	Vector myAngles = GetAbsAngles();
	myAngles.y = myAngles.y + 4;
	SetAbsAngles(myAngles);

	UTIL_MakeVectors(myAngles);

	UTIL_ParticleEffect(GetAbsOrigin(), vec3_origin, 255, 25);
	UTIL_ParticleEffect(GetAbsOrigin() + gpGlobals->v_forward * 64, vec3_origin, 255, 25);
	UTIL_ParticleEffect(GetAbsOrigin() - gpGlobals->v_forward * 64, vec3_origin, 255, 25);
	UTIL_ParticleEffect(GetAbsOrigin() + gpGlobals->v_right * 64, vec3_origin, 255, 25);
	UTIL_ParticleEffect(GetAbsOrigin() - gpGlobals->v_right * 64, vec3_origin, 255, 25);

	pev->nextthink = gpGlobals->time + 0.1;
}

void CTestHull::CallBuildNodeGraph()
{
	// TOUCH HACK -- Don't allow this entity to call anyone's "touch" function
	gTouchDisabled = true;
	BuildNodeGraph();
	gTouchDisabled = false;
	// Undo TOUCH HACK
}

void CTestHull::BuildNodeGraph()
{
	SetThink(&CTestHull::SUB_Remove);// no matter what happens, the hull gets rid of itself.
	pev->nextthink = gpGlobals->time;

	std::unique_ptr<CLink[]> pTempPool;

	try
	{
		// 	alloc a swollen temporary connection pool that we trim down after we know exactly how many connections there are.
		pTempPool = std::make_unique<CLink[]>(WorldGraph.m_cNodes * MAX_NODE_INITIAL_LINKS);
	}
	catch (const std::bad_alloc&)
	{
		ALERT(at_aiconsole, "**Could not alloc TempPool!\n");
		return;
	}

	// make sure directories have been made
	//text node report filename
	std::filesystem::path nrpFilename = std::filesystem::path{"maps"} / "graphs";
	g_pFileSystem->CreateDirHierarchy(nrpFilename.string().c_str(), "GAMECONFIG");

	nrpFilename = nrpFilename / STRING(gpGlobals->mapname);
	nrpFilename += ".nrp";

	const auto nrpFilenameString = nrpFilename.string();

	FSFile file;

	if (sv_generatenodereportfile.value)
	{
		if (!file.Open(nrpFilenameString.c_str(), "w+", "GAMECONFIG"))
		{
			ALERT(at_aiconsole, "Couldn't create %s!\n", nrpFilenameString.c_str());
		}
	}

	if (file)
	{
		file.Printf("Node Graph Report for map:  %s.bsp\n", STRING(gpGlobals->mapname));
		file.Printf("%d Total Nodes\n\n", WorldGraph.m_cNodes);
	}

	for (int i = 0; i < WorldGraph.m_cNodes; i++)
	{// print all node numbers and their locations to the file.
		WorldGraph.m_pNodes[i].m_cNumLinks = 0;
		WorldGraph.m_pNodes[i].m_iFirstLink = 0;
		memset(WorldGraph.m_pNodes[i].m_pNextBestNode, 0, sizeof(WorldGraph.m_pNodes[i].m_pNextBestNode));

		if (file)
		{
			file.Printf("Node#         %4d\n", i);
			file.Printf("Location      %4d,%4d,%4d\n", (int)WorldGraph.m_pNodes[i].m_vecOrigin.x, (int)WorldGraph.m_pNodes[i].m_vecOrigin.y, (int)WorldGraph.m_pNodes[i].m_vecOrigin.z);
			file.Printf("HintType:     %4d\n", WorldGraph.m_pNodes[i].m_sHintType);
			file.Printf("HintActivity: %4d\n", WorldGraph.m_pNodes[i].m_sHintActivity);
			file.Printf("HintYaw:      %4f\n", WorldGraph.m_pNodes[i].m_flHintYaw);
			file.Printf("-------------------------------------------------------------------------------\n");
		}
	}

	if (file)
	{
		file.Printf("\n\n");
	}

	// Automatically recognize WATER nodes and drop the LAND nodes to the floor.
	//
	for (int i = 0; i < WorldGraph.m_cNodes; i++)
	{
		if (WorldGraph.m_pNodes[i].m_afNodeInfo & bits_NODE_AIR)
		{
			// do nothing
		}
		else if (UTIL_PointContents(WorldGraph.m_pNodes[i].m_vecOrigin) == Contents::Water)
		{
			WorldGraph.m_pNodes[i].m_afNodeInfo |= bits_NODE_WATER;
		}
		else
		{
			WorldGraph.m_pNodes[i].m_afNodeInfo |= bits_NODE_LAND;

			// trace to the ground, then pop up 8 units and place node there to make it
			// easier for them to connect (think stairs, chairs, and bumps in the floor).
			// After the routing is done, push them back down.
			//
			TraceResult	tr;

			UTIL_TraceLine(WorldGraph.m_pNodes[i].m_vecOrigin,
				WorldGraph.m_pNodes[i].m_vecOrigin - Vector(0, 0, 384),
				IgnoreMonsters::Yes,
				g_pBodyQueueHead,//!!!HACKHACK no real ent to supply here, using a global we don't care about
				&tr);

			// This trace is ONLY used if we hit an entity flagged with FL_WORLDBRUSH
			TraceResult	trEnt;
			UTIL_TraceLine(WorldGraph.m_pNodes[i].m_vecOrigin,
				WorldGraph.m_pNodes[i].m_vecOrigin - Vector(0, 0, 384),
				IgnoreMonsters::No,
				g_pBodyQueueHead,//!!!HACKHACK no real ent to supply here, using a global we don't care about
				&trEnt);

			// Did we hit something closer than the floor?
			if (trEnt.flFraction < tr.flFraction)
			{
				// If it was a world brush entity, copy the node location
				if (trEnt.pHit && (trEnt.pHit->v.flags & FL_WORLDBRUSH))
					tr.vecEndPos = trEnt.vecEndPos;
			}

			WorldGraph.m_pNodes[i].m_vecOriginPeek.z =
				WorldGraph.m_pNodes[i].m_vecOrigin.z = tr.vecEndPos.z + NODE_HEIGHT;
		}
	}

	// number of links in the pool.
	int iBadNode; // this is the node that caused graph generation to fail
	int cPoolLinks = WorldGraph.LinkVisibleNodes(pTempPool.get(), file, &iBadNode);

	if (!cPoolLinks)
	{
		ALERT(at_aiconsole, "**ConnectVisibleNodes FAILED!\n");

		SetThink(&CTestHull::ShowBadNode);// send the hull off to show the offending node.
		//SetSolidType(Solid::Not);
		SetAbsOrigin(WorldGraph.m_pNodes[iBadNode].m_vecOrigin);
		return;
	}

	// send the walkhull to all of this node's connections now. We'll do this here since
	// so much of it relies on being able to control the test hull.
	if (file)
	{
		file.Printf("----------------------------------------------------------------------------\n");
		file.Printf("Walk Rejection:\n");
	}

	for (int i = 0; i < WorldGraph.m_cNodes; i++)
	{
		CNode* pSrcNode = &WorldGraph.m_pNodes[i];

		if (file)
		{
			file.Printf("-------------------------------------------------------------------------------\n");
			file.Printf("Node %4d:\n\n", i);
		}

		for (int j = 0; j < pSrcNode->m_cNumLinks; j++)
		{
			// assume that all hulls can walk this link, then eliminate the ones that can't.
			pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo = bits_LINK_SMALL_HULL | bits_LINK_HUMAN_HULL | bits_LINK_LARGE_HULL | bits_LINK_FLY_HULL;

			// do a check for each hull size.

			// if we can't fit a tiny hull through a connection, no other hulls with fit either, so we 
			// should just fall out of the loop. Do so by setting the SkipRemainingHulls flag.
			bool fSkipRemainingHulls = false;
			for (int hull = 0; hull < MAX_NODE_HULLS; hull++)
			{
				if (fSkipRemainingHulls && (hull == NODE_HUMAN_HULL || hull == NODE_LARGE_HULL)) // skip the remaining walk hulls
					continue;

				switch (hull)
				{
				case NODE_SMALL_HULL:
					SetSize(Vector(-12, -12, 0), Vector(12, 12, 24));
					break;
				case NODE_HUMAN_HULL:
					SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
					break;
				case NODE_LARGE_HULL:
					SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));
					break;
				case NODE_FLY_HULL:
					SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));
					// SetSize( vec3_origin, vec3_origin);
					break;
				}

				SetAbsOrigin(pSrcNode->m_vecOrigin);// place the hull on the node

				if (!IsBitSet(pev->flags, FL_ONGROUND))
				{
					ALERT(at_aiconsole, "OFFGROUND!\n");
				}

				// now build a yaw that points to the dest node, and get the distance.
				if (j < 0)
				{
					ALERT(at_aiconsole, "**** j = %d ****\n", j);
					return;
				}

				CNode* pDestNode = &WorldGraph.m_pNodes[pTempPool[pSrcNode->m_iFirstLink + j].m_iDestNode];

				const Vector vecSpot = pDestNode->m_vecOrigin;
				//vecSpot.z = GetAbsOrigin().z;

				if (hull < NODE_FLY_HULL)
				{
					int SaveFlags = pev->flags;
					WalkMoveMode MoveMode = WalkMoveMode::WorldOnly;
					if (pSrcNode->m_afNodeInfo & bits_NODE_WATER)
					{
						pev->flags |= FL_SWIM;
						MoveMode = WalkMoveMode::Normal;
					}

					const float flYaw = UTIL_VecToYaw(pDestNode->m_vecOrigin - GetAbsOrigin());

					const float flDist = (vecSpot - GetAbsOrigin()).Length2D();

					bool fWalkFailed = false;

					// in this loop we take tiny steps from the current node to the nodes that it links to, one at a time.
					// SetAbsAngles({GetAbsAngles().x, flYaw, GetAbsAngles().z});
					int step;
					for (step = 0; step < flDist && !fWalkFailed; step += HULL_STEP_SIZE)
					{
						float stepSize = HULL_STEP_SIZE;

						if ((step + stepSize) >= (flDist - 1))
							stepSize = (flDist - step) - 1;

						if (!UTIL_WalkMove(this, flYaw, stepSize, MoveMode))
						{// can't take the next step

							fWalkFailed = true;
							break;
						}
					}

					if (!fWalkFailed && (GetAbsOrigin() - vecSpot).Length() > 64)
					{
						// ALERT( at_console, "bogus walk\n");
						// we thought we 
						fWalkFailed = true;
					}

					if (fWalkFailed)
					{
						//pTempPool[ pSrcNode->m_iFirstLink + j ] = pTempPool [ pSrcNode->m_iFirstLink + ( pSrcNode->m_cNumLinks - 1 ) ];

						// now me must eliminate the hull that couldn't walk this connection
						switch (hull)
						{
						case NODE_SMALL_HULL:	// if this hull can't fit, nothing can, so drop the connection
							if (file)
							{
								file.Printf("NODE_SMALL_HULL step %d\n", step);
							}
							pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo &= ~(bits_LINK_SMALL_HULL | bits_LINK_HUMAN_HULL | bits_LINK_LARGE_HULL);
							fSkipRemainingHulls = true;// don't bother checking larger hulls
							break;
						case NODE_HUMAN_HULL:
							if (file)
							{
								file.Printf("NODE_HUMAN_HULL step %d\n", step);
							}
							pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo &= ~(bits_LINK_HUMAN_HULL | bits_LINK_LARGE_HULL);
							fSkipRemainingHulls = true;// don't bother checking larger hulls
							break;
						case NODE_LARGE_HULL:
							if (file)
							{
								file.Printf("NODE_LARGE_HULL step %d\n", step);
							}
							pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo &= ~bits_LINK_LARGE_HULL;
							break;
						}
					}
					pev->flags = SaveFlags;
				}
				else
				{
					TraceResult tr;

					UTIL_TraceHull(pSrcNode->m_vecOrigin + Vector(0, 0, 32), pDestNode->m_vecOriginPeek + Vector(0, 0, 32), IgnoreMonsters::Yes, Hull::Large, this, &tr);
					if (tr.fStartSolid || tr.flFraction < 1.0)
					{
						pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo &= ~bits_LINK_FLY_HULL;
					}
				}
			}

			if (pTempPool[pSrcNode->m_iFirstLink + j].m_afLinkInfo == 0)
			{
				if (file)
				{
					file.Printf("Rejected Node %3d - Unreachable by ", pTempPool[pSrcNode->m_iFirstLink + j].m_iDestNode);
				}

				pTempPool[pSrcNode->m_iFirstLink + j] = pTempPool[pSrcNode->m_iFirstLink + (pSrcNode->m_cNumLinks - 1)];

				if (file)
				{
					file.Printf("Any Hull\n");
				}

				pSrcNode->m_cNumLinks--;
				cPoolLinks--;// we just removed a link, so decrement the total number of links in the pool.
				j--;
			}

		}
	}

	if (file)
	{
		file.Printf("-------------------------------------------------------------------------------\n\n\n");
	}

	cPoolLinks -= WorldGraph.RejectInlineLinks(pTempPool.get(), file);

	try
	{
		// now alloc a pool just large enough to hold the links that are actually used
		WorldGraph.m_pLinkPool = std::make_unique<CLink[]>(cPoolLinks);
	}
	catch (const std::bad_alloc&)
	{
		// couldn't make the link pool!
		ALERT(at_aiconsole, "Couldn't alloc LinkPool!\n");
		return;
	}

	WorldGraph.m_cLinks = cPoolLinks;

	//copy only the used portions of the TempPool into the graph's link pool
	int iFinalPoolIndex = 0;
	int iOldFirstLink;

	for (int i = 0; i < WorldGraph.m_cNodes; i++)
	{
		iOldFirstLink = WorldGraph.m_pNodes[i].m_iFirstLink;// store this, because we have to re-assign it before entering the copy loop

		WorldGraph.m_pNodes[i].m_iFirstLink = iFinalPoolIndex;

		for (int j = 0; j < WorldGraph.m_pNodes[i].m_cNumLinks; j++)
		{
			WorldGraph.m_pLinkPool[iFinalPoolIndex++] = pTempPool[iOldFirstLink + j];
		}
	}

	//Free the temp pool
	pTempPool.reset();

	// Node sorting numbers linked nodes close to each other
	//
	WorldGraph.SortNodes();

	// This is used for HashSearch
	//
	WorldGraph.BuildLinkLookups();

	// are all links in the graph evenly paired?
	bool fPairsValid = true; // assume that the connection pairs are all valid to start

	if (file)
	{
		file.Printf("\n\n-------------------------------------------------------------------------------\n");
		file.Printf("Link Pairings:\n");
	}

	// link integrity check. The idea here is that if Node A links to Node B, node B should
	// link to node A. If not, we have a situation that prevents us from using a basic 
	// optimization in the FindNearestLink function. 
	for (int i = 0; i < WorldGraph.m_cNodes; i++)
	{
		for (int j = 0; j < WorldGraph.m_pNodes[i].m_cNumLinks; j++)
		{
			int iLink;
			WorldGraph.HashSearch(WorldGraph.DestNodeLink(i, j), i, iLink);
			if (iLink < 0)
			{
				fPairsValid = false;// unmatched link pair.

				if (file)
				{
					file.Printf("WARNING: Node %3d does not connect back to Node %3d\n", WorldGraph.DestNodeLink(i, j), i);
				}
			}
		}
	}

	// !!!LATER - if all connections are properly paired, when can enable an optimization in the pathfinding code
	// (in the find nearest line function)
	if (fPairsValid)
	{
		if (file)
		{
			file.Printf("\nAll Connections are Paired!\n");
		}
	}

	if (file)
	{
		file.Printf("-------------------------------------------------------------------------------\n");
		file.Printf("\n\n-------------------------------------------------------------------------------\n");
		file.Printf("Total Number of Connections in Pool: %d\n", cPoolLinks);
		file.Printf("-------------------------------------------------------------------------------\n");
		file.Printf("Connection Pool: %d bytes\n", sizeof(CLink) * cPoolLinks);
		file.Printf("-------------------------------------------------------------------------------\n");
	}

	ALERT(at_aiconsole, "%d Nodes, %d Connections\n", WorldGraph.m_cNodes, cPoolLinks);

	// This is used for FindNearestNode
	//
	WorldGraph.BuildRegionTables();

	// Push all of the LAND nodes down to the ground now. Leave the water and air nodes alone.
	//
	for (int i = 0; i < WorldGraph.m_cNodes; i++)
	{
		if ((WorldGraph.m_pNodes[i].m_afNodeInfo & bits_NODE_LAND))
		{
			WorldGraph.m_pNodes[i].m_vecOrigin.z -= NODE_HEIGHT;
		}
	}

	// We now have some graphing capabilities.
	//
	WorldGraph.m_fGraphPresent = true;//graph is in memory.
	WorldGraph.m_fGraphPointersSet = true;// since the graph was generated, the pointers are ready
	WorldGraph.m_fRoutingComplete = false; // Optimal routes aren't computed, yet.

	// Compute and compress the routing information.
	//
	WorldGraph.ComputeStaticRoutingTables();

	// save the node graph for this level	
	WorldGraph.SaveGraph(STRING(gpGlobals->mapname));
	ALERT(at_console, "Done.\n");
}

void CTestHull::PathFind()
{
	if (!WorldGraph.m_fGraphPresent || !WorldGraph.m_fGraphPointersSet)
	{// protect us in the case that the node graph isn't available
		ALERT(at_aiconsole, "Graph not ready!\n");
		return;
	}

	//TODO: FindShortestPath could overflow this buffer
	int	iPath[50];
	const int iPathSize = WorldGraph.FindShortestPath(iPath, 0, 19, 0, 0); // UNDONE use hull constant

	if (!iPathSize)
	{
		ALERT(at_aiconsole, "No Path!\n");
		return;
	}

	ALERT(at_aiconsole, "%d\n", iPathSize);

	CNode* pNode = &WorldGraph.m_pNodes[iPath[0]];

	for (int i = 0; i < iPathSize - 1; i++)
	{
		CNode* pNextNode = &WorldGraph.m_pNodes[iPath[i + 1]];

		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_SHOWLINE);

		WRITE_COORD(pNode->m_vecOrigin.x);
		WRITE_COORD(pNode->m_vecOrigin.y);
		WRITE_COORD(pNode->m_vecOrigin.z + NODE_HEIGHT);

		WRITE_COORD(pNextNode->m_vecOrigin.x);
		WRITE_COORD(pNextNode->m_vecOrigin.y);
		WRITE_COORD(pNextNode->m_vecOrigin.z + NODE_HEIGHT);
		MESSAGE_END();

		pNode = pNextNode;
	}
}
