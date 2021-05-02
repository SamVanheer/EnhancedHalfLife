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
#include <memory>

/**
*	@file
*
*	AI node tree stuff.
*/

constexpr int MAX_STACK_NODES = 100;
constexpr int NO_NODE = -1;
constexpr int MAX_NODE_HULLS = 4;

constexpr int bits_NODE_LAND = 1 << 0;		//!< Land node, so nudge if necessary.
constexpr int bits_NODE_AIR = 1 << 1;		//!< Air node, don't nudge.
constexpr int bits_NODE_WATER = 1 << 2;		//!< Water node, don't nudge.
constexpr int bits_NODE_GROUP_REALM = bits_NODE_LAND | bits_NODE_AIR | bits_NODE_WATER;

constexpr int NODE_RANGE_MIN = 0;
constexpr int NODE_RANGE_MAX = 255;

/**
*	@brief Instance of a node.
*/
class CNode
{
public:
	Vector	m_vecOrigin;		//!< location of this node in space
	Vector  m_vecOriginPeek;	//!< location of this node (LAND nodes are NODE_HEIGHT higher).
	byte    m_Region[3];		//!< Which of 256 regions do each of the coordinate belong?
	int		m_afNodeInfo;		//!< bits that tell us more about this location

	int		m_cNumLinks;		//!< how many links this node has
	int		m_iFirstLink;		//!< index of this node's first link in the link pool.

	/**
	*	@brief Where to start looking in the compressed routing table (offset into m_pRouteInfo).
	*
	*	(4 hull sizes -- smallest to largest + fly/swim), and secondly, door capability.
	*/
	int		m_pNextBestNode[MAX_NODE_HULLS][2];

	/**
	*	@brief Used in finding the shortest path.
	*
	*	m_fClosestSoFar is -1 if not visited. Then it is the distance to the source.
	*	If another path uses this node and has a closer distance, then m_iPreviousNode is also updated.
	*/
	float   m_flClosestSoFar;
	int		m_iPreviousNode;

	short	m_sHintType;	//!< there is something interesting in the world at this node's position
	short	m_sHintActivity;//!< there is something interesting in the world at this node's position
	float	m_flHintYaw;	//!< monster on this node should face this yaw to face the hint.
};

constexpr int bits_LINK_SMALL_HULL = 1 << 0;	//!< headcrab box can fit through this connection
constexpr int bits_LINK_HUMAN_HULL = 1 << 1;	//!< player box can fit through this connection
constexpr int bits_LINK_LARGE_HULL = 1 << 2;	//!< big box can fit through this connection
constexpr int bits_LINK_FLY_HULL = 1 << 3;		//!< a flying big box can fit through this connection
constexpr int bits_LINK_DISABLED = 1 << 4;		//!< link is not valid when the set

//TODO: same as Hull enum
constexpr int NODE_SMALL_HULL = 0;
constexpr int NODE_HUMAN_HULL = 1;
constexpr int NODE_LARGE_HULL = 2;
constexpr int NODE_FLY_HULL = 3;

class CDiskLink
{
public:
	int		m_iSrcNode;		//!< the node that 'owns' this link ( keeps us from having to make reverse lookups )
	int		m_iDestNode;	//!< the node on the other end of the link. 

	/**
	*	@brief the unique name of the brush model that blocks the connection (this is kept for save/restore)
	*
	*	m_szLinkEntModelname is not necessarily NULL terminated (so we can store it in a more alignment-friendly 4 bytes)
	*/
	char	m_szLinkEntModelname[4];

	int		m_afLinkInfo;	//!< information about this link
	float	m_flWeight;		//!< length of the link line segment
};

/**
*	@brief A link between 2 nodes
*/
class CLink : public CDiskLink
{
public:
	EHANDLE m_hLinkEnt; //!< the entity that blocks this connection (doors, etc)
};

struct DiskDistInfo
{
	int m_SortedBy[3];
};

struct DIST_INFO : public DiskDistInfo
{
	int m_CheckedEvent;
};

struct CACHE_ENTRY
{
	Vector v;
	short n;		//!< Nearest node or -1 if no node found.
};

class CGraph
{
public:

	// the graph has two flags, and should not be accessed unless both flags are true!
	bool	m_fGraphPresent;	//!< is the graph in memory?
	bool	m_fGraphPointersSet;//!< are the entity pointers for the graph all set?
	bool    m_fRoutingComplete; //!< are the optimal routes computed, yet?

	std::unique_ptr<CNode[]> m_pNodes;				//!< pointer to the memory block that contains all node info
	std::unique_ptr<CLink[]> m_pLinkPool;			//!< big list of all node connections
	std::unique_ptr<std::int8_t[]> m_pRouteInfo;	//!< compressed routing information the nodes use.

	int		m_cNodes;			//!< total number of nodes
	int		m_cLinks;			//!< total number of links
	int     m_nRouteInfo;		//!< size of m_pRouteInfo in bytes.

	// Tables for making nearest node lookup faster. SortedBy provided nodes in a
	// order of a particular coordinate. Instead of doing a binary search, RangeStart
	// and RangeEnd let you get to the part of SortedBy that you are interested in.
	//
	// Once you have a point of interest, the only way you'll find a closer point is
	// if at least one of the coordinates is closer than the ones you have now. So we
	// search each range. After the search is exhausted, we know we have the closest
	// node.
	//
	static constexpr int CACHE_SIZE = 128;
	static constexpr int NUM_RANGES = 256;
	std::unique_ptr<DIST_INFO[]> m_di;	//!< This is m_cNodes long, but the entries don't correspond to CNode entries.
	int m_RangeStart[3][NUM_RANGES];
	int m_RangeEnd[3][NUM_RANGES];
	float m_flShortest;
	int m_iNearest;
	int m_minX, m_minY, m_minZ, m_maxX, m_maxY, m_maxZ;
	int m_minBoxX, m_minBoxY, m_minBoxZ, m_maxBoxX, m_maxBoxY, m_maxBoxZ;
	int m_CheckedCounter;
	float m_RegionMin[3], m_RegionMax[3]; //!< The range of nodes.
	CACHE_ENTRY m_Cache[CACHE_SIZE];

	static constexpr int HashPrimesCount = 16;

	int m_HashPrimes[HashPrimesCount];
	std::unique_ptr<std::int16_t[]> m_pHashLinks;
	int m_nHashLinks;


	/**
	*	@brief kinda sleazy. In order to allow variety in active idles for monster groups in a room with more than one node,
	*	we keep track of the last node we searched from and store it here.
	*	Subsequent searches by other monsters will pick up where the last search stopped.
	*/
	int		m_iLastActiveIdleSearch;

	/**
	*	@brief another such system used to track the search for cover nodes, helps greatly with two monsters trying to get to the same node.
	*/
	int		m_iLastCoverSearch;

	// functions to create the graph
	/**
	*	@brief the first, most basic function of node graph creation, this connects every node to every other node that it can see.
	*
	*	@details Expects a pointer to an empty connection pool and a file pointer to write progress to.
	*	If there's a problem with this process, the index of the offending node will be written to piBadNode
	*	@return the total number of initial links.
	*/
	int		LinkVisibleNodes(CLink* pLinkPool, FSFile& file, int* piBadNode);

	/**
	*	@brief expects a pointer to a link pool, and a pointer to and already-open file ( if you want status reports written to disk ).
	*	@return the number of connections that were rejected
	*/
	int		RejectInlineLinks(CLink* pLinkPool, FSFile& file);

	/**
	*	@brief accepts a capability mask (afCapMask),
	*	and will only find a path usable by a monster with those capabilities returns the number of nodes copied into supplied array
	*/
	int		FindShortestPath(int* piPath, int iStart, int iDest, int iHull, int afCapMask);

	/**
	*	@brief returns the index of the node nearest the given vector -1 is failure (couldn't find a valid near node)
	*/
	int		FindNearestNode(const Vector& vecOrigin, CBaseEntity* pEntity);
	int		FindNearestNode(const Vector& vecOrigin, int afNodeTypes);

	/**
	*	@brief finds the connection (line) nearest the given point.
	*	@param iNearestLink index into the link pool, this is the nearest node at any time.
	*	@param fAlongLine whether or not the point is along the line
	*/
	//int FindNearestLink(const Vector &vecTestPoint, int& iNearestLink, bool& fAlongLine);

	/**
	*	@brief Sum up graph weights on the path from iStart to iDest to determine path length
	*/
	float	PathLength(int iStart, int iDest, int iHull, int afCapMask);

	/**
	*	@brief Parse the routing table at iCurrentNode for the next node on the shortest path to iDest
	*/
	int		NextNodeInRoute(int iCurrentNode, int iDest, int iHull, int iCap);

	enum class NodeQuery
	{
		Dynamic,	//!< A static query means we're asking about the possiblity of handling this entity at ANY time
		Static		//!< A dynamic query means we're asking about it RIGHT NOW.  So we should query the current state
	};

	/**
	*	@brief a brush ent is between two nodes that would otherwise be able to see each other.
	*	Given the monster's capability, determine whether or not the monster can go this way.
	*/
	bool HandleLinkEnt(int iNode, CBaseEntity* pLinkEnt, int afCapMask, NodeQuery queryType);

	/**
	*	@brief sometimes the ent that blocks a path is a usable door,
	*	in which case the monster just needs to face the door and fire it.
	*
	*	In other cases, the monster needs to operate a button or lever to get the door to open.
	*	This function will return a pointer to the button if the monster needs to hit a button to open the door,
	*	or returns a pointer to the door if the monster need only use the door.
	*
	*	@param pNode is the node the monster will be standing on when it will need to stop and trigger the ent.
	*/
	CBaseEntity* LinkEntForLink(CLink* pLink, CNode* pNode);

	/**
	*	@brief draws a line from the given node to all connected nodes
	*/
	void	ShowNodeConnections(int iNode);

	/**
	*	@brief prepares the graph for use. Frees any memory currently in use by the world graph, NULLs all pointers, and zeros the node count.
	*/
	void	InitGraph();

	/**
	*	@brief temporary function that mallocs a reasonable number of nodes so we can build the path which will be saved to disk.
	*/
	bool AllocNodes();

	/**
	*	@brief this function checks the date of the BSP file that was just loaded and the date of the associated .NOD file.
	*	If the NOD file is not present, or is older than the BSP file, we rebuild it.
	*
	*	@return false if the .NOD file doesn't qualify and needs to be rebuilt.

	*	!!!BUGBUG - the file times we get back are 20 hours ahead!
	*	since this happens consistently, we can still correctly determine which of the 2 files is newer.
	*	This needs to be fixed, though. ( I now suspect that we are getting GMT back from these functions and must compensate for local time ) (sjb)
	*/
	bool CheckNODFile(const char* szMapName);

	/**
	*	@brief attempts to load a node graph from disk.
	*
	*	@details if the current level is maps/snar.bsp, maps/graphs/snar.nod will be loaded.
	*	If file cannot be loaded, the node tree will be created and saved to disk.
	*/
	bool LoadGraph(const char* szMapName);

	/**
	*	@brief It's not rocket science. this WILL overwrite existing files.
	*/
	bool SaveGraph(const char* szMapName);

	/**
	*	@brief Takes the modelnames of  all of the brush ents that block connections in the node graph and resolves them into pointers to those entities.
	*	this is done after loading the graph from disk, whereupon
	*	the pointers are not valid.
	*/
	bool SetGraphPointers();
	void	CheckNode(Vector vecOrigin, int iNode);

	void    BuildRegionTables();
	void    ComputeStaticRoutingTables();

	/**
	*	@brief Test those routing tables. Doesn't really work, yet.
	*/
	void    TestRoutingTables();

	void	HashInsert(int iSrcNode, int iDestNode, int iKey);
	void    HashSearch(int iSrcNode, int iDestNode, int& iKey);
	void	HashChoosePrimes(int TableSize);
	void    BuildLinkLookups();

	/**
	*	@brief Renumber nodes so that nodes that link together are together.
	*/
	void    SortNodes();

	int			HullIndex(const CBaseEntity* pEntity);		//!< what hull the monster uses
	int			NodeType(const CBaseEntity* pEntity);		//!< what node type the monster uses
	inline int	CapIndex(int afCapMask)
	{
		if (afCapMask & (bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE))
			return 1;
		return 0;
	}


	inline	CNode& Node(int i)
	{
#ifdef _DEBUG
		if (!m_pNodes || i < 0 || i > m_cNodes)
			ALERT(at_error, "Bad Node!\n");
#endif
		return m_pNodes[i];
	}

	inline	CLink& Link(int i)
	{
#ifdef _DEBUG
		if (!m_pLinkPool || i < 0 || i > m_cLinks)
			ALERT(at_error, "Bad link!\n");
#endif
		return m_pLinkPool[i];
	}

	inline CLink& NodeLink(int iNode, int iLink)
	{
		return Link(Node(iNode).m_iFirstLink + iLink);
	}

	inline CLink& NodeLink(const CNode& node, int iLink)
	{
		return Link(node.m_iFirstLink + iLink);
	}

	inline  int	DestNodeLink(int iNode, int iLink)
	{
		return NodeLink(iNode, iLink).m_iDestNode;
	}

#if 0
	inline CNode& SourceNode(int iNode, int iLink)
	{
		return Node(NodeLink(iNode, iLink).m_iSrcNode);
	}

	inline CNode& DestNode(int iNode, int iLink)
	{
		return Node(NodeLink(iNode, iLink).m_iDestNode);
	}

	inline	CNode* PNodeLink(int iNode, int iLink)
	{
		return &DestNode(iNode, iLink);
	}
#endif
};

/**
*	@brief Nodes start out as ents in the level. The node graph  is built, then these ents are discarded.
*/
class CNodeEnt : public CBaseEntity
{
	void Spawn() override;

	/**
	*	@brief nodes start out as ents in the world. As they are spawned, the node info is recorded then the ents are discarded.
	*/
	void KeyValue(KeyValueData* pkvd) override;
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	short m_sHintType;
	short m_sHintActivity;
};

/**
*	@brief these MUST coincide with the HINTS listed under info_node in the FGD file!
*/
enum
{
	HINT_NONE = 0,
	HINT_WORLD_DOOR,
	HINT_WORLD_WINDOW,
	HINT_WORLD_BUTTON,
	HINT_WORLD_MACHINERY,
	HINT_WORLD_LEDGE,
	HINT_WORLD_LIGHT_SOURCE,
	HINT_WORLD_HEAT_SOURCE,
	HINT_WORLD_BLINKING_LIGHT,
	HINT_WORLD_BRIGHT_COLORS,
	HINT_WORLD_HUMAN_BLOOD,
	HINT_WORLD_ALIEN_BLOOD,

	HINT_TACTICAL_EXIT = 100,
	HINT_TACTICAL_VANTAGE,
	HINT_TACTICAL_AMBUSH,

	HINT_STUKA_PERCH = 300,
	HINT_STUKA_LANDING,
};

inline CGraph WorldGraph;
