#include "ABC.hpp"
#include "dsp/digital.hpp","Osc_MySinth.hpp"

#define TRIG_TIME 1e-3f
#define Fs 44100 //Hz -> Freq di campionamento

struct MySinth : Module {
	enum ParamIds {
		PITCH, //potenziometro di pitch
        OSC_WAVE, //selettore forma d'onda(CKSS): 0 = saw, 1 = square
		NUM_PARAMS,
	};
	enum InputIds {
        VOCT,   //V/Oct input
		NUM_INPUTS,
	};
	enum OutputIds {
		OUT,
		NUM_OUTPUTS,
	};
	enum LightsIds { //luce estetica
		NUM_LIGHTS,
	};

	MySinth() { //---------------- Finire a configurare i parametri (potenziometri...)
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS,NUM_LIGHTS);
		configParam(PITCH, -12.0f, 12.0f, 0.0f, "PITCH"); //Da -12 a 12 semitoni
		configParam(F0, 100.0f, 15000.0f, 100.0f,"F0");   // 100Hz <= F0 <= 20kHz
		configParam(OSC_WAVE, 0.0f, 1.0f, 0.0f, "Waveform"); //Selettore forma d'onda (0=saw, 1=square).

		phase = 0.0f;
		sampleRate = 44100.0f;
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
	}
	
	float Ts= 1.f/Fs; //Periodo campionamento

	// internal oscillator state
	float phase;
	float sampleRate;

	void process(const ProcessArgs &args) override;
};

void MySinth::process(const ProcessArgs &args) {
		float f0 = params[F0].getValue();
		float pitch = params[PITCH].getValue();
		float freq = f0 * std::pow(2.0f, pitch / 12.0f);

		float sr = args.sampleRate;
		if (sr <= 0.0f) sr = sampleRate;
		float phaseInc = freq / sr;

		phase += phaseInc;
		if (phase >= 1.0f) phase -= std::floor(phase);

		float saw = 2.0f * phase - 1.0f;
		float sq = (phase < 0.5f) ? 1.0f : -1.0f;

		float waveSel = params[OSC_WAVE].getValue();
		float outSample = (waveSel < 0.5f) ? saw : sq;

		outputs[OUT].setVoltage(5.0f * outSample);
	}


////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------- INTERFACCIA GRAFICA --------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////

	struct MySinthWidget : ModuleWidget {
		MySinthWidget(MySinth * module);
	};

	MySinthWidget::MySinthWidget(MySinth * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("MySinth");
			addChild(title);
		}

		{   // F0 label
			ATextLabel * title = new ATextLabel(Vec(30, 40));
			title->setText("F0");
			addChild(title);
		}

		{ // PITCH label
			ATextLabel * title = new ATextLabel(Vec(30, 110));
			title->setText("PITCH");
			addChild(title);
		}

		{
			ATextLabel * title = new ATextLabel(Vec(30, 180));
			title->setText("OUT");
			addChild(title);
		}

		addParam(createParam<RoundBlackKnob>(Vec(30, 70), module, MySinth::F0));
		addParam(createParam<RoundBlackKnob>(Vec(30, 140), module, MySinth::PITCH));
		addParam(createParam<CKSS>(Vec(38, 160), module, MySinth::OSC_WAVE));
		addOutput(createOutput<PJ3410Port>(Vec(30, 210), module, MySinth::OUT));
	}

	Model *modelMySinth = createModel<MySinth, MySinthWidget>("MySinth");
