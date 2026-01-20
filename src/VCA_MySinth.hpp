#pragma once

#include <algorithm>
#include <cmath>

namespace VCAGenerator {

// VCA monofonico semplice con controllo level e CV opzionale
class VCA {
public:
    // level e cv sono normalizzati in [0,1]
    void setLevel(float level) { level_ = clamp01(level); }
    void setCv(float cv) { cv_ = clamp01(cv); }
    void setExpMode(bool exp) { expMode_ = exp; }

    // Calcola il gain complessivo (level * cv), con curva opzionale esponenziale
    float gain() const {
        float g = level_ * cv_;
        if (expMode_) {
            // curva esponenziale morbida: pow(g, 4) come VCA-1 exponential
            g = std::pow(g, 4.0f);
        }
        return g;
    }

    // Processa un sample
    float process(float input) const {
        return input * gain();
    }

private:
    float level_ = 1.0f; // knob
    float cv_ = 1.0f;    // CV normalizzato [0,1]
    bool expMode_ = false;

    static float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }
};

} // namespace VCAGenerator
