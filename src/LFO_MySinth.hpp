// ------------------------ LFO for MySinth -- Triangle wave only
#pragma once

#include <cmath>

class MySinthLFO {
public:
    // Backwards-compatible wrapper: same API as previous LFO class
public:
    LFO() {
        sampleRate = 44100.0;
        rateHz = 1.0;
        phase = 0.0;
        updatePhaseInc();
    }

    // Set sample rate (Hz)
    void setSampleRate(double sr) {
        sampleRate = (sr > 0.0) ? sr : 44100.0;
        updatePhaseInc();
    }

    // Set rate in Hz (single parameter 'rate')
    void setRate(double hz) {
        rateHz = (hz >= 0.0) ? hz : 0.0;
        updatePhaseInc();
    }

    // Reset phase (0..1)
    void reset(double ph = 0.0) { phase = ph - std::floor(ph); }

    // Process one sample, returns triangle in -1..1
    inline float process() {
        phase += phaseInc;
        if (phase >= 1.0) phase -= std::floor(phase);
        double v = (phase < 0.5) ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
        return static_cast<float>(v);
    }

    double getPhase() const { return phase; }

private:
    void updatePhaseInc() { phaseInc = (sampleRate > 0.0) ? rateHz / sampleRate : 0.0; }

    double sampleRate;
    double rateHz;
    double phase;
    double phaseInc;
};
