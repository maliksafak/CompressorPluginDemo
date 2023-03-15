#pragma once

#include "BallisticFilter.h"
#include <cmath>

template <typename T>
class Compressor
{
private:
    double samplerate;
    T threshold;
    T ratio;

    T inv_threshold;
    T inv_ratio;
    T inv_ratio_minus_one;

    std::unique_ptr<BallisticFilter<T>> filter;
public:
    Compressor(double samplerate_, size_t channel_count_, T threshold_db_, T ratio_, T attack_time_ms_, T release_time_ms_) {
        filter = std::unique_ptr<BallisticFilter<T>>(new BallisticFilter<T>(samplerate, channel_count_, attack_time_ms_ / static_cast<T>(1000.0), release_time_ms_ / static_cast<T>(1000.0)));
        
        set_samplerate(samplerate_);
        
        set_threshold_db(threshold_db_);
        set_ratio(ratio_);
    }
    ~Compressor() {

    }

    T next(size_t channel, T sample) {
        T envelope = filter->process(channel, sample);
        T gain = (envelope < threshold) 
                           ? static_cast<T>(1.0)
                           : std::pow (envelope * inv_threshold, inv_ratio_minus_one);
        return gain;
    }

    T process(size_t channel, T sample) {
        T gain = next(channel, sample);
        
        return sample * gain;
    }

    void set_samplerate(double samplerate_) {
        samplerate = samplerate_;
        filter->set_samplerate(samplerate);
    }
    double get_samplerate() {
        return samplerate;
    }
    void set_attack_time(T attack_time_) {
        filter->set_attack_time(attack_time_);
    }
    T get_attack_time() {
        return filter->get_attack_time();
    }
    void set_attack_time_ms(T attack_time_ms_) {
        filter->set_attack_time(attack_time_ms_ / static_cast<T>(1000.0));
    }
    T get_attack_time_ms() {
        return filter->get_attack_time() * static_cast<T>(1000.0);
    }
    void set_release_time(T release_time_) {
        filter->set_release_time(release_time_);
    }
    T get_release_time() {
        return filter->get_release_time();
    }
    void set_release_time_ms(T release_time_ms_) {
        filter->set_release_time(release_time_ms_ / static_cast<T>(1000.0));
    }
    T get_release_time_ms() {
        return filter->get_release_time() * static_cast<T>(1000.0);
    }
    void set_threshold(T threshold_) {
        threshold = threshold_ == 0.0 ? std::numeric_limits<T>::min() : threshold;
        inv_threshold = static_cast<T>(1.0) / threshold;
    }
    T get_threshold() {
        return threshold;
    }
    void set_threshold_db(T threshold_db_) {
        threshold = std::pow(static_cast<T>(10.0), threshold_db_ / static_cast<T>(20.0));
        inv_threshold = static_cast<T>(1.0) / threshold;
    }
    T get_threshold_db() {
        return static_cast<T>(20.0) * std::log10(threshold);
    }
    void set_ratio(T ratio_) {
        ratio = ratio_ < static_cast<T>(1.0) ? static_cast<T>(1.0) : ratio_;
        inv_ratio = static_cast<T>(1.0) / ratio;
        inv_ratio_minus_one = inv_ratio - static_cast<T>(1.0);
    }
    T get_ratio() {
        return ratio;
    }
};