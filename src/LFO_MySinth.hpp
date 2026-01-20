// ------------------------ LFO for MySinth -- Triangle wave only
#pragma once

#include <cmath>

namespace LowFrequencyOscillator {

    class MySinthLFO {
public:
    // Backwards-compatible wrapper: same API as previous LFO class
public:
    MySinthLFO() {
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
        if (phase >= 1.0) phase -= std::floor(phase); // wrap-around phase
        double v = (phase < 0.5) ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
        // Traduzione:
        //(phase < 0.5) siamo nella prima metà del ciclo (0..0.5)?
        // Se sì, allora calcola la salita della rampa: da -1 a +1 in 0.5 cicli -> pendenza positiva = 4.0*phase - 1.0
        // Altrimenti, calcola la discesa della rampa: da +1 a -1 in 0.5 cicli -> pendenza negativa = 3.0 - 4.0*phase
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
}

// Vecchia versione del VCO per riferimento
/*    // Process one sample, returns triangle in -1..1
    inline float process() {
        phase += phaseInc;
        if (phase >= 1.0) phase -= std::floor(phase);
        double v = (phase < 0.5) ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
        // Traduzione:
        //(phase < 0.5) siamo nella prima metà del ciclo (0..0.5)?
        // Se sì, allora calcola la salita della rampa: da -1 a +1 in 0.5 cicli -> pendenza positiva = 4.0*phase - 1.0
        // Altrimenti, calcola la discesa della rampa: da +1 a -1 in 0.5 cicli -> pendenza negativa = 3.0 - 4.0*phase
        return static_cast<float>(v);
    }*/