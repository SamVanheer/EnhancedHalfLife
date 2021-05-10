//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include "bitvec.h"
#include "cdll_dll.h"

constexpr int VOICE_MAX_PLAYERS = MAX_CLIENTS;
constexpr int VOICE_MAX_PLAYERS_DW = (VOICE_MAX_PLAYERS / 32) + !!(VOICE_MAX_PLAYERS & 31);

typedef CBitVec<VOICE_MAX_PLAYERS> CPlayerBitVec;
