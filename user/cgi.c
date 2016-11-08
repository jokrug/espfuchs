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
  int len = 1;
  
  if (connData->conn==NULL) {
    //Connection aborted. Clean up.
    return HTTPD_CGI_DONE;
  }  
  
  //len=httpdFindArg(connData->post->buff, "currenttime", buff, sizeof(buff));
  if (len!=0) {

    //Generate the header
    //We want the header to start with HTTP code 200, which means the document is found.
    httpdStartResponse(connData, 200); 
    //We are going to send some HTML.
    httpdHeader(connData, "Content-Type", "text/xml");
    //No more headers.
    httpdEndHeaders(connData);


    struct tm nowTime;
    now( &nowTime );
    struct tm startTime;
    time_tToStructTm( flashConfig.schedule[0].startTime, &startTime );
    struct tm stopTime;
    time_tToStructTm( flashConfig.schedule[0].startTime, &stopTime );

    char battVb[20];
    getBattVoltageAsString(battVb);
    os_sprintf(buff, "<?xml version='1.0'?>"
                     "<esp_state>"
                       "<curtime>%02d %02d:%02d:%02d</curtime>"
                       "<starttime>%02d %02d:%02d:%02d</starttime>"
                       "<stoptime>%02d %02d:%02d:%02d</stoptime>"
                       "<battvb>%s</battvb>"
                     "</esp_state>", 
               nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec,
               startTime.tm_mday, startTime.tm_hour, startTime.tm_min, startTime.tm_sec,
               stopTime.tm_mday, stopTime.tm_hour, stopTime.tm_min, stopTime.tm_sec,
               battVb );
    os_printf(buff);
  }

  httpdSend(connData, buff, -1);
  return HTTPD_CGI_DONE;
}


//cgi to set the current time
int ICACHE_FLASH_ATTR cgiSetTime(HttpdConnData *connData) {
  char stringBuf[64];

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
    flashConfig.schedule[0].startTime = newTime;
  }

  bytes = httpdFindArg(connData->getArgs, "setstoptime", stringBuf, sizeof(stringBuf));
  os_printf("setstarttime Bytes: %d\n", bytes);
  if( bytes > 0 )
  {
    os_printf("Neue Stopzeit: %s\n", stringBuf);
    time_t newTime = strToTime_t( stringBuf );
    flashConfig.schedule[0].stopTime = newTime;
  }

  bytes = httpdFindArg(connData->getArgs, "frequency_enum", stringBuf, sizeof(stringBuf));
  os_printf("cgifrequency_enum Bytes: %d\n", bytes);
  if( bytes > 0 )
  {
    char* sBuf = stringBuf;
    int enumFrq = atoint( &sBuf );
    os_printf("Neue Frequenz: %s, dezimal: %d\n", stringBuf, enumFrq);
    initI2S(enumFrq);
    flashConfig.schedule[0].frequency = enumFrq;
  }

  httpdRedirect(connData, "index.tpl");

  configSave();
  configToString();

  return HTTPD_CGI_DONE;
}


static long hitCounter=0;

//Template code for the counter on the index page.
int ICACHE_FLASH_ATTR tplCounter(HttpdConnData *connData, char *token, void **arg)
{
    char buff[128];
    if (token==NULL) return HTTPD_CGI_DONE;

    if (os_strcmp(token, "counter")==0)
    {
        hitCounter++;
        os_sprintf(buff, "%ld", hitCounter);
        httpdSend(connData, buff, -1);

    } else if (os_strcmp(token, "deviceNo")==0)
    {
        os_sprintf(buff, "%d", flashConfig.foxNo);
        httpdSend(connData, buff, -1);
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
