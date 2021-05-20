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

#include "CRulePointEntity.hpp"

constexpr int SF_ENVTEXT_ALLPLAYERS = 0x0001;

/**
*	@brief NON-Localized HUD Message (use env_message to display a titles.txt message)
*	@details Flag: All players SF_ENVTEXT_ALLPLAYERS
*/
class CGameText : public CRulePointEntity
{
public:
	void	Use(const UseInfo& info) override;
	void	KeyValue(KeyValueData* pkvd) override;
	void Precache() override;
	void Spawn() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];

	inline	bool	MessageToAll() { return (pev->spawnflags & SF_ENVTEXT_ALLPLAYERS) != 0; }
	inline	void	MessageSet(const char* pMessage) { pev->message = ALLOC_STRING(pMessage); }
	inline	const char* MessageGet() { return STRING(pev->message); }

private:

	hudtextparms_t m_textParms;
};
