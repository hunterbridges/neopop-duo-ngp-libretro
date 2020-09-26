#include "../../mednafen-types.h"

void (*z80_writebyte)(uint16 a, uint8 b);
uint8 (*z80_readbyte)(uint16 a);
void (*z80_writeport)(uint16 a, uint8 b);
uint8 (*z80_readport)(uint16 a);

/* Similarly, overflow can be determined by looking at the 7th bits; again
   the hash into this table is r12 */
const uint8 overflow_add_table[] = { 0, 0, 0, Z80_FLAG_V, Z80_FLAG_V, 0, 0, 0 };
const uint8 overflow_sub_table[] = { 0, Z80_FLAG_V, 0, 0, 0, 0, Z80_FLAG_V, 0 };

/* Whether a half carry occurred or not can be determined by looking at
   the 3rd bit of the two arguments and the result; these are hashed
   into this table in the form r12, where r is the 3rd bit of the
   result, 1 is the 3rd bit of the 1st argument and 2 is the
   third bit of the 2nd argument; the tables differ for add and subtract
   operations */
const uint8 halfcarry_add_table[] =
  { 0, Z80_FLAG_H, Z80_FLAG_H, Z80_FLAG_H, 0, 0, 0, Z80_FLAG_H };
const uint8 halfcarry_sub_table[] =
  { 0, 0, Z80_FLAG_H, 0, Z80_FLAG_H, 0, Z80_FLAG_H, Z80_FLAG_H };

