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

#include "cbase.h"
#include "player.h"
#include "weapons.h"

void CBasePlayer::SelectLastItem()
{
	if (!m_hLastItem)
	{
		return;
	}

	auto activeItem = m_hActiveItem.Get();

	if (activeItem && !activeItem->CanHolster())
	{
		return;
	}

	ResetAutoaim();

	// FIX, this needs to queue them up and delay
	if (activeItem)
		activeItem->Holster();

	CBasePlayerItem* pTemp = activeItem;
	activeItem = m_hActiveItem = m_hLastItem;
	m_hLastItem = pTemp;
	activeItem->Deploy();
	activeItem->UpdateItemInfo();
}

void CBasePlayer::SelectItem(const char* pstr)
{
	if (!pstr)
		return;

	CBasePlayerItem* pItem = nullptr;

#ifndef CLIENT_DLL
	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		pItem = m_hPlayerItems[i];

		while (pItem)
		{
			if (pItem->ClassnameIs(pstr))
				break;
			pItem = pItem->m_hNext;
		}

		if (pItem)
			break;
	}
#endif

	if (!pItem)
		return;

	if (pItem == m_hActiveItem)
		return;

#ifndef CLIENT_DLL
	ResetAutoaim();
#endif

	// FIX, this needs to queue them up and delay
	if (auto activeItem = m_hActiveItem.Get(); activeItem)
		activeItem->Holster();

	m_hLastItem = m_hActiveItem;
	m_hActiveItem = pItem;

	if (auto activeItem = m_hActiveItem.Get(); activeItem)
	{
		activeItem->Deploy();
#ifndef CLIENT_DLL
		activeItem->UpdateItemInfo();
#endif
	}
}
