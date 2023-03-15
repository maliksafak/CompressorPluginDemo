#include "Plugin.h"

template <typename T>
inline T lr_mag(T left, T right) {
    if (left == static_cast<T>(0.0) && right == static_cast<T>(0.0))
    {
        return static_cast<T>(0.0);
    }
    
    T Lx = std::abs(left) * static_cast<T>(-0.7071067811865475);
    T Rx = std::abs(right) * static_cast<T>(0.7071067811865476);
    T magnitude = (Lx + Rx) / (std::sqrt((Lx * Lx) + (Rx * Rx)));
    return magnitude;
}

template <typename T>
inline T lr_mag(double samplerate, T time, T left, T right) {
    const T coeff = std::exp((-2.0 * M_PI) / (samplerate * time));
    static T z_1 = static_cast<T>(0.0);

    if (left == static_cast<T>(0.0) && right == static_cast<T>(0.0))
    {
        return static_cast<T>(0.0);
    }
    
    T Lx = std::abs(left) * static_cast<T>(-0.7071067811865475);
    T Rx = std::abs(right) * static_cast<T>(0.7071067811865476);
    T magnitude = (Lx + Rx) / (std::sqrt((Lx * Lx) + (Rx * Rx)));
    z_1 = magnitude + (coeff * (z_1 - magnitude));
    return z_1;
}

template <typename T>
inline T lowpass(double samplerate, T time, T sample, T* z_1) {
    const T coeff = std::exp((-2.0 * M_PI) / (samplerate * time));

    *z_1 = sample + (coeff * (*z_1 - sample));
    return *z_1;
}

AudioPlugin::AudioPlugin()
    : AudioProcessor(BusesProperties().withInput("Input", AudioChannelSet::stereo())
                                      .withOutput("Output", AudioChannelSet::stereo()))
{
    addParameter(cutoff = new AudioParameterFloat({"cutoff", 1}, "Cutoff", 1.0f, 1000.0f, 1.0f));
    addParameter(threshold = new AudioParameterFloat({"threshold", 1}, "Threshold", -120.0f, 0.0f, -12.0f));
    addParameter(ratio = new AudioParameterFloat({"ratio", 1}, "Ratio", 1.0f, 10.0f, 1.0f));
    addParameter(attack_time = new AudioParameterFloat({"attack_time", 1}, "Attack Time", NormalisableRange<float>(1.0f, 1000000.0f), 1000.0f));
    addParameter(release_time = new AudioParameterFloat({"release_time", 1}, "Release Time", NormalisableRange<float>(1.0f, 1000000.0f), 100000.0f));
    addParameter(seperate = new AudioParameterBool({"seperate", 1}, "Seperate Channels", false));
}

void AudioPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock)
{
    leftz_1 = new double;
    rightz_1 = new double;
    compressor = new Compressor<double>(sampleRate, 2, threshold->get(), ratio->get(), attack_time->get(), release_time->get());
}

void AudioPlugin::releaseResources()
{

}

template <typename T>
void AudioPlugin::processSamples(AudioBuffer<T> &audioBuffer, MidiBuffer &midiBuffer)
{
    auto reader = audioBuffer.getArrayOfReadPointers();
    auto writer = audioBuffer.getArrayOfWritePointers();

    compressor->set_threshold_db(threshold->get());
    compressor->set_ratio(ratio->get());
    compressor->set_attack_time_ms(attack_time->get() / 1000.0);
    compressor->set_release_time_ms(release_time->get() / 1000.0);
    
    auto samplerate = getSampleRate();
    double time = (double)cutoff->get() / 1000.0;

    if (audioBuffer.getNumChannels() == 2)
    {
        for (size_t sample = 0; sample < (size_t)audioBuffer.getNumSamples(); sample++)
        {
            T left = reader[0][sample];
            T right = reader[1][sample];
            T mag = lr_mag<T>(samplerate, time, left, right);

            T gain_left = compressor->next(0, left);
            gain_left = lowpass<double>(samplerate, time, gain_left, leftz_1);
            T gain_right = compressor->next(1, right);
            gain_right = lowpass<double>(samplerate, time, gain_right, rightz_1);
            
            writer[0][sample] = mag < 0.0 ? gain_left * left : left;
            writer[1][sample] = mag > 0.0 ? gain_right * right : right;
        }
    }
}

void AudioPlugin::processBlock(AudioBuffer<float> &audioBuffer, MidiBuffer &midiBuffer)
{
    processSamples<float>(audioBuffer, midiBuffer);
}

void AudioPlugin::processBlock(AudioBuffer<double> &audioBuffer, MidiBuffer &midiBuffer)
{
    processSamples<double>(audioBuffer, midiBuffer);
}

void AudioPlugin::getStateInformation(MemoryBlock &destData)
{
    MemoryOutputStream *memoryOutputStream = new MemoryOutputStream(destData, true);
    // memoryOutputStream->writeFloat(*gain);
    delete memoryOutputStream;
}

void AudioPlugin::setStateInformation(const void *data, int sizeInBytes)
{
    MemoryInputStream *memoryInputStream = new MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false);
    // gain->setValueNotifyingHost(memoryInputStream->readFloat());
    delete memoryInputStream;
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPlugin();
}