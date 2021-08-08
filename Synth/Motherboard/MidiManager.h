#ifndef MidiManager_h
#define MidiManager_h

#include <vector>
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI); // MIDI library init

using MidiNoteOnCallback = void (*)(byte channel, byte note, byte velocity);
using MidiNoteOffCallback = void (*)(byte channel, byte note, byte velocity);
using GlobalMidiControlChangeCallback = void (*)(byte, byte, byte);
using MidiSysExCallback = void (*)(const uint8_t*, uint16_t, bool);
using MidiControlChangeCallbackFunction = void (*)(byte, byte, byte);

class MidiManager
{
public:
    // Init
    void init();

    // Update
    void update();
    
    // Debug
    void print();

    void sendSysEx(uint32_t length, const uint8_t *data, bool hasTerm=false);
    void setMidiChannel(byte channel);
    byte getMidiChannel();
    void setHandleMidiNoteOn(MidiNoteOnCallback fptr);
    void setHandleMidiNoteOff(MidiNoteOffCallback fptr);
//    void setHandleGlobalMidiControlChange(GlobalMidiControlChangeCallback fptr);
    void setHandleMidiControlChange(byte control, String controlName, MidiControlChangeCallbackFunction fptr);
    void setHandleMidiControlChange(byte midiChannel, byte midiCC, String controlName, MidiControlChangeCallbackFunction fptr);
    void setHandleMidiSysEx(MidiSysExCallback fptr);

    struct MidiControlChangeCallback{
      String controlName = "";
      MidiControlChangeCallbackFunction callback = nullptr;
    };
    
private:
    // Only Motherboard can access this instance
    friend class Motherboard;
  
    // Singleton
    static MidiManager *getInstance();
    
    // Singleton
    static MidiManager *instance;
    MidiManager();

    // Channel
    byte midiChannel;

    // Callbacks
    MidiNoteOnCallback midiNoteOnCallback;
    MidiNoteOffCallback midiNoteOffCallback;
    GlobalMidiControlChangeCallback globalMidiControlChangeCallback;
    MidiSysExCallback midiSysExCallback;
    std::vector<MidiControlChangeCallback> midiControlChangeCallbacks;
    MidiControlChangeCallback *getMidiControlChangeCallback(String name);
      
    // Handle MIDI
    static void handleMidiSysEx(const uint8_t* data, uint16_t length, bool last);
    static void handleMidiControlChange(byte channel, byte control, byte value);  
};


// Instance pre init
MidiManager *MidiManager::instance = nullptr;

/**
 * Constructor
 */
inline MidiManager::MidiManager()
{
}

/**
 * Singleton instance
 */
inline MidiManager *MidiManager::getInstance()
{
    if (!instance)
        instance = new MidiManager;
    return instance;
}

inline void MidiManager::init()
{
  MIDI.setHandleControlChange(this->handleMidiControlChange);
//   usbMIDI.setHandleNoteOn(this->midiNoteOnCallback);
//   usbMIDI.setHandleNoteOff(this->midiNoteOffCallback);
//   usbMIDI.setHandleSystemExclusive(this->handleMidiSysEx);
//   usbMIDI.setHandleControlChange(this->handleMidiControlChange);

  Serial1.begin(31250, SERIAL_8N1_RXINV);
}

inline void MidiManager::update()
{
  MIDI.read();
  usbMIDI.read();
}

inline void MidiManager::print()
{
  Serial.print("Midi channel:");
  Serial.print(this->midiChannel);
  Serial.println("");
}


inline void MidiManager::sendSysEx(uint32_t length, const uint8_t *data, bool hasTerm){
  usbMIDI.sendSysEx(length, data, hasTerm);
}

inline void MidiManager::setMidiChannel(byte channel){
  this->midiChannel = channel;
}

inline byte MidiManager::getMidiChannel(){
  return this->midiChannel;
}

/**
 * Handle MIDI note on
 */
 inline void MidiManager::setHandleMidiNoteOn(MidiNoteOnCallback fptr){
   this->midiNoteOnCallback = fptr;
   MIDI.setHandleNoteOn(this->midiNoteOnCallback);
 }

/**
 * Handle MIDI note off
 */
// inline void MidiManager::setHandleMidiNoteOff(MidiNoteOffCallback fptr){
//   this->midiNoteOffCallback = fptr;
//}

/**
 * Handle MIDI control change
 */
// inline void MidiManager::setHandleGlobalMidiControlChange(GlobalMidiControlChangeCallback fptr){
//   this->globalMidiControlChangeCallback = fptr;
// }

/**
 * Handle MIDI control change
 */
inline void MidiManager::setHandleMidiControlChange(byte control, String controlName, MidiControlChangeCallbackFunction fptr){
  this->setHandleMidiControlChange(0, control, controlName, fptr);
}

/**
 * Set a callback to a MIDI control change message
 */
inline void MidiManager::setHandleMidiControlChange(byte channel, byte control, String controlName, MidiControlChangeCallbackFunction fptr){
  // Init the midi control with what is set in the code
//  ConfigManager::MidiControl midiControl = {
//    "", // controlName
//    midiCC: control,
//    midiChannel: channel
//  };
  
//  controlName.toCharArray(midiControl.controlName, 20);
  
  // Check if something is defined in the config for that callback
  bool midiControlFromConfig = false;
//  for(ConfigManager::MidiControl c : ConfigManager.getMidiControls()) {
//    char controlNameCharArray[20];
//    controlName.toCharArray(controlNameCharArray, 20);
//  
//    if(strcmp(c.controlName, controlNameCharArray) == 0){
//      // This callback exists in the config so we'll use it instead
//      midiControl = c;
//      midiControlFromConfig = true;
//      break;
//    }
//  }

  // Only if the MidiControl is not yet in the config we add it
  // The MidiControl would be in the config if it was loaded from memory on the Init
  if(!midiControlFromConfig){
//    ConfigManager.addMidiControl(midiControl);
  }
  // Add the callback to the list
  this->midiControlChangeCallbacks.push_back({
    controlName: controlName,
    callback: fptr
  });
}

/**
 * Handle received MIDI Control Change message
 */
inline void MidiManager::handleMidiControlChange(byte channel, byte control, byte value){  
  // Internal midi channel is from 1 to 16
  // Incoming channel is from 1 to 16
  // Callbacks channel is from 0 to 16, 0 meaning bounded with internal midi channel
  
  // If the incoming message's channel corresponds to the board's channel 
  if(getInstance()->midiChannel == channel){
    // Callbacks on internal channel 0 are to be triggered on the board's channel
//    for(ConfigManager::MidiControl mc : ConfigManager.getMidiControls()) {
//      if(mc.midiChannel == 0 && mc.midiCC == control){
//        MidiControlChangeCallback *c = MidiManager::getInstance()->getMidiControlChangeCallback(mc.controlName);
//        if(c != nullptr && c->callback != nullptr){
//          c->callback(0, control, value);
//        }
//      }
//    }
  }

  // Triggering any existing callback on the incoming channel and control
//  for(ConfigManager::MidiControl mc : ConfigManager.getMidiControls()) {
//    if(mc.midiChannel == channel && mc.midiCC == control){
//      MidiControlChangeCallback *c = MidiManager::getInstance()->getMidiControlChangeCallback(mc.controlName);
//      if(c != nullptr && c->callback != nullptr){
//        c->callback(channel, control, value);
//      }
//    }
//  }

  // Triggering the global callback
  if(MidiManager::getInstance()->globalMidiControlChangeCallback != nullptr){
    MidiManager::getInstance()->globalMidiControlChangeCallback(channel, control, value);
  }
}

/**
 * Handle MIDI SysEx
 * This is used to override the default handleMidiSysEx
 */
inline void MidiManager::setHandleMidiSysEx(MidiSysExCallback fptr){
  this->midiSysExCallback = fptr;
  usbMIDI.setHandleSystemExclusive(fptr);
}

/**
 * Handle MIDI SysEx message
 */
inline void MidiManager::handleMidiSysEx(const uint8_t* data, uint16_t length, bool last){
//  switch(data[1]){
//    // Device config request
//    // Sending reply in a JSON string
//    case 0:
//    {
//      String response = ConfigManager.toJSON();
//      
//      byte byteResponse[response.length()];
//      response.getBytes(byteResponse, response.length()+1);
//      byte byteTypedResponse[sizeof(byteResponse)+1];
//      byteTypedResponse[0] = 0;
//      for(unsigned int i=0; i<sizeof(byteResponse); i++){
//        byteTypedResponse[i+1] = byteResponse[i];
//      }
//      getInstance()->sendSysEx(sizeof(byteTypedResponse), byteTypedResponse, false);
//    }
//    break;
//    
//    // Device config import
//    case 1:
//    {
//      // The response format is:
//      // 1 [Callback_Name_Variable_Length] [midiCC] [midiChannel]
//      // 
//      // Since the name's length is variable, and there is no delimiting character surrounding it,
//      // one way of finding out if this message is for that callback is to search for known names in the message
//
//      bool somethingToSave = false;
//      
//      // Converts the response into string
//      String dataString = String((char *)data);
//
//      // Loop through all known callbacks and search each callback's name in the response
//      // If the name is found then the message is meant for that callback
//      byte i=0;
//      for(ConfigManager::MidiControl& mc: ConfigManager.getMidiControls()) {
//         MidiControlChangeCallback *c = getInstance()->getMidiControlChangeCallback(mc.controlName);
//         if(c != nullptr && c->callback != nullptr){
//           int indexOfCallbackName = dataString.lastIndexOf(mc.controlName, 4);
//           if(indexOfCallbackName > -1){
//             // This callback's name was found in the response
//
//             if(mc.midiCC != data[2 + String(mc.controlName).length()] ||
//                mc.midiChannel != data[2 + String(mc.controlName).length() + 1]){
//                 // Save only if there is actually a change
//                 somethingToSave = true;
//                
//                 mc.midiCC = data[2 + String(mc.controlName).length()];
//                 mc.midiChannel = data[2 + String(mc.controlName).length() + 1];
//                }
//             break;
//           }
//         }else{
//          // This midi control was not found in the callbacks
//          // so it should be deleted
//          ConfigManager.deleteMidiControl(i);
//         }
//         i++;
//      }
//
//      // Saving the config
//      if(somethingToSave){
//        ConfigManager.save();
//        
//        // Sending OK
//        byte byteMessage[2] = {1,1};
//        getInstance()->sendSysEx(2, byteMessage, false);
//        
//        LedManager.initSequence();
//      }else{
//        // Sending KO
//        byte byteMessage[2] = {1,0};
//        getInstance()->sendSysEx(2, byteMessage, false);
//      }
//    }
//    break;
//    
//    default:
//    break;
//  }

  if(MidiManager::getInstance()->midiSysExCallback){
    MidiManager::getInstance()->midiSysExCallback(data, length, last);
  }
}

/**
 * Get MIDI Control Change Callback
 * @param (String) name The name of the callback function
 * @return (MidiControlChangeCallback) The callback function
 */
inline MidiManager::MidiControlChangeCallback *MidiManager::getMidiControlChangeCallback(String name){
  for(MidiControlChangeCallback &c : this->midiControlChangeCallbacks) {
    if(c.controlName == name){
      return &c;
    }
  }

  return nullptr;
}

#endif
