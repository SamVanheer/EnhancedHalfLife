//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include"vgui_int.h"
#include<VGUI_BorderLayout.h>
#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"

using namespace vgui;

void VGui_ViewportPaintBackground(int extents[4])
{
	gEngfuncs.VGui_ViewportPaintBackground(extents);
}

Panel* VGui_GetPanel()
{
	return (Panel*)gEngfuncs.VGui_GetPanel();
}

void VGui_Startup()
{
	Panel* root = VGui_GetPanel();
	root->setBgColor(128, 128, 0, 0);
	root->setLayout(new BorderLayout(0));

	if (gViewPort != nullptr)
	{
		gViewPort->Initialize();
	}
	else
	{
		gViewPort = new TeamFortressViewport(0, 0, root->getWide(), root->getTall());
		gViewPort->setParent(root);
	}
}

void VGui_Shutdown()
{
	delete gViewPort;
	gViewPort = nullptr;
}





