//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

struct entity_state_t;

class CStudioModelRenderer
{
public:
	CStudioModelRenderer();
	virtual ~CStudioModelRenderer();

	virtual void Init();

public:
	// Public Interfaces
	virtual bool StudioDrawModel(int flags);
	virtual bool StudioDrawPlayer(int flags, entity_state_t* pplayer);

public:
	// Local interfaces
	//

	// Look up animation data for sequence
	virtual mstudioanim_t* StudioGetAnim(model_t* m_pSubModel, mstudioseqdesc_t* pseqdesc);

	// Interpolate model position and angles and set up matrices
	virtual void StudioSetUpTransform(int trivial_accept);

	// Set up model bone positions
	virtual void StudioSetupBones();

	// Find final attachment points
	virtual void StudioCalcAttachments();

	// Save bone matrices and names
	virtual void StudioSaveBones();

	// Merge cached bones with current bones for model
	virtual void StudioMergeBones(model_t* m_pSubModel);

	// Determine interpolation fraction
	virtual float StudioEstimateInterpolant();

	// Determine current frame for rendering
	virtual float StudioEstimateFrame(mstudioseqdesc_t* pseqdesc);

	// Apply special effects to transform matrix
	virtual void StudioFxTransform(cl_entity_t* ent, float transform[3][4]);

	// Spherical interpolation of bones
	virtual void StudioSlerpBones(vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s);

	// Compute bone adjustments ( bone controllers )
	virtual void StudioCalcBoneAdj(float dadt, float* adj, const byte* pcontroller1, const byte* pcontroller2, byte mouthopen);

	// Get bone quaternions
	virtual void StudioCalcBoneQuaterion(int frame, float s, mstudiobone_t* pbone, mstudioanim_t* panim, float* adj, float* q);

	// Get bone positions
	virtual void StudioCalcBonePosition(int frame, float s, mstudiobone_t* pbone, mstudioanim_t* panim, float* adj, float* pos);

	// Compute rotations
	virtual void StudioCalcRotations(float pos[][3], vec4_t* q, mstudioseqdesc_t* pseqdesc, mstudioanim_t* panim, float f);

	// Send bones and verts to renderer
	virtual void StudioRenderModel();

	// Finalize rendering
	virtual void StudioRenderFinal();

	// GL&D3D vs. Software renderer finishing functions
	virtual void StudioRenderFinal_Software();
	virtual void StudioRenderFinal_Hardware();

	// Player specific data
	// Determine pitch and blending amounts for players
	virtual void StudioPlayerBlend(mstudioseqdesc_t* pseqdesc, int& blend, float& pitch);

	// Estimate gait frame for player
	virtual void StudioEstimateGait(entity_state_t* pplayer);

	// Process movement of player
	virtual void StudioProcessGait(entity_state_t* pplayer);

public:

	// Client clock
	double			m_clTime = 0;
	// Old Client clock
	double			m_clOldTime = 0;

	// Do interpolation?
	bool			m_fDoInterp = true;
	// Do gait estimation?
	bool			m_fGaitEstimation = true;

	// Current render frame #
	int				m_nFrameCount = 0;

	// Cvars that studio model code needs to reference
	//
	// Use high quality models?
	cvar_t* m_pCvarHiModels = nullptr;
	// Developer debug output desired?
	cvar_t* m_pCvarDeveloper = nullptr;
	// Draw entities bone hit boxes, etc?
	cvar_t* m_pCvarDrawEntities = nullptr;

	// The entity which we are currently rendering.
	cl_entity_t* m_pCurrentEntity = nullptr;

	// The model for the entity being rendered
	model_t* m_pRenderModel = nullptr;

	// Player info for current player, if drawing a player
	player_info_t* m_pPlayerInfo = nullptr;

	// The index of the player being drawn
	int				m_nPlayerIndex = 0;

	// The player's gait movement
	float			m_flGaitMovement = 0;

	// Pointer to header block for studio model data
	studiohdr_t* m_pStudioHeader = nullptr;

	// Pointers to current body part and submodel
	mstudiobodyparts_t* m_pBodyPart = nullptr;
	mstudiomodel_t* m_pSubModel = nullptr;

	// Palette substition for top and bottom of model
	int				m_nTopColor = 0;
	int				m_nBottomColor = 0;

	//
	// Sprite model used for drawing studio model chrome
	model_t* m_pChromeSprite = nullptr;

	// Caching
	// Number of bones in bone cache
	int				m_nCachedBones = 0;
	// Names of cached bones
	char			m_nCachedBoneNames[MAXSTUDIOBONES][32]{};
	// Cached bone & light transformation matrices
	float			m_rgCachedBoneTransform[MAXSTUDIOBONES][3][4]{};
	float			m_rgCachedLightTransform[MAXSTUDIOBONES][3][4]{};

	// Software renderer scale factors
	float			m_fSoftwareXScale = 0, m_fSoftwareYScale = 0;

	// Current view vectors and render origin
	Vector			m_vUp{vec3_origin};
	Vector			m_vRight{vec3_origin};
	Vector			m_vNormal{vec3_origin};

	Vector			m_vRenderOrigin{vec3_origin};

	// Model render counters ( from engine )
	int* m_pStudioModelCount = nullptr;
	int* m_pModelsDrawn = nullptr;

	// Matrices
	// Model to world transformation
	float(*m_protationmatrix)[3][4] = nullptr;
	// Model to view transformation
	float(*m_paliastransform)[3][4] = nullptr;

	// Concatenated bone and light transforms
	float(*m_pbonetransform)[MAXSTUDIOBONES][3][4] = nullptr;
	float(*m_plighttransform)[MAXSTUDIOBONES][3][4] = nullptr;
};
