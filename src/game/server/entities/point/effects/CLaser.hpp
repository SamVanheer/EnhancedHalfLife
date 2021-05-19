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

#include "CBeam.hpp"

class CSprite;

#include "CLaser.generated.hpp"

class EHL_CLASS() CLaser : public CBeam
{
	EHL_GENERATED_BODY()

public:
	void OnRemove() override;
	void	Spawn() override;
	void	Precache() override;
	void	KeyValue(KeyValueData * pkvd) override;

	void	TurnOn();
	void	TurnOff();
	bool IsOn();

	void	FireAtPoint(TraceResult & point);

	void	EXPORT StrikeThink();
	void	Use(const UseInfo & info) override;

	EHL_FIELD(Persisted)
	EHandle<CSprite> m_hSprite;

	EHL_FIELD(Persisted)
	string_t m_iszSpriteName = iStringNull;

	EHL_FIELD(Persisted, Type = Position)
	Vector  m_firePosition;
};
