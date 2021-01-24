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

#ifndef __Z80_CONTROL__
#define __Z80_CONTROL__

#include <stdint.h>

#include "../hw_cpu/z80-fuse/z80.h"

struct neopop_z80i_t
{
	uint8_t CommByte;
	bool Z80Enabled;

	uint8_t Z80_ReadComm(void);
	void Z80_WriteComm(uint8_t data);

	void Z80_reset(void);	// z80 reset

	void Z80_irq(void);		// Cause an interrupt
	void Z80_nmi(void);		// Cause an NMI
	void Z80_SetEnable(bool set);
	bool Z80_IsEnabled(void);
	int Z80_RunOP(void);

	int StateAction(void *data, int load, int data_only);
	int z80_state_action(void *data, int load, int data_only, const char *section_name);
};

#endif
