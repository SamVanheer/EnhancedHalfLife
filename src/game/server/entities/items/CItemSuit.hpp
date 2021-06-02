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
#include "CItemSuit.generated.hpp"

enum class SuitLogonType
{
	NoLogon = 0,
	LongLogon,
	ShortLogon,
};

class EHL_CLASS(EntityName=item_suit) CItemSuit : public CItem
{
	EHL_GENERATED_BODY()

	void OnConstruct() override
	{
		CItem::OnConstruct();
		SetModelName("models/w_suit.mdl");
	}

	void KeyValue(KeyValueData* pkvd) override
	{
		if (AreStringsEqual(pkvd->szKeyName, "logon_type"))
		{
			if (AreStringsEqual(pkvd->szValue, "NoLogon"))
			{
				m_LogonType = SuitLogonType::NoLogon;
			}
			else if (AreStringsEqual(pkvd->szValue, "LongLogon"))
			{
				m_LogonType = SuitLogonType::LongLogon;
			}
			else if (AreStringsEqual(pkvd->szValue, "ShortLogon"))
			{
				m_LogonType = SuitLogonType::ShortLogon;
			}
			else
			{
				ALERT(at_warning, "Invalid logon_type value \"%s\" for \"%s\" (entity index %d)\n", pkvd->szValue, GetClassname(), entindex());
			}

			pkvd->fHandled = true;
		}
		else
		{
			CItem::KeyValue(pkvd);
		}
	}

	ItemApplyResult Apply(CBasePlayer* pPlayer) override
	{
		if (pPlayer->HasSuit())
			return {ItemApplyAction::NotUsed};

		switch (m_LogonType)
		{
		case SuitLogonType::NoLogon: break;
		case SuitLogonType::LongLogon:
			EMIT_SOUND_SUIT(pPlayer, "!HEV_AAx");
			break;
		case SuitLogonType::ShortLogon:
			EMIT_SOUND_SUIT(pPlayer, "!HEV_A0");
			break;
		}

		pPlayer->SetHasSuit(true);
		return {ItemApplyAction::Used};
	}

protected:
	EHL_FIELD(Persisted)
	SuitLogonType m_LogonType = SuitLogonType::NoLogon;
};
