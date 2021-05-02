/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

DECLARE_MESSAGE(m_TextMessage, TextMsg);

bool CHudTextMessage::Init()
{
	HOOK_MESSAGE(TextMsg);

	gHUD.AddHudElem(this);

	Reset();

	return true;
};

char* CHudTextMessage::LocaliseTextString(const char* msg, char* dst_buffer, int buffer_size)
{
	char* dst = dst_buffer;
	for (const char* src = msg; *src != 0 && buffer_size > 0; buffer_size--)
	{
		if (*src == '#')
		{
			// cut msg name out of string
			static char word_buf[255];
			const char* word_start = src;
			char* wdst = word_buf;
			for (++src; (*src >= 'A' && *src <= 'z') || (*src >= '0' && *src <= '9'); wdst++, src++)
			{
				*wdst = *src;
			}
			*wdst = 0;

			// lookup msg name in titles.txt
			client_textmessage_t* clmsg = TextMessageGet(word_buf);
			if (!clmsg || !(clmsg->pMessage))
			{
				src = word_start;
				*dst = *src;
				dst++, src++;
				continue;
			}

			// copy string into message over the msg name
			for (const char* wsrc = clmsg->pMessage; *wsrc != 0; wsrc++, dst++)
			{
				*dst = *wsrc;
			}
			*dst = 0;
		}
		else
		{
			*dst = *src;
			dst++, src++;
			*dst = 0;
		}
	}

	dst_buffer[buffer_size - 1] = 0; // ensure null termination
	return dst_buffer;
}

char* CHudTextMessage::BufferedLocaliseTextString(const char* msg)
{
	static char dst_buffer[1024];
	LocaliseTextString(msg, dst_buffer, 1024);
	return dst_buffer;
}

const char* CHudTextMessage::LookupString(const char* msg, int* msg_dest)
{
	if (!msg)
		return "";

	// '#' character indicates this is a reference to a string in titles.txt, and not the string itself
	if (msg[0] == '#')
	{
		// this is a message name, so look up the real message
		client_textmessage_t* clmsg = TextMessageGet(msg + 1);

		if (!clmsg || !(clmsg->pMessage))
			return msg; // lookup failed, so return the original string

		if (msg_dest)
		{
			// check to see if titles.txt info overrides msg destination
			// if clmsg->effect is less than 0, then clmsg->effect holds -1 * message_destination
			if (clmsg->effect < 0)  // 
				*msg_dest = -clmsg->effect;
		}

		return clmsg->pMessage;
	}
	else
	{  // nothing special about this message, so just return the same string
		return msg;
	}
}

void StripEndNewlineFromString(char* str)
{
	int s = strlen(str) - 1;
	if (str[s] == '\n' || str[s] == '\r')
		str[s] = 0;
}

/**
*	@brief converts all '\r' characters to '\n', so that the engine can deal with the properly
*	@return a pointer to str
*/
char* ConvertCRtoNL(char* str)
{
	for (char* ch = str; *ch != 0; ch++)
		if (*ch == '\r')
			*ch = '\n';
	return str;
}

bool CHudTextMessage::MsgFunc_TextMsg(const char* pszName, int iSize, void* pbuf)
{
	BufferReader reader{pbuf, iSize};

	int msg_dest = reader.ReadByte();

	constexpr int MSG_BUF_SIZE = 128;
	static char szBuf[6][MSG_BUF_SIZE];
	const char* msg_text = LookupString(reader.ReadString(), &msg_dest);
	msg_text = safe_strcpy(szBuf[0], msg_text, MSG_BUF_SIZE);

	// keep reading strings and using C format strings for subsituting the strings into the localised text string
	const char* tempsstr1 = LookupString(reader.ReadString());
	char* sstr1 = safe_strcpy(szBuf[1], tempsstr1);
	StripEndNewlineFromString(sstr1);  // these strings are meant for subsitution into the main strings, so cull the automatic end newlines
	const char* tempsstr2 = LookupString(reader.ReadString());
	char* sstr2 = safe_strcpy(szBuf[2], tempsstr2);
	StripEndNewlineFromString(sstr2);
	const char* tempsstr3 = LookupString(reader.ReadString());
	char* sstr3 = safe_strcpy(szBuf[3], tempsstr3);
	StripEndNewlineFromString(sstr3);
	const char* tempsstr4 = LookupString(reader.ReadString());
	char* sstr4 = safe_strcpy(szBuf[4], tempsstr4);
	StripEndNewlineFromString(sstr4);
	char* psz = szBuf[5];

	if (gViewPort && gViewPort->AllowedToPrintText() == false)
		return true;

	switch (msg_dest)
	{
	case HUD_PRINTCENTER:
		snprintf(psz, MSG_BUF_SIZE, msg_text, sstr1, sstr2, sstr3, sstr4);
		CenterPrint(ConvertCRtoNL(psz));
		break;

	case HUD_PRINTNOTIFY:
		psz[0] = 1;  // mark this message to go into the notify buffer
		snprintf(psz + 1, MSG_BUF_SIZE, msg_text, sstr1, sstr2, sstr3, sstr4);
		ConsolePrint(ConvertCRtoNL(psz));
		break;

	case HUD_PRINTTALK:
		snprintf(psz, MSG_BUF_SIZE, msg_text, sstr1, sstr2, sstr3, sstr4);
		gHUD.m_SayText.SayTextPrint(ConvertCRtoNL(psz), 128);
		break;

	case HUD_PRINTCONSOLE:
		snprintf(psz, MSG_BUF_SIZE, msg_text, sstr1, sstr2, sstr3, sstr4);
		ConsolePrint(ConvertCRtoNL(psz));
		break;
	}

	return true;
}
