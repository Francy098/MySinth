#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>

namespace StateVariableFilter {
/**
 *  Filtro a variabili di stato (SVF) di secondo ordine.
 * Implementazione basata sulla topologia TPT (Topology Preserving Transform).
 * NOTE: resonance deve essere in [0.0f, 0.99f] per stabilità (perché G<1).
 */
    class StateVarFil {
     public:
        StateVarFil() {
            reset();
        };
        void reset() { //Resetta gli stati del filtro per evitare "pop" o instabilità iniziali.
            s1 = 0.0f;
            s2 = 0.0f;
            }
        /**
        * Calcola i coefficienti. Da chiamare quando cambiano i parametri.
        * @param cutoff Frequenza in Hz (es. 20.0f a 18000.0f)
        * @param resonance Qualità del filtro (MUST be in [0.0f, 0.99f] for stability)
        * @param sampleRate Frequenza di campionamento corrente
        */
        void updateParameters(float cutoff, float resonance, float sampleRate) {
            // Clamp di sicurezza per la frequenza
            cutoff = std::max(10.0f, std::min(cutoff, sampleRate * 0.45f)); // Evita aliasing oltre Nyquist
        
            // Calcolo coefficiente g con pre-warping corretto per SVF a topologia TPT
            float wd = 2.0f * M_PI * cutoff;
            g = std::tan(wd / (2.0f * sampleRate));
            // Clamp g per evitare instabilità numerica
            g = std::min(g, 1.0f);
        
            // Mappatura risonanza (damping): 1.0f - resonance produce un damping pulito.
            // k controlla il feedback del filtro, deve restare positivo e < 2.0
            resonance = std::max(0.0f, std::min(resonance, 0.99f)); // Clamp di sicurezza
            k = 2.0f - 2.0f * resonance;
        }
       /* enum class FilterMode {
            LOW_PASS,
            HIGH_PASS,
            BAND_PASS,
            NOTCH
        };*/    
    private:
        // Stati interni (memoria degli integratori)
        float s1 = 0.0f;
        float s2 = 0.0f;

        // Coefficienti calcolati
        float g = 0.0f; // Coefficiente di frequenza
        float k = 0.0f; // Coefficiente di risonanza
        //float r = 0.0f; // Coefficiente di smorzamento (non usato in questa implementazione)




    public:
    //Processa un singolo campione audio.
    float process(float input) {
        // Risoluzione analitica del loop di feedback (Zero-Delay) con formule corrette per SVF TPT
        float d = 1.0f + g * k + g * g; // Denominatore comune
        float a = k + g;    
        float Yh = (input - a * s1 - s2) / d;   // High-pass output
        float Yb  = g * Yh + s1;    // Band-pass output
        s1 = g * Yh + Yb;   // Aggiornamento stato (Metodo trapezoidale)
        float Yl = g * Yb + s2; // Low-pass output
        s2 = g * Yb + Yl;   // Aggiornamento stato (Metodo trapezoidale)
        
        return Yl;
    }

    //Ci serviranno in MySinth per salvare i vecchi valori di cutoff e resonance e controllare se sono cambiati
    float cutoff_old = 0.0f;
    float resonance_old = 0.0f;
};

} // namespace StateVariableFilter



//-----------------------------------------------------------------------------------------
//Versione con enum class FilterMode, per scelta tipo di filtro
/*    float process(float input, FilterMode mode = FilterMode::LOW_PASS) {

---
---

        // Switch per restituire il segnale desiderato
        switch (mode) {
            case FilterMode::LOW_PASS:  return Yl;
            case FilterMode::HIGH_PASS: return Yh;
            case FilterMode::BAND_PASS: return Yb;
            case FilterMode::NOTCH:     return Yh + Yl;
            default:                    return Yl;
        }
            */