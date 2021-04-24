//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include<cstdio> 
#include<VGUI_InputStream.h>

namespace vgui
{

class VGUIAPI FileInputStream : public InputStream
{
private:
	FILE* _fp;
public:
	FileInputStream(const char* fileName,bool textMode);
public:
	virtual void  seekStart(bool& success);
	virtual void  seekRelative(int count,bool& success);
	virtual void  seekEnd(bool& success);
	virtual int   getAvailable(bool& success);
	virtual uchar readUChar(bool& success);
	virtual void  readUChar(uchar* buf,int count,bool& success);
	virtual void  close(bool& success);
	virtual void  close();
};

}
