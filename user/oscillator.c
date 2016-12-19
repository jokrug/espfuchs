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

#include <esp8266.h>
#include "oscillator.h"
#include "slc_register.h"
#include <c_types.h>
#include "pin_mux_register.h"
#include "dmastuff.h"
#include "io.h"

#define I2SDMABUFLEN (100)		//Length of one buffer, in 32-bit words.

//Bit clock @ 40MHz = 25ns
//WS_I2S_DIV - if 1 will actually be 2.  Can't be less than 2.
//		CLK_I2S = 80MHz / I2S_CLKM_DIV_NUM
//		BCLK = CLK_I2S / I2S_BCK_DIV_NUM
//		WS = BCLK/ 2 / (16 + I2S_BITS_MOD)
//		Note that I2S_CLKM_DIV_NUM must be >5 for I2S data
//		I2S_CLKM_DIV_NUM - 5-127
//		I2S_BCK_DIV_NUM - 2-127

static int ws_i2s_bck = 1; //Can't be less than 1.
static int ws_i2s_div = 2;
static int freq1Bits = 16;    //80MHz / 16  = 5,000MHz
static int freq2Bits = 54;    //80MHz / 54  = 1,481MHz 5-1,481 =3,518MHz

static struct sdio_queue i2sBufDesc; //I2S DMA buffer descriptor

static uint32_t i2sBD[I2SDMABUFLEN];
static int bitFieldSize = 0;
static int dataLen = 0;
static int kgv = 0;

void ICACHE_FLASH_ATTR setFrequency(enum Frequencies freq)
{
  switch(freq)
  {
    case khz3478: //direct generated frequencies, no mix products
      ws_i2s_bck = 1;
      ws_i2s_div = 2;
      freq1Bits = 23;
      freq2Bits = 0;
      break;
    case khz3555:
      ws_i2s_bck = 1;
      ws_i2s_div = 3;
      freq1Bits = 15;
      freq2Bits = 0;
      break;
    case khz3636:
      ws_i2s_bck = 1;
      ws_i2s_div = 2;
      freq1Bits = 22;
      freq2Bits = 0;
      break;
    case khz3400: //generated by multiplying two frequencies
      ws_i2s_bck = 1;
      ws_i2s_div = 2;
      freq1Bits = 16;   //5.000MHz
      freq2Bits = 50;   //1.600MHz
      break;
    case khz3431:
      ws_i2s_bck = 1;
      ws_i2s_div = 2;
      freq1Bits = 16;
      freq2Bits = 51;
      break;
    case khz3462:
      ws_i2s_bck = 1;
      ws_i2s_div = 2;
      freq1Bits = 16;
      freq2Bits = 52;
      break;
    case khz3491:
      ws_i2s_bck = 1;
      ws_i2s_div = 2;
      freq1Bits = 16;
      freq2Bits = 53;
      break;
    case khz3519:
      ws_i2s_bck = 1;
      ws_i2s_div = 2;
      freq1Bits = 16;
      freq2Bits = 54;
      break;
    case khz3545:
      ws_i2s_bck = 1;
      ws_i2s_div = 2;
      freq1Bits = 16;
      freq2Bits = 55;
      break;
    case khz3571:
      ws_i2s_bck = 1;
      ws_i2s_div = 2;
      freq1Bits = 16;
      freq2Bits = 56;
      break;
    case khz3597:
      ws_i2s_bck = 1;
      ws_i2s_div = 2;
      freq1Bits = 16;
      freq2Bits = 57;
      break;
    case khz3621:
      ws_i2s_bck = 1;
      ws_i2s_div = 2;
      freq1Bits = 16;
      freq2Bits = 58;
      break;
    case khz3644:
      ws_i2s_bck = 1;
      ws_i2s_div = 2;
      freq1Bits = 16;
      freq2Bits = 59;
      break;
  }
}

void ICACHE_FLASH_ATTR frequencyEnumToString(enum Frequencies freq, char* buf)
{
  switch(freq)
  {
    case khz3478: //direct generated frequencies, no mix products
      os_sprintf(buf, "3478");
      break;
    case khz3555:
      os_sprintf(buf, "3555");
      break;
    case khz3636:
      os_sprintf(buf, "3636");
      break;
    case khz3400: //generated by multiplying two frequencies
      os_sprintf(buf, "3400");
      break;
    case khz3431:
      os_sprintf(buf, "3431");
      break;
    case khz3462:
      os_sprintf(buf, "3462");
      break;
    case khz3491:
      os_sprintf(buf, "3491");
      break;
    case khz3519:
      os_sprintf(buf, "3519");
      break;
    case khz3545:
      os_sprintf(buf, "3545");
      break;
    case khz3571:
      os_sprintf(buf, "3571");
      break;
    case khz3597:
      os_sprintf(buf, "3597");
      break;
    case khz3621:
      os_sprintf(buf, "3621");
      break;
    case khz3644:
      os_sprintf(buf, "3644");
      break;
  }
}

int ICACHE_FLASH_ATTR ggt(int m, int n)
{
    if (n==0)
        return m;
    else
        return ggt(n, m%n);
}

int ICACHE_FLASH_ATTR calcKgv(int m, int n)
{
    int o = ggt(m,n);
    int p = (m * n) / o;
    return p;
}

int ICACHE_FLASH_ATTR setBitPattern()
{
  os_memset( i2sBD, 0x00, sizeof(i2sBD));
  int bitCount = 0;

  kgv = calcKgv( freq1Bits, 32 );

  if(freq2Bits > 0)
    kgv = calcKgv( freq2Bits, kgv );

  for(int i1 = 0; i1<kgv; i1++ )
  {
    bool bit1 = (bitCount%freq1Bits)>freq1Bits/2;
    bool bit2 = true;
    if(freq2Bits > 0)
        bit2 = (bitCount%freq2Bits)>freq2Bits/2;

    setBit(bitCount, (bit1&&bit2) );
    bitCount++;
  }
  return( bitCount );
}

void ICACHE_FLASH_ATTR setBit( int bitNr, bool val)
{
  if( val )
    i2sBD[bitNr >> 5] |= 0x80000000 >> (bitNr % 32);
  else
    i2sBD[bitNr >> 5] &= ~(0x80000000 >> (bitNr % 32));
}

int ICACHE_FLASH_ATTR getBit( int bitNr)
{
  return( (i2sBD[bitNr >> 5] >> (bitNr % 32)) & 1);
}

// For debugging: print the bit field
// If your terminal program supports color, the used part of the
// bit field is printed in green.
void ICACHE_FLASH_ATTR printBitField()
{
    os_printf("\n#############################\n");
    os_printf("bitFieldSize: %d, dataLen: %d, kgv: %d\n",
              bitFieldSize, dataLen, kgv);

    os_printf("\e[32m"); //green color (start "minicom -c on")
    for(int idx = 0; idx<I2SDMABUFLEN*32; idx++)
    {
      if((idx > 0) && (idx % 32) == 0 )
        os_printf("\n");

      if(idx == bitFieldSize)
        os_printf("\e[0m"); //color off.

      if(getBit(idx)>0)
        os_printf("1");
      else
        os_printf("0");
    }
    os_printf("#############################\n");
}

//Initialize I2S subsystem for DMA circular buffer use
void ICACHE_FLASH_ATTR initI2S(enum Frequencies freq)
{
    setFrequency(freq);
    bitFieldSize = setBitPattern();
    dataLen = (bitFieldSize >> 3); // dataLen (bytes) = bitFieldSize / 8

    //Initialize DMA buffer descriptor.
    i2sBufDesc.owner=1;
    i2sBufDesc.eof=1;
    i2sBufDesc.sub_sof=0;
    i2sBufDesc.datalen=dataLen;
    i2sBufDesc.blocksize=dataLen;
    i2sBufDesc.buf_ptr=(uint32_t)&i2sBD;
    i2sBufDesc.unused=0;
    i2sBufDesc.next_link_ptr=(int)(&i2sBufDesc);

    //Reset DMA
    SET_PERI_REG_MASK(SLC_CONF0, SLC_RXLINK_RST|SLC_TXLINK_RST);
    CLEAR_PERI_REG_MASK(SLC_CONF0, SLC_RXLINK_RST|SLC_TXLINK_RST);

    //Clear DMA int flags
    SET_PERI_REG_MASK(SLC_INT_CLR,  0xffffffff);
    CLEAR_PERI_REG_MASK(SLC_INT_CLR,  0xffffffff);

    //Enable and configure DMA
    CLEAR_PERI_REG_MASK(SLC_CONF0, (SLC_MODE<<SLC_MODE_S));
    SET_PERI_REG_MASK(SLC_CONF0,(1<<SLC_MODE_S));
    SET_PERI_REG_MASK(SLC_RX_DSCR_CONF,SLC_INFOR_NO_REPLACE|SLC_TOKEN_NO_REPLACE);
    CLEAR_PERI_REG_MASK(SLC_RX_DSCR_CONF, SLC_RX_FILL_EN|SLC_RX_EOF_MODE | SLC_RX_FILL_MODE);

    //Feed dma the 1st buffer desc addr
    //To send data to the I2S subsystem, counter-intuitively we use the RXLINK part, not the TXLINK as you might
    //expect. The TXLINK part still needs a valid DMA descriptor, even if it's unused: the DMA engine will throw
    //an error at us otherwise. Just feed it any random descriptor.
    CLEAR_PERI_REG_MASK(SLC_TX_LINK,SLC_TXLINK_DESCADDR_MASK);
    SET_PERI_REG_MASK(SLC_TX_LINK, ((uint32)&i2sBufDesc) & SLC_TXLINK_DESCADDR_MASK); //any random desc is OK, we don't use TX but it needs something valid
    CLEAR_PERI_REG_MASK(SLC_RX_LINK,SLC_RXLINK_DESCADDR_MASK);
    SET_PERI_REG_MASK(SLC_RX_LINK, ((uint32)&i2sBufDesc) & SLC_RXLINK_DESCADDR_MASK);

    //clear any interrupt flags that are set
    WRITE_PERI_REG(SLC_INT_CLR, 0xffffffff);

    //Start transmission
    SET_PERI_REG_MASK(SLC_TX_LINK, SLC_TXLINK_START);
    SET_PERI_REG_MASK(SLC_RX_LINK, SLC_RXLINK_START);

    //----

    //Init pins to i2s functions
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_I2SO_DATA);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_I2SO_WS);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_I2SO_BCK);

    //Enable clock to i2s subsystem
    i2c_writeReg_Mask_def(i2c_bbpll, i2c_bbpll_en_audio_clock_out, 1);

    //Reset I2S subsystem
    CLEAR_PERI_REG_MASK(I2SCONF,I2S_I2S_RESET_MASK);
    SET_PERI_REG_MASK(I2SCONF,I2S_I2S_RESET_MASK);
    CLEAR_PERI_REG_MASK(I2SCONF,I2S_I2S_RESET_MASK);

    //Select 16bits per channel (FIFO_MOD=0), no DMA access (FIFO only)
    CLEAR_PERI_REG_MASK(I2S_FIFO_CONF, I2S_I2S_DSCR_EN|(I2S_I2S_RX_FIFO_MOD<<I2S_I2S_RX_FIFO_MOD_S)|(I2S_I2S_TX_FIFO_MOD<<I2S_I2S_TX_FIFO_MOD_S));
    //Enable DMA in i2s subsystem
    SET_PERI_REG_MASK(I2S_FIFO_CONF, I2S_I2S_DSCR_EN);

    //tx/rx binaureal
    CLEAR_PERI_REG_MASK(I2SCONF_CHAN, (I2S_TX_CHAN_MOD<<I2S_TX_CHAN_MOD_S)|(I2S_RX_CHAN_MOD<<I2S_RX_CHAN_MOD_S));

    //Clear int
    SET_PERI_REG_MASK(I2SINT_CLR,   I2S_I2S_TX_REMPTY_INT_CLR|I2S_I2S_TX_WFULL_INT_CLR|
                    I2S_I2S_RX_WFULL_INT_CLR|I2S_I2S_PUT_DATA_INT_CLR|I2S_I2S_TAKE_DATA_INT_CLR);
    CLEAR_PERI_REG_MASK(I2SINT_CLR, I2S_I2S_TX_REMPTY_INT_CLR|I2S_I2S_TX_WFULL_INT_CLR|
                    I2S_I2S_RX_WFULL_INT_CLR|I2S_I2S_PUT_DATA_INT_CLR|I2S_I2S_TAKE_DATA_INT_CLR);

    //trans master&rece slave,MSB shift,right_first,msb right
    CLEAR_PERI_REG_MASK(I2SCONF, I2S_TRANS_SLAVE_MOD|
                                            (I2S_BITS_MOD<<I2S_BITS_MOD_S)|
                                            (I2S_BCK_DIV_NUM <<I2S_BCK_DIV_NUM_S)|
                                            (I2S_CLKM_DIV_NUM<<I2S_CLKM_DIV_NUM_S));

    SET_PERI_REG_MASK(I2SCONF, I2S_RIGHT_FIRST|I2S_MSB_RIGHT|I2S_RECE_SLAVE_MOD|
                                            I2S_RECE_MSB_SHIFT|I2S_TRANS_MSB_SHIFT|
                                            ((ws_i2s_bck&I2S_BCK_DIV_NUM )<<I2S_BCK_DIV_NUM_S)|
                                            ((ws_i2s_div&I2S_CLKM_DIV_NUM)<<I2S_CLKM_DIV_NUM_S));

    //No idea if ints are needed...
    //clear int
    SET_PERI_REG_MASK(I2SINT_CLR,   I2S_I2S_TX_REMPTY_INT_CLR|I2S_I2S_TX_WFULL_INT_CLR|
                    I2S_I2S_RX_WFULL_INT_CLR|I2S_I2S_PUT_DATA_INT_CLR|I2S_I2S_TAKE_DATA_INT_CLR);
    CLEAR_PERI_REG_MASK(I2SINT_CLR,   I2S_I2S_TX_REMPTY_INT_CLR|I2S_I2S_TX_WFULL_INT_CLR|
                    I2S_I2S_RX_WFULL_INT_CLR|I2S_I2S_PUT_DATA_INT_CLR|I2S_I2S_TAKE_DATA_INT_CLR);
    //enable int
    //SET_PERI_REG_MASK(I2SINT_ENA,   I2S_I2S_TX_REMPTY_INT_ENA|I2S_I2S_TX_WFULL_INT_ENA|
    //I2S_I2S_RX_REMPTY_INT_ENA|I2S_I2S_TX_PUT_DATA_INT_ENA|I2S_I2S_RX_TAKE_DATA_INT_ENA);

    //Start transmission
    SET_PERI_REG_MASK(I2SCONF,I2S_I2S_TX_START);
}


