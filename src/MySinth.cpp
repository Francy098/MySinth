#include "ABC.hpp"
#include "dsp/digital.hpp","Osc_MySinth.hpp"

#define TRIG_TIME 1e-3f
#define Fs 44100 //Hz -> Freq di campionamento

struct MySinth : Module {
	enum ParamIds {
		PITCH, //potenziometro di pitch (FREQUENZA)
        DETUNE, //potenziometro di detune in semitoni
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
		configParam(DETUNE, -12.0f, 12.0f, 0.0f, "DETUNE"); //Da -12 a 12 semitoni
		configParam(PITCH, 100.0f, 15000.0f, 100.0f,"PITCH"); // 100Hz <= PITCH <= 20kHz
		configParam(OSC_WAVE1, 0.0f, 1.0f, 0.0f, "Waveform"); //Selettore forma d'onda (0=saw, 1=square).
		configParam(OSC_WAVE2, 0.0f, 1.0f, 0.0f, "Waveform"); //Selettore forma d'onda (0=saw, 1=square).

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
			ATextLabel * lblF0 = new ATextLabel(Vec(27, 40));
			lblF0->setText("Pitch");
			addChild(lblF0);
		}
		addParam(createParam<RoundBlackKnob>(Vec(30, 70), module, MySinth::PITCH));

		{
			ATextLabel * lblPitch = new ATextLabel(Vec(27, 110));
			lblPitch->setText("Detune");
			addChild(lblPitch);
		}
		addParam(createParam<RoundBlackKnob>(Vec(30, 140), module, MySinth::DETUNE));

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
	    {        
			ATextLabel * lblOut2 = new ATextLabel(Vec(30, 260));    // V/Oct input
			lblOut2->setText("V/Oct");
			addChild(lblOut2);
		}
        addInput(createInput<PJ3410Port>(Vec(30, 280), module, MySinth::VOCT));
	}

	Model *modelMySinth = createModel<MySinth, MySinthWidget>("MySinth");
