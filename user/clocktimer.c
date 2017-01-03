
#include "clocktimer.h"
#include "driver/hw_timer.h"

// struct tm
// {
//   int tm_sec;
//   int tm_min;
//   int tm_hour;
//   int tm_mday;
//   int tm_mon;
//   int tm_year;
//   int tm_wday;
//   int tm_yday;
//   int tm_isdst;
// };


//struct tm dateTime;
//SystemClockStatus status = eSCS_Initial;

static time_t currTime;

static bool checkIfTmIsValid(struct tm* tmBuf);
static void everySecond();
static time_t structTmToTimeT(struct tm* tmBuf);

time_t ICACHE_FLASH_ATTR time(time_t* t)
{
  if(t) *t=currTime;
  return currTime;
}

void ICACHE_FLASH_ATTR now( struct tm* tmObj )
{
  time_tToStructTm(currTime, tmObj);
}

void ICACHE_FLASH_ATTR setTime(struct tm* newTime)
{
  currTime = structTmToTimeT( newTime );
}

// Timer routine is called every 100ms
static void clockTimerCb()
{
  static int div10 = 10;

  --div10;
  if(div10 == 0)
  {
    everySecond();
    div10 = 10;
  }
}

static void everySecond()
{
  ++currTime;
  //Save the curent time to rtc-memory so that it
  //survives a reset.
  system_rtc_mem_write( 64, (void*)&currTime, sizeof(currTime) );
}

void ICACHE_FLASH_ATTR initClockTimer()
{
  os_printf("Init clockTime.\n");
  
  //Restore time from RTC memory and check if it is valid.
  system_rtc_mem_read(64, (void*)&currTime, sizeof(currTime));
  struct tm tmTime;
  time_tToStructTm(currTime, &tmTime);
  if( ! checkIfTmIsValid( &tmTime ) )
    currTime = 0;
  
  //Timer, 100ms.
  hw_timer_init(FRC1_SOURCE,1);
  hw_timer_set_func(clockTimerCb);
  hw_timer_arm(100000);
  clockTimerCb();
}


void ICACHE_FLASH_ATTR time_tToStructTm(time_t tt, struct tm* tmBuf)
{
  if (tmBuf==NULL) return;
  os_memset(tmBuf, 0, sizeof(struct tm));
  tmBuf->tm_mday = tt / 86400;
  tmBuf->tm_hour = (tt%86400) / 3600;
  tmBuf->tm_min  = (tt%3600) / 60;
  tmBuf->tm_sec  = (tt%3600)%60;
  return;
}

time_t ICACHE_FLASH_ATTR structTmToTimeT(struct tm* tmBuf)
{
  time_t timeT = tmBuf->tm_sec + tmBuf->tm_min*60 + tmBuf->tm_hour*3600 +
                 tmBuf->tm_mday*84600;
  return timeT;
}

int ICACHE_FLASH_ATTR atoint( char** pStr)
{
  int i=0;

  while( 1 )
  {//String durchlaufen
    char ziffer = (**pStr)-0x30;
    if( (ziffer < 0) || (ziffer > 9) )
      break;

    i *= 10;
    i += ziffer;
    ++*pStr;
  }
  return i;
}

bool ICACHE_FLASH_ATTR strToStructTm(char* str, struct tm* tmBuf)
{
  char *p;
  if (str==NULL) return false;
  if (tmBuf==NULL) return false;
  os_memset(tmBuf, 0, sizeof(struct tm));

  p=str;
  tmBuf->tm_hour = atoint( &p );
  ++p;
  tmBuf->tm_min  = atoint( &p );
  ++p;
  tmBuf->tm_sec  = atoint( &p );
   
  return true;
}

time_t ICACHE_FLASH_ATTR strToTime_t(char* str)
{
  char *p;
  time_t t = 0;
  if (str==NULL) return false;

  p=str;
  t  = atoint( &p )*3600;
  ++p;
  t += atoint( &p )*60;
  ++p;
  t += atoint( &p );

  return t;
}

void ICACHE_FLASH_ATTR time_tToString(time_t tt, char* buf)
{
  struct tm tmS;
  time_tToStructTm( tt, &tmS );
  os_sprintf(buf, "%dD%02d:%02d:%02d",
             tmS.tm_mday, tmS.tm_hour, tmS.tm_min, tmS.tm_sec);
}

//return true, if tm-struct is valid.
static bool ICACHE_FLASH_ATTR checkIfTmIsValid(struct tm* tmBuf)
{
  return( 
      (tmBuf->tm_sec  >= 0) && (tmBuf->tm_sec  < 60) &&
      (tmBuf->tm_min  >= 0) && (tmBuf->tm_min  < 60) &&
      (tmBuf->tm_hour >= 0) && (tmBuf->tm_hour < 24) &&
      (tmBuf->tm_mday >= 0) && (tmBuf->tm_mday < 32) &&
      (tmBuf->tm_mon  >= 0) && (tmBuf->tm_mon  < 13) );
}

