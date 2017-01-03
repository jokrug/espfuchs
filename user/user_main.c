/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

/*
This is example code for the esphttpd library. It's a small-ish demo showing off 
the server, including WiFi connection management capabilities, some IO and
some pictures of cats.
*/

#include <esp8266.h>
#include "httpd.h"
#include "httpdespfs.h"
#include "cgi.h"
#include "config.h"
#include "cgiwifi.h"
#include "cgiflash.h"
#include "stdout.h"
#include "auth.h"
#include "espfs.h"
#include "captdns.h"
#include "webpages-espfs.h"
#include "cgiwebsocket.h"
#include "webserver.h"
#include "clocktimer.h"
#include "oscillator.h"
#include "foxcontrol.h"

//The example can print out the heap use every 3 seconds. You can use this to catch memory leaks.
//#define SHOW_HEAP_USE

static char* rst_codes[7] = {
  "normal", "wdt reset", "exception", "soft wdt", "restart", "deep sleep", "external",
};
static const char* espfuchs_version="v0.01";

static char* flash_maps[7] = {
  "512KB:256/256", "256KB", "1MB:512/512", "2MB:512/512", "4MB:512/512",
  "2MB:1024/1024", "4MB:1024/1024"
};

#ifdef SHOW_HEAP_USE
static ETSTimer prHeapTimer;

static void ICACHE_FLASH_ATTR prHeapTimerCb(void *arg) {
    os_printf("Heap: %ld\n", (unsigned long)system_get_free_heap_size());
    printBitField();
}
#endif

//You must provide:
//Critical should not lock interrupts, just disable services that have problems
//with double-interrupt faults.  I.e. turn off/on any really fast timer interrupts.
//These generally only get called when doing serious operations like reflashing.
void EnterCritical()
{

}

void ExitCritical()
{

}

void ICACHE_FLASH_ATTR print_reset_info(void)
{
    struct rst_info *rst_info = system_get_rst_info();
    os_printf("Reset cause: %d=%s", rst_info->reason, rst_codes[rst_info->reason]);
    os_printf("exccause=%d epc1=0x%x epc2=0x%x epc3=0x%x excvaddr=0x%x depc=0x%x\n",
      rst_info->exccause, rst_info->epc1, rst_info->epc2, rst_info->epc3,
      rst_info->excvaddr, rst_info->depc);
    uint32_t fid = spi_flash_get_id();
    os_printf("Flash map %s, manuf 0x%02lX chip 0x%04lX", flash_maps[system_get_flash_size_map()],
        fid & 0xff, (fid&0xff00)|((fid>>16)&0xff));
    os_printf("** %s: ready, heap=%ld", espfuchs_version, (unsigned long)system_get_free_heap_size());
}

//Main routine. Initialize stdout, the I/O, filesystem and the webserver and we're done.
void user_init(void) {

  stdoutInit();
  // Say hello (leave some time to cause break in TX after boot loader's msg
  os_delay_us(10000L);

  print_reset_info();

  configRestore();
  printCconfig();

  captdnsInit();

	// 0x40200000 is the base address for spi flash memory mapping, ESPFS_POS is the position
	// where image is written in flash that is defined in Makefile.
#ifdef ESPFS_POS
	espFsInit((void*)(0x40200000 + ESPFS_POS));
#else
	espFsInit((void*)(webpages_espfs_start));
#endif
  
  initWebserver();
  initClockTimer();
  initFoxControl();

#ifdef SHOW_HEAP_USE
	os_timer_disarm(&prHeapTimer);
	os_timer_setfn(&prHeapTimer, prHeapTimerCb, NULL);
	os_timer_arm(&prHeapTimer, 3000, 1);
#endif
}

void ICACHE_FLASH_ATTR user_set_softap_config(void)
{
  os_printf("Set SoftAP, SSID: ESPFuchs\n");

  struct softap_config config;
  config.ssid_len = 0;
  config.authmode = AUTH_OPEN;
  config.ssid_hidden = 0;
  config.max_connection = 4;
  os_strcpy((char*)&config.ssid, "ESPFuchs");
  os_strcpy((char*)&config.password, "");

 // Define our mode as an Access Point
 wifi_set_opmode_current(SOFTAP_MODE);
 wifi_softap_set_config_current(&config);
}

void user_rf_pre_init() {
  user_set_softap_config();
}

