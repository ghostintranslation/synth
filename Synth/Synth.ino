/*
 __    _  _____   ___      ___ _  _     __    _ ______ _    
/__|_|/ \(_  |     | |\|    | |_)|_||\|(_ |  |_| |  | / \|\|
\_|| |\_/__) |    _|_| |    | | \| || |__)|__| | | _|_\_/| |

SYNTH
v2.0.0

Support my work:
https://www.paypal.com/paypalme/ghostintranslation

https://www.ghostintranslation.com
https://ghostintranslation.bandcamp.com
https://www.instagram.com/ghostintranslation
https://www.youtube.com/c/ghostintranslation
https://github.com/ghostintranslation
*/

#include <Audio.h>

#include "Synth.h"

// Instanciation of Synth
Synth * synth = Synth::getInstance();

AudioOutputI2S  i2s2;
AudioOutputUSB usb;
AudioConnection patchCord1(*synth->getOutput(), 0, i2s2, 0);
AudioConnection patchCord2(*synth->getOutput(), 0, i2s2, 1);
AudioConnection patchCord3(*synth->getOutput(), 0, usb, 0);
AudioConnection patchCord4(*synth->getOutput(), 0, usb, 1);
AudioControlSGTL5000 audioBoard;

void setup() {
  Serial.begin(115200);
  
  while (!Serial && millis() < 2500); // wait for serial monitor

  synth->init();

  // Audio connections require memory to work.
  AudioMemory(40);

  audioBoard.enable();
  audioBoard.volume(0.3);
}

void loop() {
  // Synth update
  synth->update();
}
