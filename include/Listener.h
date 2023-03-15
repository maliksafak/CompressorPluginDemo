#include <JuceHeader.h>

class ThresholdListener : private AudioProcessorParameter::Listener
{
private:
    /* data */
public:
    ThresholdListener(/* args */) {}
    ~ThresholdListener() {}

    void parameterValueChanged(int parameterIndex, float newValue) override {

    }

    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {

    }
};