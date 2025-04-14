#ifndef SystemExclusiveManager_h
#define SystemExclusiveManager_h

// void printBytes(const byte *data, unsigned int size)
// {
//   while (size > 0)
//   {
//     byte b = *data++;
//     if (b < 16)
//       Serial.print(b);
//     Serial.print((char)b);
//     // Serial.print(b);
//     // if (size > 1)
//     //   Serial.print(' ');
//     size = size - 1;
//   }
//   Serial.println("");
// }

#include "Setting.h"
#include "lib/ArduinoJson-v7.1.0.h"

class SystemExclusiveManager
{
private:
  // Singleton
  static SystemExclusiveManager *instance;
  SystemExclusiveManager();
  static void mySystemExclusiveChunk(const byte *data, uint16_t length, bool last);
  static void sendConfiguration();
  static void receiveConfiguration(const byte *data);
  static void handleSystemExclusiveMessage();
  static const char CONFIG_REQUEST = 0x00;
  static const char CONFIG_EDIT = 0x01;
  static byte myDataBuffer[1080];
  static uint32_t dataBufferIndex;

public:
  static SystemExclusiveManager *getInstance();
};

// Singleton pre init
SystemExclusiveManager *SystemExclusiveManager::instance = nullptr;
// byte *SystemExclusiveManager::myDataBuffer = nullptr;
byte SystemExclusiveManager::myDataBuffer[1080] = {0};
uint32_t SystemExclusiveManager::dataBufferIndex = 0;

inline SystemExclusiveManager::SystemExclusiveManager()
{
  // myDataBuffer = new byte[1080];
  usbMIDI.setHandleSystemExclusive(SystemExclusiveManager::mySystemExclusiveChunk); // TODO: maybe move this to MidiManager
}

/**
   Singleton instance
*/
inline SystemExclusiveManager *SystemExclusiveManager::getInstance()
{
  if (!instance)
    instance = new SystemExclusiveManager;
  return instance;
}

inline void SystemExclusiveManager::mySystemExclusiveChunk(const byte *data, uint16_t length, bool last)
{
  // Starting byte + at least one byte of data + ending byte
  if (last && length < 3)
  {
    return;
  }

  // Copying data into dataBuffer
  dataBufferIndex = 0;
  for (uint16_t i = 0; i < length; i++)
  {
    myDataBuffer[dataBufferIndex++] = data[i];
  }

  if (last)
  {
    // After handling the last chunk, now let's handle the whole message
    handleSystemExclusiveMessage();
  }
}

inline void SystemExclusiveManager::handleSystemExclusiveMessage()
{
  if (dataBufferIndex < 3)
  {
    return;
  }

  // printBytes(myDataBuffer, dataBufferIndex);

  // Requesting configuration
  if (myDataBuffer[1] == CONFIG_REQUEST)
  {
    sendConfiguration();
  }
  // Receiving configuration
  else if (myDataBuffer[1] == CONFIG_EDIT)
  {
    byte *arrayPtr; // length - the 2 bytes of SysEx and 1 byte of message type
    byte array[dataBufferIndex - 3] = {0};
    for (uint32_t i = 2; i < dataBufferIndex - 1; i++)
    {
      array[i - 2] = myDataBuffer[i];
    }
    arrayPtr = array;

    SystemExclusiveManager::receiveConfiguration(arrayPtr);
  }
}

inline void SystemExclusiveManager::sendConfiguration()
{
  JsonDocument doc = Setting::serializeAll();

  uint8_t output[1080];

  size_t length = serializeJson(doc, output);

  usbMIDI.sendSysEx(length, output, false);
}

inline void SystemExclusiveManager::receiveConfiguration(const byte *data)
{
  JsonDocument doc;

  // Trying to deserualize it to validate before saving
  DeserializationError error = deserializeJson(doc, data);

  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  Setting::loadFromJson(doc);
  Setting::save();
}
#endif