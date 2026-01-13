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

#include "ABC.hpp"
#include "dsp/digital.hpp"

#define TRIG_TIME 1e-3f

struct AClock : Module {
	enum ParamIds {
		BPM_KNOB,
		NUM_PARAMS,
	};
	enum InputIds {
		NUM_INPUTS,
	};
	enum OutputIds {
		PULSE_OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds {
		PULSE_LIGHT,
		NUM_LIGHTS,
	};

	dsp::PulseGenerator pgen;
	float counter, period;

	AClock() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BPM_KNOB, 30.0, 360.0, 120.0, "Tempo", "BPM");
		counter = period = 0.f;
	}

	void process(const ProcessArgs &args) override;
};

void AClock::process(const ProcessArgs &args) {

	float BPM = params[BPM_KNOB].getValue();
	period = 60.f * args.sampleRate / BPM; // samples

	//-----------------------------------------------
	//	+	+	+	+	+	+	+	+	+	+	+	+      
	//------------------- + -------------------------	sampling
	//Se c'è un sampling in questo modo cade in mezzo a due impulsi, a questo punto quale prendiamo? PROBLEMA!!!
	//Se prendiamo quello dopo approx per eccesso e prima per eccesso ed è comunque sbagliato...
	//Allora li facciamo SOTTRARRE A VICENDA, in questo modo avremo 6 -5,5 = 0,5 e rimarrà all'interno della variabile così
	//non perdiamo il conto

	if (counter > period) {
		pgen.trigger(TRIG_TIME);
		counter -= period; // keep the fractional part
	}

	counter++;
	float out = pgen.process( args.sampleTime ); //Ad ogni campione. Lo metto dentro flat anche se so che p bool -> conversione automatica
	outputs[PULSE_OUT].setVoltage(10.f * out); //Set tensione
	lights[PULSE_LIGHT].setSmoothBrightness(out, 5e-6f); //Set lampeggio. Per essere sicuri di prenderla, rendiamo la luce smooth,
	// cioè con una coda che ci garantisca che prendiamo (altrimenti sarebbe un impulso breve che non riusciremmo mai ad acchiappare con il sampling)

}

struct AClockWidget : ModuleWidget {
	AClockWidget(AClock * module);
};

AClockWidget::AClockWidget(AClock * module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
	box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		ATitle * title = new ATitle(box.size.x);
		title->setText("AClock");
		addChild(title);
	}

	{
		ATextLabel * title = new ATextLabel(Vec(23, 25));
		title->setText("TEMPO");
		addChild(title);
	}

	{
		ATextLabel * title = new ATextLabel(Vec(17, 140));
		title->setText("CLK OUT");
		addChild(title);
	}

	addParam(createParam<RoundBlackKnob>(Vec(30, 70), module, AClock::BPM_KNOB));

	addOutput(createOutput<PJ3410Port>(Vec(30, 180), module, AClock::PULSE_OUT));

	addChild(createLight<MediumLight<GreenLight>>(Vec(66, 190), module, AClock::PULSE_LIGHT));

}

Model *modelAClock = createModel<AClock, AClockWidget>("AClock");