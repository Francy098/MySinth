			
#pragma once //Serve al compilatore per evitare inclusioni multiple

#include <algorithm> // for std::max and std::min
#include <cmath>    // for std::floor
#include <random>   // for std::mt19937 and std::normal_distribution 

namespace NoiseGenerator {

    class WhiteNoise {
    public:
        WhiteNoise() = default;

		private:
            std::mt19937 gen{std::random_device{}()};
            std::normal_distribution<float> dist{0.0f, 1.0f};

        public:
            float process() {
            const float gain = 5.f / std::sqrt(2.f);
            float white = dist(gen);
            return white * gain;
        }

    };
}

/*		// All noise is calibrated to 1 RMS.
		// Then they should be scaled to match the RMS of a sine wave with 5V amplitude.
		const float gain = 5.f / std::sqrt(2.f);            
            // White noise: equal power density
			float white = random::normal();---------------> Non disponibile
			//outputs[WHITE_OUTPUT].setVoltage(white * gain);
            return white * gain;*/