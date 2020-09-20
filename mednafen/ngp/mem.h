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

#ifndef __MEM__
#define __MEM__

#include <stdint.h>
#include <boolean.h>

#define ROM_START    0x200000
#define ROM_END		0x3FFFFF

#define HIROM_START	0x800000
#define HIROM_END    0x9FFFFF

#define BIOS_START	0xFF0000
#define BIOS_END     0xFFFFFF

#ifdef __cplusplus
extern "C" {
#endif
extern bool debug_abort_memory;
extern bool debug_mask_memory_error_messages;

uint8_t  loadB(uint32_t address);
uint16_t loadW(uint32_t address);
uint32_t loadL(uint32_t address);

void storeB(uint32_t address, uint8_t data);
void storeW(uint32_t address, uint16_t data);
void storeL(uint32_t address, uint32_t data);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
struct neopop_mem_t
{
   bool memory_unlock_flash_write;
   bool memory_flash_error;
   bool memory_flash_command;

   bool FlashStatusEnable;
   uint32_t FlashStatus;	
   uint8_t SC0BUF; /* Serial channel 0 buffer. */
   uint8_t COMMStatus;

   uint8_t CPUExRAM[16384];

/* In very very very rare conditions(like on embedded platforms with 
 * no virtual memory and very limited RAM and malloc happens to 
 * return a pointer aligned to a 64KiB boundary), a FastReadMap entry 
 * may be NULL even if it points to valid data when it's added to 
 * the address of the read, but if this happens, it will only 
 * make the emulator slightly slower. */
   uint8_t *FastReadMap[256];
   uint8_t *FastReadMapReal[256];

   void reset_memory(void);

   void dump_memory(uint32_t start, uint32_t length);

   uint8_t  loadB(uint32_t address);
   uint16_t loadW(uint32_t address);
   uint32_t loadL(uint32_t address);
   
   void storeB(uint32_t address, uint8_t data);
   void storeW(uint32_t address, uint16_t data);
   void storeL(uint32_t address, uint32_t data);

   void SetFRM(void);
   void RecacheFRM(void);

private:
   uint8_t lastpoof;

   void* translate_address_read(uint32_t address);
   void* translate_address_write(uint32_t address);
};
#endif

#endif
