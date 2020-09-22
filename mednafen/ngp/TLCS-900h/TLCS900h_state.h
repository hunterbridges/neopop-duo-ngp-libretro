#ifndef __TLCS900H_STATE__
#define __TLCS900H_STATE__
//=============================================================================

#include <stdint.h>
#include "../../mednafen-types.h"
#include "TLCS900h_state.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
#if defined(_MSC_VER) && _MSC_VER < 1800 && !defined(SN_TARGET_PS3)
	/* Hack applied for MSVC when compiling in C89 mode
	 * as it isn't C99-compliant. */
#define bool unsigned char
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif
#endif

struct tlcs900h_state
{
	uint32_t mem; //Result of addressing mode
	int size; //operand size, 0 = Byte, 1 = Word, 2 = Long
	uint8_t first; //The first byte
	uint8_t second; //The second opcode
	uint8_t R; //big R
	uint8_t rCode; //The code
	int32_t cycles; //How many state changes?
	int32_t cycles_extra; //How many extra state changes?
	bool brCode; //Register code used?

	uint32_t pc;
	uint16_t sr;
	uint8_t f_dash;

	uint32_t gprBank[4][4];
	uint32_t gpr[4];

	uint32_t rErr;

	uint8_t statusRFP;

	// GPR Access
	uint8_t gpr_init_flag;
	uint8_t* gprMapB[4][8];
	uint16_t* gprMapW[4][8];
	uint32_t* gprMapL[4][8];

	// Reg. Code Access
	uint8_t reg_init_flag;
	uint8_t* regCodeMapB[4][256];
	uint16_t* regCodeMapW[4][128];
	uint32_t* regCodeMapL[4][64];

};

extern struct tlcs900h_state *cur_tlcs900h;

#ifdef __cplusplus
};
#endif

#endif
