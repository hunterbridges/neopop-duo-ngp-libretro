//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

#ifndef __NEOPOP__
#define __NEOPOP__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "TLCS-900h/TLCS900h.h"

#include "system.h"
#include "rom.h"

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

// TODO Can we get rid of this header?

typedef enum
{
	COLOURMODE_GREYSCALE,
	COLOURMODE_COLOUR,
	COLOURMODE_AUTO
} COLOURMODE;

/* Physical screen dimensions */
#define SCREEN_WIDTH    160
#define SCREEN_HEIGHT   152

#endif
