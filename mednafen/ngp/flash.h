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

#ifndef __FLASH__
#define __FLASH__

#include <stdint.h>

//This value is used to verify flash data - it is set to the
//version number that the flash description was modified for.

#define FLASH_VALID_ID		0x0053

//Number of different flash blocks, this should be enough.

#define FLASH_MAX_BLOCKS	256

typedef struct
{
	//Flash Id
	uint16_t valid_flash_id;		// = FLASH_VALID_ID
	uint16_t block_count;			//Number of flash data blocks
	uint32_t total_file_length;		// header + block[0 - block_count]
} FlashFileHeader;

typedef struct
{
	uint32_t start_address;		// 24 bit address
	uint16_t data_length;		// length of following data
	//Followed by data_length bytes of the actual data.
} FlashFileBlockHeader;

struct neopop_flash_t
{
	FlashFileBlockHeader blocks[256];
	uint16_t block_count;

	void flash_read();

	uint8_t *make_flash_commit(int32_t *length);

	/* Marks flash blocks for saving. */
	void flash_write(uint32_t start_address, uint16_t length);
	void flash_optimise_blocks();

	/* Stores the flash data */
	void flash_commit();

	void do_flash_read(uint8_t *flashdata);

	int StateAction(void *data, int load, int data_only);
};

#endif
