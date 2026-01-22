#pragma once //Serve al compilatore per evitare inclusioni multiple

#include <algorithm>
#include <cmath>     // for std::floor

namespace MySinthOsc {

// PolyBLEP (Parker's Polynomial Band-Limited Step) anti-aliasing function
// Reduces aliasing at discontinuities
inline double polyBLEP(double t) {
	if (t < -1.0 || t > 1.0) return 0.0; // Outside the transition region

	if (t < 0.0) {	//Position before the discontinuity
		t += 1.0;
		return -0.5 * t * t;  // First half of polynomial
	} else {	// Position after the discontinuity
		t -= 1.0;
		return 0.5 * t * t;   // Second half of polynomial
	}
}

// Square oscillator: uscita +1/-1, duty adjustable (0..1) - (default 50%)
// With PolyBLEP anti-aliasing
class SquareOsc {
public:
	SquareOsc() = default;
	void setSampleRate(double sr) { sampleRate = sr; updateInc(); } //aggiornamento sample rate
	void setFrequency(double f) { frequency = f; updateInc(); } //aggiornamento frequenza
	void setPulseWidth(double pw) { pulseWidth = std::max(0.0, std::min(1.0, pw)); } //aggiornamento duty cycle
	void reset(double ph = 0.0) { phase = ph - std::floor(ph); }    // reset fase
	
	float process() {
		double lastPhase = phase;	// salva fase del campione precedente
		phase += phaseInc;	// incrementa fase per questo campione
		if (phase >= 1.0) phase -= 1.0; // wrap-around fase
		
		// Naive square wave output
		double output = (phase < pulseWidth) ? 1.0 : -1.0;
		
		// Apply PolyBLEP correction at phase wraparound
		double phaseInc_normalized = phaseInc;  // phaseInc is already normalized
		
		// Transition at pulseWidth edge (rising edge for standard square), cioè rileva la prima discontinuità
		if (lastPhase < pulseWidth && phase >= pulseWidth) {
			double t = (lastPhase + phaseInc - pulseWidth) / phaseInc_normalized;
			output += polyBLEP(t) * 2.0;  // *2.0 for amplitude correction
		}
		
		// Transition at phase wrap (falling edge), cioè rileva la seconda discontinuità
		if (lastPhase < 1.0 && phase < lastPhase) {  // phase wrapped
			double t = (lastPhase + phaseInc - 1.0) / phaseInc_normalized;
			output -= polyBLEP(t) * 2.0;  // *2.0 for amplitude correction
		}
		
		
		return static_cast<float>(output);
	}
	
	double getPhase() const { return phase; }   
private:    
	double sampleRate = 44100.0;
	double frequency = 440.0;
	double phase = 0.0;
	double phaseInc = 0.0;
	double pulseWidth = 0.5;
	void updateInc() { phaseInc = (sampleRate > 0.0) ? (frequency / sampleRate) : 0.0; }
};

// Sawtooth oscillator: naive ramp from -1 to +1
// With PolyBLEP anti-aliasing
class SawOsc {
public:
	SawOsc() = default;
	void setSampleRate(double sr) { sampleRate = sr; updateInc(); }
	void setFrequency(double f) { frequency = f; updateInc(); }
	void reset(double ph = 0.0) { phase = ph - std::floor(ph); }
	
	float process() {
		double lastPhase = phase;
		phase += phaseInc;
		if (phase >= 1.0) phase -= 1.0;
		
		// Naive sawtooth output: -1 to +1
		double output = 2.0 * phase - 1.0;
		
		// PolyBLEP correction at phase wraparound
		double phaseInc_normalized = phaseInc;
		if (lastPhase > phase) {  // phase wrapped around, detect discontinuity. Perché qui di discontinuità ce n'è solo una
			double t = (lastPhase + phaseInc - 1.0) / phaseInc_normalized;
			output -= polyBLEP(t) * 2.0;  // Subtract discontinuity jump
		}
		
		return static_cast<float>(output);
	}
	
	double getPhase() const { return phase; }
private:
	double sampleRate = 44100.0;
	double frequency = 440.0;
	double phase = 0.0;
	double phaseInc = 0.0;
	void updateInc() { phaseInc = (sampleRate > 0.0) ? (frequency / sampleRate) : 0.0; }
};

} // namespace MySinthOsc