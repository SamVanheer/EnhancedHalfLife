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

/**
*	@file
*
*	Default behaviors.
*/

#include "CBaseMonster.defaultai.hpp"

Task_t	tlFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slFail[] =
{
	{
		tlFail,
		ArraySize(tlFail),
		bits_COND_CAN_ATTACK,
		0,
		"Fail"
	},
};

Task_t	tlIdleStand1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)5		},// repick IDLESTAND every five seconds. gives us a chance to pick an active idle, fidget, etc.
};

Schedule_t	slIdleStand[] =
{
	{
		tlIdleStand1,
		ArraySize(tlIdleStand1),
		bits_COND_NEW_ENEMY |
		bits_COND_SEE_FEAR |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_SMELL_FOOD |
		bits_COND_SMELL |
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT |// sound flags
		bits_SOUND_WORLD |
		bits_SOUND_PLAYER |
		bits_SOUND_DANGER |

		bits_SOUND_MEAT |// scents
		bits_SOUND_CARCASS |
		bits_SOUND_GARBAGE,
		"IdleStand"
	},
};

Schedule_t	slIdleTrigger[] =
{
	{
		tlIdleStand1,
		ArraySize(tlIdleStand1),
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		0,
		"Idle Trigger"
	},
};


Task_t	tlIdleWalk1[] =
{
	{ TASK_WALK_PATH,			(float)9999 },
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0	},
};

Schedule_t	slIdleWalk[] =
{
	{
		tlIdleWalk1,
		ArraySize(tlIdleWalk1),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_SMELL_FOOD |
		bits_COND_SMELL |
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT |// sound flags

		bits_SOUND_MEAT |// scents
		bits_SOUND_CARCASS |
		bits_SOUND_GARBAGE,
		"Idle Walk"
	},
};

Task_t	tlAmbush[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT_INDEFINITE,		(float)0		},
};

/**
*	@brief monster stands in place and waits for a new enemy, or chance to attack an existing enemy.
*/
Schedule_t	slAmbush[] =
{
	{
		tlAmbush,
		ArraySize(tlAmbush),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_PROVOKED,

		0,
		"Ambush"
	},
};

//=========================================================
// ActiveIdle schedule - !!!BUGBUG - if this schedule doesn't
// complete on its own, the monster's HintNode will not be 
// cleared, and the rest of the monster's group will avoid
// that node because they think the group member that was 
// previously interrupted is still using that node to active
// idle.
///=========================================================
Task_t tlActiveIdle[] =
{
	{ TASK_FIND_HINTNODE,			(float)0	},
	{ TASK_GET_PATH_TO_HINTNODE,	(float)0	},
	{ TASK_STORE_LASTPOSITION,		(float)0	},
	{ TASK_WALK_PATH,				(float)0	},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0	},
	{ TASK_FACE_HINTNODE,			(float)0	},
	{ TASK_PLAY_ACTIVE_IDLE,		(float)0	},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0	},
	{ TASK_WALK_PATH,				(float)0	},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0	},
	{ TASK_CLEAR_LASTPOSITION,		(float)0	},
	{ TASK_CLEAR_HINTNODE,			(float)0	},
};

/**
*	@brief !!!BUGBUG - if this schedule doesn't complete on its own, the monster's HintNode will not be cleared,
*	and the rest of the monster's group will avoid that node because they think the group member
*	that was previously interrupted is still using that node to active idle.
*/
Schedule_t slActiveIdle[] =
{
	{
		tlActiveIdle,
		ArraySize(tlActiveIdle),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_PROVOKED |
		bits_COND_HEAR_SOUND,

		bits_SOUND_COMBAT |
		bits_SOUND_WORLD |
		bits_SOUND_PLAYER |
		bits_SOUND_DANGER,
		"Active Idle"
	}
};

Task_t tlWakeAngry1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SOUND_WAKE,			(float)0	},
	{ TASK_FACE_IDEAL,			(float)0	},
};

Schedule_t slWakeAngry[] =
{
	{
		tlWakeAngry1,
		ArraySize(tlWakeAngry1),
		0,
		0,
		"Wake Angry"
	}
};

Task_t	tlAlertFace1[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE	},
	{ TASK_FACE_IDEAL,				(float)0		},
};

Schedule_t	slAlertFace[] =
{
	{
		tlAlertFace1,
		ArraySize(tlAlertFace1),
		bits_COND_NEW_ENEMY |
		bits_COND_SEE_FEAR |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_PROVOKED,
		0,
		"Alert Face"
	},
};

Task_t	tlAlertSmallFlinch[] =
{
	{ TASK_STOP_MOVING,				0						},
	{ TASK_REMEMBER,				(float)bits_MEMORY_FLINCHED },
	{ TASK_SMALL_FLINCH,			(float)0				},
	{ TASK_SET_SCHEDULE,			(float)SCHED_ALERT_FACE	},
};

/**
*	@brief shot, but didn't see attacker, flinch then face
*/
Schedule_t	slAlertSmallFlinch[] =
{
	{
		tlAlertSmallFlinch,
		ArraySize(tlAlertSmallFlinch),
		0,
		0,
		"Alert Small Flinch"
	},
};

Task_t	tlAlertStand1[] =
{
	{ TASK_STOP_MOVING,			0						 },
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE			 },
	{ TASK_WAIT,				(float)20				 },
	{ TASK_SUGGEST_STATE,		(float)NPCState::Idle },
};

Schedule_t	slAlertStand[] =
{
	{
		tlAlertStand1,
		ArraySize(tlAlertStand1),
		bits_COND_NEW_ENEMY |
		bits_COND_SEE_ENEMY |
		bits_COND_SEE_FEAR |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_PROVOKED |
		bits_COND_SMELL |
		bits_COND_SMELL_FOOD |
		bits_COND_HEAR_SOUND,

		bits_SOUND_COMBAT |// sound flags
		bits_SOUND_WORLD |
		bits_SOUND_PLAYER |
		bits_SOUND_DANGER |

		bits_SOUND_MEAT |// scent flags
		bits_SOUND_CARCASS |
		bits_SOUND_GARBAGE,
		"Alert Stand"
	},
};

Task_t tlInvestigateSound[] =
{
	{ TASK_STOP_MOVING,				(float)0				},
	{ TASK_STORE_LASTPOSITION,		(float)0				},
	{ TASK_GET_PATH_TO_BESTSOUND,	(float)0				},
	{ TASK_FACE_IDEAL,				(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_IDLE			},
	{ TASK_WAIT,					(float)10				},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
};

/**
*	@brief sends a monster to the location of the sound that was just heard, to check things out.
*/
Schedule_t	slInvestigateSound[] =
{
	{
		tlInvestigateSound,
		ArraySize(tlInvestigateSound),
		bits_COND_NEW_ENEMY |
		bits_COND_SEE_FEAR |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"InvestigateSound"
	},
};

Task_t	tlCombatStand1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT_INDEFINITE,		(float)0		},
};

Schedule_t	slCombatStand[] =
{
	{
		tlCombatStand1,
		ArraySize(tlCombatStand1),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_CAN_ATTACK,
		0,
		"Combat Stand"
	},
};

Task_t	tlCombatFace1[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE	},
	{ TASK_FACE_ENEMY,				(float)0		},
};

Schedule_t	slCombatFace[] =
{
	{
		tlCombatFace1,
		ArraySize(tlCombatFace1),
		bits_COND_CAN_ATTACK |
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD,
		0,
		"Combat Face"
	},
};

Task_t	tlStandoff[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_WAIT_FACE_ENEMY,			(float)2					},
};

/**
*	@brief Used in combat when a monster is hiding in cover or the enemy has moved out of sight.
*	Should we look around in this schedule?
*/
Schedule_t slStandoff[] =
{
	{
		tlStandoff,
		ArraySize(tlStandoff),
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_ENEMY_DEAD |
		bits_COND_NEW_ENEMY |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"Standoff"
	}
};

Task_t	tlArmWeapon[] =
{
	{ TASK_STOP_MOVING,		0				},
	{ TASK_PLAY_SEQUENCE,	(float)ACT_ARM }
};

/**
*	@brief Arm weapon (draw gun)
*/
Schedule_t slArmWeapon[] =
{
	{
		tlArmWeapon,
		ArraySize(tlArmWeapon),
		0,
		0,
		"Arm Weapon"
	}
};

Task_t	tlReload[] =
{
	{ TASK_STOP_MOVING,			0					},
	{ TASK_PLAY_SEQUENCE,		float(ACT_RELOAD)	},
};

Schedule_t slReload[] =
{
	{
		tlReload,
		ArraySize(tlReload),
		bits_COND_HEAVY_DAMAGE,
		0,
		"Reload"
	}
};

Task_t	tlRangeAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

/**
*	@brief primary range attack
*/
Schedule_t	slRangeAttack1[] =
{
	{
		tlRangeAttack1,
		ArraySize(tlRangeAttack1),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_NO_AMMO_LOADED |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"Range Attack1"
	},
};

Task_t	tlRangeAttack2[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_RANGE_ATTACK2,		(float)0		},
};

/**
*	@brief secondary range attack
*/
Schedule_t	slRangeAttack2[] =
{
	{
		tlRangeAttack2,
		ArraySize(tlRangeAttack2),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"Range Attack2"
	},
};

Task_t	tlPrimaryMeleeAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MELEE_ATTACK1,		(float)0		},
};

/**
*	@brief primary melee attack
*/
Schedule_t	slPrimaryMeleeAttack[] =
{
	{
		tlPrimaryMeleeAttack1,
		ArraySize(tlPrimaryMeleeAttack1),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_ENEMY_OCCLUDED,
		0,
		"Primary Melee Attack"
	},
};

Task_t	tlSecondaryMeleeAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MELEE_ATTACK2,		(float)0		},
};

/**
*	@brief secondary melee attack
*/
Schedule_t	slSecondaryMeleeAttack[] =
{
	{
		tlSecondaryMeleeAttack1,
		ArraySize(tlSecondaryMeleeAttack1),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_ENEMY_OCCLUDED,
		0,
		"Secondary Melee Attack"
	},
};

Task_t	tlSpecialAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_SPECIAL_ATTACK1,		(float)0		},
};

/**
*	@brief special attack1
*/
Schedule_t	slSpecialAttack1[] =
{
	{
		tlSpecialAttack1,
		ArraySize(tlSpecialAttack1),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_NO_AMMO_LOADED |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"Special Attack1"
	},
};

Task_t	tlSpecialAttack2[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_SPECIAL_ATTACK2,		(float)0		},
};

/**
*	@brief special attack2
*/
Schedule_t	slSpecialAttack2[] =
{
	{
		tlSpecialAttack2,
		ArraySize(tlSpecialAttack2),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_NO_AMMO_LOADED |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"Special Attack2"
	},
};

Task_t tlChaseEnemy1[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_CHASE_ENEMY_FAILED	},
	{ TASK_GET_PATH_TO_ENEMY,	(float)0		},
	{ TASK_RUN_PATH,			(float)0		},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
};

Schedule_t slChaseEnemy[] =
{
	{
		tlChaseEnemy1,
		ArraySize(tlChaseEnemy1),
		bits_COND_NEW_ENEMY |
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_CAN_MELEE_ATTACK2 |
		bits_COND_TASK_FAILED |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"Chase Enemy"
	},
};

Task_t	tlChaseEnemyFailed[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	//	{ TASK_TURN_LEFT,				(float)179					},
		{ TASK_FACE_ENEMY,				(float)0					},
		{ TASK_WAIT,					(float)1					},
};

Schedule_t	slChaseEnemyFailed[] =
{
	{
		tlChaseEnemyFailed,
		ArraySize(tlChaseEnemyFailed),
		bits_COND_NEW_ENEMY |
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_CAN_MELEE_ATTACK2 |
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"tlChaseEnemyFailed"
	},
};

Task_t tlSmallFlinch[] =
{
	{ TASK_REMEMBER,			(float)bits_MEMORY_FLINCHED },
	{ TASK_STOP_MOVING,			0	},
	{ TASK_SMALL_FLINCH,		0	},
};

/**
*	@brief small flinch, played when minor damage is taken.
*/
Schedule_t slSmallFlinch[] =
{
	{
		tlSmallFlinch,
		ArraySize(tlSmallFlinch),
		0,
		0,
		"Small Flinch"
	},
};

Task_t tlDie1[] =
{
	{ TASK_STOP_MOVING,			0				 },
	{ TASK_SOUND_DIE,		(float)0			 },
	{ TASK_DIE,				(float)0			 },
};

Schedule_t slDie[] =
{
	{
		tlDie1,
		ArraySize(tlDie1),
		0,
		0,
		"Die"
	},
};

Task_t tlVictoryDance[] =
{
	{ TASK_STOP_MOVING,			0							},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_VICTORY_DANCE	},
	{ TASK_WAIT,				(float)0					},
};

Schedule_t slVictoryDance[] =
{
	{
		tlVictoryDance,
		ArraySize(tlVictoryDance),
		0,
		0,
		"Victory Dance"
	},
};

Task_t	tlBarnacleVictimGrab[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_PLAY_SEQUENCE,	(float)ACT_BARNACLE_HIT	 },
	{ TASK_SET_ACTIVITY,	(float)ACT_BARNACLE_PULL },
	{ TASK_WAIT_INDEFINITE,	(float)0				 },// just cycle barnacle pull anim while barnacle hoists. 
};

/**
*	@brief barnacle tongue just hit the monster, so play a hit animation,
*	then play a cycling pull animation as the creature is hoisting the monster.
*/
Schedule_t slBarnacleVictimGrab[] =
{
	{
		tlBarnacleVictimGrab,
		ArraySize(tlBarnacleVictimGrab),
		0,
		0,
		"Barnacle Victim"
	}
};

Task_t	tlBarnacleVictimChomp[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_PLAY_SEQUENCE,	(float)ACT_BARNACLE_CHOMP },
	{ TASK_SET_ACTIVITY,	(float)ACT_BARNACLE_CHEW  },
	{ TASK_WAIT_INDEFINITE,	(float)0				  },// just cycle barnacle pull anim while barnacle hoists. 
};

/**
*	@brief barnacle has pulled the prey to its mouth.
*	Victim should play the BARNCLE_CHOMP animation once, then loop the BARNACLE_CHEW animation indefinitely
*/
Schedule_t slBarnacleVictimChomp[] =
{
	{
		tlBarnacleVictimChomp,
		ArraySize(tlBarnacleVictimChomp),
		0,
		0,
		"Barnacle Chomp"
	}
};

Task_t	tlError[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_WAIT_INDEFINITE,				(float)0 },
};

/**
*	@brief Universal Error Schedule
*/
Schedule_t	slError[] =
{
	{
		tlError,
		ArraySize(tlError),
		0,
		0,
		"Error"
	},
};

Task_t tlScriptedWalk[] =
{
	{ TASK_WALK_TO_TARGET,		(float)TARGET_MOVE_SCRIPTED },
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
	{ TASK_PLANT_ON_SCRIPT,		(float)0		},
	{ TASK_FACE_SCRIPT,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_ENABLE_SCRIPT,		(float)0		},
	{ TASK_WAIT_FOR_SCRIPT,		(float)0		},
	{ TASK_PLAY_SCRIPT,			(float)0		},
};

Schedule_t slWalkToScript[] =
{
	{
		tlScriptedWalk,
		ArraySize(tlScriptedWalk),
		SCRIPT_BREAK_CONDITIONS,
		0,
		"WalkToScript"
	},
};

Task_t tlScriptedRun[] =
{
	{ TASK_RUN_TO_TARGET,		(float)TARGET_MOVE_SCRIPTED },
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
	{ TASK_PLANT_ON_SCRIPT,		(float)0		},
	{ TASK_FACE_SCRIPT,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_ENABLE_SCRIPT,		(float)0		},
	{ TASK_WAIT_FOR_SCRIPT,		(float)0		},
	{ TASK_PLAY_SCRIPT,			(float)0		},
};

Schedule_t slRunToScript[] =
{
	{
		tlScriptedRun,
		ArraySize(tlScriptedRun),
		SCRIPT_BREAK_CONDITIONS,
		0,
		"RunToScript"
	},
};

Task_t tlScriptedWait[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_WAIT_FOR_SCRIPT,		(float)0		},
	{ TASK_PLAY_SCRIPT,			(float)0		},
};

Schedule_t slWaitScript[] =
{
	{
		tlScriptedWait,
		ArraySize(tlScriptedWait),
		SCRIPT_BREAK_CONDITIONS,
		0,
		"WaitForScript"
	},
};

Task_t tlScriptedFace[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_SCRIPT,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_WAIT_FOR_SCRIPT,		(float)0		},
	{ TASK_PLAY_SCRIPT,			(float)0		},
};

Schedule_t slFaceScript[] =
{
	{
		tlScriptedFace,
		ArraySize(tlScriptedFace),
		SCRIPT_BREAK_CONDITIONS,
		0,
		"FaceScript"
	},
};

Task_t	tlCower[] =
{
	{ TASK_STOP_MOVING,			0					},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_COWER	},
};

/**
*	@brief this is what is usually done when attempts to escape danger fail.
*/
Schedule_t	slCower[] =
{
	{
		tlCower,
		ArraySize(tlCower),
		0,
		0,
		"Cower"
	},
};

Task_t	tlTakeCoverFromOrigin[] =
{
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_ORIGIN,		(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

/**
*	@brief move away from where you're currently standing.
*/
Schedule_t	slTakeCoverFromOrigin[] =
{
	{
		tlTakeCoverFromOrigin,
		ArraySize(tlTakeCoverFromOrigin),
		bits_COND_NEW_ENEMY,
		0,
		"TakeCoverFromOrigin"
	},
};

Task_t	tlTakeCoverFromBestSound[] =
{
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

/**
*	@brief hide from the loudest sound source
*/
Schedule_t	slTakeCoverFromBestSound[] =
{
	{
		tlTakeCoverFromBestSound,
		ArraySize(tlTakeCoverFromBestSound),
		bits_COND_NEW_ENEMY,
		0,
		"TakeCoverFromBestSound"
	},
};

Task_t	tlTakeCoverFromEnemy[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	//	{ TASK_TURN_LEFT,				(float)179					},
		{ TASK_FACE_ENEMY,				(float)0					},
		{ TASK_WAIT,					(float)1					},
};

/**
*	@brief Take cover from enemy! Tries lateral cover before node cover!
*/
Schedule_t	slTakeCoverFromEnemy[] =
{
	{
		tlTakeCoverFromEnemy,
		ArraySize(tlTakeCoverFromEnemy),
		bits_COND_NEW_ENEMY,
		0,
		"tlTakeCoverFromEnemy"
	},
};

Schedule_t* CBaseMonster::m_scheduleList[] =
{
	slIdleStand,
	slIdleTrigger,
	slIdleWalk,
	slAmbush,
	slActiveIdle,
	slWakeAngry,
	slAlertFace,
	slAlertSmallFlinch,
	slAlertStand,
	slInvestigateSound,
	slCombatStand,
	slCombatFace,
	slStandoff,
	slArmWeapon,
	slReload,
	slRangeAttack1,
	slRangeAttack2,
	slPrimaryMeleeAttack,
	slSecondaryMeleeAttack,
	slSpecialAttack1,
	slSpecialAttack2,
	slChaseEnemy,
	slChaseEnemyFailed,
	slSmallFlinch,
	slDie,
	slVictoryDance,
	slBarnacleVictimGrab,
	slBarnacleVictimChomp,
	slError,
	slWalkToScript,
	slRunToScript,
	slWaitScript,
	slFaceScript,
	slCower,
	slTakeCoverFromOrigin,
	slTakeCoverFromBestSound,
	slTakeCoverFromEnemy,
	slFail
};

Schedule_t* CBaseMonster::ScheduleFromName(const char* pName)
{
	return ScheduleInList(pName, m_scheduleList, ArraySize(m_scheduleList));
}

Schedule_t* CBaseMonster::ScheduleInList(const char* pName, Schedule_t** pList, int listCount)
{
	if (!pName)
	{
		ALERT(at_console, "%s set to unnamed schedule!\n", GetClassname());
		return nullptr;
	}

	for (int i = 0; i < listCount; i++)
	{
		if (!pList[i]->pName)
		{
			ALERT(at_console, "Unnamed schedule!\n");
			continue;
		}
		if (stricmp(pName, pList[i]->pName) == 0)
			return pList[i];
	}

	return nullptr;
}

Schedule_t* CBaseMonster::GetScheduleOfType(int Type)
{
	//	ALERT ( at_console, "Sched Type:%d\n", Type );
	switch (Type)
	{
		// This is the schedule for scripted sequences AND scripted AI
	case SCHED_AISCRIPT:
	{
		auto cine = m_hCine.Get();
		ASSERT(cine != nullptr);
		if (!cine)
		{
			ALERT(at_aiconsole, "Script failed for %s\n", GetClassname());
			CineCleanup();
			return GetScheduleOfType(SCHED_IDLE_STAND);
		}
		//			else
		//				ALERT( at_aiconsole, "Starting script %s for %s\n", STRING( m_pCine->m_iszPlay ), GetClassname() );

		switch (cine->m_fMoveTo)
		{
		case ScriptedMoveTo::No:
		case ScriptedMoveTo::Instantaneous:
			return slWaitScript;
		case ScriptedMoveTo::Walk:
			return slWalkToScript;
		case ScriptedMoveTo::Run:
			return slRunToScript;
		case ScriptedMoveTo::TurnToFace:
			return slFaceScript;
		}
		break;
	}
	case SCHED_IDLE_STAND:
	{
		if (RANDOM_LONG(0, 14) == 0 && CanActiveIdle())
		{
			return &slActiveIdle[0];
		}

		return &slIdleStand[0];
	}
	case SCHED_IDLE_WALK:
	{
		return &slIdleWalk[0];
	}
	case SCHED_WAIT_TRIGGER:
	{
		return &slIdleTrigger[0];
	}
	case SCHED_WAKE_ANGRY:
	{
		return &slWakeAngry[0];
	}
	case SCHED_ALERT_FACE:
	{
		return &slAlertFace[0];
	}
	case SCHED_ALERT_STAND:
	{
		return &slAlertStand[0];
	}
	case SCHED_COMBAT_STAND:
	{
		return &slCombatStand[0];
	}
	case SCHED_COMBAT_FACE:
	{
		return &slCombatFace[0];
	}
	case SCHED_CHASE_ENEMY:
	{
		return &slChaseEnemy[0];
	}
	case SCHED_CHASE_ENEMY_FAILED:
	{
		return &slFail[0];
	}
	case SCHED_SMALL_FLINCH:
	{
		return &slSmallFlinch[0];
	}
	case SCHED_ALERT_SMALL_FLINCH:
	{
		return &slAlertSmallFlinch[0];
	}
	case SCHED_RELOAD:
	{
		return &slReload[0];
	}
	case SCHED_ARM_WEAPON:
	{
		return &slArmWeapon[0];
	}
	case SCHED_STANDOFF:
	{
		return &slStandoff[0];
	}
	case SCHED_RANGE_ATTACK1:
	{
		return &slRangeAttack1[0];
	}
	case SCHED_RANGE_ATTACK2:
	{
		return &slRangeAttack2[0];
	}
	case SCHED_MELEE_ATTACK1:
	{
		return &slPrimaryMeleeAttack[0];
	}
	case SCHED_MELEE_ATTACK2:
	{
		return &slSecondaryMeleeAttack[0];
	}
	case SCHED_SPECIAL_ATTACK1:
	{
		return &slSpecialAttack1[0];
	}
	case SCHED_SPECIAL_ATTACK2:
	{
		return &slSpecialAttack2[0];
	}
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
	{
		return &slTakeCoverFromBestSound[0];
	}
	case SCHED_TAKE_COVER_FROM_ENEMY:
	{
		return &slTakeCoverFromEnemy[0];
	}
	case SCHED_COWER:
	{
		return &slCower[0];
	}
	case SCHED_AMBUSH:
	{
		return &slAmbush[0];
	}
	case SCHED_BARNACLE_VICTIM_GRAB:
	{
		return &slBarnacleVictimGrab[0];
	}
	case SCHED_BARNACLE_VICTIM_CHOMP:
	{
		return &slBarnacleVictimChomp[0];
	}
	case SCHED_INVESTIGATE_SOUND:
	{
		return &slInvestigateSound[0];
	}
	case SCHED_DIE:
	{
		return &slDie[0];
	}
	case SCHED_TAKE_COVER_FROM_ORIGIN:
	{
		return &slTakeCoverFromOrigin[0];
	}
	case SCHED_VICTORY_DANCE:
	{
		return &slVictoryDance[0];
	}
	case SCHED_FAIL:
	{
		return slFail;
	}
	default:
	{
		ALERT(at_console, "GetScheduleOfType()\nNo CASE for Schedule Type %d!\n", Type);

		return &slIdleStand[0];
		break;
	}
	}

	return nullptr;
}