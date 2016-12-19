
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../user/oscillator.h"
#include <time.h>

/******************************************************************************
 * Copyright 2013-2015 Espressif Systems
 *           2015 <>< Charles Lohr
 *
 * FileName: i2s_freertos.c
 *
 * Description: I2S output routines for a FreeRTOS system. Uses DMA and a queue
 * to abstract away the nitty-gritty details.
 *
 * Modification history:
 *     2015/06/01, v1.0 File created.
 *     2015/07/23, Switch to making it a WS2812 output device.
 *     2016/01/28, Modified to re-include TX_ stuff.
*******************************************************************************

Notes:

 This is pretty badly hacked together from the MP3 example.
 I spent some time trying to strip it down to avoid a lot of the TX_ stuff.
 That seems to work.

 Major suggestions that I couldn't figure out:
    * Use interrupts to disable DMA, so it isn't running nonstop.
    * Use interrupts to flag when new data can be sent.

 When I try using interrupts, it seems to work for a bit but things fall apart
 rather quickly and the engine just refuses to send anymore until reboot.

 The way it works right now is to keep the DMA running forever and just update
 the data in the buffer so it continues sending the frame.

Extra copyright info:
  Actually not much of this file is Copyright Espressif, comparativly little
  mostly just the stuff to make the I2S bus go.

*******************************************************************************/

#define WS_I2S_BCK 1  //Can't be less than 1.
#define WS_I2S_DIV 2

#define I2SDMABUFLEN (69)		//Length of one buffer, in 32-bit words.

#define ICACHE_FLASH_ATTR
#define os_memset memset
#define os_printf printf
#define os_sprintf sprintf

//Bit clock @ 40MHz = 25ns
//WS_I2S_DIV - if 1 will actually be 2.  Can't be less than 2.

//		CLK_I2S = 160MHz / I2S_CLKM_DIV_NUM
//		BCLK = CLK_I2S / I2S_BCK_DIV_NUM
//		WS = BCLK/ 2 / (16 + I2S_BITS_MOD)
//		Note that I2S_CLKM_DIV_NUM must be >5 for I2S data
//		I2S_CLKM_DIV_NUM - 5-127
//		I2S_BCK_DIV_NUM - 2-127


//40MHz / 11 = 3,6363MHz
//40MHz / 16 =
//I2S DMA buffer descriptors
//static struct sdio_queue i2sBufDesc;
unsigned int i2sBD[I2SDMABUFLEN];
int bitFieldSize = 0;
int dataLen = 0;

int setBitPattern()
{
  os_memset( i2sBD, 0x00, sizeof(i2sBD));

  int freqBits = 11;           //40MHz / 11  = 3,6363...MHz
  int ritBit = 5;             //40MHz / 110 = 0,3636MHz
  int bitCount = 0;
  int ritBitCount = 0;
  do
  {
    if(ritBitCount >= ritBit)
    {
      //setBit(bitCount++, 0);
      ritBitCount = 0;
    }
    int i2;
    for( i2 = 0; i2<freqBits; i2++)
    {//
      setBit(bitCount++, 0);
    }
    int i3;
    for(i3 = 0; i3<freqBits; i3++)
    {
      setBit(bitCount++, 1);
    }
  }while(((bitCount>>5) < I2SDMABUFLEN) &&
         (( bitCount<=32 )                 ||//nicht Zeile 0 mit sich selbst vergleichen.
          (i2sBD[0]     != i2sBD[((bitCount-1)>>5)])) );

  os_printf("bitCount: %d\n", bitCount);

  return( bitCount );
}


void ICACHE_FLASH_ATTR setBit( int bitNr, bool val)
{
  if( val )
    i2sBD[bitNr >> 5] |= 0x80000000 >> (bitNr % 32);
  else
    i2sBD[bitNr >> 5] &= ~(0x80000000 >> (bitNr % 32));
}

int getBit( int bitNr)
{
  return( (i2sBD[bitNr >> 5] >> (bitNr % 32)) & 1);
}

void printBitField()
{
    os_printf("\n#############################\n");
    os_printf("bitFieldSize: %d, dataLen: %d\n", bitFieldSize, dataLen);
    int idx;
    for(idx = 0; idx<I2SDMABUFLEN*32; idx++)
    {
      if((idx > 0) && (idx % 32) == 0 )
        os_printf("\n");

      if(getBit(idx)>0)
        os_printf("1");
      else
        os_printf("0");
    }
    os_printf("#############################\n");
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

bool ICACHE_FLASH_ATTR time_tToStructTm(time_t tt, struct tm* tmBuf)
{
  if (tmBuf==NULL) return false;
  tmBuf->tm_mday = tt / 86400;
  tmBuf->tm_hour = tt / 3600;
  tmBuf->tm_min  = (tt%3600) / 60;
  tmBuf->tm_sec  = (tt%3600)%60;
  return true;
}

void ICACHE_FLASH_ATTR time_tToString(time_t tt, char* buf)
{
  struct tm tmS;
  time_tToStructTm( tt, &tmS );
  os_sprintf(buf, "%d %02d:%02d:%02d",
             tmS.tm_mday, tmS.tm_hour, tmS.tm_min, tmS.tm_sec);
}

int main(int argc, char *argv[])
{
    printf("Unittest\n");
    //setBitPattern();
    char* str = "22:15T50";
    int i = atoint( &str );
    struct tm zeit;

    bool ret = strToStructTm("22:15:56", &zeit);
    time_t tmet = strToTime_t("22:15:56");
    memset( &zeit, 0, sizeof(zeit));
    time_tToStructTm(tmet,  &zeit);
    char strBuf[100];
    time_tToString(tmet, strBuf);

    return 0;
}
