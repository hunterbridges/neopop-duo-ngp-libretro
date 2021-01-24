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

#ifndef __INTERRUPT__
#define __INTERRUPT__

#include <boolean.h>
#include <stdint.h>

#define TIMER_HINT_RATE		515		/* CPU Ticks between horizontal interrupts */

#define TIMER_BASE_RATE		32 		/* ticks */

#define TIMER_T1_RATE		(8    * TIMER_BASE_RATE)
#define TIMER_T4_RATE		(32   * TIMER_BASE_RATE)
#define TIMER_T16_RATE		(128  * TIMER_BASE_RATE)
#define TIMER_T256_RATE		(2048 * TIMER_BASE_RATE)

#ifdef __cplusplus
extern "C" {
#endif

void int_check_pending();
void set_interrupt(uint8_t index, bool set);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
struct neopop_interrupt_t
{
	uint32_t timer_hint;
	uint32_t timer_clock[4];
	uint32_t timer_serial;
	uint8_t timer[4];	//Up-counters
	uint8_t timer_threshold[4];

	uint8_t TRUN;
	uint8_t T01MOD, T23MOD;
	uint8_t TRDC;
	uint8_t TFFCR;
	uint8_t HDMAStartVector[4];

	int32_t ipending[24];
	int32_t IntPrio[0xB]; // 0070-007a

	bool h_int;
	bool timer0;
    bool timer2;
	bool NGPFrameSkip;

	void interrupt(uint8_t index, uint8_t level);
	void set_interrupt(uint8_t index, bool set);

	void reset_timers();
	void reset_int();

	/* Call this after each instruction */
	bool updateTimers(void *data, int cputicks);

	void timer_write8(uint32_t address, uint8_t data);
	uint8_t timer_read8(uint32_t address);

	void int_write8(uint32_t address, uint8_t data);
	uint8_t int_read8(uint32_t address);
	void int_check_pending();
	void TestIntHDMA(int bios_num, int vec_num);

	int Timer_StateAction(void *data, int load, int data_only);
};
#endif

#endif
