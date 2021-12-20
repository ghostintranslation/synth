#include <FastTouch.h>

// monotone touch sequencer
// Adrian Freed 2018

void setup()
{
  SerialUSB.begin(9600);
}


 void loop()
 {
    int r;
    const int tonepin=10;
    const int notelength=140, notetonote=160;
    SerialUSB.print(r = fastTouchRead(A2)); SerialUSB.print(" ");
    if(r>1)
     { tone(tonepin, 587, notelength/(r-1)); delay(notetonote); }
     SerialUSB.print(r = fastTouchRead(A3)); SerialUSB.print(" ");
    if(r>1)
      { tone(tonepin, 440, notelength/(r-1)); delay(notetonote); }
     SerialUSB.print(r = fastTouchRead(A4)); SerialUSB.print(" ");
    if(r>1)
      { tone(tonepin, 880, notelength/(r-1)); delay(notetonote);}
     SerialUSB.print(r = fastTouchRead(A5)); SerialUSB.print(" ");
     if(r>1)
      { tone(tonepin, 153, notelength/(r-1)); delay(notetonote); }
    SerialUSB.print(r = fastTouchRead(2)); SerialUSB.print(" ");
     if(r>1)
      { tone(tonepin, 185, notelength/(r-1)); delay(notetonote);}
    SerialUSB.print(r = fastTouchRead(3)); SerialUSB.print(" ");
    if(r>1)
      { tone(tonepin, 196, notelength/(r-1)); delay(notetonote);}
    SerialUSB.print(r = fastTouchRead(9)); SerialUSB.print(" ");
     if(r>1)
      { tone(tonepin, 349, notelength/(r-1)); delay(notetonote);}
    SerialUSB.print(r = fastTouchRead(11)); SerialUSB.print(" ");
    if(r>1)
      { tone(tonepin, 370, notelength/(r-1)); delay(notetonote);}
  
   SerialUSB.println();
}

