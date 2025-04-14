#ifndef InputButton_h
#define InputButton_h

#include "Registrar.h"
#include "Input.h"

class InputButton : public Input, public Registrar<InputButton>
{
public:
    InputButton(int8_t index);
    void update(void);
    int16_t *&updateBefore(int16_t *&blockData);
    void setThresholds(int16_t onThreshold, int16_t offThreshold);
    void onChange(void (*onChangeCallback)(bool value));
    void onPush(void (*onPushCallback)());
    void onRelease(void (*onReleaseCallback)());

private:
    uint8_t index;
    int16_t offThreshold = 100;
    int16_t onThreshold = -100;
    bool value;
    void (*onChangeCallback)(bool value) = nullptr;
    void (*onPushCallback)() = nullptr;
    void (*onReleaseCallback)() = nullptr;
};

inline InputButton::InputButton(int8_t index) : Input(index)
{
}

inline void InputButton::update(void)
{
    Input::update();
}

inline int16_t *&InputButton::updateBefore(int16_t *&blockData)
{
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
    {
        if (blockData[i] > offThreshold)
        {
            blockData[i] = INT16_MIN;
        }
        else if (blockData[i] < onThreshold)
        {
            blockData[i] = INT16_MAX;
        }
    }

    if (blockData[0] == INT16_MIN)
    {
        if (this->value)
        {
            if (this->onChangeCallback)
            {
                this->onChangeCallback(false);
            }

            if (this->onReleaseCallback)
            {
                this->onReleaseCallback();
            }
        }
        this->value = false;
    }
    else
    {
        if (!this->value)
        {
            if (this->onChangeCallback)
            {
                this->onChangeCallback(true);
            }

            if (this->onPushCallback)
            {
                this->onPushCallback();
            }
        }
        this->value = true;
    }

    return blockData;
}

inline void InputButton::setThresholds(int16_t onThreshold, int16_t offThreshold)
{
    this->onThreshold = onThreshold;
    this->offThreshold = offThreshold;
}

inline void InputButton::onChange(void (*onChangeCallback)(bool value))
{
    this->onChangeCallback = onChangeCallback;
}

inline void InputButton::onPush(void (*onPushCallback)())
{
    this->onPushCallback = onPushCallback;
}

inline void InputButton::onRelease(void (*onReleaseCallback)())
{
    this->onReleaseCallback = onReleaseCallback;
}

#endif
