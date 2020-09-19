#ifndef __NGP_RTC_H
#define __NGP_RTC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* NOTE: C-style interface */
typedef struct neopop_rtc
{
   uint8 rtc_latch[7];
} neopop_rtc_t;

uint8_t rtc_read8(neopop_rtc_t *rtc, uint32_t address);

#ifdef __cplusplus
}
#endif

#endif
