//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include<VGUI.h>
#include<VGUI_Scheme.h>

namespace vgui
{

class VGUIAPI Color
{
private:
	uchar               _color[4];
	Scheme::SchemeColor _schemeColor;
public:
	Color();
	Color(int r,int g,int b,int a);
	Color(Scheme::SchemeColor sc);
private:
	virtual void init();
public:
	virtual void setColor(int r,int g,int b,int a);
	virtual void setColor(Scheme::SchemeColor sc);
	virtual void getColor(int& r,int& g,int& b,int& a);
	virtual void getColor(Scheme::SchemeColor& sc);
	virtual int  operator[](int index);
};

}
