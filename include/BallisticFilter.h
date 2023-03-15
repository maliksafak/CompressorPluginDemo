#pragma once

#include <cmath>
#include <vector>

template <typename T>
class BallisticFilter
{
public:
    double samplerate;
    T attack_time;
    T release_time;

    T attack_coeff;
    T release_coeff;

    std::vector<T> z_1;

    T calculate_coefficient(T time) {
        time = time == 0.0 ? static_cast<T>(0.0001) : time;
        return std::exp((-2.0 * M_PI) / ((T)samplerate * time));
    }
public:
    BallisticFilter(double samplerate_, size_t channels, T attack_time_, T release_time_ ) {
        samplerate = samplerate_;
        attack_time = attack_time_;
        release_time = release_time_;

        attack_coeff = calculate_coefficient(attack_time);
        release_coeff = calculate_coefficient(release_time);

        z_1.resize(channels);
        for (auto &&sample : z_1)
        {
            sample = static_cast<T>(0.0);
        }
    }
    ~BallisticFilter() {}

    T process(size_t channel, T sample) {
        sample = std::abs(sample);
        
        T coeff = sample > z_1[channel] ? attack_coeff : release_coeff;
        
        auto result = sample + (coeff * (z_1[channel] - sample));
        z_1[channel] = result;

        return result;
    }

    void set_samplerate(T samplerate_) {
        samplerate = samplerate_;
        attack_coeff = calculate_coefficient(attack_time);
        release_coeff = calculate_coefficient(release_time);
    }
    T get_samplerate() {
        return samplerate;
    }
    void set_attack_time(T attack_time_) {
        attack_time = attack_time_;
        attack_coeff = calculate_coefficient(attack_time);
    }
    T get_attack_time() {
        return attack_time;
    }
    void set_release_time(T release_time_) {
        release_time = release_time_;
        release_coeff = calculate_coefficient(release_time);
    }
    T get_release_time() {
        return release_time;
    }
    void set_channel_count(size_t channels_) {
        z_1.resize(channels_);
        for (auto &&sample : z_1)
        {
            sample = static_cast<T>(0.0);
        }
    }
    size_t get_channel_count() {
        return z_1.size();
    }
};