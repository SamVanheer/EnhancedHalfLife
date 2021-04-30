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

class CBaseEntity;
struct edict_t;

/**
*	@brief Base class for entity handles
*/
class BaseHandle
{
public:
	BaseHandle() = default;

	BaseHandle(CBaseEntity* entity)
	{
		Set(entity);
	}

	CBaseEntity* operator=(CBaseEntity* entity);

	CBaseEntity* Get();
	void Set(CBaseEntity* entity);

	operator CBaseEntity* () { return Get(); }
	CBaseEntity* operator->() { return Get(); }

private:
	edict_t* m_pent = nullptr;
	int m_serialnumber = 0;
};

/**
*	@brief Safe way to point to CBaseEntities who may die between frames
*/
template<typename TEntity>
class EHandle : protected BaseHandle
{
public:
	EHandle() = default;

	EHandle(TEntity* entity)
		: BaseHandle(entity)
	{
	}

	TEntity* operator=(TEntity* entity)
	{
		Set(entity);
		return entity;
	}

	TEntity* Get() { return static_cast<TEntity*>(BaseHandle::Get()); }
	void Set(TEntity* entity) { BaseHandle::Set(entity); }

	operator TEntity* () { return Get(); }
	TEntity* operator->() { return Get(); }
};

using EHANDLE = EHandle<CBaseEntity>;
