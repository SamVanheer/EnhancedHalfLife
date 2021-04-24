//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include<VGUI.h>

namespace vgui
{

class Panel;

class VGUIAPI ActionSignal
{
public:
	virtual void actionPerformed(Panel* panel)=0;
};

}
