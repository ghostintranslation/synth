#ifndef Voice_h
#define Voice_h

#include <Audio.h>

// Custom types
enum synthesis {FM, FMx10, AM, AMx10, RING};
//enum modes {SYNTH, ARP, DRONE};

/*
// GUItool: begin automatically generated code
AudioSynthWaveformSine   sineModulator;  //xy=77,127
AudioSynthWaveformModulated sineFM;         //xy=266,110
AudioSynthWaveformModulated sawtoothFM;     //xy=277,174
AudioMixer4              mixer;          //xy=436,143
AudioMixer4              output;         //xy=576,220
AudioEffectEnvelope      envelope;       //xy=581,131
AudioOutputI2S           i2s2;           //xy=717,219
AudioConnection          patchCord1(sineModulator, 0, sawtoothFM, 0);
AudioConnection          patchCord2(sineModulator, 0, sineFM, 0);
AudioConnection          patchCord3(sineFM, 0, mixer, 0);
AudioConnection          patchCord4(sawtoothFM, 0, mixer, 1);
AudioConnection          patchCord5(mixer, envelope);
AudioConnection          patchCord6(mixer, 0, output, 1);
AudioConnection          patchCord7(output, 0, i2s2, 0);
AudioConnection          patchCord8(output, 0, i2s2, 1);
AudioConnection          patchCord9(envelope, 0, output, 0);
// GUItool: end automatically generated code
*/


/*
 * Voice
 */
class Voice{
  private:
    // FM
    AudioSynthWaveformModulated *sineFM;
    AudioSynthWaveformModulated *sawtoothFM;
    AudioSynthWaveformSine   *sineModulator;
    // Common
    AudioMixer4              *mixer;
    AudioEffectEnvelope      *envelope;
    AudioConnection* patchCords[7];
    // Output
    AudioMixer4 *output;

    synthesis synth;
//    modes mode;
    bool notePlayed;
    float freq = 0;
    float frequencyTarget = 0;
    byte octave = 0;
    byte currentNote = 0; // The midi note currently being played.
    byte intervalGlide = 255;
    byte updateMillis = 0;

  public:
    Voice();
    void update();

    unsigned long last_played;
    
    AudioMixer4 * getOutput();
    void noteOn(byte midiNote = 0);
    void noteOff();
    // Setters
    void setAR(unsigned int attack, unsigned int release);
    void setNotePlayed(bool notePlayed);
//    void setMode(byte mode);
    void setFrequency(float freq);
    void setFrequencyTarget(int freq);
    void setOctave(byte octave);
    void setModulatorFrequency(int freq);
    void setModulatorAmplitude(float amp);
    void setShape(float shape);
    void setAttack(int att);
    void setRelease(int rel);
    void setGlide(byte glide);
    void setUpdateMillis(byte updateMillis);
    void setCurrentNote(byte midiNote);
    // Getters
    bool isActive();
    bool isNotePlayed();
    byte getCurrentNote();
};

/**
 * Constructor
 */
inline Voice::Voice(){
  // FM
  this->sineFM = new AudioSynthWaveformModulated();
  this->sineFM->amplitude(1);
  this->sawtoothFM = new AudioSynthWaveformModulated();
  this->sawtoothFM->begin(WAVEFORM_SAWTOOTH);
  this->sawtoothFM->amplitude(1);
  this->mixer = new AudioMixer4();
  this->mixer->gain(0, 0.3 );
  this->mixer->gain(1, 0.2 );
  this->sineModulator = new AudioSynthWaveformSine();
  this->envelope = new AudioEffectEnvelope();
  this->envelope->sustain(1);
  this->output = new AudioMixer4();
  this->output->gain(0, 0);
  this->output->gain(1, 1);

  this->patchCords[0] = new AudioConnection(*this->sineModulator, 0, *this->sawtoothFM, 0);
  this->patchCords[1] = new AudioConnection(*this->sineModulator, 0, *this->sineFM, 0);
  this->patchCords[2] = new AudioConnection(*this->sineFM, 0, *this->mixer, 0);
  this->patchCords[3] = new AudioConnection(*this->sawtoothFM, 0, *this->mixer, 1);
  this->patchCords[4] = new AudioConnection(*this->mixer, 0, *this->envelope, 0);
  this->patchCords[5] = new AudioConnection(*this->envelope, 0, *this->output, 0);
  this->patchCords[6] = new AudioConnection(*this->mixer, 0, *this->output, 1);

//  this->setMode(0);
  this->notePlayed = false;
}

/**
 * Update
 */
inline void Voice::update(){
//  this->intervalGlide = 255;
  if(this->intervalGlide < 254){
      if(this->frequencyTarget != this->freq){
        this->freq += ((float)this->intervalGlide * (this->frequencyTarget - this->freq) / (float)255)  / ((float)10 / (float)this->updateMillis);
      }

      if(roundf(this->freq * 100) / 100 == 0){
        this->freq = 0;
      }
  }else{
    this->freq = this->frequencyTarget;
  }
  
  this->setFrequency(this->freq);
}

/**
 * Set Attack Release
 */
inline void Voice::setAR(unsigned int attack, unsigned int release){
  this->envelope->attack(attack);
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
  this->currentNote = midiNote;
  this->last_played = millis();
  this->notePlayed=true;
  this->frequencyTarget = 440.0 * powf(2.0, (float)(this->currentNote - 93 + 12*this->octave) * 0.08333333);
  this->envelope->noteOn();
}

/**
 * Note off
 */
inline void Voice::noteOff() {
  this->envelope->noteOff();
//  this->frequencyTarget = this->freq;
  this->notePlayed = false;
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
 * Set the mode
 */
//inline void Voice::setMode(byte modeValue){
//  // Set the synth
//  switch(modeValue){
//    case 0 : this->mode = SYNTH; break;
//    case 1 : this->mode = ARP;   break;
//    case 2 : this->mode = DRONE; break;
//  }
//
//  this->output->gain(0, 0);
//  this->output->gain(1, 0);
//  
//  // Set the values according to the mode
//  switch(this->mode){
//    case SYNTH:
//    case ARP:
//      this->output->gain(0, 1);
//      this->output->gain(1, 0);
//      this->intervalGlide = 0;
//    break;
//    case DRONE:
//      this->output->gain(0, 0);
//      this->output->gain(1, 1);
//      this->intervalGlide = 0;
//    break;
//  }
//}

/**
 * Set the frequency
 */
inline void Voice::setFrequency(float freq){
  this->freq = freq;
  this->sineFM->frequency(freq);
  this->sawtoothFM->frequency(freq);
}

/**
 * Set the trgeted frequency
 */
inline void Voice::setFrequencyTarget(int frequencyTarget){
  this->frequencyTarget = frequencyTarget;
}

/**
 * Set the octave
 */
inline void Voice::setOctave(byte octave){
  this->octave = octave;
}

/**
 * Set the modulaor frequency
 */
inline void Voice::setModulatorFrequency(int freq){
  this->sineModulator->frequency(freq);
}
inline void Voice::setModulatorAmplitude(float amp){
  this->sineModulator->amplitude(amp);
}

/**
 * Set envelope attack
 */
inline void Voice::setAttack(int att){
  this->envelope->attack(att);
}

/**
 * Set envelope release
 */
inline void Voice::setRelease(int rel){
  this->envelope->release(rel);
}

/**
 * Set the shape
 */
inline void Voice::setShape(float shape){
  this->mixer->gain(0, (1 - shape) * 0.3 );
  this->mixer->gain(1, shape * 0.2 );
}


/**
 * Set the glide
 */
inline void Voice::setGlide(byte glide){
  this->intervalGlide = glide;
}

/**
 * Get the voice's current note
 */
inline byte Voice::getCurrentNote(){
  return this->currentNote;
}

inline void Voice::setCurrentNote(byte midiNote){
  this->currentNote = midiNote;
  this->freq = 440.0 * powf(2.0, (float)(this->currentNote - 69) * 0.08333333);
}

inline void Voice::setUpdateMillis(byte updateMillis){
  this->updateMillis = updateMillis;
}
#endif
