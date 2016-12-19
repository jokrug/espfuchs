#ifndef CGI_H
#define CGI_H

#include "httpd.h"

int tplInfo(HttpdConnData *connData, char *token, void **arg);
int tplFoxhunt(HttpdConnData *connData, char *token, void **arg);
int cgiCurrentState(HttpdConnData *connData);
void getBattVoltageAsString(char* buff);
int cgiSetTime(HttpdConnData *connData);
#endif
