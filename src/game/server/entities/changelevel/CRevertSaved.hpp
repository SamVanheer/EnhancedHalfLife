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

#include "CPointEntity.hpp"

#include "CRevertSaved.generated.hpp"

class EHL_CLASS() CRevertSaved : public CPointEntity
{
	EHL_GENERATED_BODY()

public:
	void	Use(const UseInfo & info) override;
	void	EXPORT MessageThink();
	void	EXPORT LoadThink();
	void	KeyValue(KeyValueData * pkvd) override;

	inline	float	Duration() { return pev->dmg_take; }
	inline	float	HoldTime() { return pev->dmg_save; }
	inline	float	MessageTime() { return m_messageTime; }
	inline	float	LoadTime() { return m_loadTime; }

	inline	void	SetDuration(float duration) { pev->dmg_take = duration; }
	inline	void	SetHoldTime(float hold) { pev->dmg_save = hold; }
	inline	void	SetMessageTime(float time) { m_messageTime = time; }
	inline	void	SetLoadTime(float time) { m_loadTime = time; }

private:
	EHL_FIELD(Persisted)
	float m_messageTime = 0; //!< These are not actual times, but durations, so save as floats

	EHL_FIELD(Persisted)
	float m_loadTime = 0;
};
