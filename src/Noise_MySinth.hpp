			
#pragma once //Serve al compilatore per evitare inclusioni multiple

#include <algorithm> // for std::max and std::min
#include <cmath>    // for std::floor
#include <random>   // for std::mt19937 and std::normal_distribution 

namespace NoiseGenerator {

    class WhiteNoise {
    public:
        WhiteNoise() = default;

		private:
            std::mt19937 gen{std::random_device{}()}; // Mersenne Twister RNG, seeded with random device
            std::normal_distribution<float> dist{0.0f, 1.0f}; // Distribuzione Gaussiana con media 0 e deviazione standard 1

        public:
            float process() {
			static constexpr float gain = 3.53553390593f; // 5 / sqrt(2)
            float white = dist(gen);
            return white * gain;
        }
        //Il gain serve per scalare il rumore bianco in modo che abbia lo stesso RMS di un'onda sinusoidale con ampiezza 5V.
        // Infatti, l'RMS di un'onda sinusoidale è A/sqrt(2), quindi per un'ampiezza di 5V, l'RMS è 5/sqrt(2) ≈ 3.5355V.
        // Moltiplicando il rumore bianco (RMS=1V) per questo fattore, otteniamo un segnale con lo stesso livello RMS=3.5355V.
    };
}

/*		// All noise is calibrated to 1 RMS.
		// Then they should be scaled to match the RMS of a sine wave with 5V amplitude.
		const float gain = 5.f / std::sqrt(2.f);            
            // White noise: equal power density
			float white = random::normal();---------------> Non disponibile
			//outputs[WHITE_OUTPUT].setVoltage(white * gain);
            return white * gain;*/