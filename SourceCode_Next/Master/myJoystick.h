#ifndef _myJoystickh_
#define _myJoystickh_

/*Arduino
  Purpose:      Needed in all Header Files
  License:      GNU Lesser General Public License
                Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <Arduino.h>

//PINS
#define J_RX A1
#define J_RY A2
#define J_SW 2

//CALIBRATION
//X Analog Value rises in the Right direction
//Y Analog Value rises in the Down  direction
#define J_X_IDLE 487
#define J_Y_IDLE 512
#define J_DEADZONE_X_RANGE 15
#define J_DEADZONE_Y_RANGE 15
#define J_DEADZONE_X_MAX (J_X_IDLE + J_DEADZONE_X_RANGE)
#define J_DEADZONE_X_MIN (J_X_IDLE - J_DEADZONE_X_RANGE)
#define J_DEADZONE_Y_MAX (J_Y_IDLE + J_DEADZONE_Y_RANGE)
#define J_DEADZONE_Y_MIN (J_Y_IDLE - J_DEADZONE_Y_RANGE)
#define J_ANALOG_2_DIGITAL_RANGE 50
#define J_ACCEPTABLE_MIN (0    + J_ANALOG_2_DIGITAL_RANGE)
#define J_ACCEPTABLE_MAX (1023 - J_ANALOG_2_DIGITAL_RANGE)

//APPLICATION
#define J_SMOOTH_MOVEMENT false //IF FALSE, BOOTH AXIS HAVE TO RETURN TO IDLE FOR A NEW CMD TO BE READ
#define J_SEQUENCE_LENGTH 4     //LENGTH OF THE SEQUENCE OF CMDs RECORDED

/*
   *  ----------------------------------
   * | joyXValue | joyYValue | joyValue |
   *  ----------------------------------
   * | IDLE      | IDLE      | 0        | //0 is not a valid CMD
   * | IDLE      | UP        | 1        |
   * | IDLE      | DOWN      | 2        |
   * | LEFT      | IDLE      | 3        |
   * | LEFT      | UP        | 4        |
   * | LEFT      | DOWN      | 5        |
   * | RIGHT     | IDLE      | 6        |
   * | RIGHT     | UP        | 7        |
   * | RIGHT     | DOWN      | 8        |
   * -----------------------------------
   */
const uint8_t J_PASWWORD[ J_SEQUENCE_LENGTH ] = { 6, 6, 2, 1 };

enum joyMeaning {
  idle = 0,
  up,
  down,
  left,
  left_up,
  left_down,
  right,
  right_up,
  right_down
};

extern void joyTimerHandler();
extern void beepAlarm();

/**
 * Setup Inputs and SW Interrupt
 */
void beginJoystick();

/**
 * Print the Analog Rx and Ry values
 */
void printAnalogValues();

/**
 * Print the Commands Sequence
 */
void printSequence();

/**
 * Return the actual position of the Sequence of Commands
 */
uint8_t getSequenceLen();

/**
 * Get Rx and Ry values
 */
void getJoyValues();

/**
 * Get joySequence[ pos ] value
 */
uint8_t getJoyValue( uint8_t pos );

/**
 * verify Sequence == Password
 */
bool joyCheckPassword();

/**
 * SW Interrupt Handler
 */
void joySWHandler();


#endif
