/** @defgroup   clocktimer System clock functions
 *  @brief      Provides system clock and timer functions
*/

#ifndef APP_SYSTEMCLOCK_H_
#define APP_SYSTEMCLOCK_H_

#include <time.h>
#include <esp8266.h>

/// System clock status
enum SystemClockStatus
{
	eSCS_Initial	= 0, ///< Clock not yet set
	eSCS_Set		  = 1 ///< Clock set
};


void initClockTimer();

time_t time(time_t* t);

/** @brief  Get the current date and time
*  @param tmObj buffer
*  @retval DateTime Current date and time
*/
void now( struct tm* tmObj );

/** @brief  Set the system clock's time
*  @param  time Time to set clock to
*  @param  timeType Time zone to use
*/
void setTime(struct tm* newTime);

/** @brief  Get current time as a string
*  @param  timeType Time zone to use
*  @retval String Current time in format: dd.mm.yy hh:mm:ss
*  @todo   Allow time format to be defined
*/
int atoint(char **str);
bool   strToStructTm(char* str, struct tm* tmBuf);
time_t strToTime_t(char* str);
void   time_tToStructTm(time_t tt, struct tm* tmBuf);
void   time_tToString(time_t tt, char* buf);

#endif /* APP_SYSTEMCLOCK_H_ */
