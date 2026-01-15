#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>

namespace StateVariableFilter {
/**
 *  Filtro a variabili di stato (SVF) di secondo ordine.
 * Implementazione basata sulla topologia TPT (Topology Preserving Transform).
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
        * @param resonance Qualità del filtro (0.0f a 1.0f)
        * @param sampleRate Frequenza di campionamento corrente
        */
        void updateParameters(float cutoff, float resonance, float sampleRate) {
            // Clamp di sicurezza per la frequenza
            cutoff = std::max(10.0f, std::min(cutoff, sampleRate * 0.45f)); // Wc: Evita aliasing oltre Nyquist
        
            // Calcolo coefficiente g (frequenza pre-warp)
            g = cutoff/(sampleRate * 2.0f);
        
            // Mappatura risonanza: 1.0f - resonance produce un damping pulito.
            // k controlla il feedback del filtro.
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
        // Risoluzione analitica del loop di feedback (Zero-Delay)
        float d = 1.0f + g * (g + k); 
        float a = 2.0f * k + g;  
        float Yh = (input - a * s1 - s2) / d;
        float Yb  = g * Yh + s1;
        s1 = g * Yh + Yb;   // Aggiornamento stato (Metodo trapezoidale)
        float Yl = g * Yb + s2;
        s2 = g * Yb + Yl;   // Aggiornamento stato (Metodo trapezoidale)
        
        return Yl;
    }
};

} // namespace MySinthDSP



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