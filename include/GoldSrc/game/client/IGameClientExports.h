//========= Copyright � 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include <string_view>

#include "interface.h"

/**
*	@brief Exports a set of functions for the GameUI interface to interact with the game client
*/
class IGameClientExports : public IBaseInterface
{
public:
	// returns the name of the server the user is connected to, if any
	virtual const char *GetServerHostName() = 0;

	// ingame voice manipulation
	virtual bool IsPlayerGameVoiceMuted(int playerIndex) = 0;
	virtual void MutePlayerGameVoice(int playerIndex) = 0;
	virtual void UnmutePlayerGameVoice(int playerIndex) = 0;
};

constexpr std::string_view GAMECLIENTEXPORTS_INTERFACE_VERSION{"GameClientExports001"};
