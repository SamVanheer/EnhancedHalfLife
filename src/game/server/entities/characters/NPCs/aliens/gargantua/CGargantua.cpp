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

#include "customentity.hpp"
#include "CBreakable.hpp"
#include "CGargantua.hpp"
#include "CSpiral.hpp"
#include "CStomp.hpp"
#include "effects/CEnvExplosion.hpp"

LINK_ENTITY_TO_CLASS(monster_gargantua, CGargantua);

const char* CGargantua::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CGargantua::pBeamAttackSounds[] =
{
	"garg/gar_flameoff1.wav",
	"garg/gar_flameon1.wav",
	"garg/gar_flamerun1.wav",
};

const char* CGargantua::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char* CGargantua::pRicSounds[] =
{
#if 0
	"weapons/ric1.wav",
	"weapons/ric2.wav",
	"weapons/ric3.wav",
	"weapons/ric4.wav",
	"weapons/ric5.wav",
#else
	"debris/metal4.wav",
	"debris/metal6.wav",
	"weapons/ric4.wav",
	"weapons/ric5.wav",
#endif
};

const char* CGargantua::pFootSounds[] =
{
	"garg/gar_step1.wav",
	"garg/gar_step2.wav",
};

const char* CGargantua::pIdleSounds[] =
{
	"garg/gar_idle1.wav",
	"garg/gar_idle2.wav",
	"garg/gar_idle3.wav",
	"garg/gar_idle4.wav",
	"garg/gar_idle5.wav",
};

const char* CGargantua::pAttackSounds[] =
{
	"garg/gar_attack1.wav",
	"garg/gar_attack2.wav",
	"garg/gar_attack3.wav",
};

const char* CGargantua::pAlertSounds[] =
{
	"garg/gar_alert1.wav",
	"garg/gar_alert2.wav",
	"garg/gar_alert3.wav",
};

const char* CGargantua::pPainSounds[] =
{
	"garg/gar_pain1.wav",
	"garg/gar_pain2.wav",
	"garg/gar_pain3.wav",
};

const char* CGargantua::pStompSounds[] =
{
	"garg/gar_stomp1.wav",
};

const char* CGargantua::pBreatheSounds[] =
{
	"garg/gar_breathe1.wav",
	"garg/gar_breathe2.wav",
	"garg/gar_breathe3.wav",
};
//=========================================================
// AI Schedules Specific to this monster
//=========================================================
#if 0
enum
{
	SCHED_ = LAST_COMMON_SCHEDULE + 1,
};
#endif

enum
{
	TASK_SOUND_ATTACK = LAST_COMMON_TASK + 1,
	TASK_FLAME_SWEEP,
};

Task_t	tlGargFlame[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_SOUND_ATTACK,		(float)0		},
	// { TASK_PLAY_SEQUENCE,		(float)ACT_SIGNAL1	},
	{ TASK_SET_ACTIVITY,		(float)ACT_MELEE_ATTACK2 },
	{ TASK_FLAME_SWEEP,			(float)4.5		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
};

Schedule_t	slGargFlame[] =
{
	{
		tlGargFlame,
		ArraySize(tlGargFlame),
		0,
		0,
		"GargFlame"
	},
};

// primary melee attack
Task_t	tlGargSwipe[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MELEE_ATTACK1,		(float)0		},
};

Schedule_t	slGargSwipe[] =
{
	{
		tlGargSwipe,
		ArraySize(tlGargSwipe),
		bits_COND_CAN_MELEE_ATTACK2,
		0,
		"GargSwipe"
	},
};

DEFINE_CUSTOM_SCHEDULES(CGargantua)
{
	slGargFlame,
		slGargSwipe,
};

IMPLEMENT_CUSTOM_SCHEDULES(CGargantua, CBaseMonster);

void CGargantua::EyeOn(int level)
{
	m_eyeBrightness = level;
}

void CGargantua::EyeOff()
{
	m_eyeBrightness = 0;
}

void CGargantua::EyeUpdate()
{
	if (auto glow = m_hEyeGlow.Get(); glow)
	{
		glow->SetRenderAmount(UTIL_Approach(m_eyeBrightness, glow->GetRenderAmount(), 26));
		if (glow->GetRenderAmount() == 0)
			glow->pev->effects |= EF_NODRAW;
		else
			glow->pev->effects &= ~EF_NODRAW;
		glow->SetAbsOrigin(GetAbsOrigin());
	}
}

void CGargantua::StompAttack()
{
	UTIL_MakeVectors(GetAbsAngles());
	const Vector vecStart = GetAbsOrigin() + Vector(0, 0, 60) + 35 * gpGlobals->v_forward;
	const Vector vecAim = ShootAtEnemy(vecStart);
	const Vector vecEnd = (vecAim * 1024) + vecStart;

	TraceResult trace;
	UTIL_TraceLine(vecStart, vecEnd, IgnoreMonsters::Yes, this, &trace);
	CStomp::StompCreate(vecStart, trace.vecEndPos, 0);
	UTIL_ScreenShake(GetAbsOrigin(), 12.0, 100.0, 2.0, 1000);
	EmitSound(SoundChannel::Weapon, pStompSounds[RANDOM_LONG(0, ArraySize(pStompSounds) - 1)], VOL_NORM, ATTN_GARG, PITCH_NORM + RANDOM_LONG(-10, 10));

	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() - Vector(0, 0, 20), IgnoreMonsters::Yes, this, &trace);
	if (trace.flFraction < 1.0)
		UTIL_DecalTrace(&trace, DECAL_GARGSTOMP1);
}

void CGargantua::FlameCreate()
{
	Vector posGun, angleGun;
	TraceResult trace;

	UTIL_MakeVectors(GetAbsAngles());

	for (int i = 0; i < 4; i++)
	{
		if (i < 2)
			m_hFlame[i] = CBeam::BeamCreate(GARG_BEAM_SPRITE_NAME.data(), 240);
		else
			m_hFlame[i] = CBeam::BeamCreate(GARG_BEAM_SPRITE2.data(), 140);
		if (auto flame = m_hFlame[i].Get(); flame)
		{
			int attach = i % 2;
			// attachment is 0 based in GetAttachment
			GetAttachment(attach + 1, posGun, angleGun);

			Vector vecEnd = (gpGlobals->v_forward * GARG_FLAME_LENGTH) + posGun;
			UTIL_TraceLine(posGun, vecEnd, IgnoreMonsters::No, this, &trace);

			flame->PointEntInit(trace.vecEndPos, entindex());
			if (i < 2)
				flame->SetColor(255, 130, 90);
			else
				flame->SetColor(0, 120, 255);
			flame->SetBrightness(190);
			flame->SetFlags(BEAM_FSHADEIN);
			flame->SetScrollRate(20);
			// attachment is 1 based in SetEndAttachment
			flame->SetEndAttachment(attach + 2);
			CSoundEnt::InsertSound(bits_SOUND_COMBAT, posGun, 384, 0.3);
		}
	}
	EmitSound(SoundChannel::Body, pBeamAttackSounds[1]);
	EmitSound(SoundChannel::Weapon, pBeamAttackSounds[2]);
}

void CGargantua::FlameControls(float angleX, float angleY)
{
	if (angleY < -180)
		angleY += 360;
	else if (angleY > 180)
		angleY -= 360;

	if (angleY < -45)
		angleY = -45;
	else if (angleY > 45)
		angleY = 45;

	m_flameX = UTIL_ApproachAngle(angleX, m_flameX, 4);
	m_flameY = UTIL_ApproachAngle(angleY, m_flameY, 8);
	SetBoneController(0, m_flameY);
	SetBoneController(1, m_flameX);
}

void CGargantua::FlameUpdate()
{
	static constexpr float	offset[2] = {60, -60};
	TraceResult trace;
	Vector vecStart, angleGun;
	bool streaks = false;

	for (int i = 0; i < 2; i++)
	{
		if (m_hFlame[i])
		{
			Vector vecAim = GetAbsAngles();
			vecAim.x += m_flameX;
			vecAim.y += m_flameY;

			UTIL_MakeVectors(vecAim);

			GetAttachment(i + 1, vecStart, angleGun);
			Vector vecEnd = vecStart + (gpGlobals->v_forward * GARG_FLAME_LENGTH); //  - offset[i] * gpGlobals->v_right;

			UTIL_TraceLine(vecStart, vecEnd, IgnoreMonsters::No, this, &trace);

			m_hFlame[i]->SetStartPos(trace.vecEndPos);
			m_hFlame[i + 2]->SetStartPos((vecStart * 0.6) + (trace.vecEndPos * 0.4));

			if (trace.flFraction != 1.0 && gpGlobals->time > m_streakTime)
			{
				StreakSplash(trace.vecEndPos, trace.vecPlaneNormal, 6, 20, 50, 400);
				streaks = true;
				UTIL_DecalTrace(&trace, DECAL_SMALLSCORCH1 + RANDOM_LONG(0, 2));
			}
			// RadiusDamage( trace.vecEndPos, this, this, gSkillData.gargantuaDmgFire, CLASS_ALIEN_MONSTER, DMG_BURN );
			FlameDamage(vecStart, trace.vecEndPos, this, this, gSkillData.gargantuaDmgFire, CLASS_ALIEN_MONSTER, DMG_BURN);

			MESSAGE_BEGIN(MessageDest::Broadcast, SVC_TEMPENTITY);
			WRITE_BYTE(TE_ELIGHT);
			WRITE_SHORT(entindex() + 0x1000 * (i + 2));		// entity, attachment
			WRITE_COORD(vecStart.x);		// origin
			WRITE_COORD(vecStart.y);
			WRITE_COORD(vecStart.z);
			WRITE_COORD(RANDOM_FLOAT(32, 48));	// radius
			WRITE_BYTE(255);	// R
			WRITE_BYTE(255);	// G
			WRITE_BYTE(255);	// B
			WRITE_BYTE(2);	// life * 10
			WRITE_COORD(0); // decay
			MESSAGE_END();
		}
	}
	if (streaks)
		m_streakTime = gpGlobals->time;
}

void CGargantua::FlameDamage(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pInflictor, CBaseEntity* pAttacker, float flDamage, int iClassIgnore, int bitsDamageType)
{
	CBaseEntity* pEntity = nullptr;
	TraceResult	tr;

	const Vector vecMid = (vecStart + vecEnd) * 0.5;

	const float searchRadius = (vecStart - vecMid).Length();

	const Vector vecAim = (vecEnd - vecStart).Normalize();

	// iterate on all entities in the vicinity.
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecMid, searchRadius)) != nullptr)
	{
		if (pEntity->GetDamageMode() != DamageMode::No)
		{
			// UNDONE: this should check a damage mask, not an ignore
			if (iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore)
			{// houndeyes don't hurt other houndeyes with their attack
				continue;
			}

			const Vector vecSpot = pEntity->BodyTarget(vecMid);

			float dist = DotProduct(vecAim, vecSpot - vecMid);
			if (dist > searchRadius)
				dist = searchRadius;
			else if (dist < -searchRadius)
				dist = searchRadius;

			const Vector vecSrc = vecMid + dist * vecAim;

			UTIL_TraceLine(vecSrc, vecSpot, IgnoreMonsters::No, this, &tr);

			if (tr.flFraction == 1.0 || InstanceOrNull(tr.pHit) == pEntity)
			{// the explosion can 'see' this entity, so hurt them!
				// decrease damage for an ent that's farther from the flame.
				dist = (vecSrc - tr.vecEndPos).Length();

				float flAdjustedDamage;
				if (dist > 64)
				{
					flAdjustedDamage = flDamage - (dist - 64) * 0.4f;
					if (flAdjustedDamage <= 0)
						continue;
				}
				else
				{
					flAdjustedDamage = flDamage;
				}

				// ALERT( at_console, "hit %s\n", pEntity->GetClassname() );
				if (tr.flFraction != 1.0)
				{
					ClearMultiDamage();
					pEntity->TraceAttack({pInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), tr, bitsDamageType});
					ApplyMultiDamage(pInflictor, pAttacker);
				}
				else
				{
					pEntity->TakeDamage({pInflictor, pAttacker, flAdjustedDamage, bitsDamageType});
				}
			}
		}
	}
}

void CGargantua::FlameDestroy()
{
	EmitSound(SoundChannel::Weapon, pBeamAttackSounds[0]);

	for (auto& flameHandle : m_hFlame)
	{
		flameHandle.Remove();
	}
}

void CGargantua::PrescheduleThink()
{
	if (!HasConditions(bits_COND_SEE_ENEMY))
	{
		m_seeTime = gpGlobals->time + 5;
		EyeOff();
	}
	else
		EyeOn(200);

	EyeUpdate();
}

int	CGargantua::Classify()
{
	return CLASS_ALIEN_MONSTER;
}

void CGargantua::SetYawSpeed()
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
		ys = 60;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 180;
		break;
	case ACT_WALK:
	case ACT_RUN:
		ys = 60;
		break;

	default:
		ys = 60;
		break;
	}

	pev->yaw_speed = ys;
}

void CGargantua::Spawn()
{
	Precache();

	SetModel("models/garg.mdl");
	SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));

	SetSolidType(Solid::SlideBox);
	SetMovetype(Movetype::Step);
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->health = gSkillData.gargantuaHealth;
	//pev->view_ofs		= Vector ( 0, 0, 96 );// taken from mdl file
	m_flFieldOfView = -0.2;// width of forward view cone ( as a dotproduct result )
	m_MonsterState = NPCState::None;

	MonsterInit();

	auto glow = m_hEyeGlow = CSprite::SpriteCreate(GARG_EYE_SPRITE_NAME.data(), GetAbsOrigin(), false);
	glow->SetTransparency(RenderMode::Glow, {255, 255, 255}, 0, RenderFX::NoDissipation);
	glow->SetAttachment(this, 1);
	EyeOff();
	m_seeTime = gpGlobals->time + 5;
	m_flameTime = gpGlobals->time + 2;
}

void CGargantua::Precache()
{
	PRECACHE_MODEL("models/garg.mdl");
	PRECACHE_MODEL(GARG_EYE_SPRITE_NAME.data());
	PRECACHE_MODEL(GARG_BEAM_SPRITE_NAME.data());
	PRECACHE_MODEL(GARG_BEAM_SPRITE2.data());
	gStompSprite = PRECACHE_MODEL(GARG_STOMP_SPRITE_NAME.data());
	gGargGibModel = PRECACHE_MODEL(GARG_GIB_MODEL.data());
	PRECACHE_SOUND(GARG_STOMP_BUZZ_SOUND.data());

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pBeamAttackSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pRicSounds);
	PRECACHE_SOUND_ARRAY(pFootSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pStompSounds);
	PRECACHE_SOUND_ARRAY(pBreatheSounds);
}

void CGargantua::TraceAttack(const TraceAttackInfo& info)
{
	ALERT(at_aiconsole, "CGargantua::TraceAttack\n");

	if (!IsAlive())
	{
		CBaseMonster::TraceAttack(info);
		return;
	}

	TraceAttackInfo adjustedInfo = info;

	// UNDONE: Hit group specific damage?
	if (adjustedInfo.GetDamageTypes() & (GARG_DAMAGE | DMG_BLAST))
	{
		if (m_painSoundTime < gpGlobals->time)
		{
			EmitSound(SoundChannel::Voice, pPainSounds[RANDOM_LONG(0, ArraySize(pPainSounds) - 1)], VOL_NORM, ATTN_GARG);
			m_painSoundTime = gpGlobals->time + RANDOM_FLOAT(2.5, 4);
		}
	}

	adjustedInfo.SetDamageTypes(adjustedInfo.GetDamageTypes() & GARG_DAMAGE);

	if (adjustedInfo.GetDamageTypes() == 0)
	{
		if (pev->dmgtime != gpGlobals->time || (RANDOM_LONG(0, 100) < 20))
		{
			UTIL_Ricochet(adjustedInfo.GetTraceResult().vecEndPos, RANDOM_FLOAT(0.5, 1.5));
			pev->dmgtime = gpGlobals->time;
			//			if ( RANDOM_LONG(0,100) < 25 )
			//				EmitSound( SoundChannel::Body, pRicSounds[ RANDOM_LONG(0,ArraySize(pRicSounds)-1) ], VOL_NORM, ATTN_NORM );
		}
		adjustedInfo.SetDamage(0);
	}

	CBaseMonster::TraceAttack(adjustedInfo);

}

bool CGargantua::TakeDamage(const TakeDamageInfo& info)
{
	ALERT(at_aiconsole, "CGargantua::TakeDamage\n");

	TakeDamageInfo adjustedInfo = info;

	if (IsAlive())
	{
		if (!(adjustedInfo.GetDamageTypes() & GARG_DAMAGE))
			adjustedInfo.SetDamage(adjustedInfo.GetDamage() * 0.01);
		if (adjustedInfo.GetDamageTypes() & DMG_BLAST)
			SetConditions(bits_COND_LIGHT_DAMAGE);
	}

	return CBaseMonster::TakeDamage(adjustedInfo);
}

void CGargantua::DeathEffect()
{
	UTIL_MakeVectors(GetAbsAngles());
	const Vector deathPos = GetAbsOrigin() + gpGlobals->v_forward * 100;

	// Create a spiral of streaks
	CSpiral::Create(deathPos, (pev->absmax.z - pev->absmin.z) * 0.6, 125, 1.5);

	Vector position = GetAbsOrigin();
	position.z += 32;
	for (int i = 0; i < 7; i += 2)
	{
		UTIL_CreateExplosion(position, vec3_origin, nullptr, 60 + (i * 20), false, 70, (i * 0.3));
		position.z += 15;
	}

	CBaseEntity* pSmoker = CBaseEntity::Create("env_smoker", GetAbsOrigin(), vec3_origin, nullptr);
	pSmoker->pev->health = 1;	// 1 smoke balls
	pSmoker->pev->scale = 46;	// 4.6X normal size
	pSmoker->pev->dmg = 0;		// 0 radial distribution
	pSmoker->pev->nextthink = gpGlobals->time + 2.5;	// Start in 2.5 seconds
}

void CGargantua::OnRemove()
{
	m_hEyeGlow.Remove();

	for (auto& flameHandle : m_hFlame)
	{
		flameHandle.Remove();
	}

	CBaseMonster::OnRemove();
}

void CGargantua::Killed(const KilledInfo& info)
{
	EyeOff();
	m_hEyeGlow.Remove();
	CBaseMonster::Killed({info.GetInflictor(), info.GetAttacker(), GibType::Never});
}

bool CGargantua::CheckMeleeAttack1(float flDot, float flDist)
{
	//	ALERT(at_aiconsole, "CheckMelee(%f, %f)\n", flDot, flDist);

	if (flDot >= 0.7)
	{
		if (flDist <= GARG_ATTACKDIST)
			return true;
	}
	return false;
}

bool CGargantua::CheckMeleeAttack2(float flDot, float flDist)
{
	//	ALERT(at_aiconsole, "CheckMelee(%f, %f)\n", flDot, flDist);

	if (gpGlobals->time > m_flameTime)
	{
		if (flDot >= 0.8 && flDist > GARG_ATTACKDIST)
		{
			if (flDist <= GARG_FLAME_LENGTH)
				return true;
		}
	}
	return false;
}

bool CGargantua::CheckRangeAttack1(float flDot, float flDist)
{
	if (gpGlobals->time > m_seeTime)
	{
		if (flDot >= 0.7 && flDist > GARG_ATTACKDIST)
		{
			return true;
		}
	}
	return false;
}

void CGargantua::HandleAnimEvent(AnimationEvent& event)
{
	switch (event.event)
	{
	case GARG_AE_SLASH_LEFT:
	{
		// HACKHACK!!!
		if (CBaseEntity* pHurt = GargantuaCheckTraceHullAttack(GARG_ATTACKDIST + 10.0, gSkillData.gargantuaDmgSlash, DMG_SLASH); pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				pHurt->pev->punchangle.x = -30; // pitch
				pHurt->pev->punchangle.y = -30;	// yaw
				pHurt->pev->punchangle.z = 30;	// roll
				//UTIL_MakeVectors(GetAbsAngles());	// called by CheckTraceHullAttack
				pHurt->SetAbsVelocity(pHurt->GetAbsVelocity() - gpGlobals->v_right * 100);
			}
			EmitSound(SoundChannel::Weapon, pAttackHitSounds[RANDOM_LONG(0, ArraySize(pAttackHitSounds) - 1)], VOL_NORM, ATTN_NORM, 50 + RANDOM_LONG(0, 15));
		}
		else // Play a random attack miss sound
			EmitSound(SoundChannel::Weapon, pAttackMissSounds[RANDOM_LONG(0, ArraySize(pAttackMissSounds) - 1)], VOL_NORM, ATTN_NORM, 50 + RANDOM_LONG(0, 15));

		Vector forward;
		AngleVectors(GetAbsAngles(), &forward, nullptr, nullptr);
	}
	break;

	case GARG_AE_RIGHT_FOOT:
	case GARG_AE_LEFT_FOOT:
		UTIL_ScreenShake(GetAbsOrigin(), 4.0, 3.0, 1.0, 750);
		EmitSound(SoundChannel::Body, pFootSounds[RANDOM_LONG(0, ArraySize(pFootSounds) - 1)], VOL_NORM, ATTN_GARG, PITCH_NORM + RANDOM_LONG(-10, 10));
		break;

	case GARG_AE_STOMP:
		StompAttack();
		m_seeTime = gpGlobals->time + 12;
		break;

	case GARG_AE_BREATHE:
		EmitSound(SoundChannel::Voice, pBreatheSounds[RANDOM_LONG(0, ArraySize(pBreatheSounds) - 1)], VOL_NORM, ATTN_GARG, PITCH_NORM + RANDOM_LONG(-10, 10));
		break;

	default:
		CBaseMonster::HandleAnimEvent(event);
		break;
	}
}

CBaseEntity* CGargantua::GargantuaCheckTraceHullAttack(float flDist, int iDamage, int iDmgType)
{
	TraceResult tr;

	UTIL_MakeVectors(GetAbsAngles());
	Vector vecStart = GetAbsOrigin();
	vecStart.z += 64;
	const Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist) - (gpGlobals->v_up * flDist * 0.3);

	UTIL_TraceHull(vecStart, vecEnd, IgnoreMonsters::No, Hull::Head, this, &tr);

	if (tr.pHit)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		if (iDamage > 0)
		{
			pEntity->TakeDamage({this, this, static_cast<float>(iDamage), iDmgType});
		}

		return pEntity;
	}

	return nullptr;
}

Schedule_t* CGargantua::GetScheduleOfType(int Type)
{
	// HACKHACK - turn off the flames if they are on and garg goes scripted / dead
	if (FlameIsOn())
		FlameDestroy();

	switch (Type)
	{
	case SCHED_MELEE_ATTACK2:
		return slGargFlame;
	case SCHED_MELEE_ATTACK1:
		return slGargSwipe;
		break;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

void CGargantua::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_FLAME_SWEEP:
		FlameCreate();
		m_flWaitFinished = gpGlobals->time + pTask->flData;
		m_flameTime = gpGlobals->time + 6;
		m_flameX = 0;
		m_flameY = 0;
		break;

	case TASK_SOUND_ATTACK:
		if (RANDOM_LONG(0, 100) < 30)
			EmitSound(SoundChannel::Voice, pAttackSounds[RANDOM_LONG(0, ArraySize(pAttackSounds) - 1)], VOL_NORM, ATTN_GARG);
		TaskComplete();
		break;

	case TASK_DIE:
		m_flWaitFinished = gpGlobals->time + 1.6;
		DeathEffect();

		[[fallthrough]];

	default:
		CBaseMonster::StartTask(pTask);
		break;
	}
}

void CGargantua::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_DIE:
		if (gpGlobals->time > m_flWaitFinished)
		{
			SetRenderFX(RenderFX::Explode);
			SetRenderColor({255, 0, 0});
			StopAnimation();
			pev->nextthink = gpGlobals->time + 0.15;
			SetThink(&CGargantua::SUB_Remove);
			const int parts = MODEL_FRAMES(gGargGibModel);
			for (int i = 0; i < 10; i++)
			{
				CGib* pGib = GetClassPtr((CGib*)nullptr);

				pGib->Spawn(GARG_GIB_MODEL.data());

				const int bodyPart = parts > 1 ? RANDOM_LONG(0, pev->body - 1) : 0;

				pGib->pev->body = bodyPart;
				pGib->m_bloodColor = BLOOD_COLOR_YELLOW;
				pGib->m_material = Materials::None;
				pGib->SetAbsOrigin(GetAbsOrigin());
				pGib->SetAbsVelocity(UTIL_RandomBloodVector() * RANDOM_FLOAT(300, 500));
				pGib->pev->nextthink = gpGlobals->time + 1.25;
				pGib->SetThink(&CGib::SUB_FadeOut);
			}
			MESSAGE_BEGIN(MessageDest::PVS, SVC_TEMPENTITY, GetAbsOrigin());
			WRITE_BYTE(TE_BREAKMODEL);

			// position
			WRITE_COORD(GetAbsOrigin().x);
			WRITE_COORD(GetAbsOrigin().y);
			WRITE_COORD(GetAbsOrigin().z);

			// size
			WRITE_COORD(200);
			WRITE_COORD(200);
			WRITE_COORD(128);

			// velocity
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);

			// randomization
			WRITE_BYTE(200);

			// Model
			WRITE_SHORT(gGargGibModel);	//model id#

			// # of shards
			WRITE_BYTE(50);

			// duration
			WRITE_BYTE(20);// 3.0 seconds

			// flags

			WRITE_BYTE(BREAK_FLESH);
			MESSAGE_END();

			return;
		}
		else
			CBaseMonster::RunTask(pTask);
		break;

	case TASK_FLAME_SWEEP:
		if (gpGlobals->time > m_flWaitFinished)
		{
			FlameDestroy();
			TaskComplete();
			FlameControls(0, 0);
			SetBoneController(0, 0);
			SetBoneController(1, 0);
		}
		else
		{
			bool cancel = false;

			Vector angles = vec3_origin;

			FlameUpdate();

			if (CBaseEntity* pEnemy = m_hEnemy; pEnemy)
			{
				Vector org = GetAbsOrigin();
				org.z += 64;
				Vector dir = pEnemy->BodyTarget(org) - org;
				angles = VectorAngles(dir);
				angles.x = -angles.x;
				angles.y -= GetAbsAngles().y;
				if (dir.Length() > 400)
					cancel = true;
			}
			if (fabs(angles.y) > 60)
				cancel = true;

			if (cancel)
			{
				m_flWaitFinished -= 0.5;
				m_flameTime -= 0.5;
			}
			// FlameControls( angles.x + 2 * sin(gpGlobals->time*8), angles.y + 28 * sin(gpGlobals->time*8.5) );
			FlameControls(angles.x, angles.y);
		}
		break;

	default:
		CBaseMonster::RunTask(pTask);
		break;
	}
}
