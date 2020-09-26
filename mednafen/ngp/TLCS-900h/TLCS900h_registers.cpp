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

    TLCS900h_registers.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

10 AUG 2002 - neopop_uk
=======================================
- Moved default PC setup to the 'reset_registers' function.

//---------------------------------------------------------------------------
*/

#include <string.h>
#include "TLCS900h.h"
#include "../rom.h"
#include "../interrupt.h"

#ifdef MSB_FIRST
#define BYTE0	3
#define BYTE1	2
#define BYTE2	1
#define BYTE3	0
#define WORD0	2
#define WORD1	0
#else
#define BYTE0	0
#define BYTE1	1
#define BYTE2	2
#define BYTE3	3
#define WORD0	0
#define WORD1	2
#endif

static TLCS900h dummy;

//=============================================================================

//Bank Data
static void initGPRMapB(TLCS900h *tlcs)
{
    static uint8* gprMapB[4][8] =
    {
        //Bank 0
        {
           (uint8*)&(dummy.gprBank[0][0]) + BYTE1,
           (uint8*)&(dummy.gprBank[0][0]) + BYTE0,
           (uint8*)&(dummy.gprBank[0][1]) + BYTE1,
           (uint8*)&(dummy.gprBank[0][1]) + BYTE0,
           (uint8*)&(dummy.gprBank[0][2]) + BYTE1,
           (uint8*)&(dummy.gprBank[0][2]) + BYTE0,
           (uint8*)&(dummy.gprBank[0][3]) + BYTE1,
           (uint8*)&(dummy.gprBank[0][3]) + BYTE0,
        },

        //Bank 1
        {
           (uint8*)&(dummy.gprBank[1][0]) + BYTE1,
           (uint8*)&(dummy.gprBank[1][0]) + BYTE0,
           (uint8*)&(dummy.gprBank[1][1]) + BYTE1,
           (uint8*)&(dummy.gprBank[1][1]) + BYTE0,
           (uint8*)&(dummy.gprBank[1][2]) + BYTE1,
           (uint8*)&(dummy.gprBank[1][2]) + BYTE0,
           (uint8*)&(dummy.gprBank[1][3]) + BYTE1,
           (uint8*)&(dummy.gprBank[1][3]) + BYTE0,
        },

        //Bank 2
        {
           (uint8*)&(dummy.gprBank[2][0]) + BYTE1,
           (uint8*)&(dummy.gprBank[2][0]) + BYTE0,
           (uint8*)&(dummy.gprBank[2][1]) + BYTE1,
           (uint8*)&(dummy.gprBank[2][1]) + BYTE0,
           (uint8*)&(dummy.gprBank[2][2]) + BYTE1,
           (uint8*)&(dummy.gprBank[2][2]) + BYTE0,
           (uint8*)&(dummy.gprBank[2][3]) + BYTE1,
           (uint8*)&(dummy.gprBank[2][3]) + BYTE0,
        },

        //Bank 3
        {
           (uint8*)&(dummy.gprBank[3][0]) + BYTE1,
           (uint8*)&(dummy.gprBank[3][0]) + BYTE0,
           (uint8*)&(dummy.gprBank[3][1]) + BYTE1,
           (uint8*)&(dummy.gprBank[3][1]) + BYTE0,
           (uint8*)&(dummy.gprBank[3][2]) + BYTE1,
           (uint8*)&(dummy.gprBank[3][2]) + BYTE0,
           (uint8*)&(dummy.gprBank[3][3]) + BYTE1,
           (uint8*)&(dummy.gprBank[3][3]) + BYTE0,
        }
    };

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            tlcs->gprMapB[i][j] = ((uint8*)(gprMapB[i][j]) - (uint8*)(&dummy) + (uint8*)tlcs);
        }
    }
}

static void initGPRMapW(TLCS900h *tlcs)
{
    static uint16* gprMapW[4][8] =
    {
        //Bank 0
        {
           (uint16*)(((uint8*)&dummy.gprBank[0][0]) + WORD0),
           (uint16*)(((uint8*)&dummy.gprBank[0][1]) + WORD0),
           (uint16*)(((uint8*)&dummy.gprBank[0][2]) + WORD0),
           (uint16*)(((uint8*)&dummy.gprBank[0][3]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[0]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[1]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[2]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[3]) + WORD0),
        },

        //Bank 1
        {
           (uint16*)(((uint8*)&dummy.gprBank[1][0]) + WORD0),
           (uint16*)(((uint8*)&dummy.gprBank[1][1]) + WORD0),
           (uint16*)(((uint8*)&dummy.gprBank[1][2]) + WORD0),
           (uint16*)(((uint8*)&dummy.gprBank[1][3]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[0]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[1]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[2]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[3]) + WORD0),
        },

        //Bank 2
        {
           (uint16*)(((uint8*)&dummy.gprBank[2][0]) + WORD0),
           (uint16*)(((uint8*)&dummy.gprBank[2][1]) + WORD0),
           (uint16*)(((uint8*)&dummy.gprBank[2][2]) + WORD0),
           (uint16*)(((uint8*)&dummy.gprBank[2][3]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[0]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[1]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[2]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[3]) + WORD0),
        },

        //Bank 3
        {
           (uint16*)(((uint8*)&dummy.gprBank[3][0]) + WORD0),
           (uint16*)(((uint8*)&dummy.gprBank[3][1]) + WORD0),
           (uint16*)(((uint8*)&dummy.gprBank[3][2]) + WORD0),
           (uint16*)(((uint8*)&dummy.gprBank[3][3]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[0]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[1]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[2]) + WORD0),
           (uint16*)(((uint8*)&dummy.gpr[3]) + WORD0),
        },
    };

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            tlcs->gprMapW[i][j] = (uint16*)((uint8*)(gprMapW[i][j]) - (uint8*)(&dummy) + (uint8*)tlcs);
        }
    }
}

static void initGPRMapL(TLCS900h *tlcs)
{
    static uint32* gprMapL[4][8] =
    {
        //Bank 0
        {
           (uint32*)&(dummy.gprBank[0][0]),
           (uint32*)&(dummy.gprBank[0][1]),
           (uint32*)&(dummy.gprBank[0][2]),
           (uint32*)&(dummy.gprBank[0][3]),
           (uint32*)&(dummy.gpr[0]),
           (uint32*)&(dummy.gpr[1]),
           (uint32*)&(dummy.gpr[2]),
           (uint32*)&(dummy.gpr[3]),
        },

        //Bank 1
        {
           (uint32*)&(dummy.gprBank[1][0]),
           (uint32*)&(dummy.gprBank[1][1]),
           (uint32*)&(dummy.gprBank[1][2]),
           (uint32*)&(dummy.gprBank[1][3]),
           (uint32*)&(dummy.gpr[0]),
           (uint32*)&(dummy.gpr[1]),
           (uint32*)&(dummy.gpr[2]),
           (uint32*)&(dummy.gpr[3]),
        },

        //Bank 2
        {
           (uint32*)&(dummy.gprBank[2][0]),
           (uint32*)&(dummy.gprBank[2][1]),
           (uint32*)&(dummy.gprBank[2][2]),
           (uint32*)&(dummy.gprBank[2][3]),
           (uint32*)&(dummy.gpr[0]),
           (uint32*)&(dummy.gpr[1]),
           (uint32*)&(dummy.gpr[2]),
           (uint32*)&(dummy.gpr[3]),
        },

        //Bank 3
        {
           (uint32*)&(dummy.gprBank[3][0]),
           (uint32*)&(dummy.gprBank[3][1]),
           (uint32*)&(dummy.gprBank[3][2]),
           (uint32*)&(dummy.gprBank[3][3]),
           (uint32*)&(dummy.gpr[0]),
           (uint32*)&(dummy.gpr[1]),
           (uint32*)&(dummy.gpr[2]),
           (uint32*)&(dummy.gpr[3]),
        },
    };

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            tlcs->gprMapL[i][j] = (uint32*)((uint8*)(gprMapL[i][j]) - (uint8*)(&dummy) + (uint8*)tlcs);
        }
    }
}

void TLCS900h::initGPRTables()
{
    if (gpr_init_flag == 1)
        return;

    initGPRMapB(this);
    initGPRMapW(this);
    initGPRMapL(this);

    gpr_init_flag = 1;
}

//=============================================================================

static void initRegCodeMapB(TLCS900h *tlcs)
{
    static uint8* regCodeMapB[4][256] =
    {
       {

          ((uint8*)&dummy.gprBank[0][0]) + BYTE0,((uint8*)&dummy.gprBank[0][0]) + BYTE1,			//BANK 0
          ((uint8*)&dummy.gprBank[0][0]) + BYTE2, ((uint8*)&dummy.gprBank[0][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][1]) + BYTE0,((uint8*)&dummy.gprBank[0][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][1]) + BYTE2, ((uint8*)&dummy.gprBank[0][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][2]) + BYTE0,((uint8*)&dummy.gprBank[0][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][2]) + BYTE2, ((uint8*)&dummy.gprBank[0][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][3]) + BYTE0,((uint8*)&dummy.gprBank[0][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][3]) + BYTE2, ((uint8*)&dummy.gprBank[0][3]) + BYTE3,

          ((uint8*)&dummy.gprBank[1][0]) + BYTE0,((uint8*)&dummy.gprBank[1][0]) + BYTE1,			//BANK 1
          ((uint8*)&dummy.gprBank[1][0]) + BYTE2, ((uint8*)&dummy.gprBank[1][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][1]) + BYTE0,((uint8*)&dummy.gprBank[1][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][1]) + BYTE2, ((uint8*)&dummy.gprBank[1][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][2]) + BYTE0,((uint8*)&dummy.gprBank[1][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][2]) + BYTE2, ((uint8*)&dummy.gprBank[1][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][3]) + BYTE0,((uint8*)&dummy.gprBank[1][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][3]) + BYTE2, ((uint8*)&dummy.gprBank[1][3]) + BYTE3,

          ((uint8*)&dummy.gprBank[2][0]) + BYTE0,((uint8*)&dummy.gprBank[2][0]) + BYTE1,			//BANK 2
          ((uint8*)&dummy.gprBank[2][0]) + BYTE2, ((uint8*)&dummy.gprBank[2][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][1]) + BYTE0,((uint8*)&dummy.gprBank[2][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][1]) + BYTE2, ((uint8*)&dummy.gprBank[2][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][2]) + BYTE0,((uint8*)&dummy.gprBank[2][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][2]) + BYTE2, ((uint8*)&dummy.gprBank[2][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][3]) + BYTE0,((uint8*)&dummy.gprBank[2][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][3]) + BYTE2, ((uint8*)&dummy.gprBank[2][3]) + BYTE3,

          ((uint8*)&dummy.gprBank[3][0]) + BYTE0,((uint8*)&dummy.gprBank[3][0]) + BYTE1,			//BANK 3
          ((uint8*)&dummy.gprBank[3][0]) + BYTE2, ((uint8*)&dummy.gprBank[3][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][1]) + BYTE0,((uint8*)&dummy.gprBank[3][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][1]) + BYTE2, ((uint8*)&dummy.gprBank[3][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][2]) + BYTE0,((uint8*)&dummy.gprBank[3][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][2]) + BYTE2, ((uint8*)&dummy.gprBank[3][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][3]) + BYTE0,((uint8*)&dummy.gprBank[3][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][3]) + BYTE2, ((uint8*)&dummy.gprBank[3][3]) + BYTE3,

          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,

          //Previous Bank
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,

          //Current Bank
          ((uint8*)&dummy.gprBank[0][0]) + BYTE0,((uint8*)&dummy.gprBank[0][0]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][0]) + BYTE2, ((uint8*)&dummy.gprBank[0][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][1]) + BYTE0,((uint8*)&dummy.gprBank[0][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][1]) + BYTE2, ((uint8*)&dummy.gprBank[0][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][2]) + BYTE0,((uint8*)&dummy.gprBank[0][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][2]) + BYTE2, ((uint8*)&dummy.gprBank[0][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][3]) + BYTE0,((uint8*)&dummy.gprBank[0][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][3]) + BYTE2, ((uint8*)&dummy.gprBank[0][3]) + BYTE3,

          ((uint8*)&dummy.gpr[0]) + BYTE0, ((uint8*)&dummy.gpr[0]) + BYTE1,
          ((uint8*)&dummy.gpr[0]) + BYTE2, ((uint8*)&dummy.gpr[0]) + BYTE3,
          ((uint8*)&dummy.gpr[1]) + BYTE0, ((uint8*)&dummy.gpr[1]) + BYTE1,
          ((uint8*)&dummy.gpr[1]) + BYTE2, ((uint8*)&dummy.gpr[1]) + BYTE3,
          ((uint8*)&dummy.gpr[2]) + BYTE0, ((uint8*)&dummy.gpr[2]) + BYTE1,
          ((uint8*)&dummy.gpr[2]) + BYTE2, ((uint8*)&dummy.gpr[2]) + BYTE3,
          ((uint8*)&dummy.gpr[3]) + BYTE0, ((uint8*)&dummy.gpr[3]) + BYTE1,
          ((uint8*)&dummy.gpr[3]) + BYTE2, ((uint8*)&dummy.gpr[3]) + BYTE3
       },

       {

          ((uint8*)&dummy.gprBank[0][0]) + BYTE0,((uint8*)&dummy.gprBank[0][0]) + BYTE1,			//BANK 0
          ((uint8*)&dummy.gprBank[0][0]) + BYTE2, ((uint8*)&dummy.gprBank[0][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][1]) + BYTE0,((uint8*)&dummy.gprBank[0][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][1]) + BYTE2, ((uint8*)&dummy.gprBank[0][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][2]) + BYTE0,((uint8*)&dummy.gprBank[0][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][2]) + BYTE2, ((uint8*)&dummy.gprBank[0][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][3]) + BYTE0,((uint8*)&dummy.gprBank[0][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][3]) + BYTE2, ((uint8*)&dummy.gprBank[0][3]) + BYTE3,

          ((uint8*)&dummy.gprBank[1][0]) + BYTE0,((uint8*)&dummy.gprBank[1][0]) + BYTE1,			//BANK 1
          ((uint8*)&dummy.gprBank[1][0]) + BYTE2, ((uint8*)&dummy.gprBank[1][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][1]) + BYTE0,((uint8*)&dummy.gprBank[1][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][1]) + BYTE2, ((uint8*)&dummy.gprBank[1][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][2]) + BYTE0,((uint8*)&dummy.gprBank[1][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][2]) + BYTE2, ((uint8*)&dummy.gprBank[1][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][3]) + BYTE0,((uint8*)&dummy.gprBank[1][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][3]) + BYTE2, ((uint8*)&dummy.gprBank[1][3]) + BYTE3,

          ((uint8*)&dummy.gprBank[2][0]) + BYTE0,((uint8*)&dummy.gprBank[2][0]) + BYTE1,			//BANK 2
          ((uint8*)&dummy.gprBank[2][0]) + BYTE2, ((uint8*)&dummy.gprBank[2][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][1]) + BYTE0,((uint8*)&dummy.gprBank[2][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][1]) + BYTE2, ((uint8*)&dummy.gprBank[2][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][2]) + BYTE0,((uint8*)&dummy.gprBank[2][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][2]) + BYTE2, ((uint8*)&dummy.gprBank[2][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][3]) + BYTE0,((uint8*)&dummy.gprBank[2][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][3]) + BYTE2, ((uint8*)&dummy.gprBank[2][3]) + BYTE3,

          ((uint8*)&dummy.gprBank[3][0]) + BYTE0,((uint8*)&dummy.gprBank[3][0]) + BYTE1,			//BANK 3
          ((uint8*)&dummy.gprBank[3][0]) + BYTE2, ((uint8*)&dummy.gprBank[3][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][1]) + BYTE0,((uint8*)&dummy.gprBank[3][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][1]) + BYTE2, ((uint8*)&dummy.gprBank[3][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][2]) + BYTE0,((uint8*)&dummy.gprBank[3][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][2]) + BYTE2, ((uint8*)&dummy.gprBank[3][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][3]) + BYTE0,((uint8*)&dummy.gprBank[3][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][3]) + BYTE2, ((uint8*)&dummy.gprBank[3][3]) + BYTE3,

          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,

          //Previous Bank
          ((uint8*)&dummy.gprBank[0][0]) + BYTE0,((uint8*)&dummy.gprBank[0][0]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][0]) + BYTE2, ((uint8*)&dummy.gprBank[0][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][1]) + BYTE0,((uint8*)&dummy.gprBank[0][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][1]) + BYTE2, ((uint8*)&dummy.gprBank[0][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][2]) + BYTE0,((uint8*)&dummy.gprBank[0][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][2]) + BYTE2, ((uint8*)&dummy.gprBank[0][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][3]) + BYTE0,((uint8*)&dummy.gprBank[0][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][3]) + BYTE2, ((uint8*)&dummy.gprBank[0][3]) + BYTE3,

          //Current Bank
          ((uint8*)&dummy.gprBank[1][0]) + BYTE0,((uint8*)&dummy.gprBank[1][0]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][0]) + BYTE2, ((uint8*)&dummy.gprBank[1][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][1]) + BYTE0,((uint8*)&dummy.gprBank[1][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][1]) + BYTE2, ((uint8*)&dummy.gprBank[1][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][2]) + BYTE0,((uint8*)&dummy.gprBank[1][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][2]) + BYTE2, ((uint8*)&dummy.gprBank[1][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][3]) + BYTE0,((uint8*)&dummy.gprBank[1][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][3]) + BYTE2, ((uint8*)&dummy.gprBank[1][3]) + BYTE3,

          ((uint8*)&dummy.gpr[0]) + BYTE0, ((uint8*)&dummy.gpr[0]) + BYTE1,
          ((uint8*)&dummy.gpr[0]) + BYTE2, ((uint8*)&dummy.gpr[0]) + BYTE3,
          ((uint8*)&dummy.gpr[1]) + BYTE0, ((uint8*)&dummy.gpr[1]) + BYTE1,
          ((uint8*)&dummy.gpr[1]) + BYTE2, ((uint8*)&dummy.gpr[1]) + BYTE3,
          ((uint8*)&dummy.gpr[2]) + BYTE0, ((uint8*)&dummy.gpr[2]) + BYTE1,
          ((uint8*)&dummy.gpr[2]) + BYTE2, ((uint8*)&dummy.gpr[2]) + BYTE3,
          ((uint8*)&dummy.gpr[3]) + BYTE0, ((uint8*)&dummy.gpr[3]) + BYTE1,
          ((uint8*)&dummy.gpr[3]) + BYTE2, ((uint8*)&dummy.gpr[3]) + BYTE3
       },

       {

          ((uint8*)&dummy.gprBank[0][0]) + BYTE0,((uint8*)&dummy.gprBank[0][0]) + BYTE1,			//BANK 0
          ((uint8*)&dummy.gprBank[0][0]) + BYTE2, ((uint8*)&dummy.gprBank[0][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][1]) + BYTE0,((uint8*)&dummy.gprBank[0][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][1]) + BYTE2, ((uint8*)&dummy.gprBank[0][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][2]) + BYTE0,((uint8*)&dummy.gprBank[0][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][2]) + BYTE2, ((uint8*)&dummy.gprBank[0][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][3]) + BYTE0,((uint8*)&dummy.gprBank[0][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][3]) + BYTE2, ((uint8*)&dummy.gprBank[0][3]) + BYTE3,

          ((uint8*)&dummy.gprBank[1][0]) + BYTE0,((uint8*)&dummy.gprBank[1][0]) + BYTE1,			//BANK 1
          ((uint8*)&dummy.gprBank[1][0]) + BYTE2, ((uint8*)&dummy.gprBank[1][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][1]) + BYTE0,((uint8*)&dummy.gprBank[1][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][1]) + BYTE2, ((uint8*)&dummy.gprBank[1][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][2]) + BYTE0,((uint8*)&dummy.gprBank[1][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][2]) + BYTE2, ((uint8*)&dummy.gprBank[1][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][3]) + BYTE0,((uint8*)&dummy.gprBank[1][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][3]) + BYTE2, ((uint8*)&dummy.gprBank[1][3]) + BYTE3,

          ((uint8*)&dummy.gprBank[2][0]) + BYTE0,((uint8*)&dummy.gprBank[2][0]) + BYTE1,			//BANK 2
          ((uint8*)&dummy.gprBank[2][0]) + BYTE2, ((uint8*)&dummy.gprBank[2][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][1]) + BYTE0,((uint8*)&dummy.gprBank[2][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][1]) + BYTE2, ((uint8*)&dummy.gprBank[2][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][2]) + BYTE0,((uint8*)&dummy.gprBank[2][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][2]) + BYTE2, ((uint8*)&dummy.gprBank[2][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][3]) + BYTE0,((uint8*)&dummy.gprBank[2][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][3]) + BYTE2, ((uint8*)&dummy.gprBank[2][3]) + BYTE3,

          ((uint8*)&dummy.gprBank[3][0]) + BYTE0,((uint8*)&dummy.gprBank[3][0]) + BYTE1,			//BANK 3
          ((uint8*)&dummy.gprBank[3][0]) + BYTE2, ((uint8*)&dummy.gprBank[3][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][1]) + BYTE0,((uint8*)&dummy.gprBank[3][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][1]) + BYTE2, ((uint8*)&dummy.gprBank[3][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][2]) + BYTE0,((uint8*)&dummy.gprBank[3][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][2]) + BYTE2, ((uint8*)&dummy.gprBank[3][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][3]) + BYTE0,((uint8*)&dummy.gprBank[3][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][3]) + BYTE2, ((uint8*)&dummy.gprBank[3][3]) + BYTE3,

          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,

          //Previous Bank
          ((uint8*)&dummy.gprBank[1][0]) + BYTE0,((uint8*)&dummy.gprBank[1][0]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][0]) + BYTE2, ((uint8*)&dummy.gprBank[1][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][1]) + BYTE0,((uint8*)&dummy.gprBank[1][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][1]) + BYTE2, ((uint8*)&dummy.gprBank[1][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][2]) + BYTE0,((uint8*)&dummy.gprBank[1][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][2]) + BYTE2, ((uint8*)&dummy.gprBank[1][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][3]) + BYTE0,((uint8*)&dummy.gprBank[1][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][3]) + BYTE2, ((uint8*)&dummy.gprBank[1][3]) + BYTE3,

          //Current Bank
          ((uint8*)&dummy.gprBank[2][0]) + BYTE0,((uint8*)&dummy.gprBank[2][0]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][0]) + BYTE2, ((uint8*)&dummy.gprBank[2][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][1]) + BYTE0,((uint8*)&dummy.gprBank[2][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][1]) + BYTE2, ((uint8*)&dummy.gprBank[2][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][2]) + BYTE0,((uint8*)&dummy.gprBank[2][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][2]) + BYTE2, ((uint8*)&dummy.gprBank[2][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][3]) + BYTE0,((uint8*)&dummy.gprBank[2][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][3]) + BYTE2, ((uint8*)&dummy.gprBank[2][3]) + BYTE3,

          ((uint8*)&dummy.gpr[0]) + BYTE0, ((uint8*)&dummy.gpr[0]) + BYTE1,
          ((uint8*)&dummy.gpr[0]) + BYTE2, ((uint8*)&dummy.gpr[0]) + BYTE3,
          ((uint8*)&dummy.gpr[1]) + BYTE0, ((uint8*)&dummy.gpr[1]) + BYTE1,
          ((uint8*)&dummy.gpr[1]) + BYTE2, ((uint8*)&dummy.gpr[1]) + BYTE3,
          ((uint8*)&dummy.gpr[2]) + BYTE0, ((uint8*)&dummy.gpr[2]) + BYTE1,
          ((uint8*)&dummy.gpr[2]) + BYTE2, ((uint8*)&dummy.gpr[2]) + BYTE3,
          ((uint8*)&dummy.gpr[3]) + BYTE0, ((uint8*)&dummy.gpr[3]) + BYTE1,
          ((uint8*)&dummy.gpr[3]) + BYTE2, ((uint8*)&dummy.gpr[3]) + BYTE3
       },

       {

          ((uint8*)&dummy.gprBank[0][0]) + BYTE0,((uint8*)&dummy.gprBank[0][0]) + BYTE1,			//BANK 0
          ((uint8*)&dummy.gprBank[0][0]) + BYTE2, ((uint8*)&dummy.gprBank[0][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][1]) + BYTE0,((uint8*)&dummy.gprBank[0][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][1]) + BYTE2, ((uint8*)&dummy.gprBank[0][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][2]) + BYTE0,((uint8*)&dummy.gprBank[0][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][2]) + BYTE2, ((uint8*)&dummy.gprBank[0][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[0][3]) + BYTE0,((uint8*)&dummy.gprBank[0][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[0][3]) + BYTE2, ((uint8*)&dummy.gprBank[0][3]) + BYTE3,

          ((uint8*)&dummy.gprBank[1][0]) + BYTE0,((uint8*)&dummy.gprBank[1][0]) + BYTE1,			//BANK 1
          ((uint8*)&dummy.gprBank[1][0]) + BYTE2, ((uint8*)&dummy.gprBank[1][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][1]) + BYTE0,((uint8*)&dummy.gprBank[1][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][1]) + BYTE2, ((uint8*)&dummy.gprBank[1][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][2]) + BYTE0,((uint8*)&dummy.gprBank[1][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][2]) + BYTE2, ((uint8*)&dummy.gprBank[1][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[1][3]) + BYTE0,((uint8*)&dummy.gprBank[1][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[1][3]) + BYTE2, ((uint8*)&dummy.gprBank[1][3]) + BYTE3,

          ((uint8*)&dummy.gprBank[2][0]) + BYTE0,((uint8*)&dummy.gprBank[2][0]) + BYTE1,			//BANK 2
          ((uint8*)&dummy.gprBank[2][0]) + BYTE2, ((uint8*)&dummy.gprBank[2][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][1]) + BYTE0,((uint8*)&dummy.gprBank[2][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][1]) + BYTE2, ((uint8*)&dummy.gprBank[2][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][2]) + BYTE0,((uint8*)&dummy.gprBank[2][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][2]) + BYTE2, ((uint8*)&dummy.gprBank[2][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][3]) + BYTE0,((uint8*)&dummy.gprBank[2][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][3]) + BYTE2, ((uint8*)&dummy.gprBank[2][3]) + BYTE3,

          ((uint8*)&dummy.gprBank[3][0]) + BYTE0,((uint8*)&dummy.gprBank[3][0]) + BYTE1,			//BANK 3
          ((uint8*)&dummy.gprBank[3][0]) + BYTE2, ((uint8*)&dummy.gprBank[3][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][1]) + BYTE0,((uint8*)&dummy.gprBank[3][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][1]) + BYTE2, ((uint8*)&dummy.gprBank[3][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][2]) + BYTE0,((uint8*)&dummy.gprBank[3][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][2]) + BYTE2, ((uint8*)&dummy.gprBank[3][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][3]) + BYTE0,((uint8*)&dummy.gprBank[3][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][3]) + BYTE2, ((uint8*)&dummy.gprBank[3][3]) + BYTE3,

          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,
          (uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,(uint8*)&dummy.rErr,

          //Previous Bank
          ((uint8*)&dummy.gprBank[2][0]) + BYTE0,((uint8*)&dummy.gprBank[2][0]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][0]) + BYTE2, ((uint8*)&dummy.gprBank[2][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][1]) + BYTE0,((uint8*)&dummy.gprBank[2][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][1]) + BYTE2, ((uint8*)&dummy.gprBank[2][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][2]) + BYTE0,((uint8*)&dummy.gprBank[2][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][2]) + BYTE2, ((uint8*)&dummy.gprBank[2][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[2][3]) + BYTE0,((uint8*)&dummy.gprBank[2][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[2][3]) + BYTE2, ((uint8*)&dummy.gprBank[2][3]) + BYTE3,

          //Current Bank
          ((uint8*)&dummy.gprBank[3][0]) + BYTE0,((uint8*)&dummy.gprBank[3][0]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][0]) + BYTE2, ((uint8*)&dummy.gprBank[3][0]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][1]) + BYTE0,((uint8*)&dummy.gprBank[3][1]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][1]) + BYTE2, ((uint8*)&dummy.gprBank[3][1]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][2]) + BYTE0,((uint8*)&dummy.gprBank[3][2]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][2]) + BYTE2, ((uint8*)&dummy.gprBank[3][2]) + BYTE3,
          ((uint8*)&dummy.gprBank[3][3]) + BYTE0,((uint8*)&dummy.gprBank[3][3]) + BYTE1,
          ((uint8*)&dummy.gprBank[3][3]) + BYTE2, ((uint8*)&dummy.gprBank[3][3]) + BYTE3,

          ((uint8*)&dummy.gpr[0]) + BYTE0, ((uint8*)&dummy.gpr[0]) + BYTE1,
          ((uint8*)&dummy.gpr[0]) + BYTE2, ((uint8*)&dummy.gpr[0]) + BYTE3,
          ((uint8*)&dummy.gpr[1]) + BYTE0, ((uint8*)&dummy.gpr[1]) + BYTE1,
          ((uint8*)&dummy.gpr[1]) + BYTE2, ((uint8*)&dummy.gpr[1]) + BYTE3,
          ((uint8*)&dummy.gpr[2]) + BYTE0, ((uint8*)&dummy.gpr[2]) + BYTE1,
          ((uint8*)&dummy.gpr[2]) + BYTE2, ((uint8*)&dummy.gpr[2]) + BYTE3,
          ((uint8*)&dummy.gpr[3]) + BYTE0, ((uint8*)&dummy.gpr[3]) + BYTE1,
          ((uint8*)&dummy.gpr[3]) + BYTE2, ((uint8*)&dummy.gpr[3]) + BYTE3
       }
    };

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 256; j++)
        {
            tlcs->regCodeMapB[i][j] = ((uint8*)(regCodeMapB[i][j]) - (uint8*)(&dummy) + (uint8*)tlcs);
        }
    }
}

static void initRegCodeMapW(TLCS900h *tlcs)
{
    static uint16* regCodeMapW[4][128] = {
       {
            /* MAP CODE W0 */

            //BANK 0
            (uint16*)(((uint8*)&dummy.gprBank[0][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][0]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[0][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][1]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[0][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][2]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[0][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][3]) + WORD1),

            //BANK 1
            (uint16*)(((uint8*)&dummy.gprBank[1][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][0]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[1][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][1]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[1][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][2]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[1][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][3]) + WORD1),

            //BANK 2
            (uint16*)(((uint8*)&dummy.gprBank[2][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][0]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[2][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][1]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[2][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][2]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[2][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][3]) + WORD1),

            //BANK 3
            (uint16*)(((uint8*)&dummy.gprBank[3][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][0]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[3][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][1]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[3][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][2]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[3][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][3]) + WORD1),

            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,

            //Previous Bank
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
            (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,

            //Current Bank
            (uint16*)(((uint8*)&dummy.gprBank[0][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][0]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[0][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][1]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[0][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][2]) + WORD1),
            (uint16*)(((uint8*)&dummy.gprBank[0][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][3]) + WORD1),

            (uint16*)((uint8*)&(dummy.gpr[0]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[0]) + WORD1),
            (uint16*)((uint8*)&(dummy.gpr[1]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[1]) + WORD1),
            (uint16*)((uint8*)&(dummy.gpr[2]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[2]) + WORD1),
            (uint16*)((uint8*)&(dummy.gpr[3]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[3]) + WORD1),
         },

         {
             /* MAP CODE W1 */

             //BANK 0
             (uint16*)(((uint8*)&dummy.gprBank[0][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][0]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[0][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][1]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[0][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][2]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[0][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][3]) + WORD1),

             //BANK 1
             (uint16*)(((uint8*)&dummy.gprBank[1][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][0]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[1][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][1]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[1][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][2]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[1][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][3]) + WORD1),

             //BANK 2
             (uint16*)(((uint8*)&dummy.gprBank[2][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][0]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[2][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][1]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[2][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][2]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[2][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][3]) + WORD1),

             //BANK 3
             (uint16*)(((uint8*)&dummy.gprBank[3][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][0]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[3][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][1]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[3][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][2]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[3][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][3]) + WORD1),

             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
             (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,

             //Previous Bank
             (uint16*)(((uint8*)&dummy.gprBank[0][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][0]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[0][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][1]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[0][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][2]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[0][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][3]) + WORD1),

             //Current Bank
             (uint16*)(((uint8*)&dummy.gprBank[1][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][0]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[1][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][1]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[1][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][2]) + WORD1),
             (uint16*)(((uint8*)&dummy.gprBank[1][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][3]) + WORD1),

             (uint16*)((uint8*)&(dummy.gpr[0]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[0]) + WORD1),
             (uint16*)((uint8*)&(dummy.gpr[1]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[1]) + WORD1),
             (uint16*)((uint8*)&(dummy.gpr[2]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[2]) + WORD1),
             (uint16*)((uint8*)&(dummy.gpr[3]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[3]) + WORD1),
          },

          {
              /* MAP CODE W2 */
              //BANK 0
              (uint16*)(((uint8*)&dummy.gprBank[0][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][0]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[0][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][1]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[0][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][2]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[0][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][3]) + WORD1),

              //BANK 1
              (uint16*)(((uint8*)&dummy.gprBank[1][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][0]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[1][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][1]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[1][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][2]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[1][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][3]) + WORD1),

              //BANK 2
              (uint16*)(((uint8*)&dummy.gprBank[2][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][0]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[2][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][1]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[2][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][2]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[2][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][3]) + WORD1),

              //BANK 3
              (uint16*)(((uint8*)&dummy.gprBank[3][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][0]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[3][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][1]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[3][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][2]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[3][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][3]) + WORD1),

              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
              (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,

              //Previous Bank
              (uint16*)(((uint8*)&dummy.gprBank[1][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][0]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[1][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][1]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[1][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][2]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[1][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][3]) + WORD1),

              //Current Bank
              (uint16*)(((uint8*)&dummy.gprBank[2][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][0]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[2][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][1]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[2][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][2]) + WORD1),
              (uint16*)(((uint8*)&dummy.gprBank[2][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][3]) + WORD1),

              (uint16*)((uint8*)&(dummy.gpr[0]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[0]) + WORD1),
              (uint16*)((uint8*)&(dummy.gpr[1]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[1]) + WORD1),
              (uint16*)((uint8*)&(dummy.gpr[2]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[2]) + WORD1),
              (uint16*)((uint8*)&(dummy.gpr[3]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[3]) + WORD1),
           },

           {
               /* MAP CODE W3 */

               //BANK 0
               (uint16*)(((uint8*)&dummy.gprBank[0][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][0]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[0][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][1]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[0][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][2]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[0][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[0][3]) + WORD1),

               //BANK 1
               (uint16*)(((uint8*)&dummy.gprBank[1][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][0]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[1][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][1]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[1][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][2]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[1][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[1][3]) + WORD1),

               //BANK 2
               (uint16*)(((uint8*)&dummy.gprBank[2][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][0]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[2][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][1]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[2][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][2]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[2][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][3]) + WORD1),

               //BANK 3
               (uint16*)(((uint8*)&dummy.gprBank[3][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][0]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[3][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][1]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[3][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][2]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[3][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][3]) + WORD1),

               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,
               (uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,(uint16*)&dummy.rErr,

               //Previous Bank
               (uint16*)(((uint8*)&dummy.gprBank[2][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][0]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[2][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][1]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[2][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][2]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[2][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[2][3]) + WORD1),

               //Current Bank
               (uint16*)(((uint8*)&dummy.gprBank[3][0]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][0]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[3][1]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][1]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[3][2]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][2]) + WORD1),
               (uint16*)(((uint8*)&dummy.gprBank[3][3]) + WORD0), (uint16*)(((uint8*)&dummy.gprBank[3][3]) + WORD1),

               (uint16*)((uint8*)&(dummy.gpr[0]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[0]) + WORD1),
               (uint16*)((uint8*)&(dummy.gpr[1]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[1]) + WORD1),
               (uint16*)((uint8*)&(dummy.gpr[2]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[2]) + WORD1),
               (uint16*)((uint8*)&(dummy.gpr[3]) + WORD0),	(uint16*)((uint8*)&(dummy.gpr[3]) + WORD1),
            }
    };

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 128; j++)
        {
            tlcs->regCodeMapW[i][j] = (uint16*)((uint8*)(regCodeMapW[i][j]) - (uint8*)(&dummy) + (uint8*)tlcs);
        }
    }
}

static void initRegCodeMapL(TLCS900h *tlcs)
{
    static uint32* regCodeMapL[4][64] =
    {
       {
            /* MAP CODE L0 */

            //BANK 0
            &(dummy.gprBank[0][0]), &(dummy.gprBank[0][1]),	&(dummy.gprBank[0][2]), &(dummy.gprBank[0][3]),

            //BANK 1
            &(dummy.gprBank[1][0]), &(dummy.gprBank[1][1]), &(dummy.gprBank[1][2]), &(dummy.gprBank[1][3]),

            //BANK 2
            &(dummy.gprBank[2][0]), &(dummy.gprBank[2][1]),	&(dummy.gprBank[2][2]), &(dummy.gprBank[2][3]),

            //BANK 3
            &(dummy.gprBank[3][0]), &(dummy.gprBank[3][1]),	&(dummy.gprBank[3][2]), &(dummy.gprBank[3][3]),

            &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
            &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
            &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
            &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
            &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,

            //Previous Bank
            &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,

            //Current Bank
            &(dummy.gprBank[0][0]), &(dummy.gprBank[0][1]),	&(dummy.gprBank[0][2]), &(dummy.gprBank[0][3]),

            &(dummy.gpr[0]), &(dummy.gpr[1]), &(dummy.gpr[2]), &(dummy.gpr[3])
         },

         {
             /* MAP CODE L1 */

             //BANK 0
             &(dummy.gprBank[0][0]), &(dummy.gprBank[0][1]),	&(dummy.gprBank[0][2]), &(dummy.gprBank[0][3]),

             //BANK 1
             &(dummy.gprBank[1][0]), &(dummy.gprBank[1][1]),	&(dummy.gprBank[1][2]), &(dummy.gprBank[1][3]),

             //BANK 2
             &(dummy.gprBank[2][0]), &(dummy.gprBank[2][1]),	&(dummy.gprBank[2][2]), &(dummy.gprBank[2][3]),

             //BANK 3
             &(dummy.gprBank[3][0]), &(dummy.gprBank[3][1]),	&(dummy.gprBank[3][2]), &(dummy.gprBank[3][3]),

             &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
             &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
             &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
             &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
             &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,

             //Previous Bank
             &(dummy.gprBank[0][0]), &(dummy.gprBank[0][1]),	&(dummy.gprBank[0][2]), &(dummy.gprBank[0][3]),

             //Current Bank
             &(dummy.gprBank[1][0]), &(dummy.gprBank[1][1]), &(dummy.gprBank[1][2]), &(dummy.gprBank[1][3]),

             &(dummy.gpr[0]), &(dummy.gpr[1]), &(dummy.gpr[2]), &(dummy.gpr[3])
          },

          {
              /* MAP CODE L2 */

              //BANK 0
              &(dummy.gprBank[0][0]), &(dummy.gprBank[0][1]),	&(dummy.gprBank[0][2]), &(dummy.gprBank[0][3]),

              //BANK 1
              &(dummy.gprBank[1][0]), &(dummy.gprBank[1][1]),	&(dummy.gprBank[1][2]), &(dummy.gprBank[1][3]),

              //BANK 2
              &(dummy.gprBank[2][0]), &(dummy.gprBank[2][1]),	&(dummy.gprBank[2][2]), &(dummy.gprBank[2][3]),

              //BANK 3
              &(dummy.gprBank[3][0]), &(dummy.gprBank[3][1]),	&(dummy.gprBank[3][2]), &(dummy.gprBank[3][3]),

              &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
              &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
              &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
              &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
              &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,

              //Previous Bank
              &(dummy.gprBank[1][0]), &(dummy.gprBank[1][1]),	&(dummy.gprBank[1][2]), &(dummy.gprBank[1][3]),

              //Current Bank
              &(dummy.gprBank[2][0]), &(dummy.gprBank[2][1]), &(dummy.gprBank[2][2]), &(dummy.gprBank[2][3]),

              &(dummy.gpr[0]), &(dummy.gpr[1]), &(dummy.gpr[2]), &(dummy.gpr[3])
           },

           {
               /* MAP CODE L3 */

               //BANK 0
               &(dummy.gprBank[0][0]), &(dummy.gprBank[0][1]),	&(dummy.gprBank[0][2]), &(dummy.gprBank[0][3]),

               //BANK 1
               &(dummy.gprBank[1][0]), &(dummy.gprBank[1][1]),	&(dummy.gprBank[1][2]), &(dummy.gprBank[1][3]),

               //BANK 2
               &(dummy.gprBank[2][0]), &(dummy.gprBank[2][1]),	&(dummy.gprBank[2][2]), &(dummy.gprBank[2][3]),

               //BANK 3
               &(dummy.gprBank[3][0]), &(dummy.gprBank[3][1]),	&(dummy.gprBank[3][2]), &(dummy.gprBank[3][3]),

               &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
               &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
               &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
               &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,
               &dummy.rErr,&dummy.rErr,&dummy.rErr,&dummy.rErr,

               //Previous Bank
               &(dummy.gprBank[2][0]), &(dummy.gprBank[2][1]), &(dummy.gprBank[2][2]), &(dummy.gprBank[2][3]),

               //Current Bank
               &(dummy.gprBank[3][0]), &(dummy.gprBank[3][1]), &(dummy.gprBank[3][2]), &(dummy.gprBank[3][3]),

               &(dummy.gpr[0]), &(dummy.gpr[1]), &(dummy.gpr[2]), &(dummy.gpr[3])
            }
    };

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            tlcs->regCodeMapL[i][j] = (uint32*)((uint8*)(regCodeMapL[i][j]) - (uint8*)(&dummy) + (uint8*)tlcs);
        }
    }
}

void TLCS900h::initRegCodeTables()
{
    if (reg_init_flag == 1)
        return;

    initRegCodeMapB(this);
    initRegCodeMapW(this);
    initRegCodeMapL(this);

    reg_init_flag = 1;
}

//=============================================================================

uint8 TLCS900h::statusIFF(void)
{
    uint8 iff = (sr & 0x7000) >> 12;

    if (iff == 1)
        return 0;
    return iff;
}

void TLCS900h::setStatusIFF(uint8 iff)
{
    sr = (sr & 0x8FFF) | ((iff & 0x7) << 12);
}

//=============================================================================

void TLCS900h::setStatusRFP(uint8 rfp)
{
    sr = (sr & 0xF8FF) | ((rfp & 0x3) << 8);
    changedSP();
}

void TLCS900h::changedSP(void)
{
    //Store global RFP for optimisation. 
    statusRFP = ((sr & 0x300) >> 8);
    int_check_pending();
}

//=============================================================================
#ifndef le32toh
#ifdef MSB_FIRST
#define le32toh(l)      ((((l)>>24) & 0xff) | (((l)>>8) & 0xff00) \
  | (((l)<<8) & 0xff0000) | (((l)<<24) & 0xff000000))
#else
#define le32toh(x) (x)
#endif
#endif

void TLCS900h::reset_registers(void)
{
    memset(gprBank, 0, sizeof(gprBank));
    memset(gpr, 0, sizeof(gpr));

    RomHeader *rom_header = rom_GetHeaderPtr();
    if (rom_header != NULL)
        pc = le32toh(rom_header->startPC) & 0xFFFFFF;
    else
        pc = 0xFFFFFE;

    sr = 0xF800;		// = %11111000???????? (?) are undefined in the manual)
    changedSP();

    f_dash = 00;

    rErr = RERR_VALUE;

    gpr[0] = 0xff23c3;
    gpr[1] = 0xff23df;
    gpr[2] = 0x006480;
    REGXSP = 0x00006C00; //Confirmed from BIOS, 
                        //immediately changes value from default of 0x100
}

//=============================================================================
