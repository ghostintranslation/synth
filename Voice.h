#ifndef Voice_h
#define Voice_h

#include <Audio.h>

// Custom types
enum synthesis {FM, FMx10, AM, AMx10, RING};
enum modes {SYNTH, ARP, DRONE};

// GUItool: begin automatically generated code
//AudioSynthWaveform       sineAM;      //xy=67.55555725097656,94.33334350585938
//AudioSynthWaveformDc     dcAM;            //xy=70,217
//AudioSynthWaveform       sawtoothAM;      //xy=80.77777862548828,140.33334350585938
//AudioSynthWaveformSine   sineModulator;          //xy=100,344
//AudioMixer4              mixerDcMod;         //xy=234,232
//AudioMixer4              mixerAM;         //xy=237.88888549804688,113.22222900390625
//AudioSynthWaveformModulated sineFM;   //xy=289,344
//AudioSynthWaveformModulated sawtoothFM;   //xy=300,408
//AudioEffectMultiply      multiplyAM;      //xy=388,196
//AudioMixer4              mixerFM;         //xy=454,368
//AudioMixer4              mixerAM2;         //xy=524,105
//AudioMixer4              mixer;         //xy=575,233
//AudioMixer4              output;         //xy=714,329
//AudioOutputI2S           i2s2;           //xy=714.4444580078125,443.6666564941406
//AudioEffectEnvelope      envelope;      //xy=724.7777709960938,232.33334350585938
//AudioConnection          patchCord1(sineAM, 0, mixerAM, 0);
//AudioConnection          patchCord2(dcAM, 0, mixerDcMod, 0);
//AudioConnection          patchCord3(sawtoothAM, 0, mixerAM, 1);
//AudioConnection          patchCord4(sineModulator, 0, sawtoothFM, 0);
//AudioConnection          patchCord5(sineModulator, 0, sineFM, 0);
//AudioConnection          patchCord6(sineModulator, 0, mixerDcMod, 1);
//AudioConnection          patchCord7(mixerDcMod, 0, multiplyAM, 1);
//AudioConnection          patchCord8(mixerAM, 0, multiplyAM, 0);
//AudioConnection          patchCord9(mixerAM, 0, mixerAM2, 0);
//AudioConnection          patchCord10(sineFM, 0, mixerFM, 0);
//AudioConnection          patchCord11(sawtoothFM, 0, mixerFM, 1);
//AudioConnection          patchCord12(multiplyAM, 0, mixerAM2, 1);
//AudioConnection          patchCord13(mixerFM, 0, mixer, 1);
//AudioConnection          patchCord14(mixerAM2, 0, mixer, 0);
//AudioConnection          patchCord15(mixer, envelope);
//AudioConnection          patchCord16(mixer, 0, output, 1);
//AudioConnection          patchCord17(output, 0, i2s2, 0);
//AudioConnection          patchCord18(output, 0, i2s2, 1);
//AudioConnection          patchCord19(envelope, 0, output, 0);
// GUItool: end automatically generated code


/*
 * Voice
 */
class Voice{
  private:
    // AM and Ring
    AudioSynthWaveformDc     *dcAM;
    AudioSynthWaveform       *sawtoothAM;
    AudioSynthWaveform       *sineAM;
    AudioEffectMultiply      *multiplyAM;
    AudioMixer4              *mixerAM;
    AudioMixer4              *mixerAM2;
    AudioMixer4              *mixerDcMod;
    // FM
    AudioSynthWaveformModulated *sineFM;
    AudioSynthWaveformModulated *sawtoothFM;
    AudioMixer4              *mixerFM;
    // Common
    AudioSynthWaveformSine   *sineModulator;
    AudioMixer4              *mixer;
    AudioEffectEnvelope      *envelope;
    AudioConnection* patchCords[18];
    // Output
    AudioMixer4 *output;

    synthesis synth;
    modes mode;
    byte note;
    bool notePlayed;

  public:
    Voice();

    byte currentNote; // The midi note currently being played.
    unsigned long last_played;
    
    AudioMixer4 * getOutput();
    void noteOn(byte midiNote = 0);
    void noteOff();
    void setADR(unsigned int attack, unsigned int decay, unsigned int release);
    bool isActive();
    bool isNotePlayed();
    void setNotePlayed(bool notePlayed);
    void setSynth(byte synth);
    void setMode(byte mode);
    void setFrequency(int frequency);
    void setModulatorFrequency(int freq);
    void setModulatorAmplitude(float amp);
    void setAttack(int att);
    void setDecay(int dec);
    void setRelease(int rel);
};

/**
 * Constructor
 */
inline Voice::Voice(){
  // AM and Ring
  this->dcAM = new AudioSynthWaveformDc();
  this->dcAM->amplitude(0.5);
  this->sineAM = new AudioSynthWaveform();
  this->sineAM->amplitude(1);
  this->sawtoothAM = new AudioSynthWaveform();
  this->sawtoothAM->begin(WAVEFORM_SAWTOOTH);
  this->sawtoothAM->amplitude(1);
  this->multiplyAM = new AudioEffectMultiply();
  this->mixerAM = new AudioMixer4();
  this->mixerAM->gain(0, 0.5 ); // TODO Link to a potentiometer
  this->mixerAM->gain(1, 0.5 ); // TODO Link to a potentiometer
  this->mixerAM2 = new AudioMixer4();
  this->mixerAM2->gain(0, 0);
  this->mixerAM2->gain(1, 1);
  this->mixerDcMod = new AudioMixer4();
  this->mixerDcMod->gain(0, 0);
  this->mixerDcMod->gain(1, 1);
  // FM
  this->sineFM = new AudioSynthWaveformModulated();
  this->sineFM->amplitude(1);
  this->sawtoothFM = new AudioSynthWaveformModulated();
  this->sawtoothFM->begin(WAVEFORM_SAWTOOTH);
  this->sawtoothFM->amplitude(1);
  this->mixerFM = new AudioMixer4();
  this->mixerFM->gain(0, 0.3 ); // TODO Link to a potentiometer
  this->mixerFM->gain(1, 0.05 ); // TODO Link to a potentiometer
  // Common
  this->sineModulator = new AudioSynthWaveformSine();
  this->mixer = new AudioMixer4();
  this->mixer->gain(0, 1);
  this->envelope = new AudioEffectEnvelope();
  this->envelope->sustain(0.5);
  this->output = new AudioMixer4();
  this->output->gain(0, 0);
  this->output->gain(1, 1);

  this->patchCords[0] = new AudioConnection(*this->sineAM, 0, *this->mixerAM, 0);
  this->patchCords[1] = new AudioConnection(*this->sawtoothAM, 0, *this->mixerAM, 1);
  this->patchCords[2] = new AudioConnection(*this->dcAM, 0, *this->mixerDcMod, 0);
  this->patchCords[3] = new AudioConnection(*this->sineModulator, 0, *this->mixerDcMod, 1);
  this->patchCords[4] = new AudioConnection(*this->mixerDcMod, 0, *this->multiplyAM, 1);
  this->patchCords[5] = new AudioConnection(*this->sineModulator, 0, *this->sawtoothFM, 0);
  this->patchCords[6] = new AudioConnection(*this->sineModulator, 0, *this->sineFM, 0);
  this->patchCords[7] = new AudioConnection(*this->mixerAM, 0, *this->multiplyAM, 0);
  this->patchCords[8] = new AudioConnection(*this->mixerAM, 0, *this->mixerAM2, 0);
  this->patchCords[9] = new AudioConnection(*this->sineFM, 0, *this->mixerFM, 0);
  this->patchCords[10] = new AudioConnection(*this->sawtoothFM, 0, *this->mixerFM, 1);
  this->patchCords[11] = new AudioConnection(*this->multiplyAM, 0, *this->mixerAM2, 1);
  this->patchCords[12] = new AudioConnection(*this->mixerFM, 0, *this->mixer, 1);
  this->patchCords[13] = new AudioConnection(*this->mixerAM2, 0, *this->mixer, 0);
  this->patchCords[14] = new AudioConnection(*this->mixer, 0, *this->envelope, 0);
  this->patchCords[15] = new AudioConnection(*this->envelope, 0, *this->output, 0);
  this->patchCords[16] = new AudioConnection(*this->mixer, 0, *this->output, 1);

  this->setSynth(0);
  this->setMode(0);
  this->notePlayed = false;
}

/**
 * Set Attack Decay Release
 */
inline void Voice::setADR(unsigned int attack, unsigned int decay, unsigned int release){
  this->envelope->attack(attack);
  this->envelope->decay(decay);
  this->envelope->release(release);
}

/**
 * Return the audio output
 */
inline AudioMixer4 * Voice::getOutput(){
  return this->output;
}

/**
 * Note on
 */
inline void Voice::noteOn(byte midiNote) {
  float freq = 440.0 * powf(2.0, (float)(midiNote - 69) * 0.08333333);
  this->sawtoothAM->frequency(freq);
  this->sineAM->frequency(freq);
  this->sawtoothFM->frequency(freq);
  this->sineFM->frequency(freq);
  this->currentNote = midiNote;
  this->envelope->noteOn();
  this->last_played = millis();
  this->notePlayed=true;
}

/**
 * Note off
 */
inline void Voice::noteOff() {
  this->envelope->noteOff();
}

/**
 * Is the voice active
 */
inline bool Voice::isActive(){
  return this->envelope->isActive();
}

inline bool Voice::isNotePlayed(){
  return this->notePlayed;
}

inline void Voice::setNotePlayed(bool notePlayed){
  this->notePlayed = notePlayed;
}


/**
 * Set the synth
 */
inline void Voice::setSynth(byte synthValue){
  // Set the synth
  switch(synthValue){
    default:
    case 0 : this->synth = FM;    break;
    case 1 : this->synth = FMx10; break;
    case 2 : this->synth = AM;    break;
    case 3 : this->synth = AMx10; break;
    case 4 : this->synth = RING;  break;
  }

  // Init the mixers gain to 0
  this->mixer->gain(0, 0 );
  this->mixer->gain(1, 0 );
  this->mixerDcMod->gain(0, 0);

  // Set the values according to the synth
  switch(this->synth){
    case FM:
    case FMx10:
      this->mixer->gain(1, 1 );
    break;
    case AM:
    case AMx10:
      this->mixerDcMod->gain(0, 1);
      this->mixer->gain(0, 1 );
    break;
    case RING:
      this->mixer->gain(0, 1 );
    break;
  }
}

/**
 * Set the mode
 */
inline void Voice::setMode(byte modeValue){
  // Set the synth
  switch(modeValue){
    case 0 : this->mode = SYNTH; break;
    case 1 : this->mode = ARP;   break;
    case 2 : this->mode = DRONE; break;
  }

  this->output->gain(0, 0);
  this->output->gain(1, 0);
  
  // Set the values according to the mode
  switch(this->mode){
    case SYNTH:
    case ARP:
      this->output->gain(0, 1);
      this->output->gain(1, 0);
    break;
    case DRONE:
      this->output->gain(0, 0);
      this->output->gain(1, 1);
    break;
  }
}

/**
 * Set the modulaor frequency
 */
inline void Voice::setFrequency(int freq){
  this->sineAM->frequency(freq);
  this->sawtoothAM->frequency(freq);
  this->sineFM->frequency(freq);
  this->sawtoothFM->frequency(freq);
}

/**
 * Set the modulaor frequency
 */
inline void Voice::setModulatorFrequency(int freq){
  switch(this->synth){
    case FM:
    case AM:
      this->sineModulator->frequency(freq);
    break;
    case FMx10:
    case AMx10:
    case RING:
      this->sineModulator->frequency(freq*10);
    break;
  }
}
inline void Voice::setModulatorAmplitude(float amp){
  // In AM sineModulator amplitude should be fixed at 0.5 and dcAM too
  switch(this->synth){
    case AM:
    case AMx10:
      this->sineModulator->amplitude(0.5);
    break;
    default:
      this->sineModulator->amplitude(amp);
    break;
  }
  
  this->mixerAM2->gain(0, 1 - amp*4);
  this->mixerAM2->gain(1, amp*4);
}
inline void Voice::setAttack(int att){
  this->envelope->attack(att);
}
inline void Voice::setDecay(int dec){
  this->envelope->decay(dec);
}
inline void Voice::setRelease(int rel){
  this->envelope->release(rel);
}
#endif
