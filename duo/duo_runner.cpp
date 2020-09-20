// ----------
// NeoPop Duo
// ----------

#include <stdlib.h>

#include "duo_runner.h"

#define MEDNAFEN_CORE_NAME_MODULE "ngp"
#define MEDNAFEN_CORE_NAME "NeoPop Duo"
#define MEDNAFEN_CORE_VERSION "v0.1.00.0"
#define MEDNAFEN_CORE_EXTENSIONS "ngp|ngc|ngpc|npc"
#define MEDNAFEN_CORE_TIMING_FPS 60.25
#define MEDNAFEN_CORE_GEOMETRY_BASE_W 160 
#define MEDNAFEN_CORE_GEOMETRY_BASE_H 152
#define MEDNAFEN_CORE_GEOMETRY_MAX_W 160
#define MEDNAFEN_CORE_GEOMETRY_MAX_H 152
#define MEDNAFEN_CORE_GEOMETRY_ASPECT_RATIO (20.0 / 19.0)
#define FB_WIDTH 160
#define FB_HEIGHT 152

#define FB_MAX_HEIGHT FB_HEIGHT

#define MAX_PLAYERS 1
#define MAX_BUTTONS 7

const char *mednafen_core_str = MEDNAFEN_CORE_NAME;

DuoRunner DuoRunner::shared;

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
}

void DuoRunner::Deinitialize(void)
{
   // Destroy runner

   // TODO Destroy each instance
   /*
	   if (surf)
	   {
		  if (surf->pixels)
			 free(surf->pixels);
		  free(surf);
	   }
	   surf = NULL;
       */

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
   // TODO
   // foreach instance
   //     instance->Reset();
}

bool DuoRunner::LoadGame(const struct retro_game_info *info)
{
   if (!info || failed_init)
      return false;

   // Init input (Runner)

   struct retro_input_descriptor desc[] = {
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "A" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "B" },
      { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Option" },

      { 0 },
   };

   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

   // Init overscan (Runner)

   overscan = false;
   environ_cb(RETRO_ENVIRONMENT_GET_OVERSCAN, &overscan);

   // Set basename (Runner)

   set_basename(info->path);

   // Read settings (Runner)

   check_variables();
   check_color_depth();

   // Load game (Load the file data in Runner, pass data to Instance)

#ifdef LOAD_FROM_MEMORY
   game = MDFNI_LoadGameData((const uint8_t *)info->data, info->size);
#else
   game = MDFNI_LoadGame(info->path);
#endif

   if (!game)
      return false;

   // Init output surface (instance)

   surf = (MDFN_Surface*)calloc(1, sizeof(*surf));
   
   if (!surf)
      return false;
   
   surf->width  = FB_WIDTH;
   surf->height = FB_HEIGHT;
   surf->pitch  = FB_WIDTH;
   surf->depth  = RETRO_PIX_DEPTH;

   surf->pixels = (uint16_t*)calloc(1, FB_WIDTH * FB_HEIGHT * sizeof(uint32_t));

   if (!surf->pixels)
   {
      free(surf);
      return false;
   }

   // Set core gfx settings (instance)
   ngpgfx_set_pixel_format(NGPGfx, RETRO_PIX_DEPTH);

   // Set core sound settings (instance)
   MDFNNGPC_SetSoundRate(RETRO_SAMPLE_RATE);

   update_video = false;
   update_audio = false;

   return game;
}

void DuoRunner::UnloadGame()
{
   if (!game)
      return;

   if (MDFNGameInfo)
   {
       rom_unload();
       if (NGPGfx)
           free(NGPGfx);
       NGPGfx = NULL;
       MDFNGI_reset(MDFNGameInfo);
   }

   if (surf)
   {
      if (surf->pixels)
         free(surf->pixels);
      free(surf);
   }
   surf = NULL;
}

void DuoRunner::Run()
{
   int total = 0;
   int32 SoundBufSize;
   unsigned width, height;
   static int16_t sound_buf[0x10000];
   static MDFN_Rect rects[FB_MAX_HEIGHT];
   EmulateSpecStruct spec = {0};
   bool updated = false;

   // Check vars (Runner)

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
      check_variables();

   // Update input (Runner)

   input_poll_cb();
   update_input();

   rects[0].w              = ~0;

   // Prepare spec (Runner)

   spec.surface            = surf;
   spec.SoundRate          = RETRO_SAMPLE_RATE;
   spec.SoundBuf           = sound_buf;
   spec.LineWidths         = rects;
   spec.SoundBufMaxSize    = sizeof(sound_buf) / 2;
   spec.SoundVolume        = 1.0;
   spec.soundmultiplier    = 1.0;
   spec.SoundBufSize       = 0;
   spec.VideoFormatChanged = update_video;
   spec.SoundFormatChanged = update_audio;

   // Update AV if needed (Runner)

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

      surf->depth = RETRO_PIX_DEPTH;

      update_video = false;
      update_audio = false;
   }

   // TODO foreach instance

   // Process frame (Instance)

   ProcessFrame(&spec);

   // Mix AV (Runner)

   SoundBufSize    = spec.SoundBufSize - spec.SoundBufSizeALMS;

   spec.SoundBufSize = spec.SoundBufSizeALMS + SoundBufSize;

   width  = spec.DisplayRect.w;
   height = spec.DisplayRect.h;

   video_cb(surf->pixels, width, height, FB_WIDTH * RETRO_PIX_BYTES);

   video_frames++;
   audio_frames += spec.SoundBufSize;

   for (total = 0; total < spec.SoundBufSize; )
      total += audio_batch_cb(spec.SoundBuf + total*2, spec.SoundBufSize - total);
}

// -----
// Input
// -----

void DuoRunner::UpdateInput(void)
{
   // TODO iterate each instance
   static unsigned map[] = {
      RETRO_DEVICE_ID_JOYPAD_UP, //X Cursor horizontal-layout games
      RETRO_DEVICE_ID_JOYPAD_DOWN, //X Cursor horizontal-layout games
      RETRO_DEVICE_ID_JOYPAD_LEFT, //X Cursor horizontal-layout games
      RETRO_DEVICE_ID_JOYPAD_RIGHT, //X Cursor horizontal-layout games
      RETRO_DEVICE_ID_JOYPAD_B,
      RETRO_DEVICE_ID_JOYPAD_A,
      RETRO_DEVICE_ID_JOYPAD_START,
   };
   unsigned i, j;
   int16_t ret = 0;
   input_buf = 0;

   if (DuoRunner::shared.libretro_supports_bitmasks)
      ret = DuoRunner::shared.input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
   else
   {
      for (j = 0; j < (RETRO_DEVICE_ID_JOYPAD_R3+1); j++)
         if (DuoRunner::shared.input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, j))
            ret |= (1 << j);
   }

   for (i = 0; i < MAX_BUTTONS; i++)
      if ((map[i] != -1) && (ret & (1 << map[i])))
         input_buf |= (1 << i);
}

// ----
// Info
// ----

void DuoRunner::GetSystemInfo(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name     = MEDNAFEN_CORE_NAME;
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif

#ifdef LOAD_FROM_MEMORY
   info->need_fullpath    = false;
#else
   info->need_fullpath    = true;
#endif

   info->library_version  = MEDNAFEN_CORE_VERSION GIT_VERSION;
   info->valid_extensions = MEDNAFEN_CORE_EXTENSIONS;
   info->block_extract    = false;
}

void DuoRunner::GetAvInfo(struct retro_system_av_info *info)
{
   memset(info, 0, sizeof(*info));
   info->timing.fps            = MEDNAFEN_CORE_TIMING_FPS;
   info->timing.sample_rate    = RETRO_SAMPLE_RATE;
   info->geometry.base_width   = MEDNAFEN_CORE_GEOMETRY_BASE_W;
   info->geometry.base_height  = MEDNAFEN_CORE_GEOMETRY_BASE_H;
   info->geometry.max_width    = MEDNAFEN_CORE_GEOMETRY_MAX_W;
   info->geometry.max_height   = MEDNAFEN_CORE_GEOMETRY_MAX_H;
   info->geometry.aspect_ratio = MEDNAFEN_CORE_GEOMETRY_ASPECT_RATIO;

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

      if(!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb888))
      {
         if(log_cb) log_cb(RETRO_LOG_ERROR, "Pixel format XRGB8888 not supported by platform.\n");

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
         if(log_cb) log_cb(RETRO_LOG_INFO, "Frontend supports RGB565 - will use that instead of XRGB1555.\n");

         RETRO_PIX_DEPTH = 16;
      }
#else
      enum retro_pixel_format rgb555 = RETRO_PIXEL_FORMAT_0RGB1555;

      if (environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb555))
      {
         if(log_cb) log_cb(RETRO_LOG_INFO, "Using default 0RGB1555 pixel format.\n");

         RETRO_PIX_DEPTH = 15;
      }
#endif
   }
}

void DuoRunner::CheckVariables(void)
{
   struct retro_variable var = {0};

   var.key   = "ngp_language";
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

MDFNGI *DuoRunner::LoadGameFromFile(const char *name)
{
   MDFNFILE *GameFile = file_open(name);

   if(!GameFile)
      goto error;

   // TODO foreach instance
   if(Load(name, GameFile, NULL, 0) <= 0)
      goto error;

   file_close(GameFile);
   GameFile     = NULL;

   return MDFNGameInfo;

error:
   if (GameFile)
      file_close(GameFile);
   GameFile     = NULL;
   MDFNGI_reset(MDFNGameInfo);
   return(0);
}

MDFNGI *DuoRunner::LoadGameFromData(const uint8_t *data, size_t size)
{
   // TODO foreach instance
    if (Load("", NULL, data, size) <= 0)
        goto error;

    return MDFNGameInfo;

error:
    MDFNGI_reset(MDFNGameInfo);
    return(0);
}

// -------------
// Serialization
// -------------

int DuoRunner::SaveStateAction(StateMem *sm, int load, int data_only)
{
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
}
