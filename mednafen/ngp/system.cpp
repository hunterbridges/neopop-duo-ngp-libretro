#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "flash.h"
#include "system.h"
#include "rom.h"

#include "../general.h"
#include "../state.h"

#include <streams/file_stream.h>
#include "../../duo/duo_runner.h"

// -----
// Comms
// -----

bool neopop_comms_t::system_comms_read(uint8_t* buffer)
{
   return 0;
}

bool neopop_comms_t::system_comms_poll(uint8_t* buffer)
{
   return 0;
}

void neopop_comms_t::system_comms_write(uint8_t data)
{
}

// --
// IO
// --

bool neopop_io_t::system_io_flash_read(uint8_t* buffer, uint32_t bufferLength)
{
   std::string pathStr = MDFN_MakeFName(MDFNMKF_SAV, 0, "flash");
   RFILE     *flash_fp = filestream_open(pathStr.c_str(),
         RETRO_VFS_FILE_ACCESS_READ, RETRO_VFS_FILE_ACCESS_HINT_NONE);

   if (!flash_fp)
      return 0;

   filestream_read(flash_fp, buffer, bufferLength);
   filestream_close(flash_fp);

   return 1;
}

bool neopop_io_t::system_io_flash_write(uint8_t *buffer, uint32_t bufferLength)
{
   std::string pathStr = MDFN_MakeFName(MDFNMKF_SAV, 0, "flash");
   RFILE     *flash_fp = filestream_open(pathStr.c_str(),
         RETRO_VFS_FILE_ACCESS_WRITE,
         RETRO_VFS_FILE_ACCESS_HINT_NONE);

   if (!flash_fp)
      return 0;

   filestream_write(flash_fp, buffer, bufferLength);
   filestream_close(flash_fp);

   return 1;
}

// ----------------
// Helper Functions
// ----------------

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
         ret = std::string(DuoRunner::shared.retro_save_directory) + slash + std::string(DuoRunner::shared.retro_base_name) +
            std::string(".") + std::string(cd1);
         break;
      case MDFNMKF_FIRMWARE:
         ret = std::string(DuoRunner::shared.retro_base_directory) + slash + std::string(cd1);
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
