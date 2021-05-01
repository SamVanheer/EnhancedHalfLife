/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "soundent.h"
#include "effects.h"
#include "customentity.h"

constexpr int SF_WAITFORTRIGGER = 0x40;

constexpr int MAX_CARRY = 24;

class COsprey : public CBaseMonster
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static	TYPEDESCRIPTION m_SaveData[];
	int		ObjectCaps() override { return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	void Spawn() override;
	void Precache() override;
	int  Classify() override { return CLASS_MACHINE; }
	int  BloodColor() override { return DONT_BLEED; }
	void Killed(const KilledInfo& info) override;

	void UpdateGoal();
	bool HasDead();
	void EXPORT FlyThink();
	void EXPORT DeployThink();
	void Flight();
	void EXPORT HitTouch(CBaseEntity* pOther);
	void EXPORT FindAllThink();
	void EXPORT HoverThink();
	CBaseMonster* MakeGrunt(Vector vecSrc);
	void EXPORT CrashTouch(CBaseEntity* pOther);
	void EXPORT DyingThink();
	void EXPORT CommandUse(const UseInfo& info);

	// bool TakeDamage(const TakeDamageInfo& info) override;
	void TraceAttack(const TraceAttackInfo& info) override;
	void ShowDamage();

	EHandle<CBaseEntity> m_hGoalEnt;
	Vector m_vel1;
	Vector m_vel2;
	Vector m_pos1;
	Vector m_pos2;
	Vector m_ang1;
	Vector m_ang2;
	float m_startTime;
	float m_dTime;

	Vector m_velocity;

	float m_flIdealtilt;
	float m_flRotortilt;

	float m_flRightHealth;
	float m_flLeftHealth;

	int	m_iUnits;
	EHANDLE m_hGrunt[MAX_CARRY];
	Vector m_vecOrigin[MAX_CARRY];
	EHANDLE m_hRepel[4];

	int m_iSoundState;
	int m_iSpriteTexture;

	int m_iPitch;

	int m_iExplode;
	int	m_iTailGibs;
	int	m_iBodyGibs;
	int	m_iEngineGibs;

	int m_iDoLeftSmokePuff;
	int m_iDoRightSmokePuff;
};

LINK_ENTITY_TO_CLASS(monster_osprey, COsprey);

TYPEDESCRIPTION	COsprey::m_SaveData[] =
{
	DEFINE_FIELD(COsprey, m_hGoalEnt, FIELD_EHANDLE),
	DEFINE_FIELD(COsprey, m_vel1, FIELD_VECTOR),
	DEFINE_FIELD(COsprey, m_vel2, FIELD_VECTOR),
	DEFINE_FIELD(COsprey, m_pos1, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(COsprey, m_pos2, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(COsprey, m_ang1, FIELD_VECTOR),
	DEFINE_FIELD(COsprey, m_ang2, FIELD_VECTOR),

	DEFINE_FIELD(COsprey, m_startTime, FIELD_TIME),
	DEFINE_FIELD(COsprey, m_dTime, FIELD_FLOAT),
	DEFINE_FIELD(COsprey, m_velocity, FIELD_VECTOR),

	DEFINE_FIELD(COsprey, m_flIdealtilt, FIELD_FLOAT),
	DEFINE_FIELD(COsprey, m_flRotortilt, FIELD_FLOAT),

	DEFINE_FIELD(COsprey, m_flRightHealth, FIELD_FLOAT),
	DEFINE_FIELD(COsprey, m_flLeftHealth, FIELD_FLOAT),

	DEFINE_FIELD(COsprey, m_iUnits, FIELD_INTEGER),
	DEFINE_ARRAY(COsprey, m_hGrunt, FIELD_EHANDLE, MAX_CARRY),
	DEFINE_ARRAY(COsprey, m_vecOrigin, FIELD_POSITION_VECTOR, MAX_CARRY),
	DEFINE_ARRAY(COsprey, m_hRepel, FIELD_EHANDLE, 4),

	// DEFINE_FIELD( COsprey, m_iSoundState, FIELD_INTEGER ),
	// DEFINE_FIELD( COsprey, m_iSpriteTexture, FIELD_INTEGER ),
	// DEFINE_FIELD( COsprey, m_iPitch, FIELD_INTEGER ),

	DEFINE_FIELD(COsprey, m_iDoLeftSmokePuff, FIELD_INTEGER),
	DEFINE_FIELD(COsprey, m_iDoRightSmokePuff, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(COsprey, CBaseMonster);

void COsprey::Spawn()
{
	Precache();
	// motor
	pev->movetype = Movetype::Fly;
	pev->solid = Solid::BBox;

	SET_MODEL(ENT(pev), "models/osprey.mdl");
	UTIL_SetSize(pev, Vector(-400, -400, -100), Vector(400, 400, 32));
	UTIL_SetOrigin(pev, pev->origin);

	pev->flags |= FL_MONSTER;
	SetDamageMode(DamageMode::Yes);
	m_flRightHealth = 200;
	m_flLeftHealth = 200;
	pev->health = 400;

	m_flFieldOfView = 0; // 180 degrees

	pev->sequence = 0;
	ResetSequenceInfo();
	pev->frame = RANDOM_LONG(0, 0xFF);

	InitBoneControllers();

	SetThink(&COsprey::FindAllThink);
	SetUse(&COsprey::CommandUse);

	if (!(pev->spawnflags & SF_WAITFORTRIGGER))
	{
		pev->nextthink = gpGlobals->time + 1.0;
	}

	m_pos2 = pev->origin;
	m_ang2 = pev->angles;
	m_vel2 = pev->velocity;
}

void COsprey::Precache()
{
	UTIL_PrecacheOther("monster_human_grunt");

	PRECACHE_MODEL("models/osprey.mdl");
	PRECACHE_MODEL("models/HVR.mdl");

	PRECACHE_SOUND("apache/ap_rotor4.wav");
	PRECACHE_SOUND("weapons/mortarhit.wav");

	m_iSpriteTexture = PRECACHE_MODEL("sprites/rope.spr");

	m_iExplode = PRECACHE_MODEL("sprites/fexplo.spr");
	m_iTailGibs = PRECACHE_MODEL("models/osprey_tailgibs.mdl");
	m_iBodyGibs = PRECACHE_MODEL("models/osprey_bodygibs.mdl");
	m_iEngineGibs = PRECACHE_MODEL("models/osprey_enginegibs.mdl");
}

void COsprey::CommandUse(const UseInfo& info)
{
	pev->nextthink = gpGlobals->time + 0.1;
}

void COsprey::FindAllThink()
{
	CBaseEntity* pEntity = nullptr;

	m_iUnits = 0;
	while (m_iUnits < MAX_CARRY && (pEntity = UTIL_FindEntityByClassname(pEntity, "monster_human_grunt")) != nullptr)
	{
		if (pEntity->IsAlive())
		{
			m_hGrunt[m_iUnits] = pEntity;
			m_vecOrigin[m_iUnits] = pEntity->pev->origin;
			m_iUnits++;
		}
	}

	if (m_iUnits == 0)
	{
		ALERT(at_console, "osprey error: no grunts to resupply\n");
		UTIL_Remove(this);
		return;
	}
	SetThink(&COsprey::FlyThink);
	pev->nextthink = gpGlobals->time + 0.1;
	m_startTime = gpGlobals->time;
}

void COsprey::DeployThink()
{
	UTIL_MakeAimVectors(pev->angles);

	const Vector vecForward = gpGlobals->v_forward;
	const Vector vecRight = gpGlobals->v_right;
	const Vector vecUp = gpGlobals->v_up;

	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -WORLD_BOUNDARY), IgnoreMonsters::Yes, ENT(pev), &tr);
	CSoundEnt::InsertSound(bits_SOUND_DANGER, tr.vecEndPos, 400, 0.3);

	Vector vecSrc = pev->origin + vecForward * 32 + vecRight * 100 + vecUp * -96;
	m_hRepel[0] = MakeGrunt(vecSrc);

	vecSrc = pev->origin + vecForward * -64 + vecRight * 100 + vecUp * -96;
	m_hRepel[1] = MakeGrunt(vecSrc);

	vecSrc = pev->origin + vecForward * 32 + vecRight * -100 + vecUp * -96;
	m_hRepel[2] = MakeGrunt(vecSrc);

	vecSrc = pev->origin + vecForward * -64 + vecRight * -100 + vecUp * -96;
	m_hRepel[3] = MakeGrunt(vecSrc);

	SetThink(&COsprey::HoverThink);
	pev->nextthink = gpGlobals->time + 0.1;
}

bool COsprey::HasDead()
{
	for (int i = 0; i < m_iUnits; i++)
	{
		if (m_hGrunt[i] == nullptr || !m_hGrunt[i]->IsAlive())
		{
			return true;
		}
		else
		{
			m_vecOrigin[i] = m_hGrunt[i]->pev->origin;  // send them to where they died
		}
	}
	return false;
}

CBaseMonster* COsprey::MakeGrunt(Vector vecSrc)
{
	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + Vector(0, 0, -WORLD_BOUNDARY), IgnoreMonsters::No, ENT(pev), &tr);
	if (tr.pHit && Instance(tr.pHit)->pev->solid != Solid::BSP)
		return nullptr;

	for (int i = 0; i < m_iUnits; i++)
	{
		if (m_hGrunt[i] == nullptr || !m_hGrunt[i]->IsAlive())
		{
			if (m_hGrunt[i] != nullptr && m_hGrunt[i]->pev->rendermode == RenderMode::Normal)
			{
				m_hGrunt[i]->SUB_StartFadeOut();
			}
			CBaseEntity* pEntity = Create("monster_human_grunt", vecSrc, pev->angles);
			CBaseMonster* pGrunt = pEntity->MyMonsterPointer();
			pGrunt->pev->movetype = Movetype::Fly;
			pGrunt->pev->velocity = Vector(0, 0, RANDOM_FLOAT(-196, -128));
			pGrunt->SetActivity(ACT_GLIDE);

			CBeam* pBeam = CBeam::BeamCreate("sprites/rope.spr", 10);
			pBeam->PointEntInit(vecSrc + Vector(0, 0, 112), pGrunt->entindex());
			pBeam->SetFlags(BEAM_FSOLID);
			pBeam->SetColor(255, 255, 255);
			pBeam->SetThink(&CBeam::SUB_Remove);
			pBeam->pev->nextthink = gpGlobals->time + -WORLD_BOUNDARY * tr.flFraction / pGrunt->pev->velocity.z + 0.5;

			// ALERT( at_console, "%d at %.0f %.0f %.0f\n", i, m_vecOrigin[i].x, m_vecOrigin[i].y, m_vecOrigin[i].z );  
			pGrunt->m_vecLastPosition = m_vecOrigin[i];
			m_hGrunt[i] = pGrunt;
			return pGrunt;
		}
	}
	// ALERT( at_console, "none dead\n");
	return nullptr;
}

void COsprey::HoverThink()
{
	int i;
	for (i = 0; i < 4; i++)
	{
		if (m_hRepel[i] != nullptr && m_hRepel[i]->pev->health > 0 && !(m_hRepel[i]->pev->flags & FL_ONGROUND))
		{
			break;
		}
	}

	if (i == 4)
	{
		m_startTime = gpGlobals->time;
		SetThink(&COsprey::FlyThink);
	}

	pev->nextthink = gpGlobals->time + 0.1;
	UTIL_MakeAimVectors(pev->angles);
	ShowDamage();
}

void COsprey::UpdateGoal()
{
	if (auto goal = m_hGoalEnt.Get(); goal)
	{
		m_pos1 = m_pos2;
		m_ang1 = m_ang2;
		m_vel1 = m_vel2;
		m_pos2 = goal->pev->origin;
		m_ang2 = goal->pev->angles;
		UTIL_MakeAimVectors(Vector(0, m_ang2.y, 0));
		m_vel2 = gpGlobals->v_forward * goal->pev->speed;

		m_startTime = m_startTime + m_dTime;
		m_dTime = 2.0 * (m_pos1 - m_pos2).Length() / (m_vel1.Length() + goal->pev->speed);

		if (m_ang1.y - m_ang2.y < -180)
		{
			m_ang1.y += 360;
		}
		else if (m_ang1.y - m_ang2.y > 180)
		{
			m_ang1.y -= 360;
		}

		if (goal->pev->speed < 400)
			m_flIdealtilt = 0;
		else
			m_flIdealtilt = -90;
	}
	else
	{
		ALERT(at_console, "osprey missing target");
	}
}

void COsprey::FlyThink()
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if (m_hGoalEnt == nullptr && !IsStringNull(pev->target))// this monster has a target
	{
		m_hGoalEnt = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));
		UpdateGoal();
	}

	if (gpGlobals->time > m_startTime + m_dTime)
	{
		if (m_hGoalEnt->pev->speed == 0)
		{
			SetThink(&COsprey::DeployThink);
		}
		//TODO: this doesn't check if target is valid and if goal ent is valid
		do {
			m_hGoalEnt = UTIL_FindEntityByTargetname(nullptr, STRING(m_hGoalEnt->pev->target));
		}
		while (m_hGoalEnt->pev->speed < 400 && !HasDead());
		UpdateGoal();
	}

	Flight();
	ShowDamage();
}

void COsprey::Flight()
{
	const float t = (gpGlobals->time - m_startTime);
	const float scale = 1.0 / m_dTime;

	const float f = UTIL_SplineFraction(t * scale, 1.0);

	const Vector pos = (m_pos1 + m_vel1 * t) * (1.0 - f) + (m_pos2 - m_vel2 * (m_dTime - t)) * f;
	const Vector ang = (m_ang1) * (1.0 - f) + (m_ang2)*f;
	m_velocity = m_vel1 * (1.0 - f) + m_vel2 * f;

	UTIL_SetOrigin(pev, pos);
	pev->angles = ang;
	UTIL_MakeAimVectors(pev->angles);
	const float flSpeed = DotProduct(gpGlobals->v_forward, m_velocity);

	// float flSpeed = DotProduct( gpGlobals->v_forward, pev->velocity );

	const float m_flIdealtilt = (160 - flSpeed) / 10.0;

	// ALERT( at_console, "%f %f\n", flSpeed, flIdealtilt );
	if (m_flRotortilt < m_flIdealtilt)
	{
		m_flRotortilt += 0.5;
		if (m_flRotortilt > 0)
			m_flRotortilt = 0;
	}
	if (m_flRotortilt > m_flIdealtilt)
	{
		m_flRotortilt -= 0.5;
		if (m_flRotortilt < -90)
			m_flRotortilt = -90;
	}

	SetBoneController(0, m_flRotortilt);

	if (m_iSoundState == 0)
	{
		EmitSound(SoundChannel::Static, "apache/ap_rotor4.wav", VOL_NORM, 0.15, 110);
		// EmitSound(SoundChannel::Static, "apache/ap_whine1.wav", 0.5, 0.2, 110);

		m_iSoundState = SND_CHANGE_PITCH; // hack for going through level transitions
	}
	else
	{
		// UNDONE: this needs to send different sounds to every player for multiplayer.	
		if (CBaseEntity* pPlayer = UTIL_FindEntityByClassname(nullptr, "player"); pPlayer)
		{
			float pitch = DotProduct(m_velocity - pPlayer->pev->velocity, (pPlayer->pev->origin - pev->origin).Normalize());

			pitch = (int)(100 + pitch / 75.0);

			pitch = std::clamp(pitch, 50.0f, 250.0f);

			if (pitch == 100)
				pitch = 101;

			if (pitch != m_iPitch)
			{
				m_iPitch = pitch;
				EmitSound(SoundChannel::Static, "apache/ap_rotor4.wav", VOL_NORM, 0.15, pitch, SND_CHANGE_PITCH | SND_CHANGE_VOL);
				// ALERT( at_console, "%.0f\n", pitch );
			}
		}
		// EmitSound(SoundChannel::Static, "apache/ap_whine1.wav", flVol, 0.2, pitch, SND_CHANGE_PITCH | SND_CHANGE_VOL);
	}
}

void COsprey::HitTouch(CBaseEntity* pOther)
{
	pev->nextthink = gpGlobals->time + 2.0;
}

/*
bool COsprey::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	if (m_flRotortilt <= -90)
	{
		m_flRotortilt = 0;
	}
	else
	{
		m_flRotortilt -= 45;
	}
	SetBoneController( 0, m_flRotortilt );
	return false;
}
*/

void COsprey::Killed(const KilledInfo& info)
{
	pev->movetype = Movetype::Toss;
	pev->gravity = 0.3;
	pev->velocity = m_velocity;
	pev->avelocity = Vector(RANDOM_FLOAT(-20, 20), 0, RANDOM_FLOAT(-50, 50));
	StopSound(SoundChannel::Static, "apache/ap_rotor4.wav");

	UTIL_SetSize(pev, Vector(-32, -32, -64), Vector(32, 32, 0));
	SetThink(&COsprey::DyingThink);
	SetTouch(&COsprey::CrashTouch);
	pev->nextthink = gpGlobals->time + 0.1;
	pev->health = 0;
	SetDamageMode(DamageMode::No);

	m_startTime = gpGlobals->time + 4.0;
}

void COsprey::CrashTouch(CBaseEntity* pOther)
{
	// only crash if we hit something solid
	if (pOther->pev->solid == Solid::BSP)
	{
		SetTouch(nullptr);
		m_startTime = gpGlobals->time;
		pev->nextthink = gpGlobals->time;
		m_velocity = pev->velocity;
	}
}

void COsprey::DyingThink()
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	pev->avelocity = pev->avelocity * 1.02;

	// still falling?
	if (m_startTime > gpGlobals->time)
	{
		UTIL_MakeAimVectors(pev->angles);
		ShowDamage();

		Vector vecSpot = pev->origin + pev->velocity * 0.2;

		// random explosions
		MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, vecSpot);
		WRITE_BYTE(TE_EXPLOSION);		// This just makes a dynamic light now
		WRITE_COORD(vecSpot.x + RANDOM_FLOAT(-150, 150));
		WRITE_COORD(vecSpot.y + RANDOM_FLOAT(-150, 150));
		WRITE_COORD(vecSpot.z + RANDOM_FLOAT(-150, -50));
		WRITE_SHORT(g_sModelIndexFireball);
		WRITE_BYTE(RANDOM_LONG(0, 29) + 30); // scale * 10
		WRITE_BYTE(12); // framerate
		WRITE_BYTE(TE_EXPLFLAG_NONE);
		MESSAGE_END();

		// lots of smoke
		MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, vecSpot);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(vecSpot.x + RANDOM_FLOAT(-150, 150));
		WRITE_COORD(vecSpot.y + RANDOM_FLOAT(-150, 150));
		WRITE_COORD(vecSpot.z + RANDOM_FLOAT(-150, -50));
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE(100); // scale * 10
		WRITE_BYTE(10); // framerate
		MESSAGE_END();

		vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;
		MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, vecSpot);
		WRITE_BYTE(TE_BREAKMODEL);

		// position
		WRITE_COORD(vecSpot.x);
		WRITE_COORD(vecSpot.y);
		WRITE_COORD(vecSpot.z);

		// size
		WRITE_COORD(800);
		WRITE_COORD(800);
		WRITE_COORD(132);

		// velocity
		WRITE_COORD(pev->velocity.x);
		WRITE_COORD(pev->velocity.y);
		WRITE_COORD(pev->velocity.z);

		// randomization
		WRITE_BYTE(50);

		// Model
		WRITE_SHORT(m_iTailGibs);	//model id#

		// # of shards
		WRITE_BYTE(8);	// let client decide

		// duration
		WRITE_BYTE(200);// 10.0 seconds

		// flags

		WRITE_BYTE(BREAK_METAL);
		MESSAGE_END();

		// don't stop it we touch a entity
		pev->flags &= ~FL_ONGROUND;
		pev->nextthink = gpGlobals->time + 0.2;
		return;
	}
	else
	{
		Vector vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;

		/*
		MESSAGE_BEGIN( MessageDest::Broadcast, SVC_TEMPENTITY );
			WRITE_BYTE( TE_EXPLOSION);		// This just makes a dynamic light now
			WRITE_COORD( vecSpot.x );
			WRITE_COORD( vecSpot.y );
			WRITE_COORD( vecSpot.z + 512 );
			WRITE_SHORT( m_iExplode );
			WRITE_BYTE( 250 ); // scale * 10
			WRITE_BYTE( 10  ); // framerate
		MESSAGE_END();
		*/

		// gibs
		MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, vecSpot);
		WRITE_BYTE(TE_SPRITE);
		WRITE_COORD(vecSpot.x);
		WRITE_COORD(vecSpot.y);
		WRITE_COORD(vecSpot.z + 512);
		WRITE_SHORT(m_iExplode);
		WRITE_BYTE(250); // scale * 10
		WRITE_BYTE(255); // brightness
		MESSAGE_END();

		/*
		MESSAGE_BEGIN( MessageDest::Broadcast, SVC_TEMPENTITY );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( vecSpot.x );
			WRITE_COORD( vecSpot.y );
			WRITE_COORD( vecSpot.z + 300 );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 250 ); // scale * 10
			WRITE_BYTE( 6  ); // framerate
		MESSAGE_END();
		*/

		// blast circle
		MESSAGE_BEGIN(MessageDest::PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_BEAMCYLINDER);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 2000); // reach damage radius over .2 seconds
		WRITE_SHORT(m_iSpriteTexture);
		WRITE_BYTE(0); // startframe
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(4); // life
		WRITE_BYTE(32);  // width
		WRITE_BYTE(0);   // noise
		WRITE_BYTE(255);   // r, g, b
		WRITE_BYTE(255);   // r, g, b
		WRITE_BYTE(192);   // r, g, b
		WRITE_BYTE(128); // brightness
		WRITE_BYTE(0);		// speed
		MESSAGE_END();

		EmitSound(SoundChannel::Static, "weapons/mortarhit.wav", VOL_NORM, 0.3);

		RadiusDamage(pev->origin, pev, pev, 300, CLASS_NONE, DMG_BLAST);

		// gibs
		vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;
		MESSAGE_BEGIN(MessageDest::PAS, SVC_TEMPENTITY, vecSpot);
		WRITE_BYTE(TE_BREAKMODEL);

		// position
		WRITE_COORD(vecSpot.x);
		WRITE_COORD(vecSpot.y);
		WRITE_COORD(vecSpot.z + 64);

		// size
		WRITE_COORD(800);
		WRITE_COORD(800);
		WRITE_COORD(128);

		// velocity
		WRITE_COORD(m_velocity.x);
		WRITE_COORD(m_velocity.y);
		WRITE_COORD(fabs(m_velocity.z) * 0.25);

		// randomization
		WRITE_BYTE(40);

		// Model
		WRITE_SHORT(m_iBodyGibs);	//model id#

		// # of shards
		WRITE_BYTE(128);

		// duration
		WRITE_BYTE(200);// 10.0 seconds

		// flags

		WRITE_BYTE(BREAK_METAL);
		MESSAGE_END();

		UTIL_Remove(this);
	}
}

void COsprey::ShowDamage()
{
	if (m_iDoLeftSmokePuff > 0 || RANDOM_LONG(0, 99) > m_flLeftHealth)
	{
		const Vector vecSrc = pev->origin + gpGlobals->v_right * -340;
		MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, vecSrc);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(vecSrc.x);
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE(RANDOM_LONG(0, 9) + 20); // scale * 10
		WRITE_BYTE(12); // framerate
		MESSAGE_END();
		if (m_iDoLeftSmokePuff > 0)
			m_iDoLeftSmokePuff--;
	}

	if (m_iDoRightSmokePuff > 0 || RANDOM_LONG(0, 99) > m_flRightHealth)
	{
		const Vector vecSrc = pev->origin + gpGlobals->v_right * 340;
		MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, vecSrc);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(vecSrc.x);
		WRITE_COORD(vecSrc.y);
		WRITE_COORD(vecSrc.z);
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE(RANDOM_LONG(0, 9) + 20); // scale * 10
		WRITE_BYTE(12); // framerate
		MESSAGE_END();
		if (m_iDoRightSmokePuff > 0)
			m_iDoRightSmokePuff--;
	}
}

void COsprey::TraceAttack(const TraceAttackInfo& info)
{
	// ALERT( at_console, "%d %.0f\n", ptr->iHitgroup, flDamage );

	// only so much per engine
	if (info.GetTraceResult().iHitgroup == 3)
	{
		if (m_flRightHealth < 0)
			return;
		else
			m_flRightHealth -= info.GetDamage();
		m_iDoLeftSmokePuff = 3 + (info.GetDamage() / 5.0);
	}

	if (info.GetTraceResult().iHitgroup == 2)
	{
		if (m_flLeftHealth < 0)
			return;
		else
			m_flLeftHealth -= info.GetDamage();
		m_iDoRightSmokePuff = 3 + (info.GetDamage() / 5.0);
	}

	// hit hard, hits cockpit, hits engines
	if (info.GetDamage() > 50 || info.GetTraceResult().iHitgroup == 1 || info.GetTraceResult().iHitgroup == 2 || info.GetTraceResult().iHitgroup == 3)
	{
		// ALERT( at_console, "%.0f\n", flDamage );
		AddMultiDamage(info.GetAttacker(), this, info.GetDamage(), info.GetDamageTypes());
	}
	else
	{
		UTIL_Sparks(info.GetTraceResult().vecEndPos);
	}
}
