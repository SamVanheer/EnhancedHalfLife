//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

/**
*	@brief the current state of the key
*/
constexpr int KEYBUTTON_DOWN = 1 << 0;

/**
*	@brief edge triggered on the up to down transition
*/
constexpr int KEYBUTTON_IMPULSE_DOWN = 1 << 1;

/**
*	@brief edge triggered on the down to up transition
*/
constexpr int KEYBUTTON_IMPULSE_UP = 1 << 2;

/**
*	@brief Is a button down at all?
*/
constexpr int KEYBUTTON_MASK_ANYDOWN = KEYBUTTON_DOWN | KEYBUTTON_IMPULSE_DOWN;

constexpr int KEYBUTTON_MASK_ALL = KEYBUTTON_DOWN | KEYBUTTON_IMPULSE_DOWN | KEYBUTTON_IMPULSE_UP;

/**
*	@details Continuous button event tracking is complicated by the fact that two different input sources
*	(say, mouse button 1 and the control key) can both press the same button,
*	but the button should only be released when both of the pressing key have been released.
*
*	When a key event issues a button command (+forward, +attack, etc),
*	it appends its key number as a parameter to the command so it can be matched up with the release.
*/
struct kbutton_t
{
	int		down[2];		// key nums holding it down
	int		state;			// low bit is down state
};
