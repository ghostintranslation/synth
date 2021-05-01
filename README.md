# SYNTH

![GitHub version](https://img.shields.io/github/v/release/ghostintranslation/synth.svg?include_prereleases)

SYNTH is a modular FM polyphonic synthesizer, based on eurorack physical format it has however no patch cables in the front but has rather midi and audio jacks in the back.

It is based on [Motherboard](https://github.com/ghostintranslation/motherboard), my modular platform, see in there for schematics.

You can get the PCBs and front panel here:<br/>
https://ghostintranslation.bandcamp.com/merch/synth-panel-and-pcb

<img src="synth-black.jpg" width="300px"/> <img src="synth-white.jpg" width="300px"/>

## Features

* FM synthesis with 1 carier and 1 modulator
* 16 voices polyphony
* Sine and triangle waveforms
* Arpeggiator and Drone modes
* Glide
* MIDI over USB
* MIDI over mini jack
* Audio jack output

## Dimensions

Height: 3U / 128.5mm

Width: 8HP / 40mm

## Getting Started

### Prerequisites

What things you need to make it work:

```
1 Teensy 4.0
1 Teensy audio board
1 MOTHERBOARD6A pcb
1 MOTHERBOARD6B pcb
1 SYNTH front panel pcb
6 vertical linear 10k potentiometers with their nuts
1 5 pins male header
1 5 pins female headers
5 14 pins male header
5 14 pins female header
2 14 pins long female header
4 CD4051 multiplexers
4 DIP16 IC sockets (optional)
2 3.5mm jack connectors
1 resistor ~ 22ohm
2 LED
1 4 positions dipswitch (optional)
```

Here is a list of useful links to get these parts: https://github.com/ghostintranslation/parts

Note: 
- The resistor doesn't need to be high because the multiplexers already are resistive, so try a few values. You can place the resistor and LEDs without soldering to test, there should be enough contact.
- The dipswitch is optional, without it the module will listen to channel 1.

### Assembly

1. Solder male 14 pins headers on the audio board
2. Solder 14 pins long female header and 5 pins male header on Teensy
3. Solder 14 pins male headers on MOTHERBOARD6A
4. Place the potentiometers and LEDs on MOTHERBOARD6A, and attach the front panel
5. Solder the potentiometers and the LEDs
6. Place the 14 pins and 5 pins female headers for the Teensy on MOTHERBOARD6B, insert the Teensy on them, then solder the headers
7. Detach the Teensy for now
8. Solder the jack connectors, the dipswitch and the resistor on MOTHERBOARD6B
9. Place 14 pins female headers on MOTHERBOARD6B, connect MOTHERBOARD6A on them, then solder the headers
10. Detach the boards for now
11. Solder IC sockets on IC1, IC2, IC5 and IC6 positions on MOTHERBOARD6B
12. Add the multiplexers on the sockets, connect the 2 boards and connect the Teensy and audio board

Note: Be careful how you place the potentiometers:

<img src="https://github.com/ghostintranslation/motherboard6/blob/master/input-traces.png" width="200px"/>

### Firmware

In order to run any sketch on the Teensy you have to install Arduino and the Teensyduino add-on.
Follow the instructions from the official page:
https://www.pjrc.com/teensy/teensyduino.html

1. Then open `Synth.ino` located in the `Synth` folder of this repo.
2. In the Tools -> USB Type menu, choose `Serial + midi`.
3. Plug the Teensy to your computer with a micro USB cable. (It's ok if the Teensy is on the module)
4. Then just click the arrow button to upload the code

## How to use

Here is a description of the 6 inputs and what they do:

```
1. Mode
    1. Synth
    2. Arp
    3. Drone
2. Param
    1. Synth: Glide
    2. Arp: Time             
        - Arp with Midi clock   (TODO)
    3. Drone: Free frequency
3. Shape
    - Sine / triangle
4. FM
    - Modulator frequency and amplitude
    - 3 divisions:
        1. 0 to 10Hz, low amplitude
        2. 0 to 40Hz, low amplitude
        3. 0 to 1000Hz, higher amplitude
5. Attack                    (0 to 2000ms)
6. Release                   (0 to 2000ms)

```

All of the inputs are available via MIDI CC.

## MIDI

SYNTH supports MIDI in via USB and TS jack. It doesn't send out any MIDI.

The MIDI in default settings are:
```
CC 0 = Mode
CC 1 = Param
CC 2 = Shape
CC 3 = FM
CC 4 = Attack
CC 5 = Release
```

These settings can be changed in the code or via the web editor: http://ghostintranslation.com/editor

**Important:**

The MIDI input and output jacks are directly connected to the Teensy serial input and output. That means there is not protection against voltage or current. It is primarily ment to connect 2 of these modules, or 2 Teensy together. If you want to connect something else to it make sure to provide a maximum of 3.3v and 250 mA.

## About me
You can find me on Bandcamp, Instagram, Youtube and my own site:

https://ghostintranslation.bandcamp.com/

https://www.instagram.com/ghostintranslation/

https://www.youtube.com/channel/UCcyUTGTM-hGLIz4194Inxyw

https://www.ghostintranslation.com/

## Support
To support my work:<br>
https://www.paypal.com/paypalme/ghostintranslation

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details