/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#pragma once

/**
*	@brief Handles core server actions
*/
class CServerLibrary final
{
public:
	CServerLibrary() = default;
	~CServerLibrary() = default;

	CServerLibrary(const CServerLibrary&) = delete;
	CServerLibrary& operator=(const CServerLibrary&) = delete;

	void MapIsEnding()
	{
		m_isStartingNewMap = true;
	}

	void CheckForNewMapStart()
	{
		if (m_isStartingNewMap)
		{
			m_isStartingNewMap = false;
			NewMapStarted();
		}
	}

private:
	void NewMapStarted();

private:
	bool m_isStartingNewMap = true;
};

inline CServerLibrary g_Server;
