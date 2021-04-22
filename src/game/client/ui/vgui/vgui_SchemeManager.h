//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include <VGUI_Font.h>

// handle to an individual scheme
typedef int SchemeHandle_t;

// Register console variables, etc..
void Scheme_Init();

/**
*	@brief Handles the loading of text scheme description from disk
*	supports different font/color/size schemes at different resolutions 
*/
class CSchemeManager
{
public:
	// initialization
	/**
	*	@brief initializes the scheme manager
	*	loading the scheme files for the current resolution
	*	@param xRes width of the output window
	*	@param yRes height of the output window
	*/
	CSchemeManager(int xRes, int yRes);

	/**
	*	@brief frees all the memory used by the scheme manager
	*/
	virtual ~CSchemeManager();

	// scheme handling
	/**
	*	@brief Finds a scheme in the list, by name
	*	@param schemeName string name of the scheme
	*	@return SchemeHandle_t handle to the scheme
	*/
	SchemeHandle_t getSchemeHandle(const char* schemeName);

	// getting info from schemes
	/**
	*	@brief Returns the schemes pointer to a font
	*/
	vgui::Font* getFont(SchemeHandle_t schemeHandle);
	void getFgColor(SchemeHandle_t schemeHandle, int& r, int& g, int& b, int& a);
	void getBgColor(SchemeHandle_t schemeHandle, int& r, int& g, int& b, int& a);
	void getFgArmedColor(SchemeHandle_t schemeHandle, int& r, int& g, int& b, int& a);
	void getBgArmedColor(SchemeHandle_t schemeHandle, int& r, int& g, int& b, int& a);
	void getFgMousedownColor(SchemeHandle_t schemeHandle, int& r, int& g, int& b, int& a);
	void getBgMousedownColor(SchemeHandle_t schemeHandle, int& r, int& g, int& b, int& a);
	void getBorderColor(SchemeHandle_t schemeHandle, int& r, int& g, int& b, int& a);

private:
	class CScheme;
	CScheme* m_pSchemeList;
	int m_iNumSchemes;

	// Resolution we were initted at.
	int		m_xRes;

	/**
	*	@brief always returns a valid scheme handle
	*/
	CScheme* getSafeScheme(SchemeHandle_t schemeHandle);
};


