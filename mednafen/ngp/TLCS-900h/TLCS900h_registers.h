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

/*
//---------------------------------------------------------------------------
//=========================================================================

	TLCS900h_registers.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 AUG 2002 - neopop_uk
=======================================
- Fixed potential precidence problems in regX and rCodeX by using ()'s

//---------------------------------------------------------------------------
*/

#ifndef __TLCS900H_REGISTERS__
#define __TLCS900H_REGISTERS__
//=============================================================================

#include "../../mednafen-types.h"
#include "TLCS900h_state.h"

#ifdef __cplusplus
extern "C" {
#endif

void reset_registers(void);
void dump_registers_TLCS900h(void);

//The value read by bad rCodes, leave 0, improves "Gals Fighters"
#define RERR_VALUE		0

//=============================================================================

//GPR Access
#define regB(x)	(*(cur_tlcs900h->gprMapB[cur_tlcs900h->statusRFP][(x)]))
#define regW(x)	(*(cur_tlcs900h->gprMapW[cur_tlcs900h->statusRFP][(x)]))
#define regL(x)	(*(cur_tlcs900h->gprMapL[cur_tlcs900h->statusRFP][(x)]))

void initGPRTables(struct tlcs900h_state *tlcs);

//Reg.Code Access
#define rCodeB(r)	(*(cur_tlcs900h->regCodeMapB[cur_tlcs900h->statusRFP][(r)]))
#define rCodeW(r)	(*(cur_tlcs900h->regCodeMapW[cur_tlcs900h->statusRFP][(r) >> 1]))
#define rCodeL(r)	(*(cur_tlcs900h->regCodeMapL[cur_tlcs900h->statusRFP][(r) >> 2]))

void initRegCodeTables(struct tlcs900h_state *tlcs);

//Common Registers
#define REGA		(regB(1))
#define REGWA		(regW(0))
#define REGBC		(regW(1))
#define REGXSP		(cur_tlcs900h->gpr[3])

//=============================================================================

uint8 statusIFF(void);
void setStatusIFF(uint8 iff);

void setStatusRFP(uint8 rfp);
void changedSP(void);

#define FLAG_S ((cur_tlcs900h->sr & 0x0080) >> 7)
#define FLAG_Z ((cur_tlcs900h->sr & 0x0040) >> 6)
#define FLAG_H ((cur_tlcs900h->sr & 0x0010) >> 4)
#define FLAG_V ((cur_tlcs900h->sr & 0x0004) >> 2)
#define FLAG_N ((cur_tlcs900h->sr & 0x0002) >> 1)
#define FLAG_C (cur_tlcs900h->sr & 1)

#define SETFLAG_S(s) { uint16 sr1 = cur_tlcs900h->sr & 0xFF7F; if (s) sr1 |= 0x0080; cur_tlcs900h->sr = sr1; }
#define SETFLAG_Z(z) { uint16 sr1 = cur_tlcs900h->sr & 0xFFBF; if (z) sr1 |= 0x0040; cur_tlcs900h->sr = sr1; }
#define SETFLAG_H(h) { uint16 sr1 = cur_tlcs900h->sr & 0xFFEF; if (h) sr1 |= 0x0010; cur_tlcs900h->sr = sr1; }
#define SETFLAG_V(v) { uint16 sr1 = cur_tlcs900h->sr & 0xFFFB; if (v) sr1 |= 0x0004; cur_tlcs900h->sr = sr1; }
#define SETFLAG_N(n) { uint16 sr1 = cur_tlcs900h->sr & 0xFFFD; if (n) sr1 |= 0x0002; cur_tlcs900h->sr = sr1; }
#define SETFLAG_C(c) { uint16 sr1 = cur_tlcs900h->sr & 0xFFFE; if (c) sr1 |= 0x0001; cur_tlcs900h->sr = sr1; }

#define SETFLAG_S0		{ cur_tlcs900h->sr &= 0xFF7F;	}
#define SETFLAG_Z0		{ cur_tlcs900h->sr &= 0xFFBF;	}
#define SETFLAG_H0		{ cur_tlcs900h->sr &= 0xFFEF;	}
#define SETFLAG_V0		{ cur_tlcs900h->sr &= 0xFFFB;	}
#define SETFLAG_N0		{ cur_tlcs900h->sr &= 0xFFFD;	}
#define SETFLAG_C0		{ cur_tlcs900h->sr &= 0xFFFE;	}

#define SETFLAG_S1		{ cur_tlcs900h->sr |= 0x0080; }
#define SETFLAG_Z1		{ cur_tlcs900h->sr |= 0x0040; }
#define SETFLAG_H1		{ cur_tlcs900h->sr |= 0x0010; }
#define SETFLAG_V1		{ cur_tlcs900h->sr |= 0x0004; }
#define SETFLAG_N1		{ cur_tlcs900h->sr |= 0x0002; }
#define SETFLAG_C1		{ cur_tlcs900h->sr |= 0x0001; }

#ifdef __cplusplus
}
#endif

//=============================================================================
#endif
