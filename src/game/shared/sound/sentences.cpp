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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "sentences.hpp"

void SENTENCEG_Init()
{
	if (fSentencesInit)
		return;

	memset(gszallsentencenames, 0, CVOXFILESENTENCEMAX * CBSENTENCENAME_MAX);
	gcallsentences = 0;

	memset(rgsentenceg, 0, CSENTENCEG_MAX * sizeof(SENTENCEG));

	auto [fileBuffer, size] = FileSystem_LoadFileIntoBuffer("sound/sentences.txt");

	byte* pMemFile = fileBuffer.get();

	if (!pMemFile)
		return;

	std::size_t filePos = 0;

	char buffer[512]{};
	char szgroup[64]{};
	int isentencegs = -1;

	// for each line in the file...
	while (memfgets(pMemFile, size, filePos, buffer, sizeof(buffer) - 1) != nullptr)
	{
		// skip whitespace
		std::size_t i = 0;
		while (buffer[i] && buffer[i] == ' ')
			i++;

		if (!buffer[i])
			continue;

		if (buffer[i] == '/' || !isalpha(buffer[i]))
			continue;

		// get sentence name
		std::size_t j = i;
		while (buffer[j] && buffer[j] != ' ')
			j++;

		if (!buffer[j])
			continue;

		if (gcallsentences > CVOXFILESENTENCEMAX)
		{
			ALERT(at_error, "Too many sentences in sentences.txt!\n");
			break;
		}

		// null-terminate name and save in sentences array
		buffer[j] = 0;
		const char* pString = buffer + i;

		if (strlen(pString) >= CBSENTENCENAME_MAX)
			ALERT(at_warning, "Sentence %s longer than %d letters\n", pString, CBSENTENCENAME_MAX - 1);

		safe_strcpy(gszallsentencenames[gcallsentences++], pString);

		j--;
		if (j <= i)
			continue;
		if (!isdigit(buffer[j]))
			continue;

		// cut out suffix numbers
		while (j > i && isdigit(buffer[j]))
			j--;

		if (j <= i)
			continue;

		buffer[j + 1] = 0;

		// if new name doesn't match previous group name, 
		// make a new group.

		if (strcmp(szgroup, &(buffer[i])))
		{
			// name doesn't match with prev name,
			// copy name into group, init count to 1
			isentencegs++;
			if (isentencegs >= CSENTENCEG_MAX)
			{
				ALERT(at_error, "Too many sentence groups in sentences.txt!\n");
				break;
			}

			safe_strcpy(rgsentenceg[isentencegs].szgroupname, &(buffer[i]));
			rgsentenceg[isentencegs].count = 1;

			safe_strcpy(szgroup, &(buffer[i]));

			continue;
		}
		else
		{
			//name matches with previous, increment group count
			if (isentencegs >= 0)
				rgsentenceg[isentencegs].count++;
		}
	}

	fileBuffer.reset();

	fSentencesInit = true;

	// init lru lists
	for (std::size_t i = 0; rgsentenceg[i].count && i < CSENTENCEG_MAX; ++i)
	{
		USENTENCEG_InitLRU(&(rgsentenceg[i].rgblru[0]), rgsentenceg[i].count);
	}
}

int SENTENCEG_Lookup(const char* sample, char* sentencenum, std::size_t sentencenumSize)
{
	// this is a sentence name; lookup sentence number
	// and give to engine as string.
	for (int i = 0; i < gcallsentences; i++)
	{
		if (!stricmp(gszallsentencenames[i], sample + 1))
		{
			if (sentencenum)
			{
				snprintf(sentencenum, sentencenumSize, "!%d", i);
			}
			return i;
		}
	}
	// sentence name not found!
	return -1;
}

int SENTENCEG_GetIndex(const char* szgroupname)
{
	if (!fSentencesInit || !szgroupname)
		return -1;

	// search rgsentenceg for match on szgroupname
	for (int i = 0; rgsentenceg[i].count; ++i)
	{
		if (!strcmp(szgroupname, rgsentenceg[i].szgroupname))
			return i;
	}

	return -1;
}

void USENTENCEG_InitLRU(unsigned char* plru, int count)
{
	if (!fSentencesInit)
		return;

	if (count > CSENTENCE_LRU_MAX)
		count = CSENTENCE_LRU_MAX;

	for (int i = 0; i < count; i++)
		plru[i] = (unsigned char)i;

	// randomize array
	for (int i = 0; i < (count * 4); i++)
	{
		const int j = RANDOM_LONG(0, count - 1);
		const int k = RANDOM_LONG(0, count - 1);
		std::swap(plru[j], plru[k]);
	}
}

int USENTENCEG_PickSequential(int isentenceg, char* szfound, std::size_t foundSize, int ipick, int freset)
{
	if (!fSentencesInit)
		return -1;

	if (isentenceg < 0)
		return -1;

	const unsigned char count = rgsentenceg[isentenceg].count;

	if (count == 0)
		return -1;

	if (ipick >= count)
		ipick = count - 1;

	snprintf(szfound, foundSize, "!%s%d", rgsentenceg[isentenceg].szgroupname, ipick);

	if (ipick >= count)
	{
		if (freset)
			// reset at end of list
			return 0;
		else
			return count;
	}

	return ipick + 1;
}

int USENTENCEG_Pick(int isentenceg, char* szfound, std::size_t foundSize)
{
	if (!fSentencesInit)
		return -1;

	if (isentenceg < 0)
		return -1;

	const unsigned char count = rgsentenceg[isentenceg].count;
	unsigned char* plru = rgsentenceg[isentenceg].rgblru;

	//Try twice; once with current cache and once with a cleared cache
	for (int j = 0; j < 2; ++j)
	{
		for (int i = 0; i < count; i++)
		{
			if (plru[i] != 0xFF)
			{
				const unsigned char ipick = plru[i];
				plru[i] = 0xFF;

				snprintf(szfound, foundSize, "!%s%d", rgsentenceg[isentenceg].szgroupname, ipick);

				return ipick;
			}
		}

		//Reinitialize cache and try again
		USENTENCEG_InitLRU(plru, count);
	}

	return -1;
}
