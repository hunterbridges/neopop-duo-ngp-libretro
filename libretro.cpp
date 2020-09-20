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

#include <libretro.h>
#include <streams/file_stream.h>

#include "libretro_core_options.h"
#include "duo/duo_runner.h"

#include "mednafen/mednafen.h"
#include "mednafen/git.h"
#include "mednafen/ngp/neopop.h"
#include "mednafen/general.h"

#include "mednafen/ngp/TLCS-900h/TLCS900h_interpret.h"
#include "mednafen/ngp/TLCS-900h/TLCS900h_registers.h"
#include "mednafen/ngp/Z80_interface.h"
#include "mednafen/ngp/interrupt.h"
#include "mednafen/ngp/mem.h"
#include "mednafen/ngp/gfx.h"
#include "mednafen/ngp/sound.h"
#include "mednafen/ngp/dma.h"
#include "mednafen/ngp/bios.h"
#include "mednafen/ngp/flash.h"
#include "mednafen/ngp/system.h"

extern "C" char retro_base_directory[1024];
extern "C" char retro_save_directory[1024];

COLOURMODE system_colour = COLOURMODE_AUTO;

MDFNGI EmulatedNGP = {};
MDFNGI *MDFNGameInfo = &EmulatedNGP;

static const MDFNSetting_EnumList LanguageList[] =
{
 { "japanese", 0, "Japanese" },
 { "0", 0 },

 { "english", 1, "English" },
 { "1", 1 },

 { NULL, 0 },
};

static MDFNSetting NGPSettings[] =
{
 { "ngp.language", MDFNSF_EMU_STATE | MDFNSF_UNTRUSTED_SAFE, "Language games should display text in.", NULL, MDFNST_ENUM, "english", NULL, NULL, NULL, NULL, LanguageList },
 { NULL }
};

static const InputDeviceInputInfoStruct IDII[] =
{
 { "up", "UP ↑", 0, IDIT_BUTTON, "down" },
 { "down", "DOWN ↓", 1, IDIT_BUTTON, "up" },
 { "left", "LEFT ←", 2, IDIT_BUTTON, "right" },
 { "right", "RIGHT →", 3, IDIT_BUTTON, "left" },
 { "a", "A", 5, IDIT_BUTTON_CAN_RAPID,  NULL },
 { "b", "B", 6, IDIT_BUTTON_CAN_RAPID, NULL },
 { "option", "OPTION", 4, IDIT_BUTTON, NULL },
};
static InputDeviceInfoStruct InputDeviceInfo[] =
{
 {
  "gamepad",
  "Gamepad",
  NULL,
  NULL,
  sizeof(IDII) / sizeof(InputDeviceInputInfoStruct),
  IDII,
 }
};

static const InputPortInfoStruct PortInfo[] =
{
 { "builtin", "Built-In", sizeof(InputDeviceInfo) / sizeof(InputDeviceInfoStruct), InputDeviceInfo, "gamepad" }
};

static InputInfoStruct InputInfo =
{
 sizeof(PortInfo) / sizeof(InputPortInfoStruct),
 PortInfo
};

static const FileExtensionSpecStruct KnownExtensions[] =
{
 { ".ngp", "Neo Geo Pocket ROM Image" },
 { ".ngc", "Neo Geo Pocket Color ROM Image" },
 { NULL, NULL }
};

// -----------------
// Private Functions
// -----------------

// TODO Put this here or in DuoRunner?
static void MDFNGI_reset(MDFNGI *gameinfo)
{
   gameinfo->Settings = NGPSettings;
   gameinfo->MasterClock = MDFN_MASTERCLOCK_FIXED(6144000);
   gameinfo->fps = 0;
   gameinfo->multires = false; // Multires possible?
  
   gameinfo->lcm_width = 160;
   gameinfo->lcm_height = 152;
   gameinfo->dummy_separator = NULL;
  
   gameinfo->nominal_width = 160;
   gameinfo->nominal_height = 152;
  
   gameinfo->fb_width = 160;
   gameinfo->fb_height = 152;
  
   gameinfo->soundchan = 2;
}

// ----------------------
// IO Extern (system.cpp)
// ----------------------

#ifdef _WIN32
static void sanitize_path(std::string &path)
{
   size_t size = path.size();
   for (size_t i = 0; i < size; i++)
      if (path[i] == '/')
         path[i] = '\\';
}
#endif

// Use a simpler approach to make sure that things go right for libretro.
std::string MDFN_MakeFName(MakeFName_Type type, int id1, const char *cd1)
{
   char slash;
#ifdef _WIN32
   slash = '\\';
#else
   slash = '/';
#endif
   std::string ret;
   switch (type)
   {
      case MDFNMKF_SAV:
         ret = std::string(retro_save_directory) + slash + std::string(DuoRunner::shared.retro_base_name) +
            std::string(".") + std::string(cd1);
         break;
      case MDFNMKF_FIRMWARE:
         ret = std::string(retro_base_directory) + slash + std::string(cd1);
#ifdef _WIN32
   sanitize_path(ret); // Because Windows path handling is mongoloid.
#endif
         break;
      default:	  
         break;
   }

   if (DuoRunner::shared.log_cb)
      DuoRunner::shared.log_cb(RETRO_LOG_INFO, "MDFN_MakeFName: %s\n", ret.c_str());
   return ret;
}

// --------------
// libretro hooks
// --------------

void retro_set_controller_port_device(unsigned in_port, unsigned device)
{
   // Blank
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   // Blank
}

void *retro_get_memory_data(unsigned type)
{
   // Blank
   return NULL;
}

size_t retro_get_memory_size(unsigned type)
{
   // Blank
   return 0;
}

void retro_cheat_reset(void)
{
   // Blank
}

void retro_cheat_set(unsigned, bool, const char *)
{
   // Blank
}

bool retro_load_game_special(unsigned, const struct retro_game_info *, size_t)
{
   // Blank
   return false;
}

void retro_set_environment(retro_environment_t cb)
{
   struct retro_vfs_interface_info vfs_iface_info;
   DuoRunner::shared.environ_cb = cb;

   libretro_set_core_options(DuoRunner::shared.environ_cb);

   vfs_iface_info.required_interface_version = 1;
   vfs_iface_info.iface                      = NULL;
   if (DuoRunner::shared.environ_cb(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, &vfs_iface_info))
	   filestream_vfs_init(&vfs_iface_info);
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   DuoRunner::shared.audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   DuoRunner::shared.input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   DuoRunner::shared.input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   DuoRunner::shared.video_cb = cb;
}

void retro_init(void)
{
   DuoRunner::shared.Initialize();
}

void retro_reset(void)
{
   DuoRunner::shared.Reset();
}

bool retro_load_game(const struct retro_game_info *info)
{
   return DuoRunner::shared.LoadGame(info);
}

void retro_unload_game(void)
{
   DuoRunner::shared.UnloadGame();
}

void retro_run(void)
{
   DuoRunner::shared.Run();
}

void retro_get_system_info(struct retro_system_info *info)
{
   return DuoRunner::shared.GetSystemInfo(info);
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   return DuoRunner::shared.GetAvInfo(info);
}

unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_deinit(void)
{
   DuoRunner::shared.Deinitialize();
}

// -----------------------
// Serialization Interface
// -----------------------

int StateAction(StateMem *sm, int load, int data_only)
{
   return DuoRunner::shared.SaveStateAction(sm, load, data_only);
}

size_t retro_serialize_size(void)
{
   StateMem st;

   st.data           = NULL;
   st.loc            = 0;
   st.len            = 0;
   st.malloced       = 0;
   st.initial_malloc = 0;

   if (!MDFNSS_SaveSM(&st, 0, 0, NULL, NULL, NULL))
      return 0;

   free(st.data);

   return st.len;
}

bool retro_serialize(void *data, size_t size)
{
   StateMem st;
   bool ret          = false;
   uint8_t *_dat     = (uint8_t*)malloc(size);

   if (!_dat)
      return false;

   /* Mednafen can realloc the buffer so we need to ensure this is safe. */
   st.data           = _dat;
   st.loc            = 0;
   st.len            = 0;
   st.malloced       = size;
   st.initial_malloc = 0;

   ret = MDFNSS_SaveSM(&st, 0, 0, NULL, NULL, NULL);

   memcpy(data, st.data, size);
   free(st.data);

   return ret;
}

bool retro_unserialize(const void *data, size_t size)
{
   StateMem st;

   st.data           = (uint8_t*)data;
   st.loc            = 0;
   st.len            = size;
   st.malloced       = 0;
   st.initial_malloc = 0;

   return MDFNSS_LoadSM(&st, 0, 0);
}

