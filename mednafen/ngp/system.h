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

#ifndef __SYSTEM__
#define __SYSTEM__

#include "../../third_party/ringbuf.h"

/* Core <--> System-IO Interface */

struct neopop_comms_t
{
	bool receive;
	int32_t rx_timer;

	bool write_flag;

	ringbuf_t tx_buf;
	ringbuf_t rx_buf;

	neopop_comms_t();
	~neopop_comms_t();

	/*! Reads a byte from the other system. If no data is available or no
		high-level communications have been established, then return FALSE.
		If buffer is NULL, then no data is read, only status is returned */
	bool system_comms_read(uint8_t* buffer);


	/*! Peeks at any data from the other system. If no data is available or
		no high-level communications have been established, then return FALSE.
		If buffer is NULL, then no data is read, only status is returned */
	bool system_comms_poll(uint8_t* buffer, int32_t tlcsCycles);


	/*! Writes a byte from the other system. This function should block until
		the data is written. USE RELIABLE COMMS! Data cannot be re-requested. */
	void system_comms_write(uint8_t data);

	int StateAction(void *data, int load, int data_only);
};

struct neopop_io_t
{
	uint32_t pad;

	/*! Reads as much of the file specified by 'filename' into the given,
		preallocated buffer. This is rom data */
	bool system_io_rom_read(char* filename, uint8_t* buffer, uint32_t bufferLength);

	/*! Reads the "appropriate" (system specific) flash data into the given
		preallocated buffer. The emulation core doesn't care where from. */
	bool system_io_flash_read(uint8_t* buffer, uint32_t bufferLength);

	/*! Writes the given flash data into an "appropriate" (system specific)
		place. The emulation core doesn't care where to. */
	bool system_io_flash_write(uint8_t* buffer, uint32_t bufferLength);
};

#endif
