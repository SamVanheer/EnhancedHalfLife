//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

/**
*	@brief This class manages the (persistent) list of squelched players.
*/
class CVoiceBanMgr
{
public:

	CVoiceBanMgr();
	~CVoiceBanMgr();

	/**
	*	@brief Init loads the list of squelched players from disk.
	*/
	bool		Init();
	void		Term();

	/**
	*	@brief Saves the state into voice_squelch.dt.
	*/
	void		SaveState();

	bool		GetPlayerBan(char const playerID[16]);
	void		SetPlayerBan(char const playerID[16], bool bSquelch);

	/**
	*	@brief Call your callback for each banned player.
	*/
	template<typename Callback>
	void ForEachBannedPlayer(Callback callback)
	{
		for (int i = 0; i < 256; i++)
		{
			for (BannedPlayer* pCur = m_PlayerHash[i].m_pNext; pCur != &m_PlayerHash[i]; pCur = pCur->m_pNext)
			{
				callback(pCur->m_PlayerID);
			}
		}
	}

protected:

	class BannedPlayer
	{
	public:
		char			m_PlayerID[16];
		BannedPlayer* m_pPrev, * m_pNext;
	};

	void				Clear();
	BannedPlayer* InternalFindPlayerSquelch(char const playerID[16]);
	BannedPlayer* AddBannedPlayer(char const playerID[16]);

protected:

	BannedPlayer	m_PlayerHash[256];
};
