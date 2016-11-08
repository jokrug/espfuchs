// Copyright 2015 by Thorsten von Eicken, see LICENSE.txt
/* Configuration stored in flash */

#include <esp8266.h>
#include <osapi.h>
#include "config.h"
#include "espfs.h"
#include "clocktimer.h"
//#include "crc16.h"

#define FLASH_MAGIC  (0xaa55)

FlashConfig flashConfig;

typedef union {
  FlashConfig fc;
  uint8_t     block[1024];
} FlashFull;

// magic number to recognize thet these are our flash settings as opposed to some random stuff
#define FLASH_MAGIC  (0xaa55)

// size of the setting sector
#define FLASH_SECT   (4096)


bool ICACHE_FLASH_ATTR configSave(void) {
  flashConfig.seq += 1;
  if( system_param_save_with_protect(ESP_PARAM_START_SEC, &flashConfig, sizeof(flashConfig)) )
    return true;

  os_printf("*** Failed to save config ***\n");
  return false;
}

bool ICACHE_FLASH_ATTR configRestore(void) {
  if( system_param_load(ESP_PARAM_START_SEC, 0, &flashConfig, sizeof(flashConfig)) &&
     (flashConfig.magic == FLASH_MAGIC) )
    return true;
  os_printf("*** Failed to restore config. Set defaults! ***\n");
  configDefault();
  return false;
}

bool ICACHE_FLASH_ATTR configDefault(void) {
    ScheduleEntry schedEntry = {1, 0, khz3555, STANDARD5_1MINSEND4MINPAUSE };

    FlashConfig flashDefault = {
      .seq = 33, .magic = FLASH_MAGIC, .crc = 0,
      .baud_rate    = 115200,
      .data_bits	= 8,
      .parity	    = 0,
      .stop_bits	= 1,
      .foxNo        = -1,
      .schedule[0]  = schedEntry,
      .noOfFoxes    = 5,
      .sendSec      = 60,
      .pauseSec     = 4*60,
    };

  if( system_param_save_with_protect(ESP_PARAM_START_SEC, &flashDefault, sizeof(flashDefault)) )
    return true;

  os_printf("*** Failed to save config ***\n");
  return false;
}

void ICACHE_FLASH_ATTR configToString(void) {
  os_printf("seq: %u, magic: %u, crc: %u\n", flashConfig.seq, flashConfig.magic, flashConfig.crc);
  os_printf("baud: %d, bits: %c, parity: %c\n", flashConfig.baud_rate, flashConfig.data_bits, flashConfig.parity);
  os_printf("foxNo: %c, noFoxes: %c, send: %d, pause: %d\n", flashConfig.foxNo, flashConfig.noOfFoxes, flashConfig.sendSec, flashConfig.pauseSec);
  char bufStart[20];
  ScheduleEntry* sched = &flashConfig.schedule[0];

  time_tToString(sched->startTime, bufStart);
  char bufStop[20];
  time_tToString(sched->stopTime, bufStop);
  os_printf("1. start: %s, stop: %s, freq: %d, mode: %d\n", bufStart, bufStop,
          flashConfig.schedule[0].frequency, flashConfig.schedule[0].workingMode);
}
