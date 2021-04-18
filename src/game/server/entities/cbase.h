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

/*

Class Hierachy

CBaseEntity
	CBaseDelay
		CBaseToggle
			CBaseItem
			CBaseMonster
				CBaseCycler
				CBasePlayer
				CBaseGroup
*/

constexpr int MAX_PATH_SIZE = 10; //!< max number of nodes available for a path.

// These are caps bits to indicate what an object's capabilities (currently used for save/restore and level transitions)
constexpr int FCAP_CUSTOMSAVE = 0x00000001;
constexpr int FCAP_ACROSS_TRANSITION = 0x00000002;	//!< should transfer between transitions
constexpr int FCAP_MUST_SPAWN = 0x00000004;			//!< Spawn after restore
constexpr int FCAP_DONT_SAVE = 0x80000000;			//!< Don't save this
constexpr int FCAP_IMPULSE_USE = 0x00000008;		//!< can be used by the player
constexpr int FCAP_CONTINUOUS_USE = 0x00000010;		//!< can be used by the player
constexpr int FCAP_ONOFF_USE = 0x00000020;			//!< can be used by the player
constexpr int FCAP_DIRECTIONAL_USE = 0x00000040;	//!< Player sends +/- 1 when using (currently only tracktrains)
constexpr int FCAP_MASTER = 0x00000080;				//!< Can be used to "master" other entities (like multisource)

// UNDONE: This will ignore transition volumes (trigger_transition), but not the PVS!!!
constexpr int FCAP_FORCE_TRANSITION = 0x00000080;	//!< ALWAYS goes across transitions

#include "steam/steamtypes.h"
#include "saverestore.hpp"
#include "globalstate.hpp"
#include "schedule.h"

#include "monsterevent.h"

#include "Platform.h"

#define EXPORT DLLEXPORT

enum USE_TYPE { USE_OFF = 0, USE_ON = 1, USE_SET = 2, USE_TOGGLE = 3 };

enum class TriggerState
{
	Off,
	On,
	Toggle
};

constexpr USE_TYPE UTIL_TriggerStateToTriggerType(TriggerState state)
{
	switch (state)
	{
	case TriggerState::Off:		return USE_OFF;
	case TriggerState::Toggle:	return USE_TOGGLE;
	default:					return USE_ON;
	}
}

void FireTargets(const char* targetName, CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

typedef void (CBaseEntity::* BASEPTR)();
typedef void (CBaseEntity::* ENTITYFUNCPTR)(CBaseEntity* pOther);
typedef void (CBaseEntity::* USEPTR)(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

// For CLASSIFY
constexpr int CLASS_NONE = 0;
constexpr int CLASS_MACHINE = 1;
constexpr int CLASS_PLAYER = 2;
constexpr int CLASS_HUMAN_PASSIVE = 3;
constexpr int CLASS_HUMAN_MILITARY = 4;
constexpr int CLASS_ALIEN_MILITARY = 5;
constexpr int CLASS_ALIEN_PASSIVE = 6;
constexpr int CLASS_ALIEN_MONSTER = 7;
constexpr int CLASS_ALIEN_PREY = 8;
constexpr int CLASS_ALIEN_PREDATOR = 9;
constexpr int CLASS_INSECT = 10;
constexpr int CLASS_PLAYER_ALLY = 11;
constexpr int CLASS_PLAYER_BIOWEAPON = 12;	//!< hornets and snarks.launched by players
constexpr int CLASS_ALIEN_BIOWEAPON = 13;	//!< hornets and snarks.launched by the alien menace
constexpr int CLASS_BARNACLE = 99;			//!< special because no one pays attention to it, and it eats a wide cross-section of creatures.

class CBaseEntity;
class CBaseMonster;
class CBasePlayerItem;
class CSquadMonster;


constexpr int SF_NORESPAWN = 1 << 30; // !!!set this bit on guns and stuff that should never respawn.

#include "ehandle.hpp"

/**
*	@brief Base Entity.  All entity types derive from this
*/
class CBaseEntity
{
public:
	/**
	*	@brief Constructor.  Set engine to use C/C++ callback functions pointers to engine data
	*
	*	Don't need to save/restore this pointer, the engine resets it
	*/
	entvars_t* pev;

	//TODO: these are not safe because the target entity can be killtargeted at any time
	// path corners
	/**
	*	@brief path corner we are heading towards
	*/
	CBaseEntity* m_pGoalEnt;

	/**
	*	@brief used for temporary link-list operations. 
	*/
	CBaseEntity* m_pLink;

	virtual ~CBaseEntity() {}

	void* operator new(std::size_t count)
	{
		auto memory = new byte[count];
		std::memset(memory, 0, count);
		return memory;
	}

	void operator delete(void* ptr)
	{
		auto memory = reinterpret_cast<byte*>(ptr);
		delete[] memory;
	}

	// initialization functions
	virtual void	Spawn() {}
	virtual void	Precache() {}
	virtual void	KeyValue(KeyValueData* pkvd) { pkvd->fHandled = false; }
	virtual bool	Save(CSave& save);
	virtual bool	Restore(CRestore& restore);
	virtual int		ObjectCaps() { return FCAP_ACROSS_TRANSITION; }
	virtual void	Activate() {}

	/**
	*	@brief Setup the object->object collision box (pev->mins / pev->maxs is the object->world collision box)
	*/
	virtual void	SetObjectCollisionBox();

	/**
	*	@brief returns the type of group (i.e, "houndeye", or "human military")
	*	so that monsters with different classnames still realize that they are teammates.
	*	(overridden for monsters that form groups)
	*/
	virtual int Classify() { return CLASS_NONE; }

	/**
	*	@brief monster maker children use this to tell the monster maker that they have died.
	*/
	virtual void DeathNotice(entvars_t* pevChild) {}


	static	TYPEDESCRIPTION m_SaveData[];

	virtual void	TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);

	/**
	*	@brief inflict damage on this entity. bitsDamageType indicates type of damage inflicted, ie: DMG_CRUSH
	*/
	virtual bool	TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	virtual bool	TakeHealth(float flHealth, int bitsDamageType);
	virtual void	Killed(entvars_t* pevAttacker, int iGib);
	virtual int		BloodColor() { return DONT_BLEED; }
	virtual void	TraceBleed(float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	virtual bool    IsTriggered(CBaseEntity* pActivator) { return true; }
	virtual CBaseMonster* MyMonsterPointer() { return nullptr; }
	virtual CSquadMonster* MySquadMonsterPointer() { return nullptr; }
	virtual	ToggleState GetToggleState() { return ToggleState::AtTop; }
	virtual float	GetDelay() { return 0; }
	virtual bool	IsMoving() { return pev->velocity != vec3_origin; }
	virtual void	OverrideReset() {}
	virtual int		DamageDecal(int bitsDamageType);
	virtual void    StartSneaking() {}
	virtual void    StopSneaking() {}
	virtual bool	OnControls(entvars_t* pev) { return false; }
	virtual bool    IsSneaking() { return false; }
	virtual bool	IsAlive() { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	virtual bool	IsBSPModel() { return pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP; }
	virtual bool	ReflectGauss() { return (IsBSPModel() && !pev->takedamage); }
	virtual bool	HasTarget(string_t targetname) { return FStrEq(STRING(targetname), STRING(pev->targetname)); }
	virtual bool    IsInWorld();
	virtual	bool	IsPlayer() { return false; }
	virtual bool	IsNetClient() { return false; }
	virtual const char* TeamID() { return ""; }


	//	virtual void	SetActivator( CBaseEntity *pActivator ) {}
	virtual CBaseEntity* GetNextTarget();

	// fundamental callbacks
	void (CBaseEntity ::* m_pfnThink)();
	void (CBaseEntity ::* m_pfnTouch)(CBaseEntity* pOther);
	void (CBaseEntity ::* m_pfnUse)(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void (CBaseEntity ::* m_pfnBlocked)(CBaseEntity* pOther);

	virtual void Think() { if (m_pfnThink) (this->*m_pfnThink)(); }
	virtual void Touch(CBaseEntity* pOther) { if (m_pfnTouch) (this->*m_pfnTouch)(pOther); }
	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
	{
		if (m_pfnUse)
			(this->*m_pfnUse)(pActivator, pCaller, useType, value);
	}
	virtual void Blocked(CBaseEntity* pOther) { if (m_pfnBlocked) (this->*m_pfnBlocked)(pOther); }

	/**
	*	@brief This updates global tables that need to know about entities being removed
	*/
	void UpdateOnRemove();

	// common member functions
	/**
	*	@brief Convenient way to delay removing oneself
	*/
	void EXPORT SUB_Remove();

	/**
	*	@brief Convenient way to explicitly do nothing (passed to functions that require a method)
	*/
	void EXPORT SUB_DoNothing();
	void EXPORT SUB_StartFadeOut();
	void EXPORT SUB_FadeOut();
	void EXPORT SUB_CallUseToggle() { this->Use(this, this, USE_TOGGLE, 0); }
	bool		ShouldToggle(USE_TYPE useType, bool currentState);
	void		FireBullets(uint32	cShots, Vector  vecSrc, Vector	vecDirShooting, Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t* pevAttacker = nullptr);
	Vector		FireBulletsPlayer(uint32	cShots, Vector  vecSrc, Vector	vecDirShooting, Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t* pevAttacker = nullptr, int shared_rand = 0);

	virtual CBaseEntity* Respawn() { return nullptr; }

	/**
	*	@brief Trigger targets specified in pev->target
	*
	*	@details Search for (string)pev->targetname in all entities that match (string)pev->target and call their Use function (if they have one)
	*
	*	CBaseDelay classes only:
	*	If m_flDelay is set, a DelayedUse entity will be created that will actually do the SUB_UseTargets after that many seconds have passed.
	*	Removes all entities with a targetname that match m_iszKillTarget, and removes them, so some events can remove other triggers.
	*/
	void SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value);
	/**
	*	@brief Do the bounding boxes of these two intersect?
	*/
	bool	Intersects(CBaseEntity* pOther);
	void	MakeDormant();
	bool	IsDormant();
	bool    IsLockedByMaster() { return false; }

	static CBaseEntity* Instance(edict_t* pent)
	{
		if (!pent)
			pent = INDEXENT(0);
		CBaseEntity* pEnt = (CBaseEntity*)GET_PRIVATE(pent);
		return pEnt;
	}

	static CBaseEntity* Instance(entvars_t* pev)
	{
		if (!pev)
			return Instance(INDEXENT(0));

		return Instance(ENT(pev));
	}

	CBaseMonster* GetMonsterPointer(entvars_t* pevMonster)
	{
		CBaseEntity* pEntity = Instance(pevMonster);
		if (pEntity)
			return pEntity->MyMonsterPointer();
		return nullptr;
	}
	CBaseMonster* GetMonsterPointer(edict_t* pentMonster)
	{
		CBaseEntity* pEntity = Instance(pentMonster);
		if (pEntity)
			return pEntity->MyMonsterPointer();
		return nullptr;
	}


	// Ugly code to lookup all functions to make sure they are exported when set.
#ifdef _DEBUG
	void FunctionCheck(void* pFunction, const char* name)
	{
		if (pFunction && !NAME_FOR_FUNCTION((uint32)pFunction))
			ALERT(at_error, "No EXPORT: %s:%s (%08lx)\n", STRING(pev->classname), name, (uint32)pFunction);
	}

	BASEPTR	ThinkSet(BASEPTR func, const char* name)
	{
		m_pfnThink = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnThink)))), name);
		return func;
	}
	ENTITYFUNCPTR TouchSet(ENTITYFUNCPTR func, const char* name)
	{
		m_pfnTouch = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnTouch)))), name);
		return func;
	}
	USEPTR	UseSet(USEPTR func, const char* name)
	{
		m_pfnUse = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnUse)))), name);
		return func;
	}
	ENTITYFUNCPTR	BlockedSet(ENTITYFUNCPTR func, const char* name)
	{
		m_pfnBlocked = func;
		FunctionCheck((void*)*((int*)((char*)this + (offsetof(CBaseEntity, m_pfnBlocked)))), name);
		return func;
	}

#endif


	// virtual functions used by a few classes
	/**
	*	@brief NOTE: szName must be a pointer to constant memory, e.g. "monster_class" because the entity will keep a pointer to it after this call.
	*/
	static CBaseEntity* Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, edict_t* pentOwner = nullptr);

	virtual bool FBecomeProne() { return false; }
	edict_t* edict() { return ENT(pev); }
	int	  entindex() { return ENTINDEX(edict()); }

	virtual Vector Center() { return (pev->absmax + pev->absmin) * 0.5; }	//!< center point of entity
	virtual Vector EyePosition() { return pev->origin + pev->view_ofs; }	//!< position of eyes
	virtual Vector EarPosition() { return pev->origin + pev->view_ofs; }	//!< position of ears
	virtual Vector BodyTarget(const Vector& posSrc) { return Center(); }	//!< position to shoot at

	virtual int Illumination() { return GETENTITYILLUM(ENT(pev)); }

	virtual	bool FVisible(CBaseEntity* pEntity);
	virtual	bool FVisible(const Vector& vecOrigin);

	void EmitSound(int channel, const char* fileName, float volume = VOL_NORM, float attenuation = ATTN_NORM, int pitch = PITCH_NORM, int flags = 0);

	void StopSound(int channel, const char* fileName);

	template<std::size_t Size>
	void EMIT_SOUND_ARRAY_DYN(int chan, const char* (&array)[Size])
	{
		EmitSound(chan, RANDOM_SOUND_ARRAY(array), VOL_NORM, ATTN_NORM, RANDOM_LONG(95, 105));
	}
};

inline bool FNullEnt(CBaseEntity* ent) { return (ent == nullptr) || FNullEnt(ent->edict()); }


// Ugly technique to override base member functions
// Normally it's illegal to cast a pointer to a member function of a derived class to a pointer to a 
// member function of a base class.  static_cast is a sleezy way around that problem.

#ifdef _DEBUG

#define SetThink( a ) ThinkSet( static_cast <void (CBaseEntity::*)()> (a), #a )
#define SetTouch( a ) TouchSet( static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a), #a )
#define SetUse( a ) UseSet( static_cast <void (CBaseEntity::*)(	CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (a), #a )
#define SetBlocked( a ) BlockedSet( static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a), #a )

#else

#define SetThink( a ) m_pfnThink = static_cast <void (CBaseEntity::*)()> (a)
#define SetTouch( a ) m_pfnTouch = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)
#define SetUse( a ) m_pfnUse = static_cast <void (CBaseEntity::*)( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )> (a)
#define SetBlocked( a ) m_pfnBlocked = static_cast <void (CBaseEntity::*)(CBaseEntity *)> (a)

#endif


class CPointEntity : public CBaseEntity
{
public:
	void	Spawn() override;
	int	ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
private:
};

/**
*	@brief sounds that doors and buttons make when locked/unlocked
*/
struct locksound_t
{
	string_t	sLockedSound;		//!< sound a door makes when it's locked
	string_t	sLockedSentence;	//!< sentence group played when door is locked
	string_t	sUnlockedSound;		//!< sound a door makes when it's unlocked
	string_t	sUnlockedSentence;	//!< sentence group played when door is unlocked

	int		iLockedSentence;		//!< which sentence in sentence group to play next
	int		iUnlockedSentence;		//!< which sentence in sentence group to play next

	float	flwaitSound;			//!< time delay between playing consecutive 'locked/unlocked' sounds
	float	flwaitSentence;			//!< time delay between playing consecutive sentences
	bool	bEOFLocked;				//!< true if hit end of list of locked sentences
	bool	bEOFUnlocked;			//!< true if hit end of list of unlocked sentences
};

void PlayLockSounds(CBaseEntity* entity, locksound_t* pls, int flocked, int fbutton);

constexpr int MAX_MULTI_TARGETS = 16; //!< maximum number of targets a single multi_manager entity may be assigned.
constexpr int MS_MAX_TARGETS = 32;

class CMultiSource : public CPointEntity
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	int	ObjectCaps() override { return (CPointEntity::ObjectCaps() | FCAP_MASTER); }
	bool IsTriggered(CBaseEntity* pActivator) override;
	void EXPORT Register();
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	EHANDLE		m_rgEntities[MS_MAX_TARGETS];
	bool		m_rgTriggered[MS_MAX_TARGETS];

	int			m_iTotal;
	string_t	m_globalstate;
};


/**
*	@brief generic Delay entity.
*/
class CBaseDelay : public CBaseEntity
{
public:
	float		m_flDelay;
	string_t m_iszKillTarget;

	void	KeyValue(KeyValueData* pkvd) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];
	// common member functions
	//TODO: this is a non-virtual override of the same function in CBaseEntity. Should probably just merge this class into CBaseEntity
	void SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value);
	void EXPORT DelayThink();
};


class CBaseAnimating : public CBaseDelay
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	// Basic Monster Animation functions
	/**
	*	@brief advance the animation frame up to the current time
	* 
	*	accumulate animation frame time from last time called until now
	*	if an flInterval is passed in, only advance animation that number of seconds
	*/
	float StudioFrameAdvance(float flInterval = 0.0);
	int	 GetSequenceFlags();
	int  LookupActivity(int activity);

	/**
	*	@brief Get activity with highest 'weight'
	*/
	int  LookupActivityHeaviest(int activity);
	int  LookupSequence(const char* label);
	void ResetSequenceInfo();

	/**
	*	@brief Handle events that have happend since last time called up until X seconds into the future
	*/
	void DispatchAnimEvents(float flFutureInterval = 0.1);
	virtual void HandleAnimEvent(MonsterEvent_t& event) {}
	float SetBoneController(int iController, float flValue);
	void InitBoneControllers();
	float SetBlending(int iBlender, float flValue);
	void GetBonePosition(int iBone, Vector& origin, Vector& angles);
	int FindTransition(int iEndingSequence, int iGoalSequence, int& iDir);
	int FindTransition(int iEndingSequence, int iGoalSequence);
	void GetAttachment(int iAttachment, Vector& origin, Vector& angles);
	void SetBodygroup(int iGroup, int iValue);
	int GetBodygroup(int iGroup);
	bool ExtractBbox(int sequence, Vector& mins, Vector& maxs);
	void SetSequenceBox();

	// animation needs
	float				m_flFrameRate;		//!< computed FPS for current sequence
	float				m_flGroundSpeed;	//!< computed linear movement rate for current sequence
	float				m_flLastEventCheck;	//!< last time the event list was checked
	bool				m_fSequenceFinished;//!< flag set when StudioAdvanceFrame moves across a frame boundry
	bool				m_fSequenceLoops;	//!< true if the sequence loops
};


constexpr int SF_ITEM_USE_ONLY = 256; //  ITEM_USE_ONLY = BUTTON_USE_ONLY = DOOR_USE_ONLY!!! 

/**
*	@brief generic Toggle entity.
*/
class CBaseToggle : public CBaseAnimating
{
public:
	void				KeyValue(KeyValueData* pkvd) override;

	ToggleState			m_toggle_state;
	float				m_flActivateFinished;	//!< like attack_finished, but for doors
	float				m_flMoveDistance;		//!< how far a door should slide or rotate
	float				m_flWait;
	float				m_flLip;
	float				m_flTWidth;		//!< for plats
	float				m_flTLength;	//!< for plats

	Vector				m_vecPosition1;
	Vector				m_vecPosition2;
	Vector				m_vecAngle1;
	Vector				m_vecAngle2;

	float				m_flHeight;
	EHANDLE				m_hActivator;
	void (CBaseToggle::* m_pfnCallWhenMoveDone)();
	Vector				m_vecFinalDest;
	Vector				m_vecFinalAngle;

	int					m_bitsDamageInflict;	//!< DMG_ damage type that the door or tigger does

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static	TYPEDESCRIPTION m_SaveData[];

	ToggleState GetToggleState() override { return m_toggle_state; }
	float	GetDelay() override { return m_flWait; }

	// common member functions
	/**
	*	@brief calculate pev->velocity and pev->nextthink to reach vecDest from pev->origin traveling at flSpeed
	*/
	void LinearMove(Vector	vecDest, float flSpeed);

	/**
	*	@brief After moving, set origin to exact final destination, call "move done" function
	*/
	void EXPORT LinearMoveDone();

	/**
	*	@brief calculate pev->velocity and pev->nextthink to reach vecDest from pev->origin traveling at flSpeed
	*
	*	Just like LinearMove, but rotational.
	*/
	void AngularMove(Vector vecDestAngle, float flSpeed);

	/**
	*	@brief After rotating, set angle to exact final angle, call "move done" function
	*/
	void EXPORT AngularMoveDone();
	bool IsLockedByMaster(); //TODO: non-virtual override

	static float		AxisValue(int flags, const Vector& angles);
	static void			AxisDir(entvars_t* pev);
	static float		AxisDelta(int flags, const Vector& angle1, const Vector& angle2);

	/**
	*	@brief If this button has a master switch, this is the targetname.
	*
	*	@details A master switch must be of the multisource type.
	*	If all of the switches in the multisource have been triggered, then the button will be allowed to operate.
	*	Otherwise, it will be deactivated.
	*/
	string_t m_sMaster;
};
#define SetMoveDone( a ) m_pfnCallWhenMoveDone = static_cast <void (CBaseToggle::*)()> (a)


// people gib if their health is <= this at the time of death
constexpr int GIB_HEALTH_VALUE = -30;

constexpr int ROUTE_SIZE = 8;				//!< how many waypoints a monster can store at one time
constexpr int MAX_OLD_ENEMIES = 4;			//!< how many old enemies to remember

constexpr int bits_CAP_DUCK = 1 << 0;		//!< crouch
constexpr int bits_CAP_JUMP = 1 << 1;		//!< jump/leap
constexpr int bits_CAP_STRAFE = 1 << 2;		//!< strafe ( walk/run sideways)
constexpr int bits_CAP_SQUAD = 1 << 3;		//!< can form squads
constexpr int bits_CAP_SWIM = 1 << 4;		//!< proficiently navigate in water
constexpr int bits_CAP_CLIMB = 1 << 5;		//!< climb ladders/ropes
constexpr int bits_CAP_USE = 1 << 6;		//!< open doors/push buttons/pull levers
constexpr int bits_CAP_HEAR = 1 << 7;		//!< can hear forced sounds
constexpr int bits_CAP_AUTO_DOORS = 1 << 8; //!< can trigger auto doors
constexpr int bits_CAP_OPEN_DOORS = 1 << 9; //!< can open manual doors
constexpr int bits_CAP_TURN_HEAD = 1 << 10;	//!< can turn head, always bone controller 0

constexpr int bits_CAP_RANGE_ATTACK1 = 1 << 11;	//!< can do a range attack 1
constexpr int bits_CAP_RANGE_ATTACK2 = 1 << 12;	//!< can do a range attack 2
constexpr int bits_CAP_MELEE_ATTACK1 = 1 << 13;	//!< can do a melee attack 1
constexpr int bits_CAP_MELEE_ATTACK2 = 1 << 14;	//!< can do a melee attack 2

constexpr int bits_CAP_FLY = 1 << 15;			//!< can fly, move all around

constexpr int bits_CAP_DOORS_GROUP = bits_CAP_USE | bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS;

// when calling KILLED(), a value that governs gib behavior is expected to be 
// one of these three values
//TODO: make this an enum
constexpr int GIB_NORMAL = 0;	//!< gib if entity was overkilled
constexpr int GIB_NEVER = 1;	//!< never gib, no matter how much death damage is done ( freezing, etc )
constexpr int GIB_ALWAYS = 2;	//!< always gib ( Houndeye Shock, Barnacle Bite )

class CBaseMonster;
class CCineMonster;
class CSound;

#include "basemonster.h"


const char* ButtonSound(int sound);				// get string of button sound number


/**
*	@brief Generic Button
*/
class CBaseButton : public CBaseToggle
{
public:
	void Spawn() override;
	void Precache() override;
	void RotSpawn();
	void KeyValue(KeyValueData* pkvd) override;

	void ButtonActivate();
	void SparkSoundCache();

	void EXPORT ButtonShot();
	void EXPORT ButtonTouch(CBaseEntity* pOther);
	void EXPORT ButtonSpark();
	void EXPORT TriggerAndWait();
	void EXPORT ButtonReturn();
	void EXPORT ButtonBackHome();
	void EXPORT ButtonUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	enum class ButtonCode
	{
		Nothing,
		Activate,
		Return
	};

	ButtonCode ButtonResponseToTouch();

	static	TYPEDESCRIPTION m_SaveData[];
	/**
	*	@brief Buttons that don't take damage can be IMPULSE used
	*/
	int	ObjectCaps() override { return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | (pev->takedamage ? 0 : FCAP_IMPULSE_USE); }

	bool	m_fStayPushed;		//!< button stays pushed in until touched again?
	bool	m_fRotating;		//!< a rotating button?  default is a sliding button.

	locksound_t m_ls;			//!< door lock sounds

	// ordinals from entity selection
	byte	m_bLockedSound;
	byte	m_bLockedSentence;
	byte	m_bUnlockedSound;
	byte	m_bUnlockedSentence;
	int		m_sounds;
};

/**
*	@brief Converts a entvars_t * to a class pointer. It will allocate the class and entity if necessary
*/
template <class T> T* GetClassPtr(T* a)
{
	entvars_t* pev = (entvars_t*)a;

	// allocate entity if necessary
	if (pev == nullptr)
		pev = VARS(CREATE_ENTITY());

	// get the private data
	a = (T*)GET_PRIVATE(ENT(pev));

	if (a == nullptr)
	{
		// allocate private data 
		a = new T();
		pev->pContainingEntity->pvPrivateData = a;
		a->pev = pev;
	}
	return a;
}

/**
*	@brief This spawns first when each level begins.
*
*	this moved here from world.cpp, to allow classes to be derived from it
*/
class CWorld : public CBaseEntity
{
public:
	~CWorld();

	void Spawn() override;
	void Precache() override;
	void KeyValue(KeyValueData* pkvd) override;
};
