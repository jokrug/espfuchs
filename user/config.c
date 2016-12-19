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
  //SconfigToString();
  if( system_param_save_with_protect(ESP_PARAM_START_SEC, &flashConfig, sizeof(flashConfig)) )
    return true;

  os_printf("*** Failed to save config ***\n");
  return false;
}

bool ICACHE_FLASH_ATTR configRestore(void) {
  os_printf("Load config...\n");
  if( system_param_load(ESP_PARAM_START_SEC, 0, &flashConfig, sizeof(flashConfig)) &&
     (flashConfig.magic == FLASH_MAGIC) )
    return true;
  os_printf("*** Failed to restore config. Set defaults! ***\n");
  configDefault();
  return false;
}

bool ICACHE_FLASH_ATTR configDefault(void) {
    ScheduleEntry schedEntry = {3600, 7200, khz3555, STANDARD5_1MINSEND4MINPAUSE };

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

void ICACHE_FLASH_ATTR printScheduleEntry(ScheduleEntry* sched)
{
  char bufStart[10];
  time_tToString(sched->startTime, bufStart);
  char bufStop[10];
  time_tToString(sched->stopTime, bufStop);
  os_printf("start: %s (%d), stop: %s (%d), freqEnum: %d, modeEnum: %d\n",
            bufStart, (int)sched->startTime, bufStop, (int)sched->stopTime, sched->frequency, sched->workingMode);
}

void ICACHE_FLASH_ATTR printCconfig(void)
{
  int magic = flashConfig.magic;
  int crc = flashConfig.crc;
  int dataBits = flashConfig.data_bits;
  int foxNo = flashConfig.foxNo;
  int noOfFoxes = flashConfig.noOfFoxes;

  os_printf("\n---- flash configuration -----\n" );
  os_printf("seq: %u, magic: %x, crc: %x\n", flashConfig.seq, magic, crc);
  os_printf("baud: %d, bits: %d, parity: %d\n", flashConfig.baud_rate, dataBits, flashConfig.parity);
  os_printf("foxNo: %d, noFoxes: %d, send: %d, pause: %d\n", foxNo, noOfFoxes, flashConfig.sendSec, flashConfig.pauseSec);

  ScheduleEntry* sched = &flashConfig.schedule[0];
  printScheduleEntry( sched );
  os_printf("---- End flash configuration -----\n" );
}

void ICACHE_FLASH_ATTR foxModeToStr(enum WorkingMode mode, char* buf)
{
    switch(mode)
    {
      case STANDARD5_1MINSEND4MINPAUSE:
        os_sprintf(buf, "Standard5");
        break;
      case SPRINT2X5_12SECSEND48SECPAUSE:
        os_sprintf(buf, "Sprint2x5");
        break;
      case FOXORING_CONTINOUS:
        os_sprintf(buf, "Foxoring");
        break;
      case USER_DEFINED:
        os_sprintf(buf, "User defined");
        break;
      default:
        break;
    }
}
