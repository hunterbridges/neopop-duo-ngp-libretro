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

#ifndef __BIOS__
#define __BIOS__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* NOTE: C-style interface */
typedef struct neopop_bios
{
	/* Holds BIOS program data */
	uint8_t bios[0x10000];

	/* Interrupt prio registers at 0x0070-0x007a don't have priority readable. */
	/* This should probably be stored in BIOS work RAM somewhere instead of a 
	 * separate array, but I don't know where! */
	uint8_t CacheIntPrio[0xB];
} neopop_bios_t;

void iBIOSHLE();

/* Fill the bios rom area with a bios. call once at program start */
int bios_install(neopop_bios_t *bios_ptr);

void BIOSHLE_Reset(neopop_bios_t *bios_ptr);
int BIOSHLE_StateAction(void *data, int load, int data_only);

#ifdef __cplusplus
}
#endif

#endif

