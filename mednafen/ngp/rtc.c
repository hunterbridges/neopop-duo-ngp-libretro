//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version. See also the license.txt file for
//      additional informations.
//---------------------------------------------------------------------------

#include "../mednafen-types.h"
#include <time.h>
#include "rtc.h"

static void update_rtc_latch(neopop_rtc_t *rtc)
{
   uint8 low, high;

   struct tm *localTime;
   time_t long_time;

   //Get the system time
   time(&long_time);
   localTime = localtime(&long_time);
   if (localTime)
   {
      low = localTime->tm_year - 100; high = low;             /* Years */
      rtc->rtc_latch[0x00] = ((high / 10) << 4) | (low % 10);

      low = localTime->tm_mon + 1; high = low;                /* Months */
      rtc->rtc_latch[0x01] = ((high / 10) << 4) | (low % 10);

      low = localTime->tm_mday; high = low;                   /* Days */
      rtc->rtc_latch[0x02] = ((high / 10) << 4) | (low % 10);

      low = localTime->tm_hour; high = low;                   /* Hours */
      rtc->rtc_latch[0x03] = ((high / 10) << 4) | (low % 10);

      low = localTime->tm_min; high = low;                    /* Minutes */
      rtc->rtc_latch[0x04] = ((high / 10) << 4) | (low % 10);

      low = localTime->tm_sec; high = low;                    /* Seconds */
      rtc->rtc_latch[0x05] = ((high / 10) << 4) | (low % 10);

      rtc->rtc_latch[0x06] = ((rtc->rtc_latch[0x00] % 4)<<4) | (localTime->tm_wday & 0x0F);
   }
}

uint8 rtc_read8(neopop_rtc_t *rtc, uint32 address)
{
   if(address >= 0x0091 && address <= 0x0097)
   {
      if(address == 0x0091)
         update_rtc_latch(rtc);

      return rtc->rtc_latch[address - 0x0091];
   }
   return 0;
}
