#include <time.h>
//#include "clocktimer.h"

#include <time.h>
#include <stdio.h>

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


//! \brief Fill the tm-object from usTime in microseconds.
void nowFromUs(unsigned long usTime, struct tm* tmObj)
{
  const int cseconds_in_day = 86400;
  const int cseconds_in_hour = 3600;
  const int cseconds_in_minute = 60;

  int sysSecs = usTime  / 1000000;
  
  tmObj->tm_mday = sysSecs / cseconds_in_day;
  tmObj->tm_hour = (sysSecs % cseconds_in_day) / cseconds_in_hour;
  tmObj->tm_min  = (sysSecs % cseconds_in_hour) / cseconds_in_minute;
  tmObj->tm_sec  = sysSecs % cseconds_in_minute;
}



int main(int argc, char* argv[])
{
  struct tm gmtm;
  unsigned long timeUs = 0;
  char* dt;
  
  if(argc > 1)
    sscanf( argv[1], "%lu", &timeUs );
  
  nowFromUs( timeUs, &gmtm );
  
  dt = asctime( &gmtm );
  printf("The UTC date and time is: %s\n", dt);
  
}
