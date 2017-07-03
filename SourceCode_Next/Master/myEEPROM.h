//myEEPROM
//  Purpose:      SET DATA ON EEPROM MEMORY OR READ STRINGS FROM EEPROM MEMORY
//  Author:       Wellington Rodrigo Gallo
//  License:      GNU Lesser General Public License
//                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#ifndef _myEEPROMh_
#define _myEEPROMh_

/*Arduino
  Purpose:      Needed in all Header Files
  License:      GNU Lesser General Public License
                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <Arduino.h>

/*EEPROM
  Purpose:      Needed for Read and Write EEPROM functions
  License:      GNU Lesser General Public License
                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <EEPROM.h>

#ifndef EEPROM_LENGTH
#define EEPROM_LENGTH 1024
#endif

/*
 * MEMORY MAP
 *  ----------------------------------------------------------------------------------------------------------
 * | ADDRESS START                                                                   |      PURPOSE           |
 * |---------------------------------------------------------------------------------|------------------------|
 * | 0                                                                               |      NIGHT TIME        | 
 * |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -|- - - - - - - - - - - - |
 * | 1*3 + NIGHTTIME_LEN                                                             |      LAMP STATE        |
 * |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -|- - - - - - - - - - - - |
 * | 2*3 + NIGHTTIME_LEN + LAMPSTATE_LEN                                             |      MOTION STATE      |
 * |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -|- - - - - - - - - - - - |
 * | 3*3 + NIGHTTIME_LEN + LAMPSTATE_LEN + MOTIONSTATE_LEN                           |      ALARM 0           |
 *  ----------------------------------------------------------------------------------------------------------
 * 
 * ---------------------------------------------------
 * | Caution:                                         |
 * |    Changing the max_length of any variable       |
 * |    requires all the data stored to be rewritten! |
 * ---------------------------------------------------
 */
#define NIGHTTIME_LEN 9     //HH MM HH MM s
#define LAMPSTATE_LEN 1     //# (0 or 1)
#define MOTIONSTATE_LEN 1   //# (0 or 1)
#define ALARM_LEN 6         //HH MM W s

void setThisByteToAll( byte value = 255);
void readEEPROM(bool detailed=true);

void getNightTime(uint8_t* nightTimeStart, uint8_t* nightTimeEnd, bool* set);
void getLampState(uint8_t* state);
void getMotionState(bool* state);
void getAlarm0(uint8_t* alarmTime, uint8_t* alarmWeekday, bool* set);

void setNightTime(uint8_t* nightTimeStart, uint8_t* nightTimeEnd, bool set);
void setLampState(uint8_t value);
void setMotionState(bool value);
void setAlarm0(uint8_t* alarmTime, uint8_t alarmWeekday, bool set);

#endif
