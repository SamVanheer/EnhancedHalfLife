//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "Platform.h"
#include "voice_banmgr.h"
#include "filesystem_shared.hpp"

constexpr int BANMGR_FILEVERSION = 1;
char const *g_pBanMgrFilename = "voice_ban.dt";



// Hash a player ID to a byte.
unsigned char HashPlayerID(char const playerID[16])
{
	unsigned char curHash = 0;

	for(int i=0; i < 16; i++)
		curHash += (unsigned char)playerID[i];

	return curHash;
}



CVoiceBanMgr::CVoiceBanMgr()
{
	Clear();
}


CVoiceBanMgr::~CVoiceBanMgr()
{
	Term();
}


bool CVoiceBanMgr::Init()
{
	Term();

	// Load in the squelch file.
	auto fileHandle = g_pFileSystem->Open(g_pBanMgrFilename, "rb", "GAMECONFIG");

	if(fileHandle != FILESYSTEM_INVALID_HANDLE)
	{
		int version;
		if (sizeof(version) == g_pFileSystem->Read(&version, sizeof(version), fileHandle))
		{
			if (version == BANMGR_FILEVERSION)
			{
				g_pFileSystem->Seek(fileHandle, 0, FILESYSTEM_SEEK_TAIL);
				int nIDs = (g_pFileSystem->Tell(fileHandle) - sizeof(version)) / 16;
				g_pFileSystem->Seek(fileHandle, sizeof(version), FILESYSTEM_SEEK_HEAD);

				for (int i = 0; i < nIDs; i++)
				{
					char playerID[16];
					if (sizeof(playerID) == g_pFileSystem->Read(playerID, sizeof(playerID), fileHandle))
					{
						AddBannedPlayer(playerID);
					}
				}
			}
		}

		g_pFileSystem->Close(fileHandle);
	}

	return true;
}


void CVoiceBanMgr::Term()
{
	// Free all the player structures.
	for(int i=0; i < 256; i++)
	{
		BannedPlayer *pListHead = &m_PlayerHash[i];
		BannedPlayer *pNext;
		for(BannedPlayer *pCur=pListHead->m_pNext; pCur != pListHead; pCur=pNext)
		{
			pNext = pCur->m_pNext;
			delete pCur;
		}
	}

	Clear();
}


void CVoiceBanMgr::SaveState()
{
	// Save the file out.
	auto fileHandle = g_pFileSystem->Open(g_pBanMgrFilename, "wb", "GAMECONFIG");

	if(fileHandle != FILESYSTEM_INVALID_HANDLE)
	{
		int version = BANMGR_FILEVERSION;
		g_pFileSystem->Write(&version, sizeof(version), fileHandle);

		for(int i=0; i < 256; i++)
		{
			BannedPlayer *pListHead = &m_PlayerHash[i];
			for(BannedPlayer *pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
			{
				g_pFileSystem->Write(pCur->m_PlayerID, 16, fileHandle);
			}
		}

		g_pFileSystem->Close(fileHandle);
	}
}


bool CVoiceBanMgr::GetPlayerBan(char const playerID[16])
{
	return !!InternalFindPlayerSquelch(playerID);
}


void CVoiceBanMgr::SetPlayerBan(char const playerID[16], bool bSquelch)
{
	if(bSquelch)
	{
		// Is this guy already squelched?
		if(GetPlayerBan(playerID))
			return;
	
		AddBannedPlayer(playerID);
	}
	else
	{
		BannedPlayer *pPlayer = InternalFindPlayerSquelch(playerID);
		if(pPlayer)
		{
			pPlayer->m_pPrev->m_pNext = pPlayer->m_pNext;
			pPlayer->m_pNext->m_pPrev = pPlayer->m_pPrev;
			delete pPlayer;
		}
	}
}


void CVoiceBanMgr::ForEachBannedPlayer(void (*callback)(char id[16]))
{
	for(int i=0; i < 256; i++)
	{
		for(BannedPlayer *pCur=m_PlayerHash[i].m_pNext; pCur != &m_PlayerHash[i]; pCur=pCur->m_pNext)
		{
			callback(pCur->m_PlayerID);
		}
	}
}


void CVoiceBanMgr::Clear()
{
	// Tie off the hash table entries.
	for(int i=0; i < 256; i++)
		m_PlayerHash[i].m_pNext = m_PlayerHash[i].m_pPrev = &m_PlayerHash[i];
}


CVoiceBanMgr::BannedPlayer* CVoiceBanMgr::InternalFindPlayerSquelch(char const playerID[16])
{
	int index = HashPlayerID(playerID);
	BannedPlayer *pListHead = &m_PlayerHash[index];
	for(BannedPlayer *pCur=pListHead->m_pNext; pCur != pListHead; pCur=pCur->m_pNext)
	{
		if(memcmp(playerID, pCur->m_PlayerID, 16) == 0)
			return pCur;
	}

	return nullptr;
}


CVoiceBanMgr::BannedPlayer* CVoiceBanMgr::AddBannedPlayer(char const playerID[16])
{
	BannedPlayer *pNew = new BannedPlayer;
	if(!pNew)
		return nullptr;

	int index = HashPlayerID(playerID);
	memcpy(pNew->m_PlayerID, playerID, 16);
	pNew->m_pNext = &m_PlayerHash[index];
	pNew->m_pPrev = m_PlayerHash[index].m_pPrev;
	pNew->m_pPrev->m_pNext = pNew->m_pNext->m_pPrev = pNew;
	return pNew;
}

