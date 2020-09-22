// ----------
// NeoPop Duo
// ----------

#include <stdlib.h>

#include "duo_runner.h"
#include "duo_helper.h"
#include "duo_settings.h"
#include "../mednafen/mednafen-types.h"

const char *mednafen_core_str = MEDNAFEN_CORE_NAME;

DuoRunner DuoRunner::shared;

// ------
// Global
// ------

extern "C" const char *DuoRunner_GetBaseDir()
{
	return DuoRunner::shared.retro_base_directory;
}

extern "C" const char *DuoRunner_GetSaveDir()
{
	return DuoRunner::shared.retro_save_directory;
}

extern "C" uint32_t DuoRunner_GetLangSetting()
{
	return DuoRunner::shared.setting_ngp_language;
}

// ---------
// Lifecycle
// ---------

static bool clean_and_copy_dir_path(char *out_str, const char *in_str)
{
	std::string tmp;

	tmp = in_str;
	// Make sure that we don't have any lingering slashes, etc, as they break Windows.
	size_t last = tmp.find_last_not_of("/\\");
	if (last != std::string::npos)
		last++;

	tmp = tmp.substr(0, last);

	strcpy(out_str, tmp.c_str());
	return true;
}

void DuoRunner::Initialize(void)
{
	struct retro_log_callback log;
	if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
		log_cb = log.log;
	else
		log_cb = NULL;

	const char *dir = NULL;

	if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
	{
		clean_and_copy_dir_path(retro_base_directory, dir);
	}
	else
	{
		/* TODO: Add proper fallback */
		if (log_cb)
			log_cb(RETRO_LOG_WARN, "System directory is not defined. Fallback on using same dir as ROM for system directory later ...\n");
		failed_init = true;
	}

	if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &dir) && dir)
	{
		clean_and_copy_dir_path(retro_save_directory, dir);
	}
	else
	{
		/* TODO: Add proper fallback */
		if (log_cb)
			log_cb(RETRO_LOG_WARN, "Save directory is not defined. Fallback on using SYSTEM directory ...\n");
		strcpy(retro_save_directory, retro_base_directory);
	}

	perf_get_cpu_features_cb = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_PERF_INTERFACE, &perf_cb))
		perf_get_cpu_features_cb = perf_cb.get_cpu_features;

	if (environ_cb(RETRO_ENVIRONMENT_GET_INPUT_BITMASKS, NULL))
		libretro_supports_bitmasks = true;

	CheckSystemSpecs();

	for (int i = 0; i < instance_count; i++)
	{
		DuoInstance *duo = &DuoInstance::instances[i];
		memset(duo, 0, sizeof(DuoInstance));
		new (duo) DuoInstance();
		duo->Initialize();
	}
}

void DuoRunner::Deinitialize(void)
{
	for (int i = 0; i < instance_count; i++)
	{
		DuoInstance *duo = &DuoInstance::instances[i];
		duo->Deinitialize();
		memset(duo, 0, sizeof(DuoInstance));
	}

	// Log av info
	if (log_cb)
	{
		log_cb(RETRO_LOG_INFO, "[%s]: Samples / Frame: %.5f\n",
			mednafen_core_str, (double)audio_frames / video_frames);
		log_cb(RETRO_LOG_INFO, "[%s]: Estimated FPS: %.5f\n",
			mednafen_core_str, (double)video_frames * 44100 / audio_frames);
	}

	libretro_supports_bitmasks = false;
}

void DuoRunner::Reset(void)
{
	for (int i = 0; i < instance_count; i++)
	{
		DuoInstance *duo = &DuoInstance::instances[i];
		DuoInstance::StageInstance(duo);
		duo->Reset();
		DuoInstance::UnstageCurrentInstance();
	}
}

void DuoRunner::Run()
{
	bool updated = false;

	// Check vars
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
		CheckVariables();

	// Update input
	UpdateInput();

	// Prepare specs for each instance
	for (int i = 0; i < instance_count; i++)
	{
		DuoInstance *duo = &DuoInstance::instances[i];
		duo->ConfigureSpec();
	}

	// Update AV settings if needed
	if (update_video || update_audio)
	{
		struct retro_system_av_info system_av_info;

		if (update_video)
		{
			memset(&system_av_info, 0, sizeof(system_av_info));
			environ_cb(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &system_av_info);
		}

		retro_get_system_av_info(&system_av_info);
		environ_cb(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &system_av_info);

		for (int i = 0; i < instance_count; i++)
		{
			DuoInstance *duo = &DuoInstance::instances[i];
			duo->surface->depth = RETRO_PIX_DEPTH;
		}

		update_video = false;
		update_audio = false;
	}

	DuoInstance *duoA = &DuoInstance::instances[0];
	DuoInstance *duoB = &DuoInstance::instances[1];
	duoA->ProcessFrame_Interleaved(duoB);

	// Process frame for each instance
	/*
	for (int i = 0; i < instance_count; i++)
	{
		DuoInstance *duo = &DuoInstance::instances[i];
		DuoInstance::StageInstance(duo);
		duo->ProcessFrame();
		DuoInstance::UnstageCurrentInstance();
	}
	*/ 

	// Mix AV
	MixFrameAV();
}

void DuoRunner::MixFrameAV()
{
	static uint8_t joined_buf_16[FB_WIDTH * FB_HEIGHT * 2 * 2]; // two screens' worth of 16-bit data
	static uint8_t joined_buf_24[FB_WIDTH * FB_HEIGHT * 4 * 2];
	const int half_16 = sizeof(joined_buf_16) / 2;
	const int half_24 = sizeof(joined_buf_16) / 2;
	int pitch = FB_WIDTH * ((RETRO_PIX_DEPTH + 7) / 8);;

	static int16_t audio_mix_buf[0x10000];

	uint8_t *joined_buf = joined_buf_16;
	int half = half_16;
	if (RETRO_PIX_BYTES == 4)
	{
		joined_buf = joined_buf_24;
		half = half_24;
	}

	int total = 0;

	// ------------
	// Video Mixing
	// ------------

	if (videoMixMode == AV_BOTH_PLAYERS && instance_count > 1)
	{
		// Display both screens
		DuoInstance *duoP1 = &DuoInstance::instances[0];
		DuoInstance *duoP2 = &DuoInstance::instances[1];

		if (videoLayout & AV_LAYOUT_SWITCH_MASK)
		{
			DuoInstance *tmp = duoP2;
			duoP2 = duoP1;
			duoP1 = tmp;
		}

		if (videoLayout & AV_LAYOUT_V)
		{
			memcpy(joined_buf, duoP1->surface->pixels, half);
			memcpy(joined_buf + half, duoP2->surface->pixels, half);
			video_cb(joined_buf, duoP1->width, duoP1->height * 2, pitch);
		}
		else
		{
			for (int row = 0; row < duoP1->height; row++)
			{
				memcpy(joined_buf + ptrdiff_t(pitch * (2 * row)), (uint8_t*)duoP1->surface->pixels + ptrdiff_t(pitch * row), pitch);
				memcpy(joined_buf + ptrdiff_t(pitch * (2 * row + 1)), (uint8_t*)duoP2->surface->pixels + ptrdiff_t(pitch * row), pitch);
			}

			video_cb(joined_buf, duoP1->width * 2, duoP1->height, ptrdiff_t(pitch * 2));
		}

		video_frames++;
	}
	else
	{
		// Display single screen
		int instance_i = (videoMixMode == AV_P2_ONLY ? 1 : 0);

		DuoInstance *duo = &DuoInstance::instances[instance_i];
		video_cb(duo->surface->pixels, duo->width, duo->height, pitch);
		video_frames++;
	}

	// ------------
	// Audio Mixing
	// ------------

	if (audioMixMode == AV_BOTH_PLAYERS && instance_count > 1)
	{
		// Mix P1 and P2 audio

		DuoInstance *duoP1 = &DuoInstance::instances[0];
		DuoInstance *duoP2 = &DuoInstance::instances[1];

		memset(audio_mix_buf, 0, sizeof(audio_mix_buf));

		int minSize = MIN(duoP1->spec.SoundBufSize, duoP2->spec.SoundBufSize);

		memcpy(audio_mix_buf, duoP1->spec.SoundBuf, sizeof(audio_mix_buf));
		for (int i = 0; i < minSize * 2; i++)
		{
			audio_mix_buf[i] += duoP2->spec.SoundBuf[i];
		}

		audio_frames += minSize;

		for (total = 0; total < minSize; )
			total += audio_batch_cb(audio_mix_buf + total * 2, minSize - total);
	}
	else
	{
		// P1 or P2 audio only
		int instance_i = (audioMixMode == AV_P2_ONLY ? 1 : 0);

		DuoInstance *duo = &DuoInstance::instances[instance_i];
		audio_frames += duo->spec.SoundBufSize;

		for (total = 0; total < duo->spec.SoundBufSize; )
			total += audio_batch_cb(duo->spec.SoundBuf + total * 2, duo->spec.SoundBufSize - total);
	}
}

// -----
// Input
// -----

void DuoRunner::UpdateInput(void)
{
	input_poll_cb();

	static unsigned map[] = {
	   RETRO_DEVICE_ID_JOYPAD_UP, //X Cursor horizontal-layout games
	   RETRO_DEVICE_ID_JOYPAD_DOWN, //X Cursor horizontal-layout games
	   RETRO_DEVICE_ID_JOYPAD_LEFT, //X Cursor horizontal-layout games
	   RETRO_DEVICE_ID_JOYPAD_RIGHT, //X Cursor horizontal-layout games
	   RETRO_DEVICE_ID_JOYPAD_B,
	   RETRO_DEVICE_ID_JOYPAD_A,
	   RETRO_DEVICE_ID_JOYPAD_START,
	};

	for (int instance_i = 0; instance_i < instance_count; instance_i++)
	{
		DuoInstance *duo = &DuoInstance::instances[instance_i];
		unsigned i, j;
		int16_t ret = 0;
		duo->input_buf = 0;

		if (libretro_supports_bitmasks)
			ret = input_state_cb(instance_i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
		else
		{
			for (j = 0; j < (RETRO_DEVICE_ID_JOYPAD_R3 + 1); j++)
				if (input_state_cb(instance_i, RETRO_DEVICE_JOYPAD, 0, j))
					ret |= (1 << j);
		}

		for (i = 0; i < MAX_BUTTONS; i++)
			if ((map[i] != -1) && (ret & (1 << map[i])))
				duo->input_buf |= (1 << i);
	}
}

// ----
// Info
// ----

void DuoRunner::GetSystemInfo(struct retro_system_info *info)
{
	memset(info, 0, sizeof(*info));
	info->library_name = MEDNAFEN_CORE_NAME;
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif

#ifdef LOAD_FROM_MEMORY
	info->need_fullpath = false;
#else
	info->need_fullpath = true;
#endif

	info->library_version = MEDNAFEN_CORE_VERSION GIT_VERSION;
	info->valid_extensions = MEDNAFEN_CORE_EXTENSIONS;
	info->block_extract = false;
}

void DuoRunner::GetAvInfo(struct retro_system_av_info *info)
{
	memset(info, 0, sizeof(*info));
	info->timing.fps = MEDNAFEN_CORE_TIMING_FPS;
	info->timing.sample_rate = RETRO_SAMPLE_RATE;
	info->geometry.base_width = MEDNAFEN_CORE_GEOMETRY_BASE_W;
	info->geometry.base_height = MEDNAFEN_CORE_GEOMETRY_BASE_H;
	info->geometry.max_width = MEDNAFEN_CORE_GEOMETRY_MAX_W * 2;
	info->geometry.max_height = MEDNAFEN_CORE_GEOMETRY_MAX_H * 2;

	if (videoMixMode == AV_BOTH_PLAYERS)
	{
		if (videoLayout & AV_LAYOUT_V)
			info->geometry.base_height *= 2;
		else
			info->geometry.base_width *= 2;

		info->geometry.aspect_ratio = float(info->geometry.base_width) / float(info->geometry.base_height);
	}
	else
	{
		info->geometry.aspect_ratio = MEDNAFEN_CORE_GEOMETRY_ASPECT_RATIO;
	}

	CheckColorDepth();
}

void DuoRunner::SetBasename(const char *path)
{
	const char *base = strrchr(path, '/');
	if (!base)
		base = strrchr(path, '\\');

	if (base)
		retro_base_name = base + 1;
	else
		retro_base_name = path;

	retro_base_name = retro_base_name.substr(0, retro_base_name.find_last_of('.'));
}

// --------
// Settings
// --------

void DuoRunner::CheckSystemSpecs(void)
{
	unsigned level = 0;
	environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

void DuoRunner::CheckColorDepth(void)
{
	if (RETRO_PIX_DEPTH == 24)
	{
		enum retro_pixel_format rgb888 = RETRO_PIXEL_FORMAT_XRGB8888;

		if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb888))
		{
			if (log_cb) log_cb(RETRO_LOG_ERROR, "Pixel format XRGB8888 not supported by platform.\n");

			RETRO_PIX_BYTES = 2;
			RETRO_PIX_DEPTH = 15;
		}
	}

	if (RETRO_PIX_BYTES == 2)
	{
#if defined(FRONTEND_SUPPORTS_RGB565)
		enum retro_pixel_format rgb565 = RETRO_PIXEL_FORMAT_RGB565;

		if (environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb565))
		{
			if (log_cb) log_cb(RETRO_LOG_INFO, "Frontend supports RGB565 - will use that instead of XRGB1555.\n");

			RETRO_PIX_DEPTH = 16;
		}
#else
		enum retro_pixel_format rgb555 = RETRO_PIXEL_FORMAT_0RGB1555;

		if (environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb555))
		{
			if (log_cb) log_cb(RETRO_LOG_INFO, "Using default 0RGB1555 pixel format.\n");

			RETRO_PIX_DEPTH = 15;
		}
#endif
	}
}

void DuoRunner::CheckVariables(void)
{
	struct retro_variable var = { 0 };

	var.key = "ngp_language";
	var.value = NULL;

	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
	{
		/* user must manually restart core for change to happen */
		if (!strcmp(var.value, "japanese"))
			setting_ngp_language = 0;
		else if (!strcmp(var.value, "english"))
			setting_ngp_language = 1;
	}

	var.key = "ngp_sound_sample_rate";
	var.value = NULL;

	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
	{
		int old_value = RETRO_SAMPLE_RATE;

		RETRO_SAMPLE_RATE = atoi(var.value);

		if (old_value != RETRO_SAMPLE_RATE)
			update_audio = true;
	}

	var.key = "ngp_gfx_colors";
	var.value = NULL;

	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
	{
		int old_value = RETRO_PIX_BYTES;

		if (strcmp(var.value, "16bit") == 0)
		{
			RETRO_PIX_BYTES = 2;
			RETRO_PIX_DEPTH = 16;
		}
		else if (strcmp(var.value, "24bit") == 0)
		{
			RETRO_PIX_BYTES = 4;
			RETRO_PIX_DEPTH = 24;
		}

		if (old_value != RETRO_PIX_BYTES)
			update_video = true;
	}
}

// -------
// Loading
// -------

void DuoRunner::InitGameInfo()
{
	memset(&game_info, 0, sizeof(MDFNGI));
	game_info.fps = (uint32)((uint64)6144000 * 65536 * 256 / 515 / 198); // 3072000 * 2 * 10000 / 515 / 198
}

bool DuoRunner::LoadGameFromFile(const char *name)
{
	MDFNFILE *game_file = file_open(name);

	if (!game_file)
		goto error;

	InitGameInfo();

	for (int i = 0; i < instance_count; i++)
	{
		DuoInstance *duo = &DuoInstance::instances[i];
		DuoInstance::StageInstance(duo);
		if (duo->LoadGame(&game_info, game_file->size, game_file->data) == false)
			goto error;

		DuoInstance::UnstageCurrentInstance();
	}

	file_close(game_file);

	return true;

error:
	if (game_file)
		file_close(game_file);

	DuoInstance::UnstageCurrentInstance();
	MDFNGI_reset(&game_info);
	return false;
}

bool DuoRunner::LoadGameFromData(const uint8_t *data, size_t size)
{
	InitGameInfo();

	for (int i = 0; i < instance_count; i++)
	{
		DuoInstance *duo = &DuoInstance::instances[i];
		DuoInstance::StageInstance(duo);
		if (duo->LoadGame(&game_info, size, data) == false)
			goto error;

		DuoInstance::UnstageCurrentInstance();
	}

	return true;

error:
	DuoInstance::UnstageCurrentInstance();
	MDFNGI_reset(&game_info);
	return false;
}

bool DuoRunner::LoadGame(const struct retro_game_info *info)
{
	if (!info || failed_init)
		return false;

	// Init input descriptors
	struct retro_input_descriptor desc[] = {
	   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "P1 D-Pad Left" },
	   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "P1 D-Pad Up" },
	   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "P1 D-Pad Down" },
	   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "P1 D-Pad Right" },
	   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "P1 A" },
	   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "P1 B" },
	   { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "P1 Option" },


	   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "P2 D-Pad Left" },
	   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "P2 D-Pad Up" },
	   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "P2 D-Pad Down" },
	   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "P2 D-Pad Right" },
	   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "P2 A" },
	   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "P2 B" },
	   { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "P2 Option" },

	   { 0 },
	};

	environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

	// Init overscan
	overscan = false;
	environ_cb(RETRO_ENVIRONMENT_GET_OVERSCAN, &overscan);

	// Set basename
	SetBasename(info->path);

	// Read settings
	CheckVariables();
	CheckColorDepth();

	// Load game
	bool loaded_game = false;
#ifdef LOAD_FROM_MEMORY
	loaded_game = LoadGameFromData((const uint8_t *)info->data, info->size);
#else
	loaded_game = LoadGameFromFile(info->path);
#endif

	if (loaded_game == false)
		return false;

	update_video = false;
	update_audio = false;

	return true;
}

void DuoRunner::UnloadGame()
{
	for (int i = 0; i < instance_count; i++)
	{
		DuoInstance *duo = &DuoInstance::instances[i];
		DuoInstance::StageInstance(duo);
		duo->UnloadGame();
		DuoInstance::UnstageCurrentInstance();
	}

	MDFNGI_reset(&game_info);
}

// -------------
// Serialization
// -------------

int DuoRunner::SaveStateAction(StateMem *sm, int load, int data_only)
{
	// TODO
	return 0;

	/*
SFORMAT StateRegs[] =
{
   SFVAR(z80_runtime),
   SFARRAY(CPUExRAM, 16384),
   SFVAR(FlashStatusEnable),
   SFEND
};

SFORMAT TLCS_StateRegs[] =
{
   SFVARN(pc, "PC"),
   SFVARN(sr, "SR"),
   SFVARN(f_dash, "F_DASH"),
   SFARRAY32N(gpr, 4, "GPR"),
   SFARRAY32N(gprBank[0], 4, "GPRB0"),
   SFARRAY32N(gprBank[1], 4, "GPRB1"),
   SFARRAY32N(gprBank[2], 4, "GPRB2"),
   SFARRAY32N(gprBank[3], 4, "GPRB3"),
   SFEND
};

if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAIN", false))
   return(0);

if(!MDFNSS_StateAction(sm, load, data_only, TLCS_StateRegs, "TLCS", false))
   return(0);

if(!MDFNNGPCDMA_StateAction(sm, load, data_only))
   return(0);

if(!MDFNNGPCSOUND_StateAction(sm, load, data_only))
   return(0);

if(!ngpgfx_StateAction(NGPGfx, sm, load, data_only))
   return(0);

if(!MDFNNGPCZ80_StateAction(sm, load, data_only))
   return(0);

if(!int_timer_StateAction(sm, load, data_only))
   return(0);

if(!BIOSHLE_StateAction(sm, load, data_only))
   return(0);

if(!FLASH_StateAction(sm, load, data_only))
   return(0);

if(load)
{
   RecacheFRM();
   changedSP();
}
return(1);
*/
}
