//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include "progdefs.h"

constexpr int MAX_ENT_LEAFS = 48;

struct edict_t
{
	qboolean	free = false;
	int			serialnumber = 0;
	link_t		area;				// linked to a division node or leaf

	int			headnode = 0;			// -1 to use normal leaf check
	int			num_leafs = 0;
	short		leafnums[MAX_ENT_LEAFS]{};

	float		freetime = 0;			// sv.time when the object was freed

	void* pvPrivateData = nullptr;		// Alloced and freed by engine, used by DLLs

	entvars_t	v;					// C exported fields from progs

	// other fields from progs come immediately after
};
