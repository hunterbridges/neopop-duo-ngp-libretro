#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "flash.h"
#include "system.h"
#include "rom.h"
#include "mem.h"

#include "../general.h"
#include "../state.h"

#include <streams/file_stream.h>
#include "../../duo/duo_runner.h"
#include "../../duo/duo_instance.h"

// -----
// Comms
// -----

DuoInstance *GetOtherInstance(DuoInstance *fromInstance)
{
    if (fromInstance == &DuoInstance::instances[0])
        return &DuoInstance::instances[1];
    else
        return &DuoInstance::instances[0];
}

neopop_comms_t::neopop_comms_t()
{
    tx_buf = ringbuf_new(64);

    rx_timer = 0;
    rx_buf = ringbuf_new(64);
    receive = false;

	write_flag = false;

	read_flag = false;
}

neopop_comms_t::~neopop_comms_t()
{
    ringbuf_free(&rx_buf);
    rx_buf = NULL;
}

bool neopop_comms_t::system_comms_read(uint8_t* buffer)
{
   if (buffer == NULL)
	  return ringbuf_bytes_used(rx_buf) > 0;

   if (ringbuf_bytes_used(rx_buf) == 0)
      return false;

   ringbuf_memcpy_from(buffer, rx_buf, 1);
   return true;
}

bool neopop_comms_t::system_comms_poll(uint8_t* buffer, int32 tlcsCycles)
{
    static const int32 clock_rate = 6144000;
    static const int32 baud_rate = 19200;
    static const int32 cycles_per_bit = clock_rate / baud_rate;
    // static const int32 cycles_per_byte = 8 * cycles_per_bit;
    static const int32 cycles_per_byte = 1;

    DuoInstance *duo = GetDuoFromModule(this, comms);
    DuoInstance *other = GetOtherInstance(duo);

    uint8 tmp = 0xFF;

    rx_timer += tlcsCycles;
    if (rx_timer < cycles_per_byte)
      return false;

    //rx_timer %= cycles_per_byte;
    rx_timer = 0;

    if (ringbuf_bytes_used(other->comms->tx_buf) == 0)
        return false;

    ringbuf_memcpy_from(&tmp, other->comms->tx_buf, 1);

    if (receive == false)
        return false;

    if (ringbuf_bytes_free(rx_buf) == 0)
        return false;

    ringbuf_memcpy_into(rx_buf, &tmp, 1);
    if (buffer)
        *buffer = tmp;

    duo->mem->SC0BUF = tmp;

    return true;
}

void neopop_comms_t::system_comms_write(uint8_t data)
{
    if (write_flag == false)
        return;

    ringbuf_memcpy_into(tx_buf, &data, 1);
    write_flag = false;
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
