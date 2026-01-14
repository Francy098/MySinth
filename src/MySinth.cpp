#include "ABC.hpp"
#include "dsp/digital.hpp","Osc_MySinth.hpp"

#define TRIG_TIME 1e-3f
#define Fs 44100 //Hz -> Freq di campionamento

struct MySinth : Module {
	enum ParamIds {
        F0,    //potenziometro di frequenza
		PITCH, //potenziometro di pitch
        OSC_WAVE1, //selettore forma d'onda(CKSS): 0 = saw, 1 = square
		OSC_WAVE2,
        NUM_PARAMS,
	};
	enum InputIds {
        VOCT,   //V/Oct input
		NUM_INPUTS,
	};
	enum OutputIds { 
        OUT1,   //Output Osc 1
        OUT2,   //Output Osc 2 
		NUM_OUTPUTS,
	};
	enum LightsIds { //luce estetica
		NUM_LIGHTS,
	};

	MySinth() { //---------------- Finire a configurare i parametri (potenziometri...)
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS,NUM_LIGHTS);
		configParam(PITCH, -12.0f, 12.0f, 0.0f, "PITCH"); //Da -12 a 12 semitoni
		configParam(F0, 100.0f, 15000.0f, 100.0f,"F0");   // 100Hz <= F0 <= 20kHz
		configParam(OSC_WAVE1, 0.0f, 1.0f, 0.0f, "Waveform"); //Selettore forma d'onda (0=saw, 1=square).
		configParam(OSC_WAVE2, 0.0f, 1.0f, 0.0f, "Waveform"); //Selettore forma d'onda (0=saw, 1=square).

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

		float waveSel1 = params[OSC_WAVE1].getValue();
		float waveSel2 = params[OSC_WAVE2].getValue();
		float out_osc1 = (waveSel1 < 0.5f) ? saw : sq;
		float out_osc2 = (waveSel2 < 0.5f) ? saw : sq;

		outputs[OUT1].setVoltage(5.0f * out_osc1);
        outputs[OUT2].setVoltage(5.0f * out_osc2);  
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
		box.size = Vec(30*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("MySinth");
			addChild(title);
		}

		// Primary controls (shared F0 / PITCH)
		{
			ATextLabel * lblF0 = new ATextLabel(Vec(30, 40));
			lblF0->setText("F0");
			addChild(lblF0);
		}
		addParam(createParam<RoundBlackKnob>(Vec(30, 70), module, MySinth::F0));

		{
			ATextLabel * lblPitch = new ATextLabel(Vec(30, 110));
			lblPitch->setText("PITCH");
			addChild(lblPitch);
		}
		addParam(createParam<RoundBlackKnob>(Vec(30, 140), module, MySinth::PITCH));

		// Two waveform selectors in two columns
		{
			ATextLabel * lbl1 = new ATextLabel(Vec(18, 150));
			lbl1->setText("OSC 1");
			addChild(lbl1);
		}
		addParam(createParam<CKSS>(Vec(18, 170), module, MySinth::OSC_WAVE1));

		{
			ATextLabel * lbl2 = new ATextLabel(Vec(78, 150));
			lbl2->setText("OSC 2");
			addChild(lbl2);
		}
		addParam(createParam<CKSS>(Vec(78, 170), module, MySinth::OSC_WAVE2));

		// Outputs under each oscillator column
		{
			ATextLabel * lblOut1 = new ATextLabel(Vec(18, 205));
			lblOut1->setText("OUT1");
			addChild(lblOut1);
		}
		addOutput(createOutput<PJ3410Port>(Vec(18, 225), module, MySinth::OUT1));

		{
			ATextLabel * lblOut2 = new ATextLabel(Vec(78, 205));
			lblOut2->setText("OUT2");
			addChild(lblOut2);
		}
		addOutput(createOutput<PJ3410Port>(Vec(78, 225), module, MySinth::OUT2));
	}

	Model *modelMySinth = createModel<MySinth, MySinthWidget>("MySinth");
