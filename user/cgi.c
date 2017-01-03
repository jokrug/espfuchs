/*
Some random cgi routines. Used in the LED example and the page that returns the entire
flash as a binary. Also handles the hit counter on the main page.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */
#include <esp8266.h>
#include "cgi.h"
#include "config.h"
#include "clocktimer.h"
#include "oscillator.h"

//Cgi 
int ICACHE_FLASH_ATTR cgiCurrentState(HttpdConnData *connData) {
  char buff[1024];

  if (connData->conn==NULL) {
    //Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }  
  
  //Generate the header
  //We want the header to start with HTTP code 200, which means the document is found.
  httpdStartResponse(connData, 200);
  //We are going to send some HTML.
  httpdHeader(connData, "Content-Type", "text/xml");
  //No more headers.
  httpdEndHeaders(connData);

  struct tm nowTime;
  now( &nowTime );
  char battVb[20];
  getBattVoltageAsString(battVb);
  os_sprintf(buff, "<?xml version='1.0'?>"
                   "<esp_state>"
                     "<curtime>%02d %02d:%02d:%02d</curtime>"
                     "<battvb>%s</battvb>"
                   "</esp_state>",
             nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec,
             battVb );

  httpdSend(connData, buff, -1);
  return HTTPD_CGI_DONE;
}


//cgi to set the current time
int ICACHE_FLASH_ATTR cgiSetFoxParams(HttpdConnData *connData)
{
  char stringBuf[64];
  ScheduleEntry* schedEntry = &flashConfig.schedule[0];

  if (connData->conn==NULL)
  {    //Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }
  
  int bytes = httpdFindArg(connData->getArgs, "setcurrtime", stringBuf, sizeof(stringBuf));
  os_printf("cgiSetTime Bytes: %d\n", bytes);
  if( bytes > 0 )
  {
    os_printf("Neue Zeit: %s\n", stringBuf);
    struct tm newTm;
    os_memset( &newTm, 0, sizeof(newTm) );
    strToStructTm( stringBuf, &newTm );
    setTime( &newTm );
  }

  bytes = httpdFindArg(connData->getArgs, "setstarttime", stringBuf, sizeof(stringBuf));
  os_printf("setstarttime Bytes: %d\n", bytes);
  if( bytes > 0 )
  {
    os_printf("Neue Startzeit: %s\n", stringBuf);
    time_t newTime = strToTime_t( stringBuf );
    schedEntry->startTime = newTime;
  }

  bytes = httpdFindArg(connData->getArgs, "setstoptime", stringBuf, sizeof(stringBuf));
  os_printf("setstarttime Bytes: %d\n", bytes);
  if( bytes > 0 )
  {
    os_printf("Neue Stopzeit: %s\n", stringBuf);
    time_t newTime = strToTime_t( stringBuf );
    schedEntry->stopTime = newTime;
  }

  bytes = httpdFindArg(connData->getArgs, "frequency_enum", stringBuf, sizeof(stringBuf));
  os_printf("cgifrequency_enum Bytes: %d\n", bytes);
  if( bytes > 0 )
  {
    int enumFrq = atoi( stringBuf );
    if(enumFrq > 0)
    {
      os_printf("new frequency: %s, dezimal: %d\n", stringBuf, enumFrq);
      setFrequency(enumFrq);
      schedEntry->frequency = enumFrq;
    }
  }

  bytes = httpdFindArg(connData->getArgs, "settransmittime", stringBuf, sizeof(stringBuf));
  os_printf("settransmittime Bytes: %d\n", bytes);
  if( bytes > 0 )
  {
    os_printf("new transmit time: %s\n", stringBuf);
    int newTime = atoi( stringBuf );
    schedEntry->transmitSec = newTime;
  }

  bytes = httpdFindArg(connData->getArgs, "setpausetime", stringBuf, sizeof(stringBuf));
  os_printf("setpausetime Bytes: %d\n", bytes);
  if( bytes > 0 )
  {
    os_printf("new pause time: %s\n", stringBuf);
    int newTime = atoi( stringBuf );
    schedEntry->pauseSec = newTime;
  }

  bytes = httpdFindArg(connData->getArgs, "setmorsespeed", stringBuf, sizeof(stringBuf));
  os_printf("setmorsespeed Bytes: %d\n", bytes);
  if( bytes > 0 )
  {
    os_printf("new morse speed: %s\n", stringBuf);
    int newMorseSpeed = atoi( stringBuf );
    schedEntry->morseSpeed = newMorseSpeed;
  }

  bytes = httpdFindArg(connData->getArgs, "setcall", stringBuf, sizeof(stringBuf));
  os_printf("setcall Bytes: %d\n", bytes);
  if( bytes > 0 )
  {
    os_printf("new call: %s\n", stringBuf);
    os_sprintf(schedEntry->morseCall, stringBuf);
  }

  httpdRedirect(connData, (char*)connData->cgiArg);

  configSave();
  printCconfig();

  return HTTPD_CGI_DONE;
}

//Template code for the index page.
int ICACHE_FLASH_ATTR tplInfo(HttpdConnData *connData, char *token, void **arg)
{
  char buff[128];
  if (token==NULL) return HTTPD_CGI_DONE;

  if (os_strcmp(token, "freq")==0)
  {
      frequencyEnumToString(flashConfig.schedule[0].frequency, buff);
      httpdSend(connData, buff, -1);
  } else if (os_strcmp(token, "foxno")==0)
  {
      os_sprintf(buff, "%d", flashConfig.foxNo);
      httpdSend(connData, buff, -1);
//  } else if (os_strcmp(token, "foxmode")==0)
//  {
//      foxModeToStr( flashConfig.schedule[0].workingMode, buff );
//      httpdSend(connData, buff, -1);
  } else if (os_strcmp(token, "starttime")==0)
  {
      time_tToString( flashConfig.schedule[0].startTime, buff );
      httpdSend(connData, buff, -1);
  } else if (os_strcmp(token, "stoptime")==0)
  {
      time_tToString( flashConfig.schedule[0].stopTime, buff );
      httpdSend(connData, buff, -1);
  } else if (os_strcmp(token, "transmittime")==0)
  {
    os_sprintf(buff, "%d", flashConfig.schedule[0].transmitSec);
    httpdSend(connData, buff, -1);
  } else if (os_strcmp(token, "pausetime")==0)
  {
    os_sprintf(buff, "%d", flashConfig.schedule[0].pauseSec);
    httpdSend(connData, buff, -1);
  } else if (os_strcmp(token, "morsespeed")==0)
  {
    os_sprintf(buff, "%d", flashConfig.schedule[0].morseSpeed);
    httpdSend(connData, buff, -1);
  } else if (os_strcmp(token, "call")==0)
  {
    os_sprintf(buff, "%s", flashConfig.schedule[0].morseCall);
    httpdSend(connData, buff, -1);
  } else if (os_strcmp(token, "frequencyselectoptions")==0)
  {
    httpdSend(connData, frequencySelectOptions, -1);
  }
  return HTTPD_CGI_DONE;
}

int ICACHE_FLASH_ATTR tplFoxhunt(HttpdConnData *connData, char *token, void **arg)
{
  if (token==NULL) return HTTPD_CGI_DONE;

  if (os_strcmp(token, "frequencyselectoptions")==0)
  {
    httpdSend(connData, frequencySelectOptions, -1);
  }
  return HTTPD_CGI_DONE;
}

void getBattVoltageAsString(char* buff)
{
#define FILTERSIZE 8
  static unsigned int filterArray[FILTERSIZE];
  static int filterPtr = 0;
  
  filterArray[filterPtr] = system_adc_read();
  if( ++filterPtr >= FILTERSIZE )
    filterPtr = 0;
  
  unsigned int adcValue = 0;
  
  // Filtern
  for( int i=0; i<FILTERSIZE; i++ )
    adcValue += filterArray[i];
  adcValue /= FILTERSIZE;
  
  // Kalibrieren
  adcValue = adcValue * 100 / 95;
  
  // Formatieren
  int volt     = adcValue / 1000;
  int mVolt    = adcValue % 1000;
  os_sprintf(buff, "%02d.%03d", volt, mVolt ); 
}
