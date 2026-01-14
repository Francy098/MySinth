#pragma once //Serve al compilatore per evitare inclusioni multiple

#include <algorithm>
#include <cmath>     // for std::floor

namespace MySinthOsc {

// Square oscillator: uscita +1/-1, duty adjustable (0..1) - (default 50%)
class SquareOsc {
public:
	SquareOsc() = default;
	void setSampleRate(double sr) { sampleRate = sr; updateInc(); } //aggiornamento sample rate
	void setFrequency(double f) { frequency = f; updateInc(); } //aggiornamento frequenza
	void setPulseWidth(double pw) { pulseWidth = std::max(0.0, std::min(1.0, pw)); } //aggiornamento duty cycle
	void reset(double ph = 0.0) { phase = ph - std::floor(ph); }    // reset fase
	float process() {
		phase += phaseInc;
		if (phase >= 1.0) phase -= 1.0;
		return (phase < pulseWidth) ? 1.0f : -1.0f;
	}
	double getPhase() const { return phase; }   
private:    
	double sampleRate = 44100.0;
	double frequency = 440.0;
	double phase = 0.0;
	double phaseInc = 0.0;
	double pulseWidth = 0.5; // 50% duty by default
	void updateInc() { phaseInc = (sampleRate > 0.0) ? (frequency / sampleRate) : 0.0; }
};

// Sawtooth oscillator: naive ramp from -1 to +1
class SawOsc {
public:
	SawOsc() = default;
	void setSampleRate(double sr) { sampleRate = sr; updateInc(); }
	void setFrequency(double f) { frequency = f; updateInc(); }
	void reset(double ph = 0.0) { phase = ph - std::floor(ph); }
	float process() {
		phase += phaseInc;
		if (phase >= 1.0) phase -= 1.0;
		return static_cast<float>(2.0 * phase - 1.0);
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