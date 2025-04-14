#ifndef Setting_h
#define Setting_h

#include "Registrar.h"
#include <EEPROM.h>
#include "lib/ArduinoJson-v7.1.0.h"
#include "lib/ArduinoStreamUtils/StreamUtils.h"

class Setting : public Registrar<Setting>
{
public:
    Setting(String id, String name, float defaultValue, float min, float max, float step);
    String getId();
    String getName();
    float getDefaultValue();
    float getValue();
    float getMin();
    float getMax();
    float getStep();
    void setValue(float value);
    JsonDocument serialize(bool onlyIdandValue);
    static JsonDocument serializeAll(bool onlyIdandValue);
    static void loadFromMemory();
    static void loadFromJson(JsonDocument doc);
    static void save();

private:
    String id;
    String name;
    float defaultValue = NAN;
    float value = NAN;
    float min = NAN;
    float max = NAN;
    float step = NAN;
};

inline Setting::Setting(String id, String name, float defaultValue, float min, float max, float step)
{
    this->id = id;
    this->name = name;
    this->defaultValue = defaultValue;
    this->min = min;
    this->max = max;
    this->step = step;

    Setting::loadFromMemory();
}

inline String Setting::getId()
{
    return this->id;
}

inline String Setting::getName()
{
    return this->name;
}

inline float Setting::getValue()
{
    return this->value;
}

inline float Setting::getDefaultValue()
{
    return this->defaultValue;
}

inline float Setting::getMin()
{
    return this->min;
}

inline float Setting::getMax()
{
    return this->max;
}

inline float Setting::getStep()
{
    return this->step;
}

inline void Setting::setValue(float value)
{
    this->value = value;
}

inline JsonDocument Setting::serialize(bool onlyIdandValue = false)
{
    JsonDocument doc;

    // create an object
    JsonObject setting = doc.to<JsonObject>();
    setting["id"] = this->getId();
    setting["value"] = this->getValue();
    if (!onlyIdandValue)
    {
        setting["name"] = this->getName();
        setting["defaultValue"] = this->getDefaultValue();
        setting["min"] = this->getMin();
        setting["max"] = this->getMax();
        setting["step"] = this->getStep();
    }
    return doc;
}

inline JsonDocument Setting::serializeAll(bool onlyIdandValue = false)
{
    JsonDocument doc;

    for (unsigned int i = 0; i < Setting::getCount(); i++)
    {
        doc["settings"][i] = Setting::get(i)->serialize(onlyIdandValue);
    }

    doc.shrinkToFit();

    return doc;
}

inline void Setting::loadFromMemory()
{
    // Find EOF index to know the length
    uint16_t eofIndex = 0;
    for (int i = 0; i < EEPROM.length(); i++)
    {
        // Serial.print((char)EEPROM.read(i));
        if (EEPROM.read(i) == 26)
        {
            eofIndex = i;
            break;
        }
    }

    JsonDocument doc;
    EepromStream eepromStream(0, eofIndex + 1);

    DeserializationError error = deserializeJson(doc, eepromStream);

    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    Setting::loadFromJson(doc);
}

inline void Setting::loadFromJson(JsonDocument doc)
{
    JsonArray settings = doc["settings"];
    for (JsonVariant item : settings)
    {
        for (unsigned int i = 0; i < Setting::getCount(); i++)
        {
            Setting *setting = Setting::get(i);
            if (item["id"] == setting->get(i)->getId())
            {
                setting->setValue(item["value"]);
                // TODO: Add callback on value change
            }
        }
    }
}

inline void Setting::save()
{
    EepromStream eepromStream(0, EEPROM.length());

    JsonDocument doc = Setting::serializeAll(true);
    uint16_t length = serializeJson(doc, eepromStream);
    EEPROM.write(length, 26); // End of file char
}

#endif