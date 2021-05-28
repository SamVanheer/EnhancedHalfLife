//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include "VGUI_Label.h"
#include "VGUI_LineBorder.h"
#include "VGUI_ImagePanel.h"
#include "VGUI_BitmapTGA.h"
#include "VGUI_InputSignal.h"
#include "VGUI_Button.h"
#include "voice_common.hpp"
#include "cl_entity.hpp"
#include "voice_banmgr.hpp"
#include "vgui_checkbutton2.hpp"
#include "vgui_defaultinputsignal.hpp"


class CVoiceStatus;


class CVoiceLabel
{
public:
	vgui::Label* m_pLabel;
	vgui::Label* m_pBackground;
	vgui::ImagePanel* m_pIcon;		//!< Voice icon next to player name.
	int					m_clientindex;	//!< Client index of the speaker. -1 if this label isn't being used.
};


/**
*	@brief This is provided by each mod to access data that may not be the same across mods.
*/
class IVoiceStatusHelper
{
public:
	virtual					~IVoiceStatusHelper() {}

	/**
	*	@brief Get RGB color for voice status text about this player.
	*/
	virtual void			GetPlayerTextColor(int entindex, int color[3]) = 0;

	/**
	*	@brief Force it to update the cursor state.
	*/
	virtual void			UpdateCursorState() = 0;

	/**
	*	@brief Return the height above the bottom that the voice ack icons should be drawn at.
	*/
	virtual int				GetAckIconHeight() = 0;

	/**
	*	@brief Return true if the voice manager is allowed to show speaker labels (mods usually return false when the scoreboard is up).
	*/
	virtual bool			CanShowSpeakerLabels() = 0;
};

/**
*	@brief Holds a color for the shared image
*/
class VoiceImagePanel : public vgui::ImagePanel
{
	virtual void paintBackground()
	{
		if (_image != nullptr)
		{
			vgui::Color col;
			getFgColor(col);
			_image->setColor(col);
			_image->doPaint(this);
		}
	}
};

class CVoiceStatus : public CHudBase, public vgui::CDefaultInputSignal
{
public:
	CVoiceStatus() = default;
	virtual ~CVoiceStatus();

	// CHudBase overrides.
public:

	/**
	*	@brief Initialize the cl_dll's voice manager.
	*/
	virtual bool Init(
		IVoiceStatusHelper* m_pHelper,
		vgui::Panel** pParentPanel);

	/**
	*	@brief ackPosition is the bottom position of where CVoiceStatus will draw the voice acknowledgement labels.
	*/
	bool VidInit() override;

	void Shutdown() override;

public:

	/**
	*	@brief Call from HUD_Frame each frame.
	*/
	void Frame(double frametime);

	/**
	*	@brief Called when a player starts or stops talking.
	*	@details entindex is -1 to represent the local client talking (before the data comes back from the server).
	*	When the server acknowledges that the local client is talking, then entindex will be gEngfuncs.GetLocalPlayer().
	*	entindex is -2 to represent the local client's voice being acked by the server.
	*/
	void UpdateSpeakerStatus(int entindex, bool bTalking);

	/**
	*	@brief sets the correct image in the label for the player
	*/
	void UpdateSpeakerImage(vgui::Label* pLabel, int iPlayer);

	/**
	*	@brief Call from the HUD_CreateEntities function so it can add sprites above player heads.
	*/
	void CreateEntities();

	/**
	*	@brief Called when the server registers a change to who this client can hear.
	*/
	void HandleVoiceMaskMsg(int iSize, void* pbuf);

	/**
	*	@brief The server sends this message initially to tell the client to send their state.
	*/
	void HandleReqStateMsg(int iSize, void* pbuf);


	// Squelch mode functions.
public:

	// When you enter squelch mode, pass in 
	void StartSquelchMode();
	void StopSquelchMode();
	bool IsInSquelchMode();

	/**
	*	@brief returns true if the target client has been banned
	*	@param iPlayerIndex is of range 1..maxplayers
	*/
	bool IsPlayerBlocked(int iPlayerIndex);

	/**
	*	@brief returns false if the player can't hear the other client due to game rules (eg. the other team)
	*/
	bool  IsPlayerAudible(int iPlayerIndex);

	/**
	*	@brief blocks/unblocks the target client from being heard
	*/
	void SetPlayerBlockedState(int iPlayerIndex, bool blocked);

public:
	/**
	*	@brief Find a CVoiceLabel representing the specified speaker.
	*	@param clientindex can be -1 if you want a currently-unused voice label.
	*	@return nullptr if none.
	*/
	CVoiceLabel* FindVoiceLabel(int clientindex);

	/**
	*	@brief Get an unused voice label. Returns nullptr if none.
	*/
	CVoiceLabel* GetFreeVoiceLabel();

	void RepositionLabels();

	void FreeBitmaps();

	void UpdateServerState(bool bForce);

	/**
	*	@brief Update the button artwork to reflect the client's current state.
	*/
	void UpdateBanButton(int iClient);


public:
	enum { MAX_VOICE_SPEAKERS = 7 };

	float m_LastUpdateServerState = 0;	//!< Last time we called this function.
	int m_bServerModEnable = -1;		//!< What we've sent to the server about our "voice_modenable" cvar.

	vgui::Panel** m_pParentPanel = nullptr;
	CPlayerBitVec m_VoicePlayers;		//!< Who is currently talking. Indexed by client index.

	/**
	*	@brief This is the gamerules-defined list of players that you can hear.
	*	It is based on what teams people are on and is totally separate from the ban list.
	*	Indexed by client index.
	*/
	CPlayerBitVec m_AudiblePlayers;

	/**
	*	@brief Players who have spoken at least once in the game so far
	*/
	CPlayerBitVec m_VoiceEnabledPlayers;

	/**
	*	@brief This is who the server THINKS we have banned (it can become incorrect when a new player arrives on the server).
	*	It is checked periodically, and the server is told to squelch or unsquelch the appropriate players.
	*/
	CPlayerBitVec m_ServerBannedPlayers;

	/**
	*	@brief These aren't necessarily in the order of players.
	*	They are just a place for it to put data in during CreateEntities.
	*/
	cl_entity_t m_VoiceHeadModels[VOICE_MAX_PLAYERS]{};

	IVoiceStatusHelper* m_pHelper = nullptr;		//!< Each mod provides an implementation of this.

	// Scoreboard icons.
	double m_BlinkTimer = 0;			//!< Blink scoreboard icons..
	vgui::BitmapTGA* m_pScoreboardNeverSpoken = nullptr;
	vgui::BitmapTGA* m_pScoreboardNotSpeaking = nullptr;
	vgui::BitmapTGA* m_pScoreboardSpeaking = nullptr;
	vgui::BitmapTGA* m_pScoreboardSpeaking2 = nullptr;
	vgui::BitmapTGA* m_pScoreboardSquelch = nullptr;
	vgui::BitmapTGA* m_pScoreboardBanned = nullptr;

	vgui::Label* m_pBanButtons[VOICE_MAX_PLAYERS]{};		//!< scoreboard buttons.

	// Squelch mode stuff.
	bool m_bInSquelchMode = false;

	HSPRITE m_VoiceHeadModel{};		//!< Voice head model (goes above players who are speaking).
	float m_VoiceHeadModelHeight = 0;	//!< Height above their head to place the model.

	vgui::Image* m_pSpeakerLabelIcon = nullptr;	// Icon next to speaker labels.

	// Lower-right icons telling when the local player is talking..
	vgui::BitmapTGA* m_pLocalBitmap = nullptr;		//!< Represents the local client talking.
	vgui::BitmapTGA* m_pAckBitmap = nullptr;		//!< Represents the server ack'ing the client talking.
	vgui::ImagePanel* m_pLocalLabel = nullptr;		//!< Represents the local client talking.

	bool m_bTalking = false;				//!< Set to true when the client thinks it's talking.
	bool m_bServerAcked = false;			//!< Set to true when the server knows the client is talking.

public:
	CVoiceBanMgr m_BanMgr;				//!< Tracks which users we have squelched and don't want to hear.

public:
	bool m_bBanMgrInitialized = false;

	/**
	*	@brief Labels telling who is speaking.
	*/
	CVoiceLabel m_Labels[MAX_VOICE_SPEAKERS]{};
};


/**
*	@brief Get the (global) voice manager.
*/
CVoiceStatus* GetClientVoiceMgr();