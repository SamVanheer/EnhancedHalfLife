/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#pragma once

/**
*	@brief determine if a uchar32 represents a valid Unicode code point
*/
bool Q_IsValidUChar32(char32_t uVal);

/**
*	@brief // Decode one character from a UTF-8 encoded string.
*
*	Treats 6-byte CESU-8 sequences as a single character, as if they were a correctly-encoded 4-byte UTF-8 sequence.
*/
int Q_UTF8ToUChar32(const char* pUTF8_, char32_t& uValueOut, bool& bErrorOut);

/**
*	@brief Returns true if UTF-8 string contains invalid sequences.
*/
bool Q_UnicodeValidate(const char* pUTF8);

inline char com_token[1500];

/**
*	@brief Parse a token out of a string
*/
char* COM_Parse(char* data);

/**
*	@brief Returns 1 if additional data is waiting to be processed on this line
*/
bool COM_TokenWaiting(char* buffer);
