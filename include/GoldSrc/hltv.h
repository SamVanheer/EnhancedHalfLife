#pragma once

/**
*	@file
* 
*	all shared consts between server, clients and proxy
*/

#include "director_cmds.h"

constexpr int TYPE_CLIENT = 0;		//!< client is a normal HL client (default)
constexpr int TYPE_PROXY = 1;		//!< client is another proxy
constexpr int TYPE_COMMENTATOR = 3;	//!< client is a commentator
constexpr int TYPE_DEMO = 4;		//!< client is a demo file

// sub commands of svc_hltv:
constexpr int HLTV_ACTIVE = 0;		//!< tells client that he's an spectator and will get director commands
constexpr int HLTV_STATUS = 1;		//!< send status infos about proxy 
constexpr int HLTV_LISTEN = 2;		//!< tell client to listen to a multicast stream
