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

#include "CBasePlayer.hpp"
#include "CItem.hpp"
#include "UserMessages.h"

class EHL_CLASS() CItemLongJump : public CItem
{
	void OnConstruct() override
	{
		CItem::OnConstruct();
		SetModelName("models/w_longjump.mdl");
	}

	void KeyValue(KeyValueData* pkvd) override
	{
		if (AreStringsEqual(pkvd->szKeyName, "play_suit_sentence"))
		{
			m_PlaySuitSentence = atoi(pkvd->szValue) != 0;
			pkvd->fHandled = true;
		}
		else
		{
			CItem::KeyValue(pkvd);
		}
	}

	ItemApplyResult Apply(CBasePlayer* pPlayer) override
	{
		if (pPlayer->m_fLongJump)
		{
			return {ItemApplyAction::NotUsed};
		}

		if (pPlayer->HasSuit())
		{
			pPlayer->SetHasLongJump(true);// player now has longjump module

			MESSAGE_BEGIN(MessageDest::One, gmsgItemPickup, pPlayer);
			WRITE_STRING(GetClassname());
			MESSAGE_END();

			if (m_PlaySuitSentence)
			{
				EMIT_SOUND_SUIT(pPlayer, "!HEV_A1");	// Play the longjump sound UNDONE: Kelly? correct sound?
			}

			return {ItemApplyAction::Used};
		}
		return {ItemApplyAction::NotUsed};
	}

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

protected:
	bool m_PlaySuitSentence = true;
};
