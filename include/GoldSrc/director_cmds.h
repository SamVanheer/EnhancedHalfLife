//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// sub commands for svc_director

#pragma once

constexpr int DRC_ACTIVE = 0;	//!< tells client that he's an spectator and will get director command
constexpr int DRC_STATUS = 1;	//!< send status infos about proxy 
constexpr int DRC_CAMERA = 2;	//!< set the actual director camera position
constexpr int DRC_EVENT = 3;	//!< informs the dircetor about ann important game event

// director command types:
constexpr int DRC_CMD_NONE = 0;			// NULL director command
constexpr int DRC_CMD_START = 1;		// start director mode
constexpr int DRC_CMD_EVENT = 2;		// informs about director command
constexpr int DRC_CMD_MODE = 3;			// switches camera modes
constexpr int DRC_CMD_CAMERA = 4;		// set fixed camera
constexpr int DRC_CMD_TIMESCALE = 5;	// sets time scale
constexpr int DRC_CMD_MESSAGE = 6;		// send HUD centerprint
constexpr int DRC_CMD_SOUND = 7;		// plays a particular sound
constexpr int DRC_CMD_STATUS = 8;		// HLTV broadcast status
constexpr int DRC_CMD_BANNER = 9;		// set GUI banner
constexpr int DRC_CMD_STUFFTEXT = 10;	// like the normal svc_stufftext but as director command
constexpr int DRC_CMD_CHASE = 11;		// chase a certain player
constexpr int DRC_CMD_INEYE = 12;		// view player through own eyes
constexpr int DRC_CMD_MAP = 13;			// show overview map
constexpr int DRC_CMD_CAMPATH = 14;		// define camera waypoint
constexpr int DRC_CMD_WAYPOINTS = 15;	// start moving camera, inetranl message

constexpr int DRC_CMD_LAST = DRC_CMD_WAYPOINTS;

// DRC_CMD_EVENT event flags
constexpr int DRC_FLAG_PRIO_MASK = 0x0F;		// priorities between 0 and 15 (15 most important)
constexpr int DRC_FLAG_SIDE = 1 << 4;			// 
constexpr int DRC_FLAG_DRAMATIC = 1 << 5;		// is a dramatic scene
constexpr int DRC_FLAG_SLOWMOTION = 1 << 6;		// would look good in SloMo
constexpr int DRC_FLAG_FACEPLAYER = 1 << 7;		// player is doning something (reload/defuse bomb etc)
constexpr int DRC_FLAG_INTRO = 1 << 8;			// is a introduction scene
constexpr int DRC_FLAG_FINAL = 1 << 9;			// is a final scene
constexpr int DRC_FLAG_NO_RANDOM = 1 << 10;		// don't randomize event data

// DRC_CMD_WAYPOINT flags
constexpr int DRC_FLAG_STARTPATH = 1;	// end with speed 0.0
constexpr int DRC_FLAG_SLOWSTART = 2;	// start with speed 0.0
constexpr int DRC_FLAG_SLOWEND = 4;		// end with speed 0.0

// commands of the director API function CallDirectorProc(...)

constexpr int DRCAPI_NOP = 0;				//!< no operation
constexpr int DRCAPI_ACTIVE = 1;			//!< de/acivates director mode in engine
constexpr int DRCAPI_STATUS = 2;			//!< request proxy information
constexpr int DRCAPI_SETCAM = 3;			//!< set camera n to given position and angle
constexpr int DRCAPI_GETCAM = 4;			//!< request camera n position and angle
constexpr int DRCAPI_DIRPLAY = 5;			//!< set director time and play with normal speed
constexpr int DRCAPI_DIRFREEZE = 6;			//!< freeze directo at this time
constexpr int DRCAPI_SETVIEWMODE = 7;		//!< overview or 4 cameras 
constexpr int DRCAPI_SETOVERVIEWPARAMS = 8;	//!< sets parameter for overview mode
constexpr int DRCAPI_SETFOCUS = 9;			//!< set the camera which has the input focus
constexpr int DRCAPI_GETTARGETS = 10;		//!< queries engine for player list
constexpr int DRCAPI_SETVIEWPOINTS = 11;	//!< gives engine all waypoints
