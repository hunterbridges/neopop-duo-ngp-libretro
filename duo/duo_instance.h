// ----------
// NeoPop Duo
// ----------

#ifndef DUO_INSTANCE_H
#define DUO_INSTANCE_H

#include <stdint.h>
#include "../mednafen/ngp/bios.h"
#include "../mednafen/ngp/rtc.h"
#include "../mednafen/git.h"
#include "../mednafen/video.h"

struct neopop_comms_t;
struct neopop_dma_t;
struct neopop_flash_t;
struct neopop_gfx_t;
struct neopop_interrupt_t;
struct neopop_io_t;
struct neopop_mem_t;
struct neopop_rom_t;
struct neopop_sound_t;
struct neopop_z80i_t;

struct DuoTLCS900hState
{

	/*! Captures the state from the global TLCS900h interpreter and
		stores it in this struct */
	void Capture();

	/*! Restores the state stored in this struct into the global
		TLCS900h interpreter */
	void Restore();
};

struct DuoZ80State
{

	/*! Captures the state from the global z80 interpreter and
		stores it in this struct */
	void Capture();

	/*! Restores the state stored in this struct into the global
		z80 interpreter */
	void Restore();
};

class DuoInstance
{
public:
	enum Maximums
	{
		MAX_INSTANCES = 2
	};

	// Mednafen metadata
	MDFNGI *game;
	MDFN_Surface *surface;

	uint8_t system_colour;
	int32_t z80_runtime;

	// Input
	uint8_t input_buf;
	uint8_t NGPJoyLatch;

	// Processor state
	DuoTLCS900hState tlcs900hState;
	DuoZ80State z80State;

	// NeoPop core
	neopop_bios_t *bios;
	neopop_comms_t *comms;
	neopop_dma_t *dma;
	neopop_flash_t *flash;
	neopop_gfx_t *gfx;
	neopop_interrupt_t *interrupt;
	neopop_io_t *io;
	neopop_mem_t *mem;
	neopop_rom_t *rom;
	neopop_rtc_t *rtc;
	neopop_sound_t *sound;
	neopop_z80i_t *z80i;

	DuoInstance();
	DuoInstance(MDFNGI *game, MDFN_Surface *surface);

	/*! WARNING: Make sure the instance is active before running this! */
	void Reset();

	/*! WARNING: Make sure the instance is active before running this! */
	int LoadGame(const char *name, MDFNFILE *fp, const uint8_t *data, size_t size);

	/*! WARNING: Make sure the instance is active before running this! */
	void ProcessFrame(EmulateSpecStruct *espec);

private:

	// ------
	// Static
	// ------
public:
	static DuoInstance *currentInstance;
	static DuoInstance instances[MAX_INSTANCES];

	/*! Prepares a given DuoInstance for use and stores it in
		the currentInstance pointer */
	static void StageInstance(DuoInstance *instance);

	/*! Unstages the current instance and backs up any changes
		that may have happened in the processor interpreters */
	static void UnstageCurrentInstance();

};

#define GetDuoFromModule(MODULE_PTR, MODULE_NAME) \
	((DuoInstance*)(((uint8_t*)MODULE_PTR) - offsetof(DuoInstance, MODULE_NAME)))

#endif
