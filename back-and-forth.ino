/*
Based upon http://www.hessmer.org/blog/2013/12/28/ibt-2-h-bridge-with-arduino/.

Connection to the IBT-2 board:
IBT-2 pin 1 (RPWM) to Arduino pin 5(PWM)
IBT-2 pin 2 (LPWM) to Arduino pin 6(PWM)
IBT-2 pins 3 (R_EN), 4 (L_EN), 7 (VCC) to Arduino 5V pin
IBT-2 pin 8 (GND) to Arduino GND
IBT-2 pins 5 (R_IS) and 6 (L_IS) not connected
*/

const int pinRPWM = 5;
const int pinLPWM = 6;
const int pinSwitch = 0;

/*
The basic cycle is:
1. Spin forwards;
2. Stop;
3. Pause;
4. Spin in reverse;
5. Stop;
6. Soak;
7. Repeat.

When a switch is closed (shorted to ground), interrupt the cycle and spin
constantly in a single direction.
*/
const int forwardDuration = 3000; // ms
const int pauseDuration = 2000; // ms
const int reverseDuration = 3000; // ms
const int soakDuration = 7000; // ms
const int forwardSpeed = 255; // 0 to 255
const int reverseSpeed = -255; // -255 to 0
const int spinSpeed = 255; // -255 to 255

// increment the PWM duty cycle gradually to make motor speed
// and direction transitions smooth, delaying between each increment
const int glideDelay = 5; // ms
int lastPWM = 0;


void spin(int PWM) // -255 to 255
{
  if(PWM >= 0)
  {
    analogWrite(pinRPWM, 0);
    analogWrite(pinLPWM, PWM);
  }
  else
  {
    analogWrite(pinLPWM, 0);
    analogWrite(pinRPWM, -PWM);
  }
  lastPWM = PWM;
}

void glideTo(int PWM) // -255 to 255
{
  if(PWM > lastPWM)
  {
    for(int i = lastPWM; i <= PWM; i++)
    {
      spin(i);
      delayWhileSwitchOff(glideDelay);
    }
  }
  else
  {
    for(int i = lastPWM; i >= PWM; i--)
    {
      spin(i);
      delayWhileSwitchOff(glideDelay);
    }
  }
}

bool isSwitchOn()
{
  // pulled high when switch open
  return digitalRead(pinSwitch) == LOW;
}

void delayWhileSwitchOff(int duration) // duration in ms
{
  // "interruptible" (not using interrupts) delay that exits if a switch is
  // turned on
  const int checkInterval = 10; // ms
  if (duration < checkInterval)
  {
    delay(duration);
    return;
  }
  int elapsed = 0;
  while(elapsed < duration && !isSwitchOn())
  {
    delay(checkInterval);
    elapsed += checkInterval;
  }
}

void setup()
{
  pinMode(pinRPWM, OUTPUT);
  pinMode(pinLPWM, OUTPUT);
  // the switch should short to ground when closed
  pinMode(pinSwitch, INPUT_PULLUP);
}

void loop()
{
  if(isSwitchOn())
  {
    glideTo(spinSpeed);
    return;
  }
  glideTo(forwardSpeed);
  delayWhileSwitchOff(forwardDuration);
  glideTo(0);
  delayWhileSwitchOff(pauseDuration);
  glideTo(reverseSpeed);
  delayWhileSwitchOff(reverseDuration);
  glideTo(0);
  delayWhileSwitchOff(soakDuration);
}
