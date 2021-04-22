//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include "cl_entity.h"
#include "interpolation.h"

struct cl_entity_t;
struct model_t;

constexpr int INSET_OFF = 0;
constexpr int INSET_CHASE_FREE = 1;
constexpr int INSET_IN_EYE = 2;
constexpr int INSET_MAP_FREE = 3;
constexpr int INSET_MAP_CHASE = 4;

constexpr int MAX_SPEC_HUD_MESSAGES = 8;

constexpr int OVERVIEW_TILE_SIZE = 128;		// don't change this
constexpr int OVERVIEW_MAX_LAYERS = 1;

struct overviewInfo_t
{
	char		map[64];	// cl.levelname or empty
	Vector		origin;		// center of map
	float		zoom;		// zoom of map images
	int			layers;		// how may layers do we have
	float		layersHeights[OVERVIEW_MAX_LAYERS];
	char		layersImages[OVERVIEW_MAX_LAYERS][255];
	bool		rotated;	// are map images rotated (90 degrees) ?

	int			insetWindowX;
	int			insetWindowY;
	int			insetWindowHeight;
	int			insetWindowWidth;
};

struct overviewEntity_t
{

	HSPRITE					hSprite;
	cl_entity_t* entity;
	double					killTime;
};

struct cameraWayPoint_t
{
	float	time;
	Vector	position;
	Vector	angle;
	float	fov;
	int		flags;
};

constexpr int MAX_OVERVIEW_ENTITIES = 128;
constexpr int MAX_CAM_WAYPOINTS = 32;

/**
*	@brief Handles the drawing of the spectator stuff (camera & top-down map and all the things on it )
*/
class CHudSpectator : public CHudBase
{
public:
	void Reset() override;
	int ToggleInset(bool allowOff);
	void CheckSettings();
	void InitHUDData() override;
	bool AddOverviewEntityToList(HSPRITE sprite, cl_entity_t* ent, double killTime);
	void DeathMessage(int victim);
	bool AddOverviewEntity(int type, cl_entity_t* ent, const char* modelname);
	void CheckOverviewEntities();
	void DrawOverview();
	void DrawOverviewEntities();
	void DrawOverviewLayer();
	void LoadMapSprites();
	bool ParseOverviewFile();
	bool IsActivePlayer(cl_entity_t* ent);
	void SetModes(int iMainMode, int iInsetMode);
	void HandleButtonsDown(int ButtonPressed);
	void HandleButtonsUp(int ButtonPressed);
	void FindNextPlayer(bool bReverse);
	void FindPlayer(const char* name);
	void DirectorMessage(int iSize, void* pbuf);

	/**
	*	@brief Get valid map position and 'beam' spectator to this position
	*/
	void SetSpectatorStartPosition();
	bool Init() override;

	/**
	*	@brief Loads new icons
	*/
	bool VidInit() override;

	bool Draw(float flTime) override;

	void AddWaypoint(float time, Vector pos, Vector angle, float fov, int flags);
	void SetCameraView(Vector pos, Vector angle, float fov);
	float GetFOV();
	bool GetDirectorCamera(Vector& position, Vector& angle);
	void SetWayInterpolation(cameraWayPoint_t* prev, cameraWayPoint_t* start, cameraWayPoint_t* end, cameraWayPoint_t* next);

	int m_iDrawCycle = 0;
	client_textmessage_t m_HUDMessages[MAX_SPEC_HUD_MESSAGES]{};
	char m_HUDMessageText[MAX_SPEC_HUD_MESSAGES][128]{};
	int m_lastHudMessage = 0;
	overviewInfo_t m_OverviewData{};
	overviewEntity_t m_OverviewEntities[MAX_OVERVIEW_ENTITIES]{};
	int m_iObserverFlags = 0;
	int m_iSpectatorNumber = 0;

	float m_mapZoom = 0;		//!< zoom the user currently uses
	Vector m_mapOrigin{vec3_origin};	//!< origin where user rotates around
	cvar_t* m_drawnames = nullptr;
	cvar_t* m_drawcone = nullptr;
	cvar_t* m_drawstatus = nullptr;
	cvar_t* m_autoDirector = nullptr;
	cvar_t* m_pip = nullptr;
	bool m_chatEnabled = false;

	bool m_IsInterpolating = false;
	int m_ChaseEntity = 0;	//!< if != 0, follow this entity with viewangles
	int m_WayPoint = 0;		//!< current waypoint 1
	int m_NumWayPoints = 0;	//!< current number of waypoints
	Vector m_cameraOrigin{vec3_origin};	//!< a help camera
	Vector m_cameraAngles{vec3_origin};	//!< and it's angles
	CInterpolation m_WayInterpolation{};

private:
	Vector m_vPlayerPos[MAX_PLAYERS]{};
	HSPRITE m_hsprPlayerBlue{};
	HSPRITE m_hsprPlayerRed{};
	HSPRITE m_hsprPlayer{};
	HSPRITE m_hsprCamera{};
	HSPRITE m_hsprPlayerDead{};
	HSPRITE m_hsprViewcone{};
	HSPRITE m_hsprUnkownMap{};
	HSPRITE m_hsprBeam{};
	HSPRITE m_hCrosshair{};

	wrect_t m_crosshairRect;

	model_t* m_MapSprite = nullptr;	// each layer image is saved in one sprite, where each tile is a sprite frame
	float m_flNextObserverInput = 0;
	float m_FOV = 0;
	float m_zoomDelta = 0;
	float m_moveDelta = 0;
	int m_lastPrimaryObject = 0;
	int m_lastSecondaryObject = 0;

	cameraWayPoint_t m_CamPath[MAX_CAM_WAYPOINTS]{};
};
