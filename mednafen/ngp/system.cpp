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

