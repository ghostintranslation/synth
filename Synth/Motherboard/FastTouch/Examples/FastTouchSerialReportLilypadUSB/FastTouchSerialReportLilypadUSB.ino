#include <FastTouch.h>

// Adrian Freed 2018

void setup()
{
  SerialUSB.begin(9600);
}


 void loop()
 {
    SerialUSB.print(fastTouchRead(A2)); SerialUSB.print(" ");
     SerialUSB.print(fastTouchRead(A3)); SerialUSB.print(" ");
     SerialUSB.print(fastTouchRead(A4)); SerialUSB.print(" ");
     SerialUSB.print(fastTouchRead(A5)); SerialUSB.print(" ");
     SerialUSB.print(fastTouchRead(2)); SerialUSB.print(" ");
     SerialUSB.print(fastTouchRead(3)); SerialUSB.print(" ");
 
  for(int i=9;i<=11;++i)
  {
       SerialUSB.print(fastTouchRead(i)); SerialUSB.print(" ");
  }
  SerialUSB.println();
  delay(100);
}

