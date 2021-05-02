#pragma once

#include "steam/steamtypes.h"
#include "netadr.h"

struct cl_enginefunc_t;
struct cl_entity_t;
struct client_data_t;
struct clientdata_t;
struct engine_studio_api_t;
struct entity_state_t;
struct kbutton_t;
struct local_state_t;
struct mstudioevent_t;
struct netadr_t;
struct playermove_t;
struct r_studio_interface_t;
struct ref_params_t;
struct TEMPENTITY;
struct usercmd_t;
struct weapon_data_t;

constexpr int CLDLL_INTERFACE_VERSION = 7;

/**
*	@brief Functions exported by the client .dll
*/
struct cldll_func_t
{
	int (*pInitFunc)(cl_enginefunc_t*, int);

	/**
	*	@brief Called when the game initializes and whenever the vid_mode is changed so the HUD can reinitialize itself.
	*/
	void (*pHudInitFunc)();

	/**
	*	@brief Called whenever the client connects to a server.
	*	Reinitializes all the hud variables.
	*/
	int (*pHudVidInitFunc)();

	/**
	*	@brief called every screen frame to redraw the HUD.
	*/
	int (*pHudRedrawFunc)(float, int);

	/**
	*	@brief called every time shared client dll/engine data gets changed, and gives the cdll a chance to modify the data.
	*	@return true if anything has been changed, false otherwise.
	*/
	int (*pHudUpdateClientDataFunc)(client_data_t*, float);

	/**
	*	@brief Called at start and end of demos to restore to "non"HUD state.
	*/
	void (*pHudResetFunc)();
	void (*pClientMove)(playermove_t* ppmove, qboolean server);
	void (*pClientMoveInit)(playermove_t* ppmove);
	char (*pClientTextureType)(char* name);
	void (*pIN_ActivateMouse)();
	void (*pIN_DeactivateMouse)();
	void (*pIN_MouseEvent)(int mstate);
	void (*pIN_ClearStates)();
	void (*pIN_Accumulate)();

	/**
	*	@brief Send the intended movement message to the server
	*	if active == true then we are
	*	1) not playing back demos ( where our commands are ignored ) and
	*	2 ) we have finished signing on to server
	*/
	void (*pCL_CreateMove)(float frametime, usercmd_t* cmd, int active);
	int (*pCL_IsThirdPerson)();
	void (*pCL_GetCameraOffsets)(Vector* ofs);

	/**
	*	@brief Allows the engine to get a kbutton_t directly ( so it can check +mlook state, etc ) for saving out to .cfg files
	*/
	kbutton_t* (*pFindKey)(const char* name);
	void (*pCamThink)();
	void (*pCalcRefdef)(ref_params_t* pparams);

	/**
	*	@brief Return false to filter entity from visible list for rendering
	*/
	int	(*pAddEntity)(int type, cl_entity_t* ent, const char* modelname);

	/**
	*	@brief Gives us a chance to add additional entities to the render this frame
	*/
	void (*pCreateEntities)();

	/**
	*	@brief Non-transparent triangles-- add them here
	*/
	void (*pDrawNormalTriangles)();

	/**
	*	@brief Render any triangles with transparent rendermode needs here
	*/
	void (*pDrawTransparentTriangles)();

	/**
	*	@brief The entity's studio model description indicated an event was fired during this frame,
	*	handle the event by it's tag ( e.g., muzzleflash, sound )
	*/
	void (*pStudioEvent)(const mstudioevent_t* event, const cl_entity_t* entity);

	/**
	*	@brief Client calls this during prediction, after it has moved the player and updated any info changed into to->
	*	@param cmd is the command that caused the movement, etc
	*	@param runfuncs is true if this is the first time we've predicted this command.
	*					If so, sounds and effects should play, otherwise, they should be ignored
	*	@param time is the current client clock based on prediction
	*/
	void (*pPostRunCmd) (local_state_t* from, local_state_t* to, usercmd_t* cmd, int runfuncs, double time, unsigned int random_seed);
	void (*pShutdown)();

	/**
	*	@brief The server sends us our origin with extra precision as part of the clientdata structure,
	*	not during the normal playerstate update in entity_state_t.
	*	In order for these overrides to eventually get to the appropriate playerstate structure,
	*	we need to copy them into the state structure at this point.
	*/
	void (*pTxferLocalOverrides)(entity_state_t* state, const clientdata_t* client);

	/**
	*	@brief We have received entity_state_t for this player over the network.
	*	We need to copy appropriate fields to the playerstate structure
	*/
	void (*pProcessPlayerState)(entity_state_t* dst, const entity_state_t* src);

	/**
	*	@brief Because we can predict an arbitrary number of frames before the server responds with an update,
	*	we need to be able to copy client side prediction data in from the state that the server ack'd receiving,
	*	which can be anywhere along the predicted frame path
	*	( i.e., we could predict 20 frames into the future and the server ack's up through 10 of those frames,
	*	so we need to copy persistent client-side only state from the 10th predicted frame to the slot the serverupdate is occupying.)
	*/
	void (*pTxferPredictionData)(entity_state_t* ps, const entity_state_t* pps, clientdata_t* pcd, const clientdata_t* ppcd,
		weapon_data_t* wd, const weapon_data_t* pwd);

	/**
	*	@brief Engine wants us to parse some data from the demo stream
	*/
	void (*pReadDemoBuffer)(int size, unsigned char* buffer);

	/**
	*	@brief Return true if the packet is valid. Set response_buffer_size if you want to send a response packet.
	*	Incoming, it holds the max size of the response_buffer, so you must zero it out if you choose not to respond.
	*/
	int (*pConnectionlessPacket)(const netadr_t* net_from, const char* args, char* response_buffer, int* response_buffer_size);

	/**
	*	@brief Engine calls this to enumerate player collision hulls, for prediction. Return false if the hullnumber doesn't exist.
	*/
	int	(*pGetHullBounds)(int hullnumber, Vector* mins, Vector* maxs);

	/**
	*	@brief Called by engine every frame that client .dll is loaded
	*/
	void (*pHudFrame)(double);

	/**
	*	@brief Return true to allow engine to process the key, otherwise, act on it as needed
	*/
	int (*pKeyEvent)(int eventcode, int keynum, const char* pszCurrentBinding);

	/**
	*	@brief Simulation and cleanup of temporary entities
	*/
	void (*pTempEntUpdate)(double frametime, double client_time, double cl_gravity, TEMPENTITY** ppTempEntFree, TEMPENTITY** ppTempEntActive,
		int (*Callback_AddVisibleEntity)(cl_entity_t* pEntity), void (*Callback_TempEntPlaySound)(TEMPENTITY* pTemp, float damp));

	/**
	*	@brief If you specify negative numbers for beam start and end point entities,
	*	then the engine will call back into this function requesting a pointer
	*	to a cl_entity_t object that describes the entity to attach the beam onto.
	*	Indices must start at 1, not zero.
	*/
	cl_entity_t* (*pGetUserEntity)(int index);

	/**
	*	@brief Called when a player starts or stops talking.
	*	Possibly null on old client dlls.
	*/
	void (*pVoiceStatus)(int entindex, qboolean bTalking);

	/**
	*	@brief Called when a director event message was received
	*	Possibly null on old client dlls.
	*/
	void (*pDirectorMessage)(int iSize, void* pbuf);

	/**
	*	@brief Export this function for the engine to use the studio renderer class to render objects.
	*	Not used by all clients
	*/
	int (*pStudioInterface)(int version, r_studio_interface_t** ppinterface, engine_studio_api_t* pstudio);

	/**
	*	@brief Sets the location of the input for chat text
	*	Not used by all clients
	*/
	void (*pChatInputPosition)(int* x, int* y);

	/**
	*	@brief Gets the team index for the given player
	*	Not used by all clients
	*	Doesn't seem to be used in the engine anywhere
	*/
	int (*pGetPlayerTeam)(int iplayer);

	/**
	*	@details this should be CreateInterfaceFn but that means including interface.h
	*	which is a C++ file and some of the client files a C only...
	*	so we return a void * which we then do a typecast on later.
	*/
	void* (*pClientFactory)();
};
