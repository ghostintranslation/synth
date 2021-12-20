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
    const int tonepin=A0;
    const int notelength=140, notetonote=190;
    SerialUSB.print(r = fastTouchRead(A2)); SerialUSB.print(" ");
    if(r>1)
     { tone(tonepin, 587/2, notelength/(r-1)); delay(notetonote); }
     SerialUSB.print(r = fastTouchRead(A3)); SerialUSB.print(" ");
    if(r>1)
      { tone(tonepin, 440, notelength/(r-1)); delay(notetonote); }
     SerialUSB.print(r = fastTouchRead(A4)); SerialUSB.print(" ");
    if(r>1)
      { tone(tonepin, 220, notelength/(r-1)); delay(notetonote);}
     SerialUSB.print(r = fastTouchRead(A5)); SerialUSB.print(" ");
     if(r>1)
      { tone(tonepin, 153, notelength/(r-1)); delay(notetonote); }
    SerialUSB.print(r = fastTouchRead(A1)); SerialUSB.print(" ");
     if(r>1)
      { tone(tonepin, 185, notelength/(r-1)); delay(notetonote);}
    SerialUSB.print(r = fastTouchRead(A6)); SerialUSB.print(" ");
    if(r>1)
      { tone(tonepin, 196, notelength/(r-1)); delay(notetonote);}
    SerialUSB.print(r = fastTouchRead(A7)); SerialUSB.print(" ");
     if(r>1)
      { tone(tonepin, 349, notelength/(r-1)); delay(notetonote);}
  
   SerialUSB.println();
}

