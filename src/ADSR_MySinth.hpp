#pragma once

#include <algorithm>
#include <cmath>

namespace EnvelopeGenerator {

// Envelop Generator monofonico, parametri normalizzati [0,1]
class ADSR {
public:
    // Costanti di tempo come nel modulo ADSR originale
    static constexpr float MIN_TIME = 1e-3f;   // 1 ms
    static constexpr float MAX_TIME = 10.0f;   // 10 s
    static constexpr float LAMBDA_BASE = MAX_TIME / MIN_TIME; // 10000
    static constexpr float ATT_TARGET = 1.2f;  // overshoot target per attack

    ADSR() { reset(); }

    void reset() {
        env_ = 0.0f;
        stage_ = Stage::Idle;
    }

    // Aggiorna i parametri normalizzati [0,1]
    void setAttack(float attackNorm)  { attackNorm_  = clamp01(attackNorm); } //clamp normalizza il valore tra 0 e 1
    void setDecay(float decayNorm)    { decayNorm_   = clamp01(decayNorm); }
    void setSustain(float sustainNorm){ sustainNorm_ = clamp01(sustainNorm); }
    void setRelease(float releaseNorm){ releaseNorm_ = clamp01(releaseNorm); }

    // Processa un campione. gateOn true = nota tenuta, false = release
    // Restituisce l'envelope in [0, 1.2] circa (poi scala esternamente)
    float process(bool gateOn, float sampleTime) {
        // precompute lambdas
        const float attackLambda  = normToLambda(attackNorm_); // tempo in secondi -> lambda = 1/T
        const float decayLambda   = normToLambda(decayNorm_);
        const float releaseLambda = normToLambda(releaseNorm_);
        const float sustain       = sustainNorm_;

        // Controllo stato della nota
        if (gateOn) stage_ = (stage_ == Stage::Idle || stage_ == Stage::Release) ? Stage::Attack : stage_;
        else stage_ = (stage_ != Stage::Idle && stage_ != Stage::Release) ? Stage::Release : stage_; 
        
        //Versione estesa di sopra: NON CANCELLARE
        /*if (gateOn) {
            if (stage_ == Stage::Idle || stage_ == Stage::Release) { // note on
                stage_ = Stage::Attack;
            }
        } else {
            if (stage_ != Stage::Idle && stage_ != Stage::Release) { // note off
                stage_ = Stage::Release;
            }
        }*/

        switch (stage_) {
            case Stage::Attack:
                env_ += (ATT_TARGET - env_) * attackLambda * sampleTime;
                if (env_ >= 1.0f) {
                    env_ = 1.0f;
                    stage_ = Stage::Decay;
                }
                break;
            case Stage::Decay:
                env_ += (sustain - env_) * decayLambda * sampleTime;
                if (env_ <= sustain) {
                    env_ = sustain;
                    stage_ = Stage::Sustain;
                }
                break;
            case Stage::Sustain:
                env_ = sustain;
                break;
            case Stage::Release:
                env_ += (0.0f - env_) * releaseLambda * sampleTime;
                if (env_ <= 0.0001f) {
                    env_ = 0.0f;
                    stage_ = Stage::Idle;
                }
                break;
            case Stage::Idle:
            default:
                env_ = 0.0f;
                break;
        }
        return env_;
    }

    float value() const { return env_; }

private:
    enum class Stage { Idle, Attack, Decay, Sustain, Release };

    float env_ = 0.0f;
    Stage stage_ = Stage::Idle;

    float attackNorm_  = 0.5f;
    float decayNorm_   = 0.5f;
    float sustainNorm_ = 0.5f;
    float releaseNorm_ = 0.5f;

    //Normalizza il valore tra 0 e 1
    static float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }

    //Converti il parametro normalizzato in lambda, trasformando il tempo in secondi
    static float normToLambda(float tNorm) {
        // tNorm in [0,1] -> tempo in secondi 1e-3 .. 10s -> lambda = 1/T
        float timeSec = MIN_TIME * std::pow(LAMBDA_BASE, tNorm);
        return 1.0f / timeSec;
    }
};

} // namespace EnvelopeGenerator
