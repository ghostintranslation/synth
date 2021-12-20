#include <FastTouch.h>
//
// Adrian Freed 2019
//
// report on whether pins are being touched based on how
// slow the pins arrive at HIGH from ground when a PULLUP resistor is enabled
// all pin except the built-in LED are probed. Note that each pin will be hard pulled
// to ground and that interrupts are briefly stopped during the measurement.
//

void setup()
{
  SerialUSB.begin(9600);
}


 void loop()
 {
   for(int i=0;i<CORE_NUM_DIGITAL;++i)
  {
      if(i!=LED_BUILTIN)
      {
        SerialUSB.print(fastTouchRead(i)); SerialUSB.print(" ");
      }
       else
        SerialUSB.print("LED ");
  }
  SerialUSB.println();
  delay(200);
}
