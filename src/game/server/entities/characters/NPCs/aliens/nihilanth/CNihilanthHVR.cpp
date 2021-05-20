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

#include "CNihilanth.hpp"
#include "CNihilanthHVR.hpp"

LINK_ENTITY_TO_CLASS(nihilanth_energy_ball, CNihilanthHVR);

TYPEDESCRIPTION	CNihilanthHVR::m_SaveData[] =
{
	DEFINE_FIELD(CNihilanthHVR, m_flIdealVel, FIELD_FLOAT),
	DEFINE_FIELD(CNihilanthHVR, m_vecIdeal, FIELD_VECTOR),
	DEFINE_FIELD(CNihilanthHVR, m_hNihilanth, FIELD_EHANDLE),
	DEFINE_FIELD(CNihilanthHVR, m_hTouch, FIELD_EHANDLE),
	DEFINE_FIELD(CNihilanthHVR, m_nFrames, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CNihilanthHVR, CBaseMonster);

void CNihilanthHVR::Spawn()
{
	Precache();

	SetRenderMode(RenderMode::TransAdd);
	SetRenderAmount(255);
	pev->scale = 3.0;
}

void CNihilanthHVR::Precache()
{
	PRECACHE_MODEL("sprites/flare6.spr");
	PRECACHE_MODEL("sprites/nhth1.spr");
	PRECACHE_MODEL("sprites/exit1.spr");
	PRECACHE_MODEL("sprites/tele1.spr");
	PRECACHE_MODEL("sprites/animglow01.spr");
	PRECACHE_MODEL("sprites/xspark4.spr");
	PRECACHE_MODEL("sprites/muzzleflash3.spr");
	PRECACHE_SOUND("debris/zap4.wav");
	PRECACHE_SOUND("weapons/electro4.wav");
	PRECACHE_SOUND("x/x_teleattack1.wav");
}

void CNihilanthHVR::CircleInit(CBaseEntity* pTarget)
{
	SetMovetype(Movetype::Noclip);
	SetSolidType(Solid::Not);

	// SetModel( "sprites/flare6.spr");
	// pev->scale = 3.0;
	// SetModel( "sprites/xspark4.spr");
	SetModel("sprites/muzzleflash3.spr");
	SetRenderColor({255, 224, 192});
	pev->scale = 2.0;
	m_nFrames = 1;
	SetRenderAmount(255);

	SetSize(vec3_origin, vec3_origin);
	SetAbsOrigin(GetAbsOrigin());

	SetThink(&CNihilanthHVR::HoverThink);
	SetTouch(&CNihilanthHVR::BounceTouch);
	pev->nextthink = gpGlobals->time + 0.1;

	m_hTargetEnt = pTarget;
}

CBaseEntity* CNihilanthHVR::RandomClassname(const char* szName)
{
	int total = 0;

	CBaseEntity* pEntity = nullptr;
	CBaseEntity* pNewEntity = nullptr;
	while ((pNewEntity = UTIL_FindEntityByClassname(pNewEntity, szName)) != nullptr)
	{
		total++;
		if (RANDOM_LONG(0, total - 1) < 1)
			pEntity = pNewEntity;
	}
	return pEntity;
}

void CNihilanthHVR::HoverThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (m_hTargetEnt != nullptr)
	{
		CircleTarget(m_hTargetEnt->GetAbsOrigin() + Vector(0, 0, 16 * N_SCALE));
	}
	else
	{
		UTIL_Remove(this);
	}

	if (RANDOM_LONG(0, 99) < 5)
	{
		/*
				CBaseEntity *pOther = RandomClassname( GetClassname() );

				if (pOther && pOther != this)
				{
					MESSAGE_BEGIN( MessageDest::Broadcast, SVC_TEMPENTITY );
						WRITE_BYTE( TE_BEAMENTS );
						WRITE_SHORT( this->entindex() );
						WRITE_SHORT( pOther->entindex() );
						WRITE_SHORT( g_sModelIndexLaser );
						WRITE_BYTE( 0 ); // framestart
						WRITE_BYTE( 0 ); // framerate
						WRITE_BYTE( 10 ); // life
						WRITE_BYTE( 80 );  // width
						WRITE_BYTE( 80 );   // noise
						WRITE_BYTE( 255 );   // r, g, b
						WRITE_BYTE( 128 );   // r, g, b
						WRITE_BYTE( 64 );   // r, g, b
						WRITE_BYTE( 255 );	// brightness
						WRITE_BYTE( 30 );		// speed
					MESSAGE_END();
				}
		*/
		/*
				MESSAGE_BEGIN( MessageDest::Broadcast, SVC_TEMPENTITY );
					WRITE_BYTE( TE_BEAMENTS );
					WRITE_SHORT( this->entindex() );
					WRITE_SHORT( m_hTargetEnt->entindex() + 0x1000 );
					WRITE_SHORT( g_sModelIndexLaser );
					WRITE_BYTE( 0 ); // framestart
					WRITE_BYTE( 0 ); // framerate
					WRITE_BYTE( 10 ); // life
					WRITE_BYTE( 80 );  // width
					WRITE_BYTE( 80 );   // noise
					WRITE_BYTE( 255 );   // r, g, b
					WRITE_BYTE( 128 );   // r, g, b
					WRITE_BYTE( 64 );   // r, g, b
					WRITE_BYTE( 255 );	// brightness
					WRITE_BYTE( 30 );		// speed
				MESSAGE_END();
		*/
	}

	pev->frame = ((int)pev->frame + 1) % m_nFrames;
}

void CNihilanthHVR::ZapInit(CBaseEntity* pEnemy)
{
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	SetModel("sprites/nhth1.spr");

	SetRenderColor({255, 255, 255});
	pev->scale = 2.0;

	SetAbsVelocity((pEnemy->GetAbsOrigin() - GetAbsOrigin()).Normalize() * 200);

	m_hEnemy = pEnemy;
	SetThink(&CNihilanthHVR::ZapThink);
	SetTouch(&CNihilanthHVR::ZapTouch);
	pev->nextthink = gpGlobals->time + 0.1;

	EmitSound(SoundChannel::Weapon, "debris/zap4.wav");
}

void CNihilanthHVR::ZapThink()
{
	pev->nextthink = gpGlobals->time + 0.05;

	// check world boundaries
	if (m_hEnemy == nullptr || !UTIL_IsInWorld(GetAbsOrigin()))
	{
		SetTouch(nullptr);
		UTIL_Remove(this);
		return;
	}

	if (GetAbsVelocity().Length() < 2000)
	{
		SetAbsVelocity(GetAbsVelocity() * 1.2);
	}

	// MovetoTarget( m_hEnemy->Center( ) );

	if ((m_hEnemy->Center() - GetAbsOrigin()).Length() < 256)
	{
		TraceResult tr;

		UTIL_TraceLine(GetAbsOrigin(), m_hEnemy->Center(), IgnoreMonsters::No, this, &tr);

		if (CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit); pEntity != nullptr && pEntity->pev->takedamage)
		{
			ClearMultiDamage();
			pEntity->TraceAttack({this, gSkillData.nihilanthZap, GetAbsVelocity(), tr, DMG_SHOCK});
			ApplyMultiDamage(this, this);
		}

		MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
		WRITE_BYTE(TE_BEAMENTPOINT);
		WRITE_SHORT(entindex());
		WRITE_COORD(tr.vecEndPos.x);
		WRITE_COORD(tr.vecEndPos.y);
		WRITE_COORD(tr.vecEndPos.z);
		WRITE_SHORT(g_sModelIndexLaser);
		WRITE_BYTE(0); // frame start
		WRITE_BYTE(10); // framerate
		WRITE_BYTE(3); // life
		WRITE_BYTE(20);  // width
		WRITE_BYTE(20);   // noise
		WRITE_BYTE(64);   // r, g, b
		WRITE_BYTE(196);   // r, g, b
		WRITE_BYTE(255);   // r, g, b
		WRITE_BYTE(255);	// brightness
		WRITE_BYTE(10);		// speed
		MESSAGE_END();

		UTIL_EmitAmbientSound(this, tr.vecEndPos, "weapons/electro4.wav", 0.5, ATTN_NORM, 0, RANDOM_LONG(140, 160));

		SetTouch(nullptr);
		UTIL_Remove(this);
		pev->nextthink = gpGlobals->time + 0.2;
		return;
	}

	pev->frame = (int)(pev->frame + 1) % 11;

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_ELIGHT);
	WRITE_SHORT(entindex());		// entity, attachment
	WRITE_COORD(GetAbsOrigin().x);		// origin
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z);
	WRITE_COORD(128);	// radius
	WRITE_BYTE(128);	// R
	WRITE_BYTE(128);	// G
	WRITE_BYTE(255);	// B
	WRITE_BYTE(10);	// life * 10
	WRITE_COORD(128); // decay
	MESSAGE_END();

	// Crawl( );
}

void CNihilanthHVR::ZapTouch(CBaseEntity* pOther)
{
	UTIL_EmitAmbientSound(this, GetAbsOrigin(), "weapons/electro4.wav", 1.0, ATTN_NORM, 0, RANDOM_LONG(90, 95));

	RadiusDamage(this, this, 50, CLASS_NONE, DMG_SHOCK);
	SetAbsVelocity(GetAbsVelocity() * 0);

	/*
	for (int i = 0; i < 10; i++)
	{
		Crawl( );
	}
	*/

	SetTouch(nullptr);
	UTIL_Remove(this);
	pev->nextthink = gpGlobals->time + 0.2;
}

void CNihilanthHVR::TeleportInit(CNihilanth* pOwner, CBaseEntity* pEnemy, CBaseEntity* pTarget, CBaseEntity* pTouch)
{
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	SetRenderColor({255, 255, 255});
	SetAbsVelocity({GetAbsVelocity().x, GetAbsVelocity().y, GetAbsVelocity().z * 0.2f});

	SetModel("sprites/exit1.spr");

	m_hNihilanth = pOwner;
	m_hEnemy = pEnemy;
	m_hTargetEnt = pTarget;
	m_hTouch = pTouch;

	SetThink(&CNihilanthHVR::TeleportThink);
	SetTouch(&CNihilanthHVR::TeleportTouch);
	pev->nextthink = gpGlobals->time + 0.1;

	EmitSound(SoundChannel::Weapon, "x/x_teleattack1.wav", VOL_NORM, 0.2);
}

void CNihilanthHVR::GreenBallInit()
{
	SetMovetype(Movetype::Fly);
	SetSolidType(Solid::BBox);

	SetRenderColor({255, 255, 255});
	pev->scale = 1.0;

	SetModel("sprites/exit1.spr");

	SetTouch(&CNihilanthHVR::RemoveTouch);
}

void CNihilanthHVR::TeleportThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	// check world boundaries
	if (m_hEnemy == nullptr || !m_hEnemy->IsAlive() || !UTIL_IsInWorld(GetAbsOrigin()))
	{
		StopSound(SoundChannel::Weapon, "x/x_teleattack1.wav");
		UTIL_Remove(this);
		return;
	}

	if ((m_hEnemy->Center() - GetAbsOrigin()).Length() < 128)
	{
		StopSound(SoundChannel::Weapon, "x/x_teleattack1.wav");
		UTIL_Remove(this);

		if (m_hTargetEnt != nullptr)
			m_hTargetEnt->Use({m_hEnemy, m_hEnemy, UseType::On, 1.0});

		if (m_hTouch != nullptr && m_hEnemy != nullptr)
			m_hTouch->Touch(m_hEnemy);
	}
	else
	{
		MovetoTarget(m_hEnemy->Center());
	}

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_ELIGHT);
	WRITE_SHORT(entindex());		// entity, attachment
	WRITE_COORD(GetAbsOrigin().x);		// origin
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z);
	WRITE_COORD(256);	// radius
	WRITE_BYTE(0);	// R
	WRITE_BYTE(255);	// G
	WRITE_BYTE(0);	// B
	WRITE_BYTE(10);	// life * 10
	WRITE_COORD(256); // decay
	MESSAGE_END();

	pev->frame = (int)(pev->frame + 1) % 20;
}

void CNihilanthHVR::AbsorbInit()
{
	SetThink(&CNihilanthHVR::DissipateThink);
	SetRenderAmount(255);

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMENTS);
	WRITE_SHORT(this->entindex());
	WRITE_SHORT(m_hTargetEnt->entindex() + 0x1000);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); // framestart
	WRITE_BYTE(0); // framerate
	WRITE_BYTE(50); // life
	WRITE_BYTE(80);  // width
	WRITE_BYTE(80);   // noise
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(128);   // r, g, b
	WRITE_BYTE(64);   // r, g, b
	WRITE_BYTE(255);	// brightness
	WRITE_BYTE(30);		// speed
	MESSAGE_END();
}

void CNihilanthHVR::TeleportTouch(CBaseEntity* pOther)
{
	CBaseEntity* pEnemy = m_hEnemy;

	if (pOther == pEnemy)
	{
		if (m_hTargetEnt != nullptr)
			m_hTargetEnt->Use({pEnemy, pEnemy, UseType::On, 1.0});

		if (m_hTouch != nullptr && pEnemy != nullptr)
			m_hTouch->Touch(pEnemy);
	}
	else
	{
		m_hNihilanth->MakeFriend(GetAbsOrigin());
	}

	SetTouch(nullptr);
	StopSound(SoundChannel::Weapon, "x/x_teleattack1.wav");
	UTIL_Remove(this);
}

void CNihilanthHVR::DissipateThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->scale > 5.0)
		UTIL_Remove(this);

	SetRenderAmount(GetRenderAmount() - 2);
	pev->scale += 0.1;

	if (m_hTargetEnt != nullptr)
	{
		CircleTarget(m_hTargetEnt->GetAbsOrigin() + Vector(0, 0, WORLD_BOUNDARY));
	}
	else
	{
		UTIL_Remove(this);
	}

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_ELIGHT);
	WRITE_SHORT(entindex());		// entity, attachment
	WRITE_COORD(GetAbsOrigin().x);		// origin
	WRITE_COORD(GetAbsOrigin().y);
	WRITE_COORD(GetAbsOrigin().z);
	WRITE_COORD(GetRenderAmount());	// radius
	WRITE_BYTE(255);	// R
	WRITE_BYTE(192);	// G
	WRITE_BYTE(64);	// B
	WRITE_BYTE(2);	// life * 10
	WRITE_COORD(0); // decay
	MESSAGE_END();
}

bool CNihilanthHVR::CircleTarget(Vector vecTarget)
{
	bool fClose = false;

	Vector vecDest = vecTarget;
	Vector vecEst = GetAbsOrigin() + GetAbsVelocity() * 0.5;
	Vector vecSrc = GetAbsOrigin();
	vecDest.z = 0;
	vecEst.z = 0;
	vecSrc.z = 0;
	float d1 = (vecDest - vecSrc).Length() - 24 * N_SCALE;
	const float d2 = (vecDest - vecEst).Length() - 24 * N_SCALE;

	if (m_vecIdeal == vec3_origin)
	{
		m_vecIdeal = GetAbsVelocity();
	}

	if (d1 < 0 && d2 <= d1)
	{
		// ALERT( at_console, "too close\n");
		m_vecIdeal = m_vecIdeal - (vecDest - vecSrc).Normalize() * 50;
	}
	else if (d1 > 0 && d2 >= d1)
	{
		// ALERT( at_console, "too far\n");
		m_vecIdeal = m_vecIdeal + (vecDest - vecSrc).Normalize() * 50;
	}
	pev->avelocity.z = d1 * 20;

	if (d1 < 32)
	{
		fClose = true;
	}

	m_vecIdeal = m_vecIdeal + Vector(RANDOM_FLOAT(-2, 2), RANDOM_FLOAT(-2, 2), RANDOM_FLOAT(-2, 2));
	m_vecIdeal = Vector(m_vecIdeal.x, m_vecIdeal.y, 0).Normalize() * 200
		/* + Vector( -m_vecIdeal.y, m_vecIdeal.x, 0 ).Normalize( ) * 32 */
		+ Vector(0, 0, m_vecIdeal.z);
	// m_vecIdeal = m_vecIdeal + Vector( -m_vecIdeal.y, m_vecIdeal.x, 0 ).Normalize( ) * 2;

	// move up/down
	d1 = vecTarget.z - GetAbsOrigin().z;
	if (d1 > 0 && m_vecIdeal.z < 200)
		m_vecIdeal.z += 20;
	else if (d1 < 0 && m_vecIdeal.z > -200)
		m_vecIdeal.z -= 20;

	SetAbsVelocity(m_vecIdeal);

	// ALERT( at_console, "%.0f %.0f %.0f\n", m_vecIdeal.x, m_vecIdeal.y, m_vecIdeal.z );
	return fClose;
}

void CNihilanthHVR::MovetoTarget(Vector vecTarget)
{
	if (m_vecIdeal == vec3_origin)
	{
		m_vecIdeal = GetAbsVelocity();
	}

	// accelerate
	const float flSpeed = m_vecIdeal.Length();
	if (flSpeed > 300)
	{
		m_vecIdeal = m_vecIdeal.Normalize() * 300;
	}
	m_vecIdeal = m_vecIdeal + (vecTarget - GetAbsOrigin()).Normalize() * 300;
	SetAbsVelocity(m_vecIdeal);
}

void CNihilanthHVR::Crawl()
{
	const Vector vecAim = Vector(RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1)).Normalize();
	const Vector vecPnt = GetAbsOrigin() + GetAbsVelocity() * 0.2 + vecAim * 128;

	MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMENTPOINT);
	WRITE_SHORT(entindex());
	WRITE_COORD(vecPnt.x);
	WRITE_COORD(vecPnt.y);
	WRITE_COORD(vecPnt.z);
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(0); // frame start
	WRITE_BYTE(10); // framerate
	WRITE_BYTE(3); // life
	WRITE_BYTE(20);  // width
	WRITE_BYTE(80);   // noise
	WRITE_BYTE(64);   // r, g, b
	WRITE_BYTE(128);   // r, g, b
	WRITE_BYTE(255);   // r, g, b
	WRITE_BYTE(255);	// brightness
	WRITE_BYTE(10);		// speed
	MESSAGE_END();
}

void CNihilanthHVR::RemoveTouch(CBaseEntity* pOther)
{
	StopSound(SoundChannel::Weapon, "x/x_teleattack1.wav");
	UTIL_Remove(this);
}

void CNihilanthHVR::BounceTouch(CBaseEntity* pOther)
{
	Vector vecDir = m_vecIdeal.Normalize();

	const TraceResult tr = UTIL_GetGlobalTrace();

	const float n = -DotProduct(tr.vecPlaneNormal, vecDir);

	vecDir = 2.0 * tr.vecPlaneNormal * n + vecDir;

	m_vecIdeal = vecDir * m_vecIdeal.Length();
}
