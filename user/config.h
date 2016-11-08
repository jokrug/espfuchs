#ifndef CONFIG_H
#define CONFIG_H

#include <time.h>

#include <esp8266.h>
#include "oscillator.h" //wg. enum frequencies

/* NOTICE---this is for 512KB spi flash.
 * you can change to other sector if you use other size spi flash. */
#define ESP_PARAM_START_SEC		0x3D
#define SCHEDULE_ENTRIES        4

enum WorkingMode {
    STANDARD5_1MINSEND4MINPAUSE,
    SPRINT2X5_12SECSEND48SECPAUSE,
    FOXORING_CONTINOUS,
    USER_DEFINED
};

typedef struct {
  time_t   startTime;
  time_t   stopTime;
  enum Frequencies frequency;
  enum WorkingMode workingMode;
} ScheduleEntry;

// Flash configuration settings. When adding new items always add them at the end and formulate
// them such that a value of zero is an appropriate default or backwards compatible. Existing
// modules that are upgraded will have zero in the new fields. This ensures that an upgrade does
// not wipe out the old settings.
typedef struct {
  uint32 seq; // flash write sequence number
  uint16_t magic, crc;

  int32  baud_rate;
  int8   data_bits;
  int8   parity;
  int8   stop_bits;

  int8   foxNo;
  ScheduleEntry schedule[SCHEDULE_ENTRIES];

  //setting for user-defined  mode
  int8   noOfFoxes;
  int32   sendSec;
  int32   pauseSec;

} FlashConfig;

extern FlashConfig flashConfig;

bool configSave(void);
bool configRestore(void);
bool configDefault(void);
void configToString(void);

#endif
