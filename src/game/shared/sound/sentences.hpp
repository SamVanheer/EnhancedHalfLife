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
*	@file
*
*	Sentences parsing and lookup
*/

#include <cstddef>

// sentence groups
constexpr int CBSENTENCENAME_MAX = 16;

/**
*	@brief max number of sentences in game. NOTE: this must match CVOXFILESENTENCEMAX in engine\sound.h!!!
*/
constexpr int CVOXFILESENTENCEMAX = 1536;

constexpr int CSENTENCE_LRU_MAX = 32;	//!< max number of elements per sentence group
constexpr int CSENTENCEG_MAX = 200;		//!< max number of sentence groups

/**
*	@brief group of related sentences
*/
struct SENTENCEG
{
	char szgroupname[CBSENTENCENAME_MAX];
	int count;
	unsigned char rgblru[CSENTENCE_LRU_MAX];
};

inline SENTENCEG rgsentenceg[CSENTENCEG_MAX]{};
inline bool fSentencesInit = false;
inline char gszallsentencenames[CVOXFILESENTENCEMAX][CBSENTENCENAME_MAX]{};
inline int gcallsentences = 0;

/**
*	@brief open sentences.txt, scan for groups, build rgsentenceg
*	Should be called from world spawn, only works on the first call and is ignored subsequently.
*/
void SENTENCEG_Init();

/**
*	@brief convert sentence (sample) name to !sentencenum
*	@return !sentencenum
*/
int SENTENCEG_Lookup(const char* sample, char* sentencenum, std::size_t sentencenumSize);

/**
*	@brief Given sentence group rootname (name without number suffix), get sentence group index (isentenceg).
*	@return -1 if no such name.
*/
int SENTENCEG_GetIndex(const char* szrootname);

/**
*	@brief pick a random sentence from rootname0 to rootnameX.
*	picks from the rgsentenceg[isentenceg] least recently used, modifies lru array. returns the sentencename.
*	@details note, lru must be seeded with 0-n randomized sentence numbers, with the rest of the lru filled with -1.
*	The first integer in the lru is actually the size of the list.
*	@return ipick, the ordinal of the picked sentence within the group.
*/
int USENTENCEG_Pick(int isentenceg, char* szfound, std::size_t foundSize);

/**
*	@brief ignore lru. pick next sentence from sentence group.
*	Go in order until we hit the last sentence, then repeat list if freset is true.
*	@param freset if false, then repeat last sentence.
*	@param ipick requested sentence ordinal.
*	@return ipick 'next' is returned. -1 indicates an error.
*/
int USENTENCEG_PickSequential(int isentenceg, char* szfound, std::size_t foundSize, int ipick, int freset);

/**
*	@brief randomize list of sentence name indices
*/
void USENTENCEG_InitLRU(unsigned char* plru, int count);
