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

constexpr int SF_TRIGGER_ALLOWMONSTERS = 1;	//!< monsters allowed to fire this trigger
constexpr int SF_TRIGGER_NOCLIENTS = 2;		//!< players not allowed to fire this trigger
constexpr int SF_TRIGGER_PUSHABLES = 4;		//!< only pushables can fire this trigger

/**
*	@brief Modify an entity's friction
*	@details Sets toucher's friction to m_frictionFraction (1.0 = normal friction)
*/
class CFrictionModifier : public CBaseEntity
{
public:
	void		Spawn() override;
	void		KeyValue(KeyValueData* pkvd) override;
	void EXPORT	ChangeFriction(CBaseEntity* pOther);
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	static	TYPEDESCRIPTION m_SaveData[];

	float		m_frictionFraction;		// Sorry, couldn't resist this name :)
};

constexpr int SF_AUTO_FIREONCE = 0x0001;

/**
*	@brief This trigger will fire when the level spawns (or respawns if not fire once)
*	It will check a global state before firing. It supports delay and killtargets
*/
class CAutoTrigger : public CBaseDelay
{
public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Precache() override;
	void Think() override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

private:
	string_t m_globalstate;
	USE_TYPE	triggerType;
};

constexpr int SF_RELAY_FIREONCE = 0x0001;

class CTriggerRelay : public CBaseDelay
{
public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

private:
	USE_TYPE	triggerType;
};

constexpr int SF_MULTIMAN_CLONE = 0x80000000;
constexpr int SF_MULTIMAN_THREAD = 0x00000001;

/**
*	@brief The Multimanager Entity - when fired, will fire up to MAX_MULTI_TARGETS targets at specified times.
*	@details FLAG: THREAD (create clones when triggered)
*	FLAG: CLONE (this is a clone for a threaded execution)
*	@see MAX_MULTI_TARGETS
*/
class CMultiManager : public CBaseToggle
{
public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void EXPORT ManagerThink();
	void EXPORT ManagerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

#if _DEBUG
	void EXPORT ManagerReport();
#endif

	bool		HasTarget(string_t targetname) override;

	int ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	int		m_cTargets;	//!< the total number of targets in this manager's fire list.
	int		m_index;	//!< Current target
	float	m_startTime;//!< Time we started firing
	string_t m_iTargetName[MAX_MULTI_TARGETS];//!< list if indexes into global string array
	float	m_flTargetDelay[MAX_MULTI_TARGETS];//!< delay (in seconds) from time of manager fire to target fire
private:
	inline bool IsClone() { return (pev->spawnflags & SF_MULTIMAN_CLONE) != 0; }
	inline bool ShouldClone()
	{
		if (IsClone())
			return false;

		return (pev->spawnflags & SF_MULTIMAN_THREAD) != 0;
	}

	CMultiManager* Clone();
};

// Flags to indicate masking off various render parameters that are normally copied to the targets
constexpr int SF_RENDER_MASKFX = 1 << 0;
constexpr int SF_RENDER_MASKAMT = 1 << 1;
constexpr int SF_RENDER_MASKMODE = 1 << 2;
constexpr int SF_RENDER_MASKCOLOR = 1 << 3;

/**
*	@brief This entity will copy its render parameters (renderfx, rendermode, rendercolor, renderamt) to its targets when triggered.
*/
class CRenderFxManager : public CBaseEntity
{
public:
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

class CBaseTrigger : public CBaseToggle
{
public:
	int	ObjectCaps() override { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void KeyValue(KeyValueData* pkvd) override;

	void InitTrigger();
	void ActivateMultiTrigger(CBaseEntity* pActivator);

	/**
	*	@brief the wait time has passed, so set back up for another activation
	*/
	void EXPORT MultiWaitOver();

	/**
	*	@brief If this is the USE function for a trigger, its state will toggle every time it's fired
	*/
	void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};

constexpr int SF_TRIGGER_HURT_TARGETONCE = 1;		//!< Only fire hurt target once
constexpr int SF_TRIGGER_HURT_START_OFF = 2;		//!< spawnflag that makes trigger_push spawn turned OFF
constexpr int SF_TRIGGER_HURT_NO_CLIENTS = 8;		//!< spawnflag that makes trigger_push spawn turned OFF
constexpr int SF_TRIGGER_HURT_CLIENTONLYFIRE = 16;	//!< trigger hurt will only fire its target if it is hurting a client
constexpr int SF_TRIGGER_HURT_CLIENTONLYTOUCH = 32;	//!< only clients may touch this trigger.

/**
*	@brief hurts anything that touches it. if the trigger has a targetname, firing it will toggle state
*	@details trigger hurt that causes radiation will do a radius check
*	and set the player's geiger counter level according to distance from center of trigger
*/
class CTriggerHurt : public CBaseTrigger
{
public:
	void Spawn() override;
	/**
	*	@brief When touched, a hurt trigger does DMG points of damage each half-second
	*/
	void EXPORT HurtTouch(CBaseEntity* pOther);
	void EXPORT RadiationThink();
};

class CTriggerMonsterJump : public CBaseTrigger
{
public:
	void Spawn() override;
	void Touch(CBaseEntity* pOther) override;
	void Think() override;
};

/**
*	@brief starts/stops cd audio tracks
*	Changes tracks or stops CD when player touches or when triggered
*/
class CTriggerCDAudio : public CBaseTrigger
{
public:
	void Spawn() override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void PlayTrack();
	void Touch(CBaseEntity* pOther) override;
};

/**
*	@brief This plays a CD track when fired or when the player enters it's radius
*/
class CTargetCDAudio : public CPointEntity
{
public:
	void			Spawn() override;
	void			KeyValue(KeyValueData* pkvd) override;

	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void			Think() override;
	void			Play();
};

/**
*	@brief Variable sized repeatable trigger. Must be targeted at one or more entities.
*	@details "wait" : Seconds between triggerings. (.2 default)
*/
class CTriggerMultiple : public CBaseTrigger
{
public:
	void Spawn() override;

	void EXPORT MultiTouch(CBaseEntity* pOther);
};

/**
*	@brief Variable sized trigger. Triggers once, then removes itself.
*	@details You must set the key "target" to the name of another object in the level that has a matching "targetname".
*	if "killtarget" is set, any objects that have a matching "target" will be removed when the trigger is fired.
*	if "angle" is set, the trigger will only fire when someone is facing the direction of the angle.  Use "360" for an angle of 0.
*/
class CTriggerOnce : public CTriggerMultiple
{
public:
	void Spawn() override;
};

constexpr int SF_COUNTER_NOMESSAGE = 1;

/**
*	@brief Acts as an intermediary for an action that takes multiple inputs.
*	@details If nomessage is not set, it will print "1 more.. " etc when triggered and "sequence complete" when finished.
*	After the counter has been triggered "cTriggersLeft" times (default 2), it will fire all of it's targets and remove itself.
*/
class CTriggerCounter : public CBaseTrigger
{
public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;

	void EXPORT CounterUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	int m_cTriggersLeft; //!< # of activations remaining
};

/**
*	@brief makes an area vertically negotiable
*/
class CLadder : public CBaseTrigger
{
public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Precache() override;
};

constexpr int SF_TRIGGER_PUSH_ONCE = 1;
constexpr int SF_TRIGGER_PUSH_START_OFF = 2;		//!< spawnflag that makes trigger_push spawn turned OFF

/**
*	@brief Pushes the player and other entities
*/
class CTriggerPush : public CBaseTrigger
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Touch(CBaseEntity* pOther) override;
};

class CTriggerTeleport : public CBaseTrigger
{
public:
	void Spawn() override;

	void EXPORT TeleportTouch(CBaseEntity* pOther);
};

class CTriggerGravity : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT GravityTouch(CBaseEntity* pOther);
};

/**
*	@brief this is a really bad idea.
*/
class CTriggerChangeTarget : public CBaseDelay
{
public:
	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int ObjectCaps() override { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

private:
	string_t m_iszNewTarget;
};

constexpr int SF_CAMERA_PLAYER_POSITION = 1;
constexpr int SF_CAMERA_PLAYER_TARGET = 2;
constexpr int SF_CAMERA_PLAYER_TAKECONTROL = 4;

class CTriggerCamera : public CBaseDelay
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void EXPORT FollowTarget();
	void Move();

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	static	TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hPlayer;
	EHANDLE m_hTarget;
	CBaseEntity* m_pentPath;
	string_t m_sPath;
	float m_flWait;
	float m_flReturnTime;
	float m_flStopTime;
	float m_moveDistance;
	float m_targetSpeed;
	float m_initialSpeed;
	float m_acceleration;
	float m_deceleration;
	bool m_state;
};
