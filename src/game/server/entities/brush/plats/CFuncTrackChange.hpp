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

#include "CFuncPlatRot.hpp"
#include "CFuncTrackChange.generated.hpp"

class CFuncTrackTrain;
class CPathTrack;

constexpr int SF_TRACK_ACTIVATETRAIN = 0x00000001;
constexpr int SF_TRACK_RELINK = 0x00000002;
constexpr int SF_TRACK_ROTMOVE = 0x00000004;
constexpr int SF_TRACK_STARTBOTTOM = 0x00000008;
constexpr int SF_TRACK_DONT_MOVE = 0x00000010;

enum class TrainCode
{
	Safe,
	Blocking,
	Following
};

/**
*	@brief Track changer / Train elevator
*	@details This entity is a rotating/moving platform that will carry a train to a new track.
*	It must be larger in X-Y planar area than the train,
*	since it must contain the train within these dimensions in order to operate when the train is near it.
*/
class EHL_CLASS(EntityName=func_trackchange) CFuncTrackChange : public CFuncPlatRot
{
	EHL_GENERATED_BODY()

public:
	void Spawn() override;
	void Precache() override;

	//	void	Blocked() override;
	void	EXPORT GoUp() override;
	void	EXPORT GoDown() override;

	void			KeyValue(KeyValueData* pkvd) override;
	void			Use(const UseInfo& info) override;
	void			EXPORT Find();
	TrainCode		EvaluateTrain(CPathTrack* pcurrent);
	void			UpdateTrain(Vector& dest);
	void	HitBottom() override;
	void	HitTop() override;
	void			Touch(CBaseEntity* pOther) override;

	/**
	*	@brief Normal track change
	*/
	virtual void	UpdateAutoTargets(ToggleState toggleState);
	bool	IsTogglePlat() override { return true; }

	void			DisableUse() { m_use = false; }
	void			EnableUse() { m_use = true; }
	bool UseEnabled() { return m_use; }

	void	OverrideReset() override;

	EHL_FIELD(Persisted)
	EHandle<CPathTrack> m_hTrackTop;

	EHL_FIELD(Persisted)
	EHandle<CPathTrack> m_hTrackBottom;

	EHL_FIELD(Persisted)
	EHandle<CFuncTrackTrain> m_hTrain;

	EHL_FIELD(Persisted)
	string_t m_trackTopName = iStringNull;

	EHL_FIELD(Persisted)
	string_t m_trackBottomName = iStringNull;

	EHL_FIELD(Persisted)
	string_t m_trainName = iStringNull;

	EHL_FIELD(Persisted)
	TrainCode m_code = TrainCode::Safe;

	EHL_FIELD(Persisted)
	ToggleState m_targetState = ToggleState::AtTop;

	EHL_FIELD(Persisted)
	bool m_use = false;
};
