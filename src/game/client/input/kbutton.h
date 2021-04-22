//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

/**
*	@details Continuous button event tracking is complicated by the fact that two different input sources
*	(say, mouse button 1 and the control key) can both press the same button,
*	but the button should only be released when both of the pressing key have been released.
*
*	When a key event issues a button command (+forward, +attack, etc),
*	it appends its key number as a parameter to the command so it can be matched up with the release.
*
*	state bit 0 is the current state of the key
*	state bit 1 is edge triggered on the up to down transition
*	state bit 2 is edge triggered on the down to up transition
*/
//TODO: define state bits
struct kbutton_t
{
	int		down[2];		// key nums holding it down
	int		state;			// low bit is down state
};
