//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include "voice_common.hpp"

class CGameRules;
class CBasePlayer;
struct edict_t;

class IVoiceGameMgrHelper
{
public:
	virtual				~IVoiceGameMgrHelper() {}

	/**
	*	@brief Called each frame to determine which players are allowed to hear each other.
	*	This overrides whatever squelch settings players have.
	*/
	virtual bool		CanPlayerHearPlayer(CBasePlayer* pListener, CBasePlayer* pTalker) = 0;
};

/**
*	@brief manages which clients can hear which other clients.
*/
class CVoiceGameMgr
{
public:
	CVoiceGameMgr() = default;
	virtual ~CVoiceGameMgr() = default;

	bool				Init(
		IVoiceGameMgrHelper* m_pHelper,
		int maxClients
	);

	void				SetHelper(IVoiceGameMgrHelper* pHelper);

	/**
	*	@brief Updates which players can hear which other players.
	*
	*	@details If gameplay mode is DM, then only players within the PVS can hear each other.
	*	If gameplay mode is teamplay, then only players on the same team can hear each other.
	*	Player masks are always applied.
	*/
	void				Update(double frametime);

	/**
	*	@brief Called when a new client connects (unsquelches its entity for everyone).
	*/
	void				ClientConnected(edict_t* pEdict);

	/**
	*	@brief Called on ClientCommand. Checks for the squelch and unsquelch commands.
	*	@return true if it handled the command.
	*/
	bool				ClientCommand(CBasePlayer* pPlayer, const char* cmd);

	/**
	*	@brief Called to determine if the Receiver has muted (blocked) the Sender
	*	@return true if the receiver has blocked the sender
	*/
	bool				PlayerHasBlockedPlayer(CBasePlayer* pReceiver, CBasePlayer* pSender);

private:

	/**
	*	@brief Force it to update the client masks.
	*/
	void				UpdateMasks();

private:
	int m_msgPlayerVoiceMask = 0;
	int m_msgRequestState = 0;

	IVoiceGameMgrHelper* m_pHelper = nullptr;
	int m_nMaxPlayers = 0;
	double m_UpdateInterval = 0;		//!< How long since the last update.
};

inline CVoiceGameMgr g_VoiceGameMgr;
