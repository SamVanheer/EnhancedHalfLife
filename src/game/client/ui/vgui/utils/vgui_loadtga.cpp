//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "../client/wrect.h"
#include "../client/cl_dll.h"
#include "VGUI.h"
#include "vgui_loadtga.h"
#include "VGUI_InputStream.h"
#include "filesystem_shared.hpp"


// ---------------------------------------------------------------------- //
// Helper class for loading tga files.
// ---------------------------------------------------------------------- //
class MemoryInputStream : public vgui::InputStream
{
public:
				MemoryInputStream()
				{
					m_pData = nullptr;
					m_DataLen = m_ReadPos = 0;
				}
		
	void  seekStart(bool& success) override {m_ReadPos=0; success=true;}
	void  seekRelative(int count,bool& success) override {m_ReadPos+=count; success=true;}
	void  seekEnd(bool& success) override {m_ReadPos=m_DataLen; success=true;}
	int   getAvailable(bool& success) override {success=false; return 0;} // This is what vgui does for files...
	
	uchar readUChar(bool& success) override
	{
		if(m_ReadPos>=0 && m_ReadPos<m_DataLen)
		{
			success=true;
			uchar ret = m_pData[m_ReadPos];
			++m_ReadPos;
			return ret;
		}
		else
		{
			success=false;
			return 0;
		}
	}

	void  readUChar(uchar* buf,int count,bool& success) override
	{
		for(int i=0; i < count; i++)
			buf[i] = readUChar(success);
	}

	void  close(bool& success) override
	{
		m_pData = nullptr;
		m_DataLen = m_ReadPos = 0;
	}

	uchar		*m_pData;
	int			m_DataLen;
	int			m_ReadPos;
};

static vgui::BitmapTGA* vgui_LoadTGA(char const* pFilename, bool invertAlpha)
{
	auto [fileBuffer, size] = FileSystem_LoadFileIntoBuffer(pFilename);

	MemoryInputStream stream;

	stream.m_pData = fileBuffer.get();
	stream.m_DataLen = static_cast<int>(size);

	if (!stream.m_pData)
		return nullptr;

	stream.m_ReadPos = 0;
	vgui::BitmapTGA* pRet = new vgui::BitmapTGA(&stream, invertAlpha);

	return pRet;
}

vgui::BitmapTGA* vgui_LoadTGA(char const *pFilename)
{
	return vgui_LoadTGA(pFilename, true);
}

vgui::BitmapTGA* vgui_LoadTGANoInvertAlpha(char const *pFilename)
{
	return vgui_LoadTGA(pFilename, false);
}
