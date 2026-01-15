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
	//Aggiunta clamp
	/*void setFrequency(double f) { 
    frequency = std::max(0.0, f);  // Assicura freq >= 0
    updateInc(); 
}*/

	void setPulseWidth(double pw) { pulseWidth = std::max(0.0, std::min(1.0, pw)); } //aggiornamento duty cycle
	//------------------------------------------ std::min e std::max servono per limitare il valore del duty cycle tra 0 e 1 -> std::min() prende il valore minore tra 1 e pw, mentre std::max prende il valore maggiore tra 0 e il risultato di std::min 
	void reset(double ph = 0.0) { phase = ph - std::floor(ph); }    // reset fase
	//---------------------------------------- std::floor() arrotonda SEMPRE per difetto all'intero più vicino (ES. floor(2.9) = 2.0, floor(-2.9) = -3.0)
	float process() {
		phase += phaseInc;	// incrementa fase
		if (phase >= 1.0) phase -= 1.0; // wrap-around fase -> La fase va da o ad 1 (ciclo completo)
		return (phase < pulseWidth) ? 1.0f : -1.0f;	// output in base al duty cycle: se fase < duty cycle -> +1, altrimenti -1
	}
	double getPhase() const { return phase; }   
private:    
	double sampleRate = 44100.0;
	double frequency = 440.0; //C4=440Hz, frequenza di rifermento 
	double phase = 0.0;
	double phaseInc = 0.0; // incremento fase
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
		return static_cast<float>(2.0 * phase - 1.0);	// output saw from -1 to +1. Perché 2*phase va da 0 a 2, quindi sottraendo 1 otteniamo il range -1 a +1
		//------------------------------ cast statico per convertire il double in float
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