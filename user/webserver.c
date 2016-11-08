/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

/*
*/

#include <esp8266.h>
#include "httpd.h"
#include "io.h"
#include "httpdespfs.h"
#include "cgi.h"
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

//static ETSTimer websockTimer;
//Prototypes

/*
This is the main url->function dispatching data struct.
In short, it's a struct with various URLs plus their handlers. The handlers can
be 'standard' CGI functions you wrote, or 'special' CGIs requiring an argument.
They can also be auth-functions. An asterisk will match any url starting with
everything before the asterisks; "*" matches everything. The list will be
handled top-down, so make sure to put more specific rules above the more
general ones. Authorization things (like authBasic) act as a 'barrier' and
should be placed above the URLs they protect.
*/
HttpdBuiltInUrl builtInUrls[]={
  {"*", cgiRedirectApClientToHostname, "espfuchs.wlan"},
  {"/", cgiRedirect, "/index.tpl"},
  {"/index.tpl", cgiEspFsTemplate, tplCounter},
  {"/settime", cgiSetTime, NULL},
   
  {"/xml", cgiCurrentState, NULL},

  {"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
  {NULL, NULL, NULL}
};




void initWebserver() {
  
  httpdInit(builtInUrls, 80);
  
  //os_timer_disarm(&websockTimer);
  //os_timer_setfn(&websockTimer, websockTimerCb, NULL);
  //os_timer_arm(&websockTimer, 1000, 1);
  os_printf("Webserver init ready\n");  
}

