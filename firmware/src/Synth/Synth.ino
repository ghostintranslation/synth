/*
GHOST IN TRANSLATION
DRONE
v2.0.0

https://www.ghostintranslation.com/

Support my work:
https://www.buymeacoffee.com/ghostintranslation
https://ko-fi.com/ghostintranslation
https://liberapay.com/ghostintranslation/
https://github.com/sponsors/ghostintranslation

Stay in touch
https://ghostintranslation.bandcamp.com/
https://www.instagram.com/ghostintranslation/
https://www.youtube.com/ghostintranslation
https://github.com/ghostintranslation
*/

#include <Audio.h>
#include "Synth.h"
#include "NoiseFix.h"

// Instanciation of Synth
Synth* synth = Synth::getInstance();

NoiseFix noiseFixLeft;
NoiseFix noiseFixRight;
AudioOutputI2S i2s;
// AudioOutputUSB usb;

void setup() {
  Serial.begin(115200);

  while (!Serial && millis() < 2500); // wait for serial monitor

  // delay(100);

  // Audio connections require memory to work.
  AudioMemory(400);

  // A delay seems required to give Teensy time to boot
  delay(500);

  synth->init();

  new AudioConnection(*synth->getOutputL(), 0, noiseFixLeft, 0);
  new AudioConnection(*synth->getOutputR(), 0, noiseFixRight, 0);
  new AudioConnection(noiseFixLeft, 0, i2s, 0);
  new AudioConnection(noiseFixRight, 0, i2s, 1);
  // new AudioConnection(*synth->getOutputL(), 0, usb, 0);
  // new AudioConnection(*synth->getOutputR(), 0, usb, 1);
}

void loop() {
}
