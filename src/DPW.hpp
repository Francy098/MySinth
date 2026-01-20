/*--------------------------- ABC ---------------------------------*
 *
 * Author: Leonardo Gabrielli <l.gabrielli@univpm.it>
 * License: GPLv3
 *
 * For a detailed guide of the code and functions see the book:
 * "Developing Virtual Synthesizers with VCV Rack" by L.Gabrielli
 *
 * Copyright 2020, Leonardo Gabrielli
 *
 *-----------------------------------------------------------------*/

#pragma once

#include "rack.hpp"

using namespace rack;

typedef enum {
	DPW_1 = 1,
	DPW_2 = 2,
	DPW_3 = 3,
	DPW_4 = 4,
	MAX_ORDER = DPW_4,
} DPWORDER;

typedef enum {
	TYPE_SAW,
	TYPE_SQU,
	TYPE_TRI,
} WAVETYPE;

template <typename T>
struct DPW {
	T pitch, phase;
	T gain = 1.0;
	unsigned int dpwOrder = 1;
	WAVETYPE waveType;
	T diffB[MAX_ORDER];
	unsigned int dbw = 0; // diffB write index
	int init;

	DPW() {
		waveType = TYPE_SAW;
		memset(diffB, 0, sizeof(diffB));
		tri_old = 0.0;
		paramsCompute();
		init = dpwOrder;
	}

	unsigned int onDPWOrderChange(unsigned int newdpw) {
		if (newdpw > MAX_ORDER)
			newdpw = MAX_ORDER;

		dpwOrder = newdpw;
		memset(diffB, 0, sizeof(diffB));
		tri_old = 0.0;
		paramsCompute();
		init = dpwOrder;
		return newdpw;
	}

	//----------	Differentiate ord-1 times 
	T dpwDiff(int ord) {
		ord = clamp(ord, 0, MAX_ORDER);

		T tmpA[dpwOrder];
		memset(tmpA, 0, sizeof(tmpA));
		int dbr = (dbw - 1) % ord;

		for (int i = 0; i < ord; i++) {
			tmpA[i] = diffB[dbr--];
			if (dbr < 0) dbr = ord - 1;
		}

		while(ord) {
			for (int i = 0; i < ord-1; i++) {
				tmpA[i] = gain * ( tmpA[i] - tmpA[i+1] );
			}
			ord--;
		}
		return tmpA[0];
	}

	
	//----------	Compute the polynomial (until fourth) and call the differentiator
	T process() {
		// TODO: Implement proper band-limited square (PolyBLEP or MinBLEP)

		// next step of the trivial waveform, advance phase
		T triv = trivialStep(waveType);
		phase += pitch * APP->engine->getSampleTime();
		if (phase >= 1.0) phase -= 1.0;

		T sqr = triv * triv;
		T poly;

		switch (dpwOrder) {
		case DPW_1:
		default:
			return triv;
		case DPW_2:
			poly = sqr;
			break;
		case DPW_3:
			poly = sqr * triv - triv;
			return poly;
			break;
		case DPW_4:
			poly = sqr * sqr - 2.0 * sqr;
			break;
		}

		diffB[dbw++] = poly;
		if (dbw >= dpwOrder) dbw = 0;
		if (init) {
			init--;
			return poly;
		}
		return dpwDiff(dpwOrder);
	}

	
	//----------	Generate the trivial waveform
	T trivialStep(int type) {
		switch(type) {
		case TYPE_SAW:	// Sawtooth wave: range -1..1
			return 2 * phase - 1;
		case TYPE_TRI:	// Triangle wave: range -1..1
			return (phase < 0.5) ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
		case TYPE_SQU: {	// Square wave as numerical derivative of triangle wave
			// Square as numerical derivative of triangle wave
			// The triangle has slope ±4, so we need to normalize the derivative
			T tri = (phase < 0.5) ? (4.0 * phase - 1.0) : (3.0 - 4.0 * phase);
			T dt = pitch * APP->engine->getSampleTime();
			T sqr = (dt > 0.0) ? (tri - tri_old) / dt * 0.25 : 0.0;  // Normalize by 1/4
			tri_old = tri;
			return sqr;
		}
		default:
			return 0;
		}
	}

	
	//Diff gain compute
	void paramsCompute() { // Call when sample rate or dpwOrder change

		if (dpwOrder > 1)
			gain = std::pow(1.f / factorial(dpwOrder) * std::pow(M_PI / (2.f*sin(M_PI*pitch * APP->engine->getSampleTime())),
					dpwOrder-1.f), 1.0 / (dpwOrder-1.f));
		else
			gain=1.0;
	}

	void setPitch(T newPitch) {	// pitch in Hz
		if (pitch != newPitch) {
			pitch = newPitch;
			paramsCompute();
		}
	}

private:
	T tri_old = 0.0; // Per calcolare la derivata numerica della onda triangolare -> square
};
