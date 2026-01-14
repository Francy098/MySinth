#include "ABC.hpp"
#include "dsp/digital.hpp","Osc_MySinth.hpp"

#define TRIG_TIME 1e-3f
#define Fs 44100 //Hz -> Freq di campionamento

struct MySinth : Module {
	enum ParamIds {
			PITCH, // potenziometro di pitch (FREQUENZA)
            DETUNE, // potenziometro di detune in semitoni
            LEVEL1, // level controllo per osc1 (0..1)
            LEVEL2, // level controllo per osc2 (0..1)
			OSC_WAVE1, // selettore forma d'onda (CKSS): 0 = saw, 1 = square
			OSC_WAVE2,  // selettore forma d'onda (CKSS): 0 = saw, 1 = square
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
		configParam(DETUNE, -12.0f, 12.0f, 0.0f, "DETUNE"); // Da -12 a 12 semitoni
		configParam(PITCH, 20.0f, 20000.0f, 440.0f, "PITCH"); // base frequency
		configParam(LEVEL1, 0.0f, 1.0f, 1.0f, "LEVEL1");    //Livello di uscita osc1
		configParam(LEVEL2, 0.0f, 1.0f, 1.0f, "LEVEL2");    //Livello di uscita osc2
		configParam(OSC_WAVE1, 0.0f, 1.0f, 0.0f, "Waveform"); // selettore forma d'onda (0=saw, 1=square)
		configParam(OSC_WAVE2, 0.0f, 1.0f, 0.0f, "Waveform"); // selettore forma d'onda (0=saw, 1=square)

		phase1 = 0.0f;
		phase2 = 0.0f;
		sampleRate = 44100.0f;
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
	}
	
	float Ts= 1.f/Fs; //Periodo campionamento

	// internal oscillator state
	float phase1;
	float phase2;
	float sampleRate;

	void process(const ProcessArgs &args) override;
};

void MySinth::process(const ProcessArgs &args) {
		float pitch = params[PITCH].getValue();
        float detune = params[DETUNE].getValue();
        float Voct_input = inputs[VOCT].getVoltage();
        float level1 = params[LEVEL1].getValue();
        float level2 = params[LEVEL2].getValue();

		// Compute frequencies
		// Voct input is volts per octave: 1V -> octave -> freq multiplier = 2^(Voct)
		float freq1 = pitch * std::pow(2.0f, Voct_input); 
		float freq2 = freq1 * std::pow(2.0f, detune / 12.0f);

		// Phase increments
		float sr = args.sampleRate; 
		if (sr <= 0.0f) sr = sampleRate;    
		float phaseInc1 = freq1 / sr; 
		float phaseInc2 = freq2 / sr; 

		// Update phases independently
		phase1 += phaseInc1;
		if (phase1 >= 1.0f) phase1 -= std::floor(phase1);

		phase2 += phaseInc2;
		if (phase2 >= 1.0f) phase2 -= std::floor(phase2);

		// Wave generation per oscillator
		float saw1 = 2.0f * phase1 - 1.0f;
		float sq1 = (phase1 < 0.5f) ? 1.0f : -1.0f;

		float saw2 = 2.0f * phase2 - 1.0f;
		float sq2 = (phase2 < 0.5f) ? 1.0f : -1.0f;

		float waveSel1 = params[OSC_WAVE1].getValue();
		float waveSel2 = params[OSC_WAVE2].getValue();
		float out_osc1 = (waveSel1 < 0.5f) ? saw1 : sq1;
		float out_osc2 = (waveSel2 < 0.5f) ? saw2 : sq2;

		outputs[OUT1].setVoltage(5.0f * out_osc1 * level1);
		outputs[OUT2].setVoltage(5.0f * out_osc2 * level2);
	}


////////////////////////////////////////////////////////////////////////////////////////////
//------------------------------- INTERFACCIA GRAFICA --------------------------------------
////////////////////////////////////////////////////////////////////////////////////////////

	struct MySinthWidget : ModuleWidget {
		MySinthWidget(MySinth * module);
	};
    //colonne - righe |   colonne x 380 righe  
	MySinthWidget::MySinthWidget(MySinth * module) {

		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
		box.size = Vec(30*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("MySinth");
			addChild(title);
		}

		// Top row: OSC1 and OSC2 with waveform switch + level knobs, Pitch to the right
		{
			ATextLabel * lbl1 = new ATextLabel(Vec(20, 10));
			lbl1->setText("OSC 1");
			addChild(lbl1);
		}
		addParam(createParam<CKSS>(Vec(20, 50), module, MySinth::OSC_WAVE1));
		addParam(createParam<RoundBlackKnob>(Vec(50, 45), module, MySinth::LEVEL1));

		{
			ATextLabel * lbl2 = new ATextLabel(Vec(90, 10));
			lbl2->setText("OSC 2");
			addChild(lbl2);
		}
		addParam(createParam<CKSS>(Vec(90, 50), module, MySinth::OSC_WAVE2));
		addParam(createParam<RoundBlackKnob>(Vec(120, 45), module, MySinth::LEVEL2));

		// Pitch to the right of Level1
		{
			ATextLabel * lblPitch = new ATextLabel(Vec(160, 10));
			lblPitch->setText("Pitch");
			addChild(lblPitch);
		}
		addParam(createParam<RoundBlackKnob>(Vec(160, 30), module, MySinth::PITCH));

		// Detune under Level2
		{
			ATextLabel * lblDet = new ATextLabel(Vec(120, 70));
			lblDet->setText("Detune");
			addChild(lblDet);
		}
		addParam(createParam<RoundBlackKnob>(Vec(120, 90), module, MySinth::DETUNE));

		// V/Oct input near pitch
		{
			ATextLabel * lblV = new ATextLabel(Vec(160, 70));
			lblV->setText("V/Oct");
			addChild(lblV);
		}
		addInput(createInput<PJ3410Port>(Vec(160, 90), module, MySinth::VOCT));

		// Outputs last at bottom
		{
			ATextLabel * lblOut1 = new ATextLabel(Vec(40, 260));
			lblOut1->setText("OUT1");
			addChild(lblOut1);
		}
		addOutput(createOutput<PJ3410Port>(Vec(40, 280), module, MySinth::OUT1));

		{
			ATextLabel * lblOut2 = new ATextLabel(Vec(120, 260));
			lblOut2->setText("OUT2");
			addChild(lblOut2);
		}
		addOutput(createOutput<PJ3410Port>(Vec(120, 280), module, MySinth::OUT2));
	}

	Model *modelMySinth = createModel<MySinth, MySinthWidget>("MySinth");
