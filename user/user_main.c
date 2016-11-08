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
#include "io.h"
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
#include "cgi-test.h"
#include "webserver.h"
#include "clocktimer.h"
#include "oscillator.h"

//The example can print out the heap use every 3 seconds. You can use this to catch memory leaks.
#define SHOW_HEAP_USE


#ifdef SHOW_HEAP_USE
static ETSTimer prHeapTimer;

static void ICACHE_FLASH_ATTR prHeapTimerCb(void *arg) {
    os_printf("Heap: %ld\n", (unsigned long)system_get_free_heap_size());
    //printBitField();
}
#endif

//Main routine. Initialize stdout, the I/O, filesystem and the webserver and we're done.
void user_init(void) {

  configRestore();
  stdoutInit();
  ioInit();
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
  initI2S( khz3555 );

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
