# SYNTH

SYNTH is a modular FM polyphonic synthesizer, based on eurorack physical format it has however no patch cables in the front but has rather midi and audio jacks in the back.

It is based on [Motherboard6](https://github.com/ghostintranslation/motherboard6).

NOTE: This is a work in progress.

<img src="synth.jpg" width="200px"/> <img src="synth-side.jpg" width="200px"/>

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
6 vertical linear 10k potentiometers
3 14 pins male header
5 14 pins female header
4 CD4051 multiplexers
4 DIP16 IC sockets (optionak)
2 3.5mm jack connectors
1 resistor ~ 22ohm
1 LED
1 4 positions dipswitch (optional)
```

I personally get these parts from multiple stores: digikey.com, thonk.co.uk, modularaddict.com

In order to run any sketch on the Teensy you have to install the Teensyduino add-on to Arduino.
Follow the instructions from the official page:
https://www.pjrc.com/teensy/teensyduino.html

Then open the sketch located in the Synth folder of this repo.

In the Tools -> USB Type menu, choose "Serial + midi".

Then you are ready to compile and upload the sketch.

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

## MIDI

The MIDI input and output jacks are directly connected to the Teensy serial input and output. That means there is not protection against voltage or current. It is primarily ment to connect 2 of these modules, or 2 Teensy together. If you want to connect something else to it make sure to provide a maximum of 3.3v and 250 mA.

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

# About me
You can find me on Bandcamp and Instagram:

https://ghostintranslation.bandcamp.com/

https://www.instagram.com/ghostintranslation/

https://www.youtube.com/channel/UCcyUTGTM-hGLIz4194Inxyw

https://www.ghostintranslation.com/