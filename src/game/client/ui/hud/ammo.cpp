/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "pm_shared.h"

#include "ammohistory.h"
#include "vgui_TeamFortressViewport.h"

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"

/**
*	@brief Finds and returns the matching sprite name 'psz' and resolution 'iRes' in the given sprite list 'pList'
*	@param iCount is the number of items in the pList
*/
client_sprite_t* GetSpriteList(client_sprite_t* pList, const char* psz, int iRes, int iCount);

WeaponsResource gWR;

int g_weaponselect = 0;

void WeaponsResource::LoadAllWeaponSprites()
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (rgWeapons[i].iId)
			LoadWeaponSprites(&rgWeapons[i]);
	}
}

int WeaponsResource::CountAmmo(int iId)
{
	if (iId < 0)
		return 0;

	return riAmmo[iId];
}

bool WeaponsResource::HasAmmo(WEAPON* p)
{
	if (!p)
		return false;

	// weapons with no max ammo can always be selected
	if (p->iMax1 == -1)
		return true;

	return (p->iAmmoType == -1) || p->iClip > 0 || CountAmmo(p->iAmmoType)
		|| CountAmmo(p->iAmmo2Type) || (p->iFlags & ITEM_FLAG_SELECTONEMPTY);
}

void WeaponsResource::LoadWeaponSprites(WEAPON* pWeapon)
{
	int i, iRes;

	if (ScreenWidth < 640)
		iRes = 320;
	else
		iRes = 640;

	char sz[256];

	if (!pWeapon)
		return;

	memset(&pWeapon->rcActive, 0, sizeof(wrect_t));
	memset(&pWeapon->rcInactive, 0, sizeof(wrect_t));
	memset(&pWeapon->rcAmmo, 0, sizeof(wrect_t));
	memset(&pWeapon->rcAmmo2, 0, sizeof(wrect_t));
	pWeapon->hInactive = 0;
	pWeapon->hActive = 0;
	pWeapon->hAmmo = 0;
	pWeapon->hAmmo2 = 0;

	snprintf(sz, sizeof(sz), "sprites/%s.txt", pWeapon->szName);
	client_sprite_t* pList = SPR_GetList(sz, &i);

	if (!pList)
		return;

	client_sprite_t* p;

	p = GetSpriteList(pList, "crosshair", iRes, i);
	if (p)
	{
		snprintf(sz, sizeof(sz), "sprites/%s.spr", p->szSprite);
		pWeapon->hCrosshair = SPR_Load(sz);
		pWeapon->rcCrosshair = p->rc;
	}
	else
		pWeapon->hCrosshair = 0;

	p = GetSpriteList(pList, "autoaim", iRes, i);
	if (p)
	{
		snprintf(sz, sizeof(sz), "sprites/%s.spr", p->szSprite);
		pWeapon->hAutoaim = SPR_Load(sz);
		pWeapon->rcAutoaim = p->rc;
	}
	else
		pWeapon->hAutoaim = 0;

	p = GetSpriteList(pList, "zoom", iRes, i);
	if (p)
	{
		snprintf(sz, sizeof(sz), "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedCrosshair = SPR_Load(sz);
		pWeapon->rcZoomedCrosshair = p->rc;
	}
	else
	{
		pWeapon->hZoomedCrosshair = pWeapon->hCrosshair; //default to non-zoomed crosshair
		pWeapon->rcZoomedCrosshair = pWeapon->rcCrosshair;
	}

	p = GetSpriteList(pList, "zoom_autoaim", iRes, i);
	if (p)
	{
		snprintf(sz, sizeof(sz), "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedAutoaim = SPR_Load(sz);
		pWeapon->rcZoomedAutoaim = p->rc;
	}
	else
	{
		pWeapon->hZoomedAutoaim = pWeapon->hZoomedCrosshair;  //default to zoomed crosshair
		pWeapon->rcZoomedAutoaim = pWeapon->rcZoomedCrosshair;
	}

	p = GetSpriteList(pList, "weapon", iRes, i);
	if (p)
	{
		snprintf(sz, sizeof(sz), "sprites/%s.spr", p->szSprite);
		pWeapon->hInactive = SPR_Load(sz);
		pWeapon->rcInactive = p->rc;

		gHR.iHistoryGap = std::max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
	}
	else
		pWeapon->hInactive = 0;

	p = GetSpriteList(pList, "weapon_s", iRes, i);
	if (p)
	{
		snprintf(sz, sizeof(sz), "sprites/%s.spr", p->szSprite);
		pWeapon->hActive = SPR_Load(sz);
		pWeapon->rcActive = p->rc;
	}
	else
		pWeapon->hActive = 0;

	p = GetSpriteList(pList, "ammo", iRes, i);
	if (p)
	{
		snprintf(sz, sizeof(sz), "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo = SPR_Load(sz);
		pWeapon->rcAmmo = p->rc;

		gHR.iHistoryGap = std::max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
	}
	else
		pWeapon->hAmmo = 0;

	p = GetSpriteList(pList, "ammo2", iRes, i);
	if (p)
	{
		snprintf(sz, sizeof(sz), "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo2 = SPR_Load(sz);
		pWeapon->rcAmmo2 = p->rc;

		gHR.iHistoryGap = std::max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
	}
	else
		pWeapon->hAmmo2 = 0;

}

WEAPON* WeaponsResource::GetFirstPos(int iSlot)
{
	WEAPON* pret = nullptr;

	for (int i = 0; i < MAX_WEAPON_POSITIONS; i++)
	{
		if (rgSlots[iSlot][i] && HasAmmo(rgSlots[iSlot][i]))
		{
			pret = rgSlots[iSlot][i];
			break;
		}
	}

	return pret;
}

WEAPON* WeaponsResource::GetNextActivePos(int iSlot, int iSlotPos)
{
	if (iSlotPos >= MAX_WEAPON_POSITIONS || iSlot >= MAX_WEAPON_SLOTS)
		return nullptr;

	WEAPON* p = gWR.rgSlots[iSlot][iSlotPos + 1];

	if (!p || !gWR.HasAmmo(p))
		return GetNextActivePos(iSlot, iSlotPos + 1);

	return p;
}

int giBucketHeight, giBucketWidth, giABHeight, giABWidth; // Ammo Bar width and height

HSPRITE ghsprBuckets;					// Sprite for top row of weapons menu

DECLARE_MESSAGE(m_Ammo, CurWeapon);	// Current weapon and clip
DECLARE_MESSAGE(m_Ammo, WeaponList);	// new weapon type
DECLARE_MESSAGE(m_Ammo, AmmoX);			// update known ammo type's count
DECLARE_MESSAGE(m_Ammo, AmmoPickup);	// flashes an ammo pickup record
DECLARE_MESSAGE(m_Ammo, WeapPickup);    // flashes a weapon pickup record
DECLARE_MESSAGE(m_Ammo, HideWeapon);	// hides the weapon, ammo, and crosshair displays temporarily
DECLARE_MESSAGE(m_Ammo, ItemPickup);

DECLARE_COMMAND(m_Ammo, Slot1);
DECLARE_COMMAND(m_Ammo, Slot2);
DECLARE_COMMAND(m_Ammo, Slot3);
DECLARE_COMMAND(m_Ammo, Slot4);
DECLARE_COMMAND(m_Ammo, Slot5);
DECLARE_COMMAND(m_Ammo, Slot6);
DECLARE_COMMAND(m_Ammo, Slot7);
DECLARE_COMMAND(m_Ammo, Slot8);
DECLARE_COMMAND(m_Ammo, Slot9);
DECLARE_COMMAND(m_Ammo, Slot10);
DECLARE_COMMAND(m_Ammo, Close);
DECLARE_COMMAND(m_Ammo, NextWeapon);
DECLARE_COMMAND(m_Ammo, PrevWeapon);

// width of ammo fonts
constexpr int AMMO_SMALL_WIDTH = 10;
constexpr int AMMO_LARGE_WIDTH = 20;

constexpr std::string_view HISTORY_DRAW_TIME{"5"};

bool CHudAmmo::Init()
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(CurWeapon);
	HOOK_MESSAGE(WeaponList);
	HOOK_MESSAGE(AmmoPickup);
	HOOK_MESSAGE(WeapPickup);
	HOOK_MESSAGE(ItemPickup);
	HOOK_MESSAGE(HideWeapon);
	HOOK_MESSAGE(AmmoX);

	HOOK_COMMAND("slot1", Slot1);
	HOOK_COMMAND("slot2", Slot2);
	HOOK_COMMAND("slot3", Slot3);
	HOOK_COMMAND("slot4", Slot4);
	HOOK_COMMAND("slot5", Slot5);
	HOOK_COMMAND("slot6", Slot6);
	HOOK_COMMAND("slot7", Slot7);
	HOOK_COMMAND("slot8", Slot8);
	HOOK_COMMAND("slot9", Slot9);
	HOOK_COMMAND("slot10", Slot10);
	HOOK_COMMAND("cancelselect", Close);
	HOOK_COMMAND("invnext", NextWeapon);
	HOOK_COMMAND("invprev", PrevWeapon);

	Reset();

	CVAR_CREATE("hud_drawhistory_time", HISTORY_DRAW_TIME.data(), 0);
	CVAR_CREATE("hud_fastswitch", "0", FCVAR_ARCHIVE);		// controls whether or not weapons can be selected in one keypress

	m_iFlags |= HUD_ACTIVE; //!!!

	gWR.Init();
	gHR.Init();

	return true;
};

void CHudAmmo::Reset()
{
	m_fFade = 0;
	m_iFlags |= HUD_ACTIVE; //!!!

	m_ActiveSel = {};
	gHUD.m_iHideHUDDisplay = 0;

	gWR.Reset();
	gHR.Reset();
}

bool CHudAmmo::VidInit()
{
	// Load sprites for buckets (top row of weapon menu)
	m_HUD_bucket0 = gHUD.GetSpriteIndex("bucket1");
	m_HUD_selection = gHUD.GetSpriteIndex("selection");

	ghsprBuckets = gHUD.GetSprite(m_HUD_bucket0);
	giBucketWidth = gHUD.GetSpriteRect(m_HUD_bucket0).right - gHUD.GetSpriteRect(m_HUD_bucket0).left;
	giBucketHeight = gHUD.GetSpriteRect(m_HUD_bucket0).bottom - gHUD.GetSpriteRect(m_HUD_bucket0).top;

	gHR.iHistoryGap = std::max(gHR.iHistoryGap, gHUD.GetSpriteRect(m_HUD_bucket0).bottom - gHUD.GetSpriteRect(m_HUD_bucket0).top);

	// If we've already loaded weapons, let's get new sprites
	gWR.LoadAllWeaponSprites();

	if (ScreenWidth >= 640)
	{
		giABWidth = 20;
		giABHeight = 4;
	}
	else
	{
		giABWidth = 10;
		giABHeight = 2;
	}

	return true;
}

void CHudAmmo::Think()
{
	if (gHUD.m_fPlayerDead)
		return;

	if (gHUD.m_iWeaponBits != gWR.iOldWeaponBits)
	{
		gWR.iOldWeaponBits = gHUD.m_iWeaponBits;

		for (int i = MAX_WEAPONS - 1; i > 0; i--)
		{
			WEAPON* p = gWR.GetWeapon(i);

			if (p)
			{
				if (gHUD.m_iWeaponBits & (1 << p->iId))
					gWR.PickupWeapon(p);
				else
					gWR.DropWeapon(p);
			}
		}
	}

	if (m_ActiveSel.Type == SelectionType::Off)
		return;

	// has the player selected one?
	if (gHUD.m_iKeyBits & IN_ATTACK)
	{
		if (m_ActiveSel.Type != SelectionType::MenuBar)
		{
			ServerCmd(m_ActiveSel.Selection->szName);
			g_weaponselect = m_ActiveSel.Selection->iId;
		}

		m_LastSel = m_ActiveSel;
		m_ActiveSel = {};
		gHUD.m_iKeyBits &= ~IN_ATTACK;

		PlaySound("common/wpn_select.wav", 1);
	}

}

HSPRITE* WeaponsResource::GetAmmoPicFromWeapon(int iAmmoId, wrect_t& rect)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (rgWeapons[i].iAmmoType == iAmmoId)
		{
			rect = rgWeapons[i].rcAmmo;
			return &rgWeapons[i].hAmmo;
		}
		else if (rgWeapons[i].iAmmo2Type == iAmmoId)
		{
			rect = rgWeapons[i].rcAmmo2;
			return &rgWeapons[i].hAmmo2;
		}
	}

	return nullptr;
}

void CHudAmmo::SelectSlot(int iSlot, bool fAdvance, int iDirection)
{
	if (gHUD.m_Menu.IsMenuDisplayed() && (!fAdvance) && (iDirection == 1))
	{ // menu is overriding slot use commands
		gHUD.m_Menu.SelectMenuItem(iSlot + 1);  // slots are one off the key numbers
		return;
	}

	if (iSlot > MAX_WEAPON_SLOTS)
		return;

	if (gHUD.m_fPlayerDead || gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL))
		return;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return;

	if (!(gHUD.m_iWeaponBits & ~(1 << (WEAPON_SUIT))))
		return;

	WEAPON* p = nullptr;
	bool fastSwitch = CVAR_GET_FLOAT("hud_fastswitch") != 0;

	if ((m_ActiveSel.Type != SelectionType::ActiveWeapon) || (iSlot != m_ActiveSel.Selection->iSlot))
	{
		PlaySound("common/wpn_hudon.wav", 1);
		p = gWR.GetFirstPos(iSlot);

		if (p && fastSwitch) // check for fast weapon switch mode
		{
			// if fast weapon switch is on, then weapons can be selected in a single keypress
			// but only if there is only one item in the bucket
			WEAPON* p2 = gWR.GetNextActivePos(p->iSlot, p->iSlotPos);
			if (!p2)
			{	// only one active item in bucket, so change directly to weapon
				ServerCmd(p->szName);
				g_weaponselect = p->iId;
				return;
			}
		}
	}
	else
	{
		PlaySound("common/wpn_moveselect.wav", 1);
		if (m_ActiveSel.Type == SelectionType::ActiveWeapon)
			p = gWR.GetNextActivePos(m_ActiveSel.Selection->iSlot, m_ActiveSel.Selection->iSlotPos);
		if (!p)
			p = gWR.GetFirstPos(iSlot);
	}


	if (!p)  // no selection found
	{
		// just display the weapon list, unless fastswitch is on just ignore it
		if (!fastSwitch)
			m_ActiveSel.Type = SelectionType::MenuBar;
		else
			m_ActiveSel.Type = SelectionType::Off;

		m_ActiveSel.Selection = nullptr;
	}
	else
	{
		m_ActiveSel.Type = SelectionType::ActiveWeapon;
		m_ActiveSel.Selection = p;
	}
}

bool CHudAmmo::MsgFunc_AmmoX(const char* pszName, int iSize, void* pbuf)
{
	BufferReader reader{pbuf, iSize};

	int iIndex = reader.ReadByte();
	int iCount = reader.ReadByte();

	gWR.SetAmmo(iIndex, abs(iCount));

	return true;
}

bool CHudAmmo::MsgFunc_AmmoPickup(const char* pszName, int iSize, void* pbuf)
{
	BufferReader reader{pbuf, iSize};
	int iIndex = reader.ReadByte();
	int iCount = reader.ReadByte();

	// Add ammo to the history
	gHR.AddToHistory(HISTSLOT_AMMO, iIndex, abs(iCount));

	return true;
}

bool CHudAmmo::MsgFunc_WeapPickup(const char* pszName, int iSize, void* pbuf)
{
	BufferReader reader{pbuf, iSize};
	int iIndex = reader.ReadByte();

	// Add the weapon to the history
	gHR.AddToHistory(HISTSLOT_WEAP, iIndex);

	return true;
}

bool CHudAmmo::MsgFunc_ItemPickup(const char* pszName, int iSize, void* pbuf)
{
	BufferReader reader{pbuf, iSize};
	const char* szName = reader.ReadString();

	// Add the weapon to the history
	gHR.AddToHistory(HISTSLOT_ITEM, szName);

	return true;
}

bool CHudAmmo::MsgFunc_HideWeapon(const char* pszName, int iSize, void* pbuf)
{
	BufferReader reader{pbuf, iSize};

	gHUD.m_iHideHUDDisplay = reader.ReadByte();

	if (gEngfuncs.IsSpectateOnly())
		return true;

	if (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL))
	{
		static wrect_t nullrc;
		m_ActiveSel = {};
		SetCrosshair(0, nullrc, 0, 0, 0);
	}
	else
	{
		if (m_pWeapon)
			SetCrosshair(m_pWeapon->hCrosshair, m_pWeapon->rcCrosshair, 255, 255, 255);
	}

	return true;
}

bool CHudAmmo::MsgFunc_CurWeapon(const char* pszName, int iSize, void* pbuf)
{
	static wrect_t nullrc;

	BufferReader reader{pbuf, iSize};

	const WeaponState state = static_cast<WeaponState>(reader.ReadByte());
	int iId = reader.ReadChar();
	int iClip = reader.ReadChar();

	// detect if we're also on target
	const bool fOnTarget = state == WeaponState::OnTarget;

	if (iId < 1)
	{
		SetCrosshair(0, nullrc, 0, 0, 0);
		m_pWeapon = nullptr;
		return false;
	}

	if (g_iUser1 != OBS_IN_EYE)
	{
		// Is player dead???
		if ((iId == -1) && (iClip == -1))
		{
			gHUD.m_fPlayerDead = true;
			m_ActiveSel = {};
			return true;
		}
		gHUD.m_fPlayerDead = false;
	}

	WEAPON* pWeapon = gWR.GetWeapon(iId);

	if (!pWeapon)
		return false;

	if (iClip < -1)
		pWeapon->iClip = abs(iClip);
	else
		pWeapon->iClip = iClip;


	if (state == WeaponState::NotActive)	// we're not the current weapon, so update no more
		return true;

	m_pWeapon = pWeapon;

	if (gHUD.m_iFOV >= 90)
	{ // normal crosshairs
		if (fOnTarget && m_pWeapon->hAutoaim)
			SetCrosshair(m_pWeapon->hAutoaim, m_pWeapon->rcAutoaim, 255, 255, 255);
		else
			SetCrosshair(m_pWeapon->hCrosshair, m_pWeapon->rcCrosshair, 255, 255, 255);
	}
	else
	{ // zoomed crosshairs
		if (fOnTarget && m_pWeapon->hZoomedAutoaim)
			SetCrosshair(m_pWeapon->hZoomedAutoaim, m_pWeapon->rcZoomedAutoaim, 255, 255, 255);
		else
			SetCrosshair(m_pWeapon->hZoomedCrosshair, m_pWeapon->rcZoomedCrosshair, 255, 255, 255);

	}

	m_fFade = 200.0f; //!!!
	m_iFlags |= HUD_ACTIVE;

	return true;
}

bool CHudAmmo::MsgFunc_WeaponList(const char* pszName, int iSize, void* pbuf)
{
	BufferReader reader{pbuf, iSize};

	WEAPON Weapon;

	safe_strcpy(Weapon.szName, reader.ReadString());
	Weapon.iAmmoType = (int)reader.ReadChar();

	Weapon.iMax1 = reader.ReadByte();
	if (Weapon.iMax1 == 255)
		Weapon.iMax1 = -1;

	Weapon.iAmmo2Type = reader.ReadChar();
	Weapon.iMax2 = reader.ReadByte();
	if (Weapon.iMax2 == 255)
		Weapon.iMax2 = -1;

	Weapon.iSlot = reader.ReadChar();
	Weapon.iSlotPos = reader.ReadChar();
	Weapon.iId = reader.ReadChar();
	Weapon.iFlags = reader.ReadByte();
	Weapon.iClip = 0;

	gWR.AddWeapon(&Weapon);

	return true;

}

void CHudAmmo::SlotInput(int iSlot)
{
	if (gViewPort && gViewPort->SlotInput(iSlot))
		return;

	SelectSlot(iSlot, false, 1);
}

void CHudAmmo::UserCmd_Slot1()
{
	SlotInput(0);
}

void CHudAmmo::UserCmd_Slot2()
{
	SlotInput(1);
}

void CHudAmmo::UserCmd_Slot3()
{
	SlotInput(2);
}

void CHudAmmo::UserCmd_Slot4()
{
	SlotInput(3);
}

void CHudAmmo::UserCmd_Slot5()
{
	SlotInput(4);
}

void CHudAmmo::UserCmd_Slot6()
{
	SlotInput(5);
}

void CHudAmmo::UserCmd_Slot7()
{
	SlotInput(6);
}

void CHudAmmo::UserCmd_Slot8()
{
	SlotInput(7);
}

void CHudAmmo::UserCmd_Slot9()
{
	SlotInput(8);
}

void CHudAmmo::UserCmd_Slot10()
{
	SlotInput(9);
}

void CHudAmmo::UserCmd_Close()
{
	if (m_ActiveSel.Type != SelectionType::Off)
	{
		m_LastSel = m_ActiveSel;
		m_ActiveSel = {};
		PlaySound("common/wpn_hudoff.wav", 1);
	}
	else
		EngineClientCmd("escape");
}

void CHudAmmo::UserCmd_NextWeapon()
{
	if (gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)))
		return;

	if (m_ActiveSel.Type != SelectionType::ActiveWeapon)
	{
		m_ActiveSel.Type = m_pWeapon ? SelectionType::ActiveWeapon : SelectionType::Off;
		m_ActiveSel.Selection = m_pWeapon;
	}

	int pos = 0;
	int slot = 0;
	if (m_ActiveSel.Type == SelectionType::ActiveWeapon)
	{
		pos = m_ActiveSel.Selection->iSlotPos + 1;
		slot = m_ActiveSel.Selection->iSlot;
	}

	for (int loop = 0; loop <= 1; loop++)
	{
		for (; slot < MAX_WEAPON_SLOTS; slot++)
		{
			for (; pos < MAX_WEAPON_POSITIONS; pos++)
			{
				WEAPON* wsp = gWR.GetWeaponSlot(slot, pos);

				if (wsp && gWR.HasAmmo(wsp))
				{
					m_ActiveSel.Type = SelectionType::ActiveWeapon;
					m_ActiveSel.Selection = wsp;
					return;
				}
			}

			pos = 0;
		}

		slot = 0;  // start looking from the first slot again
	}

	m_ActiveSel = {};
}

void CHudAmmo::UserCmd_PrevWeapon()
{
	if (gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)))
		return;

	if (m_ActiveSel.Type != SelectionType::ActiveWeapon)
	{
		m_ActiveSel.Type = m_pWeapon ? SelectionType::ActiveWeapon : SelectionType::Off;
		m_ActiveSel.Selection = m_pWeapon;
	}

	int pos = MAX_WEAPON_POSITIONS - 1;
	int slot = MAX_WEAPON_SLOTS - 1;
	if (m_ActiveSel.Type == SelectionType::ActiveWeapon)
	{
		pos = m_ActiveSel.Selection->iSlotPos - 1;
		slot = m_ActiveSel.Selection->iSlot;
	}

	for (int loop = 0; loop <= 1; loop++)
	{
		for (; slot >= 0; slot--)
		{
			for (; pos >= 0; pos--)
			{
				WEAPON* wsp = gWR.GetWeaponSlot(slot, pos);

				if (wsp && gWR.HasAmmo(wsp))
				{
					m_ActiveSel.Type = SelectionType::ActiveWeapon;
					m_ActiveSel.Selection = wsp;
					return;
				}
			}

			pos = MAX_WEAPON_POSITIONS - 1;
		}

		slot = MAX_WEAPON_SLOTS - 1;
	}

	m_ActiveSel = {};
}

bool CHudAmmo::Draw(float flTime)
{
	int a, x, y, r, g, b;
	int AmmoWidth;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return true;

	if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)))
		return true;

	// Draw Weapon Menu
	DrawWList(flTime);

	// Draw ammo pickup history
	gHR.DrawAmmoHistory(flTime);

	if (!(m_iFlags & HUD_ACTIVE))
		return false;

	if (!m_pWeapon)
		return false;

	WEAPON* pw = m_pWeapon; // shorthand

	// SPR_Draw Ammo
	if ((pw->iAmmoType < 0) && (pw->iAmmo2Type < 0))
		return false;


	int iFlags = DHN_DRAWZERO; // draw 0 values

	AmmoWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;

	a = std::max(MIN_ALPHA, static_cast<int>(m_fFade));

	if (m_fFade > 0)
		m_fFade -= (gHUD.m_flTimeDelta * 20);

	UnpackRGB(r, g, b, RGB_YELLOWISH);

	ScaleColors(r, g, b, a);

	// Does this weapon have a clip?
	y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight / 2;

	// Does weapon have any ammo at all?
	if (m_pWeapon->iAmmoType > 0)
	{
		int iIconWidth = m_pWeapon->rcAmmo.right - m_pWeapon->rcAmmo.left;

		if (pw->iClip >= 0)
		{
			// room for the number and the '|' and the current ammo

			x = ScreenWidth - (8 * AmmoWidth) - iIconWidth;
			x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, pw->iClip, r, g, b);

			wrect_t rc;
			rc.top = 0;
			rc.left = 0;
			rc.right = AmmoWidth;
			rc.bottom = 100;

			int iBarWidth = AmmoWidth / 10;

			x += AmmoWidth / 2;

			UnpackRGB(r, g, b, RGB_YELLOWISH);

			// draw the | bar
			FillRGBA(x, y, iBarWidth, gHUD.m_iFontHeight, r, g, b, a);

			x += iBarWidth + AmmoWidth / 2;

			// GL Seems to need this
			ScaleColors(r, g, b, a);
			x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, gWR.CountAmmo(pw->iAmmoType), r, g, b);


		}
		else
		{
			// SPR_Draw a bullets only line
			x = ScreenWidth - 4 * AmmoWidth - iIconWidth;
			x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, gWR.CountAmmo(pw->iAmmoType), r, g, b);
		}

		// Draw the ammo Icon
		int iOffset = (m_pWeapon->rcAmmo.bottom - m_pWeapon->rcAmmo.top) / 8;
		SPR_Set(m_pWeapon->hAmmo, r, g, b);
		SPR_DrawAdditive(0, x, y - iOffset, &m_pWeapon->rcAmmo);
	}

	// Does weapon have seconday ammo?
	if (pw->iAmmo2Type > 0)
	{
		int iIconWidth = m_pWeapon->rcAmmo2.right - m_pWeapon->rcAmmo2.left;

		// Do we have secondary ammo?
		if ((pw->iAmmo2Type != 0) && (gWR.CountAmmo(pw->iAmmo2Type) > 0))
		{
			y -= gHUD.m_iFontHeight + gHUD.m_iFontHeight / 4;
			x = ScreenWidth - 4 * AmmoWidth - iIconWidth;
			x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, gWR.CountAmmo(pw->iAmmo2Type), r, g, b);

			// Draw the ammo Icon
			SPR_Set(m_pWeapon->hAmmo2, r, g, b);
			int iOffset = (m_pWeapon->rcAmmo2.bottom - m_pWeapon->rcAmmo2.top) / 8;
			SPR_DrawAdditive(0, x, y - iOffset, &m_pWeapon->rcAmmo2);
		}
	}
	return true;
}

/**
*	@brief Draws the ammo bar on the hud
*/
int DrawBar(int x, int y, int width, int height, float f)
{
	int r, g, b;

	if (f < 0)
		f = 0;
	if (f > 1)
		f = 1;

	if (f)
	{
		int w = f * width;

		// Always show at least one pixel if we have ammo.
		if (w <= 0)
			w = 1;
		UnpackRGB(r, g, b, RGB_GREENISH);
		FillRGBA(x, y, w, height, r, g, b, 255);
		x += w;
		width -= w;
	}

	UnpackRGB(r, g, b, RGB_YELLOWISH);

	FillRGBA(x, y, width, height, r, g, b, 128);

	return (x + width);
}

void DrawAmmoBar(WEAPON* p, int x, int y, int width, int height)
{
	if (!p)
		return;

	if (p->iAmmoType != -1)
	{
		if (!gWR.CountAmmo(p->iAmmoType))
			return;

		float f = (float)gWR.CountAmmo(p->iAmmoType) / (float)p->iMax1;

		x = DrawBar(x, y, width, height, f);


		// Do we have secondary ammo too?

		if (p->iAmmo2Type != -1)
		{
			f = (float)gWR.CountAmmo(p->iAmmo2Type) / (float)p->iMax2;

			x += 5; //!!!

			DrawBar(x, y, width, height, f);
		}
	}
}

/**
*	@brief Draw Weapon Menu
*/
bool CHudAmmo::DrawWList(float flTime)
{
	int r, g, b, x, y, a, i;

	if (m_ActiveSel.Type == SelectionType::Off)
		return false;

	int iActiveSlot;

	if (m_ActiveSel.Type == SelectionType::MenuBar)
		iActiveSlot = -1;	// current slot has no weapons
	else
		iActiveSlot = m_ActiveSel.Selection->iSlot;

	x = 10; //!!!
	y = 10; //!!!


	// Ensure that there are available choices in the active slot
	if (iActiveSlot > 0)
	{
		if (!gWR.GetFirstPos(iActiveSlot))
		{
			m_ActiveSel.Type = SelectionType::MenuBar;
			m_ActiveSel.Selection = nullptr;
			iActiveSlot = -1;
		}
	}

	// Draw top line
	for (i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		int iWidth;

		UnpackRGB(r, g, b, RGB_YELLOWISH);

		if (iActiveSlot == i)
			a = 255;
		else
			a = 192;

		ScaleColors(r, g, b, 255);
		SPR_Set(gHUD.GetSprite(m_HUD_bucket0 + i), r, g, b);

		// make active slot wide enough to accomodate gun pictures
		if (i == iActiveSlot)
		{
			WEAPON* p = gWR.GetFirstPos(iActiveSlot);
			if (p)
				iWidth = p->rcActive.right - p->rcActive.left;
			else
				iWidth = giBucketWidth;
		}
		else
			iWidth = giBucketWidth;

		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_bucket0 + i));

		x += iWidth + 5;
	}


	a = 128; //!!!
	x = 10;

	// Draw all of the buckets
	for (i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		y = giBucketHeight + 10;

		// If this is the active slot, draw the bigger pictures,
		// otherwise just draw boxes
		if (i == iActiveSlot)
		{
			WEAPON* p = gWR.GetFirstPos(i);
			int iWidth = giBucketWidth;
			if (p)
				iWidth = p->rcActive.right - p->rcActive.left;

			for (int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++)
			{
				p = gWR.GetWeaponSlot(i, iPos);

				if (!p || !p->iId)
					continue;

				UnpackRGB(r, g, b, RGB_YELLOWISH);

				// if active, then we must have ammo.

				if (m_ActiveSel.Selection == p)
				{
					SPR_Set(p->hActive, r, g, b);
					SPR_DrawAdditive(0, x, y, &p->rcActive);

					SPR_Set(gHUD.GetSprite(m_HUD_selection), r, g, b);
					SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_selection));
				}
				else
				{
					// Draw Weapon if Red if no ammo

					if (gWR.HasAmmo(p))
						ScaleColors(r, g, b, 192);
					else
					{
						UnpackRGB(r, g, b, RGB_REDISH);
						ScaleColors(r, g, b, 128);
					}

					SPR_Set(p->hInactive, r, g, b);
					SPR_DrawAdditive(0, x, y, &p->rcInactive);
				}

				// Draw Ammo Bar

				DrawAmmoBar(p, x + giABWidth / 2, y, giABWidth, giABHeight);

				y += p->rcActive.bottom - p->rcActive.top + 5;
			}

			x += iWidth + 5;

		}
		else
		{
			// Draw Row of weapons.

			UnpackRGB(r, g, b, RGB_YELLOWISH);

			for (int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++)
			{
				WEAPON* p = gWR.GetWeaponSlot(i, iPos);

				if (!p || !p->iId)
					continue;

				if (gWR.HasAmmo(p))
				{
					UnpackRGB(r, g, b, RGB_YELLOWISH);
					a = 128;
				}
				else
				{
					UnpackRGB(r, g, b, RGB_REDISH);
					a = 96;
				}

				FillRGBA(x, y, giBucketWidth, giBucketHeight, r, g, b, a);

				y += giBucketHeight + 5;
			}

			x += giBucketWidth + 5;
		}
	}

	return true;

}

//TODO: move this
client_sprite_t* GetSpriteList(client_sprite_t* pList, const char* psz, int iRes, int iCount)
{
	if (!pList)
		return nullptr;

	int i = iCount;
	client_sprite_t* p = pList;

	while (i--)
	{
		if ((!strcmp(psz, p->szName)) && (p->iRes == iRes))
			return p;
		p++;
	}

	return nullptr;
}
