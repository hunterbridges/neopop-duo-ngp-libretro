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

#include "neopop.h"
#include "../../duo/duo_instance.h"
#include "mem.h"
#include "sound.h"
#include "Z80_interface.h"
#include "TLCS-900h/TLCS900h.h"
#include "interrupt.h"
#include "dma.h"
#include "../hw_cpu/z80-fuse/z80.h"
#include "../hw_cpu/z80-fuse/z80_macros.h"

#include "../state.h"

uint8_t neopop_z80i_t::Z80_ReadComm(void)
{
   return CommByte;
}

void neopop_z80i_t::Z80_WriteComm(uint8_t data)
{
   CommByte = data;
}

static uint8_t NGP_z80_readbyte(uint16_t address)
{
   DuoInstance *duo = DuoInstance::currentInstance;

   if (address <= 0xFFF)
      return duo->mem->loadB(0x7000 + address);

   switch (address)
   {
      case 0x8000:
         return duo->z80i->CommByte;
      default:
         break;
   }

   return 0;
}

static void NGP_z80_writebyte(uint16_t address, uint8_t value)
{
   DuoInstance *duo = DuoInstance::currentInstance;

   if (address <= 0x0FFF)
   {
      duo->mem->storeB(0x7000 + address, value);
      return;
   }

   switch (address)
   {
      case 0x8000:
          duo->z80i->CommByte = value;
         break;
      case 0x4001:
         duo->sound->Write_SoundChipLeft(value);
         break;
      case 0x4000:
         duo->sound->Write_SoundChipRight(value);
         break;
      case 0xC000:
          duo->interrupt->TestIntHDMA(6, 0x0C);
         break;
   }

}

static void NGP_z80_writeport(uint16_t port, uint8_t value)
{
   DuoInstance *duo = DuoInstance::currentInstance;

	//printf("Portout: %04x %02x\n", port, value);
   duo->z80_state.z80_set_interrupt(0);
}

static uint8_t NGP_z80_readport(uint16_t port)
{
	//printf("Portin: %04x\n", port);
	return 0;
}

void neopop_z80i_t::Z80_nmi(void)
{
    DuoInstance *duo = GetDuoFromModule(this, z80i);

	duo->z80_state.z80_nmi();
}

void neopop_z80i_t::Z80_irq(void)
{
    DuoInstance *duo = GetDuoFromModule(this, z80i);

    duo->z80_state.z80_set_interrupt(1);
}

void neopop_z80i_t::Z80_reset(void)
{
    DuoInstance *duo = GetDuoFromModule(this, z80i);

	Z80Enabled = 0;

	z80_writebyte = NGP_z80_writebyte;
	z80_readbyte = NGP_z80_readbyte;
	z80_writeport = NGP_z80_writeport;
	z80_readport = NGP_z80_readport;

    duo->z80_state.z80_init();
    duo->z80_state.z80_reset();
}

void neopop_z80i_t::Z80_SetEnable(bool set)
{
   DuoInstance *duo = GetDuoFromModule(this, z80i);

   Z80Enabled = set;
   if(!set)
       duo->z80_state.z80_reset();
}

bool neopop_z80i_t::Z80_IsEnabled(void)
{
   return(Z80Enabled);
}

int neopop_z80i_t::Z80_RunOP(void)
{
   DuoInstance *duo = GetDuoFromModule(this, z80i);

   if(!Z80Enabled)
      return -1;

   return(duo->z80_state.z80_do_opcode());
}

// ------------
// State Action
// ------------

int neopop_z80i_t::z80_state_action(void *data, int load, int data_only, const char *section_name)
{
   Z80 *z80_state = &GetDuoFromModule(this, z80i)->z80_state;

   uint8_t r_register;

   SFORMAT StateRegs[] =
   {
      { &(z80_state->z80.af.w), sizeof(z80_state->z80.af.w), 0x80000000, "AF" },
      { &(z80_state->z80.bc.w), sizeof(z80_state->z80.bc.w), 0x80000000, "BC" },
      { &(z80_state->z80.de.w), sizeof(z80_state->z80.de.w), 0x80000000, "DE" },
      { &(z80_state->z80.hl.w), sizeof(z80_state->z80.hl.w), 0x80000000, "HL" },
      { &(z80_state->z80.af_.w), sizeof(z80_state->z80.af_.w), 0x80000000, "AF_" },
      { &(z80_state->z80.bc_.w), sizeof(z80_state->z80.bc_.w), 0x80000000, "BC_" },
      { &(z80_state->z80.de_.w), sizeof(z80_state->z80.de_.w), 0x80000000, "DE_" },
      { &(z80_state->z80.hl_.w), sizeof(z80_state->z80.hl_.w), 0x80000000, "HL_" },
      { &(z80_state->z80.ix.w), sizeof(z80_state->z80.ix.w), 0x80000000, "IX" },
      { &(z80_state->z80.iy.w), sizeof(z80_state->z80.iy.w), 0x80000000, "IY" },
      { &(z80_state->z80.i), sizeof(z80_state->z80.i), 0x80000000, "I" },
      { &(z80_state->z80.sp.w), sizeof(z80_state->z80.sp.w), 0x80000000, "SP" },
      { &(z80_state->z80.pc.w), sizeof(z80_state->z80.pc.w), 0x80000000, "PC" },
      { &(z80_state->z80.iff1), sizeof(z80_state->z80.iff1), 0x80000000, "IFF1" },
      { &(z80_state->z80.iff2), sizeof(z80_state->z80.iff2), 0x80000000, "IFF2" },
      { &(z80_state->z80.im), sizeof(z80_state->z80.im), 0x80000000, "IM" },
      { &(r_register), sizeof(r_register), 0x80000000, "R" },

      { &(z80_state->z80.interrupts_enabled_at), sizeof(z80_state->z80.interrupts_enabled_at), 0x80000000, "interrupts_enabled_at" },
      { &(z80_state->z80.halted), sizeof(z80_state->z80.halted), 0x80000000, "halted" },

      { &((z80_state->z80_tstates)), sizeof((z80_state->z80_tstates)), 0x80000000, "z80_tstates" },
      { &((z80_state->last_z80_tstates)), sizeof((z80_state->last_z80_tstates)), 0x80000000, "last_z80_tstates" },

      { 0, 0, 0, 0 }
   };

   if(!load)
      r_register = (z80_state->z80.r7 & 0x80) | (z80_state->z80.r & 0x7f);

   if(!MDFNSS_StateAction(data, load, data_only, StateRegs, section_name, false))
      return(0);

   if(load)
   {
       z80_state->z80.r7 = r_register & 0x80;
       z80_state->z80.r = r_register & 0x7F;
   }

   return(1);
}

int neopop_z80i_t::StateAction(void *data, int load, int data_only)
{
   SFORMAT StateRegs[] =
   {
      SFVAR(CommByte),
      SFVAR(Z80Enabled),
      SFEND
   };

   if(!MDFNSS_StateAction(data, load, data_only, StateRegs, "Z80X", false))
      return 0;

   if(!z80_state_action(data, load, data_only, "Z80"))
      return 0;

   return 1;
}
