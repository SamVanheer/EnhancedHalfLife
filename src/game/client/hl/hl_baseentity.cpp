/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

/**
*	@file
*
*	This file contains "stubs" of class member implementations so that we can predict certain weapons client side.
*	From time to time you might find that you need to implement part of the these functions.
*	If so, cut it from here, paste it in hl_weapons.cpp or somewhere else and add in the functionality you need.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "navigation/nodes.h"
#include "soundent.h"
#include "skill.h"

// Globals used by game logic
enginefuncs_t g_engfuncs;

// Local version of game .dll global variables ( time, etc. )
static globalvars_t	Globals;
globalvars_t* gpGlobals = &Globals;

// CBaseEntity Stubs
bool CBaseEntity::GiveHealth(float flHealth, int bitsDamageType) { return true; }
bool CBaseEntity::TakeDamage(const TakeDamageInfo& info) { return true; }
CBaseEntity* CBaseEntity::GetNextTarget() { return nullptr; }
bool CBaseEntity::Save(CSave& save) { return true; }
bool CBaseEntity::Restore(CRestore& restore) { return true; }
void CBaseEntity::SetObjectCollisionBox() { }
bool CBaseEntity::Intersects(CBaseEntity* pOther) { return false; }
void CBaseEntity::MakeDormant() { }
bool CBaseEntity::IsDormant() { return false; }
bool CBaseEntity::IsInWorld() { return true; }
bool CBaseEntity::ShouldToggle(UseType useType, bool currentState) { return false; }
void CBaseEntity::SetAbsOrigin(const Vector& origin) {}
void CBaseEntity::SetSize(const Vector& mins, const Vector& maxs) {}
int	CBaseEntity::DamageDecal(int bitsDamageType) { return -1; }
CBaseEntity* CBaseEntity::Create(const char* szName, const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner, bool callSpawn) { return nullptr; }
void CBaseEntity::SUB_Remove() { }
void CBaseEntity::EmitSound(SoundChannel channel, const char* fileName, float volume, float attenuation, int pitch, int flags) {}

// CBaseDelay Stubs
void CBaseDelay::KeyValue(KeyValueData*) { }
bool CBaseDelay::Restore(class CRestore&) { return true; }
bool CBaseDelay::Save(class CSave&) { return true; }

// CBaseAnimating Stubs
bool CBaseAnimating::Restore(class CRestore&) { return true; }
bool CBaseAnimating::Save(class CSave&) { return true; }

// DEBUG Stubs
void DBG_AssertFunction(bool fExpr, const char* szExpr, const char* szFile, int szLine, const char* szMessage) { }

// UTIL_* Stubs
void UTIL_PrecacheOther(const char* szClassname) { }
void UTIL_BloodDrips(const Vector& origin, const Vector& direction, int color, int amount) { }
void UTIL_DecalTrace(TraceResult* pTrace, int decalNumber) { }
void UTIL_GunshotDecalTrace(TraceResult* pTrace, int decalNumber) { }
void UTIL_MakeVectors(const Vector& vecAngles) { }
bool UTIL_IsValidEntity(CBaseEntity* pEntity) { return true; }
void UTIL_LogPrintf(const char*, ...) { }
void UTIL_ClientPrintAll(int, char const*, char const*, char const*, char const*, char const*) { }
CBaseEntity* UTIL_EntityByIndex(int index) { return nullptr; }

// CBaseToggle Stubs
bool CBaseToggle::Restore(class CRestore&) { return true; }
bool CBaseToggle::Save(class CSave&) { return true; }
void CBaseToggle::KeyValue(KeyValueData*) { }

// CGrenade Stubs
void CGrenade::BounceSound() { }
void CGrenade::Explode(TraceResult*, int) { }
void CGrenade::Killed(const KilledInfo& info) { }
void CGrenade::Spawn() { }
CGrenade* CGrenade::ShootTimed(CBaseEntity* pOwner, const Vector& vecStart, const Vector& vecVelocity, float time) { return nullptr; }
CGrenade* CGrenade::ShootContact(CBaseEntity* pOwner, const Vector& vecStart, const Vector& vecVelocity) { return nullptr; }
void CGrenade::DetonateUse(const UseInfo& info) { }

void UTIL_Remove(CBaseEntity* pEntity) { }
CBaseEntity* UTIL_FindEntityInSphere(CBaseEntity* pStartEntity, const Vector& vecCenter, float flRadius) { return nullptr; }

CSprite* CSprite::SpriteCreate(const char* pSpriteName, const Vector& origin, bool animate) { return nullptr; }
void CBeam::PointEntInit(const Vector& start, int endIndex) { }
CBeam* CBeam::BeamCreate(const char* pSpriteName, int width) { return nullptr; }
void CSprite::Expand(float scaleSpeed, float fadeSpeed) { }


CBaseEntity* CBaseMonster::CheckTraceHullAttack(float flDist, int iDamage, int iDmgType) { return nullptr; }
void CBaseMonster::Eat(float flFullDuration) { }
bool CBaseMonster::ShouldEat() { return true; }
void CBaseMonster::BarnacleVictimBitten(CBaseEntity* pBarnacle) { }
void CBaseMonster::BarnacleVictimReleased() { }
void CBaseMonster::Listen() { }
float CBaseMonster::SoundVolume(CSound* pSound) { return 0.0; }
bool CBaseMonster::ValidateHintType(short sHint) { return false; }
void CBaseMonster::Look(int iDistance) { }
int CBaseMonster::SoundMask() { return 0; }
CSound* CBaseMonster::BestSound() { return nullptr; }
CSound* CBaseMonster::BestScent() { return nullptr; }
float CBaseAnimating::StudioFrameAdvance(float flInterval) { return 0.0; }
void CBaseMonster::MonsterThink() { }
void CBaseMonster::MonsterUse(const UseInfo& info) { }
int CBaseMonster::IgnoreConditions() { return 0; }
void CBaseMonster::RouteClear() { }
void CBaseMonster::RouteNew() { }
bool CBaseMonster::IsRouteClear() { return false; }
bool CBaseMonster::RefreshRoute() { return false; }
bool CBaseMonster::MoveToEnemy(Activity movementAct, float waitTime) { return false; }
bool CBaseMonster::MoveToLocation(Activity movementAct, float waitTime, const Vector& goal) { return false; }
bool CBaseMonster::MoveToTarget(Activity movementAct, float waitTime) { return false; }
bool CBaseMonster::MoveToNode(Activity movementAct, float waitTime, const Vector& goal) { return false; }
int ShouldSimplify(int routeType) { return true; }
void CBaseMonster::RouteSimplify(CBaseEntity* pTargetEnt) { }
bool CBaseMonster::BecomeProne() { return true; }
bool CBaseMonster::CheckRangeAttack1(float flDot, float flDist) { return false; }
bool CBaseMonster::CheckRangeAttack2(float flDot, float flDist) { return false; }
bool CBaseMonster::CheckMeleeAttack1(float flDot, float flDist) { return false; }
bool CBaseMonster::CheckMeleeAttack2(float flDot, float flDist) { return false; }
void CBaseMonster::CheckAttacks(CBaseEntity* pTarget, float flDist) { }
bool CBaseMonster::CanCheckAttacks() { return false; }
bool CBaseMonster::CheckEnemy(CBaseEntity* pEnemy) { return false; }
void CBaseMonster::PushEnemy(CBaseEntity* pEnemy, const Vector& vecLastKnownPos) { }
bool CBaseMonster::PopEnemy() { return false; }
void CBaseMonster::SetActivity(Activity NewActivity) { }
void CBaseMonster::SetSequenceByName(const char* szSequence) { }

LocalMoveResult CBaseMonster::CheckLocalMove(const Vector& vecStart, const Vector& vecEnd, CBaseEntity* pTarget, float* pflDist)
{
	return LocalMoveResult::Invalid;
}

float CBaseMonster::OpenDoorAndWait(CBaseEntity* pDoor) { return 0.0; }
void CBaseMonster::AdvanceRoute(float distance) { }
int CBaseMonster::RouteClassify(int iMoveFlag) { return 0; }
bool CBaseMonster::BuildRoute(const Vector& vecGoal, int iMoveFlag, CBaseEntity* pTarget) { return false; }
void CBaseMonster::InsertWaypoint(Vector vecLocation, int afMoveFlags) { }
bool CBaseMonster::Triangulate(const Vector& vecStart, const Vector& vecEnd, float flDist, CBaseEntity* pTargetEnt, Vector* pApex) { return false; }
void CBaseMonster::Move(float flInterval) { }
bool CBaseMonster::ShouldAdvanceRoute(float flWaypointDist) { return false; }
void CBaseMonster::MoveExecute(CBaseEntity* pTargetEnt, const Vector& vecDir, float flInterval) { }
void CBaseMonster::MonsterInit() { }
void CBaseMonster::MonsterInitThink() { }
void CBaseMonster::StartMonster() { }
void CBaseMonster::MovementComplete() { }
bool CBaseMonster::TaskIsRunning() { return false; }
Relationship CBaseMonster::GetRelationship(CBaseEntity* pTarget) { return Relationship::None; }
bool CBaseMonster::FindCover(Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist) { return false; }
bool CBaseMonster::BuildNearestRoute(Vector vecThreat, const Vector& vecViewOffset, float flMinDist, float flMaxDist) { return false; }
CBaseEntity* CBaseMonster::BestVisibleEnemy() { return nullptr; }
bool CBaseMonster::IsInViewCone(CBaseEntity* pEntity) { return false; }
bool CBaseMonster::IsInViewCone(const Vector& origin) { return false; }
bool CBaseEntity::IsVisible(CBaseEntity* pEntity) { return false; }
bool CBaseEntity::IsVisible(const Vector& vecOrigin) { return false; }
void CBaseMonster::MakeIdealYaw(const Vector& vecTarget) { }
float	CBaseMonster::YawDiff() { return 0.0; }
float CBaseMonster::ChangeYaw(int yawSpeed) { return 0; }
float	CBaseMonster::VecToYaw(const Vector& vecDir) { return 0.0; }
int CBaseAnimating::LookupActivity(int activity) { return 0; }
int CBaseAnimating::LookupActivityHeaviest(int activity) { return 0; }
void CBaseMonster::SetEyePosition() { }
int CBaseAnimating::LookupSequence(const char* label) { return 0; }
void CBaseAnimating::ResetSequenceInfo() { }
int CBaseAnimating::GetSequenceFlags() { return 0; }
void CBaseAnimating::DispatchAnimEvents(float flInterval) { }
void CBaseMonster::HandleAnimEvent(AnimationEvent& event) { }
float CBaseAnimating::SetBoneController(int iController, float flValue) { return 0.0; }
void CBaseAnimating::InitBoneControllers() { }
float CBaseAnimating::SetBlending(int iBlender, float flValue) { return 0; }
void CBaseAnimating::GetBonePosition(int iBone, Vector& origin, Vector& angles) { }
void CBaseAnimating::GetAttachment(int iAttachment, Vector& origin, Vector& angles) { }
int CBaseAnimating::FindTransition(int iEndingSequence, int iGoalSequence, int& iDir) { return -1; }
void CBaseAnimating::SetBodygroup(int iGroup, int iValue) { }
int CBaseAnimating::GetBodygroup(int iGroup) { return 0; }
Vector CBaseMonster::GetGunPosition() { return vec3_origin; }
void CBaseEntity::TraceAttack(const TraceAttackInfo& info) { }
void CBaseEntity::FireBullets(std::uint32_t cShots, const Vector& vecSrc, const Vector& vecDirShooting, const Vector& vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, CBaseEntity* pAttacker) { }
void CBaseEntity::TraceBleed(const TraceAttackInfo& info) { }
void CBaseMonster::MakeDamageBloodDecal(int cCount, float flNoise, TraceResult* ptr, const Vector& vecDir) { }
bool CBaseMonster::GetNodeRoute(Vector vecDest) { return true; }
int CBaseMonster::FindHintNode() { return NO_NODE; }
void CBaseMonster::ReportAIState() { }
void CBaseMonster::KeyValue(KeyValueData* pkvd) { }
bool CBaseMonster::CheckAITrigger() { return false; }
bool CBaseMonster::CanPlaySequence(bool fDisregardMonsterState, int interruptLevel) { return false; }
bool CBaseMonster::FindLateralCover(const Vector& vecThreat, const Vector& vecViewOffset) { return false; }
Vector CBaseMonster::ShootAtEnemy(const Vector& shootOrigin) { return vec3_origin; }
bool CBaseMonster::FacingIdeal() { return false; }
bool CBaseMonster::CanActiveIdle() { return false; }
void CBaseMonster::PlaySentence(const char* pszSentence, float duration, float volume, float attenuation) { }
void CBaseMonster::PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, bool bConcurrent, CBaseEntity* pListener) { }
void CBaseMonster::SentenceStop() { }
void CBaseMonster::CorpseFallThink() { }
void CBaseMonster::MonsterInitDead() { }
bool CBaseMonster::BBoxFlat() { return true; }
bool CBaseMonster::GetEnemy() { return false; }
void CBaseMonster::TraceAttack(const TraceAttackInfo& info) { }
CBaseEntity* CBaseMonster::DropItem(const char* pszItemName, const Vector& vecPos, const Vector& vecAng) { return nullptr; }
bool CBaseMonster::ShouldFadeOnDeath() { return false; }
void CBaseMonster::RadiusDamage(CBaseEntity* pInflictor, CBaseEntity* pAttacker, float flDamage, int iClassIgnore, int bitsDamageType) { }
void CBaseMonster::RadiusDamage(const Vector& vecSrc, CBaseEntity* pInflictor, CBaseEntity* pAttacker, float flDamage, int iClassIgnore, int bitsDamageType) { }
void CBaseMonster::FadeMonster() { }
void CBaseMonster::GibMonster() { }
bool CBaseMonster::HasHumanGibs() { return false; }
bool CBaseMonster::HasAlienGibs() { return false; }
Activity CBaseMonster::GetDeathActivity() { return ACT_DIE_HEADSHOT; }
NPCState CBaseMonster::GetIdealState() { return NPCState::Alert; }
Schedule_t* CBaseMonster::GetScheduleOfType(int Type) { return nullptr; }
Schedule_t* CBaseMonster::GetSchedule() { return nullptr; }
void CBaseMonster::RunTask(Task_t* pTask) { }
void CBaseMonster::StartTask(Task_t* pTask) { }
Schedule_t* CBaseMonster::ScheduleFromName(const char* pName) { return nullptr; }
void CBaseMonster::BecomeDead() {}
void CBaseMonster::RunAI() {}
void CBaseMonster::Killed(const KilledInfo& info) {}
bool CBaseMonster::GiveHealth(float flHealth, int bitsDamageType) { return false; }
bool CBaseMonster::TakeDamage(const TakeDamageInfo& info) { return false; }
bool CBaseMonster::Restore(class CRestore&) { return true; }
bool CBaseMonster::Save(class CSave&) { return true; }

int TrainSpeed(int iSpeed, int iMax) { return 0; }
void CBasePlayer::DeathSound() { }
bool CBasePlayer::GiveHealth(float flHealth, int bitsDamageType) { return false; }
void CBasePlayer::TraceAttack(const TraceAttackInfo& info) { }
bool CBasePlayer::TakeDamage(const TakeDamageInfo& info) { return false; }
void CBasePlayer::PackDeadPlayerItems() { }
void CBasePlayer::RemoveAllItems(bool removeSuit) { }
void CBasePlayer::SetAnimation(PlayerAnim playerAnim) { }
void CBasePlayer::WaterMove() { }
bool CBasePlayer::IsOnLadder() { return false; }
void CBasePlayer::PlayerDeathThink() { }
void CBasePlayer::StartDeathCam() { }
void CBasePlayer::StartObserver(Vector vecPosition, Vector vecViewAngle) { }
void CBasePlayer::PlayerUse() { }
void CBasePlayer::Jump() { }
void CBasePlayer::Duck() { }
int  CBasePlayer::Classify() { return 0; }
void CBasePlayer::PreThink() { }
void CBasePlayer::CheckTimeBasedDamage() { }
void CBasePlayer::UpdateGeigerCounter() { }
void CBasePlayer::CheckSuitUpdate() { }
void CBasePlayer::SetSuitUpdate(const char* name, SuitSoundType type, int iNoRepeatTime) { }
void CBasePlayer::UpdatePlayerSound() { }
void CBasePlayer::PostThink() { }
void CBasePlayer::Precache() { }
bool CBasePlayer::Save(CSave& save) { return false; }
bool CBasePlayer::Restore(CRestore& restore) { return false; }
bool CBasePlayer::HasWeapons() { return false; }
bool CBasePlayer::FlashlightIsOn() { return false; }
void CBasePlayer::FlashlightTurnOn() { }
void CBasePlayer::FlashlightTurnOff() { }
void CBasePlayer::ForceClientDllUpdate() { }
void CBasePlayer::ImpulseCommands() { }
void CBasePlayer::CheatImpulseCommands(int iImpulse) { }
ItemApplyResult CBasePlayer::AddPlayerWeapon(CBasePlayerWeapon* pWeapon) { return {ItemApplyAction::NotUsed}; }
bool CBasePlayer::RemovePlayerWeapon(CBasePlayerWeapon* pWeapon) { return false; }
void CBasePlayer::WeaponPreFrame() { }
void CBasePlayer::WeaponPostFrame() { }
int CBasePlayer::AmmoInventory(int iAmmoIndex) { return -1; }
int CBasePlayer::GetAmmoIndex(const char* psz) { return -1; }
void CBasePlayer::SendAmmoUpdate() { }
void CBasePlayer::UpdateClientData() { }
bool CBasePlayer::BecomeProne() { return true; }
void CBasePlayer::BarnacleVictimBitten(CBaseEntity* pBarnacle) { }
void CBasePlayer::BarnacleVictimReleased() { }
int CBasePlayer::Illumination() { return 0; }
void CBasePlayer::EnableControl(bool fControl) { }
Vector CBasePlayer::GetAutoaimVector(float flDelta) { return vec3_origin; }
Vector CBasePlayer::AutoaimDeflection(const Vector& vecSrc, float flDist, float flDelta) { return vec3_origin; }
void CBasePlayer::ResetAutoaim() { }
void CBasePlayer::SetCustomDecalFrames(int nFrames) { }
int CBasePlayer::GetCustomDecalFrames() { return -1; }
void CBasePlayer::DropPlayerWeapon(const char* pszWeaponName) { }
bool CBasePlayer::HasPlayerWeapon(CBasePlayerWeapon* pCheckWeapon) { return false; }
bool CBasePlayer::SwitchWeapon(CBasePlayerWeapon* pWeapon) { return false; }
Vector CBasePlayer::GetGunPosition() { return vec3_origin; }
const char* CBasePlayer::TeamID() { return ""; }
int CBasePlayer::GiveAmmo(int iCount, const char* szName) { return 0; }
void CBasePlayer::AddPoints(int score, bool bAllowNegativeScore) { }
void CBasePlayer::AddPointsToTeam(int score, bool bAllowNegativeScore) { }

void SpawnBlood(const Vector& vecSpot, int bloodColor, float flDamage) { }
int DamageDecal(CBaseEntity* pEntity, int bitsDamageType) { return 0; }
void DecalGunshot(TraceResult* pTrace, int iBulletType) { }
void EjectBrass(const Vector& vecOrigin, const Vector& vecVelocity, float rotation, int model, int soundtype) { }
bool CBaseItem::Restore(class CRestore&) { return true; }
bool CBaseItem::Save(class CSave&) { return true; }
bool CBasePlayerWeapon::Restore(class CRestore&) { return true; }
bool CBasePlayerWeapon::Save(class CSave&) { return true; }
bool CBaseAmmo::Restore(class CRestore&) { return true; }
bool CBaseAmmo::Save(class CSave&) { return true; }
float CBasePlayerWeapon::GetNextAttackDelay(float flTime) { return flTime; }
void CBasePlayerWeapon::SetObjectCollisionBox() { }
void CBasePlayerWeapon::KeyValue(KeyValueData* pkvd) {}
void CBasePlayerWeapon::FallInit() { }
ItemApplyResult CBasePlayerWeapon::Apply(CBasePlayer* pPlayer) { return {ItemApplyAction::NotUsed}; }
void CBasePlayerWeapon::DestroyWeapon() { }
void CBasePlayerWeapon::Drop() { }
void CBasePlayerWeapon::Kill() { }
void CBasePlayerWeapon::AttachToPlayer(CBasePlayer* pPlayer) { }
CBasePlayerWeapon* CBasePlayerWeapon::GetItemToRespawn(const Vector& respawnPoint) { return nullptr; }
bool CBasePlayerWeapon::AddDuplicate(CBasePlayerWeapon* pOriginal) { return false; }
bool CBasePlayerWeapon::AddToPlayer(CBasePlayer* pPlayer) { return false; }
bool CBasePlayerWeapon::UpdateClientData(CBasePlayer* pPlayer) { return false; }
bool CBasePlayerWeapon::AddPrimaryAmmo(int iCount, const char* szName) { return true; }
bool CBasePlayerWeapon::AddSecondaryAmmo(int iCount, const char* szName) { return true; }
bool CBasePlayerWeapon::IsUseable() { return true; }
bool CBasePlayerWeapon::ExtractAmmo(CBasePlayerWeapon* pWeapon) { return false; }
int CBasePlayerWeapon::ExtractClipAmmo(CBasePlayerWeapon* pWeapon) { return 0; }
void CBasePlayerWeapon::RetireWeapon() { }
void CBaseAmmo::KeyValue(KeyValueData* pkvd) {}
void CBaseAmmo::Precache() {}
ItemApplyResult CBaseAmmo::Apply(CBasePlayer* player) { return {ItemApplyAction::NotUsed}; }
void CSoundEnt::InsertSound(int iType, const Vector& vecOrigin, int iVolume, float flDuration) {}

CBaseEntity* UTIL_FindEntityByClassname(CBaseEntity* pStartEntity, const char* szName) { return nullptr; }
