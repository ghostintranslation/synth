#ifndef MidiIO_h
#define MidiIO_h

class MidiIO
{
public:
    MidiIO(Setting *setting);
    byte getValue();
    void setValue(byte value);
    int16_t *getBlockData();
    Setting *getSetting();
    void onChange(void (*onChangeCallback)(byte value));
    // TODO: setChannelSetting

protected:
    Setting *setting = nullptr;
    byte value = 0;
    int16_t mappedValue = 0; // TODO: rename
    int16_t blockData[AUDIO_BLOCK_SAMPLES] = {0};
    void (*onChangeCallback)(byte) = nullptr;
};

inline MidiIO::MidiIO(Setting *setting = nullptr)
{
    this->setting = setting;
}

inline byte MidiIO::getValue()
{
    return this->value;
}

inline void MidiIO::setValue(byte value)
{
    if(this->value != value){
        if(this->onChangeCallback){
            this->onChangeCallback(value);
        }
    }
    this->value = value;
}

inline int16_t *MidiIO::getBlockData()
{
    return this->blockData;
}

inline Setting *MidiIO::getSetting()
{
    return this->setting;
}

inline void MidiIO::onChange(void (*onChangeCallback)(byte))
{
    this->onChangeCallback = onChangeCallback;
}
#endif