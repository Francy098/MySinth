#include "ABC.hpp"
#include "Noise_MySinth.hpp"
#include "dsp/digital.hpp"
#include "Osc_MySinth.hpp" //trivial oscillators with Parker polyBLEP anti-aliasing filter on transitions
#include "LFO_MySinth.hpp"
#include "LPF_MySinth.hpp"
#include "ADSR_MySinth.hpp"
#include "VCA_MySinth.hpp"


#define TRIG_TIME 1e-3f
#define Fs 44100 //Hz -> Freq di campionamento


struct MySinth : Module {
	enum ParamIds {
        //------------------------ OSCILLATORI ---------------------------
		PITCH, // potenziometro di pitch (FREQUENZA)
        DETUNE, // potenziometro di detune in semitoni
        LEVEL1, // level controllo per osc1 (0..1)
        LEVEL2, // level controllo per osc2 (0..1)
		OSC_WAVE1, // selettore forma d'onda (CKSS): 0 = saw, 1 = square
		OSC_WAVE2,  // selettore forma d'onda (CKSS): 0 = saw, 1 = square
        LFO_AMOUNT, // profondità di modulazione
        //------------------------- LFO ---------------------------------
        RATE,  // velocità di LFO
		//------------------------- NOISE -----------------------------
		NOISE_LEVEL, // livello del rumore
		LFO_AMOUNT_NOISE, // profondità di modulazione del rumore
		//------------------------ LPF ------------------------------
		CUT_OFF, // Cutoff LPF
		RESONANCE,    // Risonanza LPF
		//------------------------ Envelope Generator - VCA -----------------------------
		ATTACK_PARAM,
		DECAY_PARAM,
		SUSTAIN_PARAM,
		RELEASE_PARAM,
       	NUM_PARAMS,
	};
	enum InputIds {
        VOCT,   //V/Oct input
		CUT_OFF_IN, // Cutoff CV input
		GATE_IN, // Gate input for Envelope Generator
		NUM_INPUTS,
	};
	enum OutputIds { 
        OUT1,   //Output Osc 1
        OUT2,   //Output Osc 2 
        LFO_OUT, //Output LFO
		OUT_MIDDLE, //Output somma oscillatori + rumore
		OUT_LPF, //Output LPF
		OUTPUT_FINAL, // Final output after envelope e VCA
		NUM_OUTPUTS,
	};
	enum LightsIds { //luce estetica
		NUM_LIGHTS,
	};

	MySinth() { //---------------- Finire a configurare i parametri (potenziometri...)
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS,NUM_LIGHTS);
		configParam(DETUNE, -12.0f, 12.0f, 0.0f, "DETUNE","semitones"); // Da -12 a 12 semitoni
		configParam(PITCH, 20.0f, 20000.0f, 440.0f, "PITCH"," Hz"); // base frequency
		configParam(LEVEL1, 0.0f, 1.0f, 1.0f, "LEVEL1");    //Livello di uscita osc1
		configParam(LEVEL2, 0.0f, 1.0f, 1.0f, "LEVEL2");    //Livello di uscita osc2
		configParam(OSC_WAVE1, 0.0f, 1.0f, 0.0f, "Waveform"); // selettore forma d'onda (0=saw, 1=square)
		configParam(OSC_WAVE2, 0.0f, 1.0f, 0.0f, "Waveform"); // selettore forma d'onda (0=saw, 1=square)
		configParam(RATE, 0.0f, 10.0f, 1.0f, "LFO RATE"," Hz");
        configParam(LFO_AMOUNT, 0.0f, 1.0f, 1.0f, "LFO AMOUNT"); // profondità di modulazione
		configParam(NOISE_LEVEL, 0.0f, 1.0f, 1.0f, "NOISE LEVEL");
		configParam(LFO_AMOUNT_NOISE, 0.0f, 1.0f, 1.0f, "LFO AMOUNT NOISE");
		configParam(CUT_OFF, 20.0f, 20000.0f, 1000.0f, "LPF CUTOFF"," Hz");	// Cutoff LPF
		configParam(RESONANCE, 0.0f, 0.99f,	0.5f, "LPF RESONANCE"); // Evitare 1.0f per stabilità
		configParam(ATTACK_PARAM, 0.f, 1.f, 0.5f, "ATTACK");
		configParam(DECAY_PARAM, 0.f, 1.f, 0.5f, "DECAY");
		configParam(SUSTAIN_PARAM, 0.f, 1.f, 0.5f, "SUSTAIN");
		configParam(RELEASE_PARAM, 0.f, 1.f, 0.5f, "RELEASE");

		sampleRate = 44100.0f;
		lfo.reset(0.0); // reset phase to 0
		envelopeGen.reset();
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
		Ts = 1.f / sampleRate;
	}
	
	float Ts= 1.f/Fs; //Periodo campionamento

	// internal oscillator state
	float sampleRate;

	MySinthOsc::SawOsc sawOsc1, sawOsc2;	// Sawtooth Oscillator instances (trivial)
	MySinthOsc::SquareOsc sqOsc1, sqOsc2;	// Square Oscillator instances (trivial)
	LowFrequencyOscillator::MySinthLFO lfo; // LFO instance (value)
	NoiseGenerator::WhiteNoise WhiteNoise;	 // White Noise instance
	StateVariableFilter::StateVarFil SVFilter1, SVFilter2; // LPF instance (2 in series)
	EnvelopeGenerator::ADSR envelopeGen; // Envelope Generator instance
	VCAGenerator::VCA vca; // VCA instance

	void process(const ProcessArgs &args) override;
};

void MySinth::process(const ProcessArgs &args) {
		float pitch = params[PITCH].getValue();
        float detune = params[DETUNE].getValue();
        float Voct_input = inputs[VOCT].getVoltage();
        float level1 = params[LEVEL1].getValue();
        float level2 = params[LEVEL2].getValue();
        float lfo_amount = params[LFO_AMOUNT].getValue();
        float lfo_rate = params[RATE].getValue();
		float noise_level = params[NOISE_LEVEL].getValue();
        float lfo_amount_noise = params[LFO_AMOUNT_NOISE].getValue();
		float cutoff = params[CUT_OFF].getValue();
		float resonance = params[RESONANCE].getValue();
		float att = params[ATTACK_PARAM].getValue();
		float dec = params[DECAY_PARAM].getValue();
		float sus = params[SUSTAIN_PARAM].getValue();
		float rel = params[RELEASE_PARAM].getValue();


		//------------ LFO processing (use member lfo)------------
		lfo.setSampleRate(args.sampleRate);
		lfo.setRate(lfo_rate);
		float lfo_out = lfo.process(); // lfo_out range [-1,1]
		// If LFO rate is very low, don't modulate
		if (lfo_rate < 0.01f) lfo_out = 1.0f;	// così 
		outputs[LFO_OUT].setVoltage(5.0f * lfo_out); // LFO output scaled to +/-5V

		//------------ Trivial Oscillators processing ----------------
        // Compute frequencies with LFO modulation
		// Voct input is volts per octave: 1V -> octave -> freq multiplier = 2^(Voct)
		float freq1_base = pitch * std::pow(2.0f, Voct_input); // base frequency osc1
		float freq2_base = freq1_base * std::pow(2.0f, detune / 12.0f); // detune in semitones
		
		// Apply LFO modulation to frequencies
		float freq1 = freq1_base + lfo_out * lfo_amount;
		float freq2 = freq2_base + lfo_out * lfo_amount;

		// Waveform selection: 0 = saw, 1 = square
		float waveSel1 = params[OSC_WAVE1].getValue();
		float waveSel2 = params[OSC_WAVE2].getValue();

		// Set sample rate and frequency for oscillators
		sawOsc1.setSampleRate(args.sampleRate);
		sawOsc1.setFrequency(freq1);
		sqOsc1.setSampleRate(args.sampleRate);
		sqOsc1.setFrequency(freq1);
		sawOsc2.setSampleRate(args.sampleRate);
		sawOsc2.setFrequency(freq2);
		sqOsc2.setSampleRate(args.sampleRate);
		sqOsc2.setFrequency(freq2);

		// Generate waveforms: 0 = saw, 1 = square, scaled to ±5V with levels
		float out_osc1 = 5.0f * ((waveSel1 < 0.5f) ? sawOsc1.process() : sqOsc1.process()) * level1;
		float out_osc2 = 5.0f * ((waveSel2 < 0.5f) ? sawOsc2.process() : sqOsc2.process()) * level2;

		//-------------- White Noise output----------------
		float out_noise = WhiteNoise.process(); //rumore bianco
		out_noise += out_noise * lfo_out * lfo_amount_noise;	// Modulation of noise level with LFO
		//Output in the middle (somma di oscillatori e rumore)
		float Y_in_the_middle = (out_osc1 + out_osc2) * 0.5f + out_noise*noise_level; // somma dei due oscillatori e del rumore per il livello rumore

		//--------------- LPF processing ----------------
		// Modulation of cutoff with input CV
		if (inputs[CUT_OFF_IN].isConnected()) cutoff += rescale(inputs[CUT_OFF_IN].getVoltage(), -10.0f, 10.0f, 20.0f, 20000.0f);
		// Aggiorna i parametri del filtro, solo se sono cambiati i parametri cutoff o resonance
		if (cutoff != SVFilter1.cutoff_old || resonance != SVFilter1.resonance_old) 
			SVFilter1.updateParameters(cutoff, resonance, sampleRate);
		//if (cutoff != SVFilter2.cutoff_old || resonance != SVFilter2.resonance_old) SVFilter2.updateParameters(cutoff, resonance, sampleRate);
		//Salviamo i vecchi valori per il prossimo ciclo
		SVFilter1.cutoff_old = cutoff;
		//SVFilter2.cutoff_old = cutoff;
		SVFilter1.resonance_old = resonance;
		//SVFilter2.resonance_old = resonance;
		// Processa il segnale attraverso due LPF in serie
		float lpf_out1 = SVFilter1.process(Y_in_the_middle);
		//float lpf_out2 = SVFilter2.process(lpf_out1);

		//--------------- Envelope Generator processing ----------------
		envelopeGen.setAttack(att);
		envelopeGen.setDecay(dec);
		envelopeGen.setSustain(sus);
		envelopeGen.setRelease(rel);
		// Gate semplice: VOCT > 1V
		bool gate = inputs[GATE_IN].getVoltage() > 1.0f;
		float env_out = envelopeGen.process(gate, args.sampleTime);
		
		//--------------- VCA processing ----------------
		vca.setLevel(1.0f); // level knob at max (perché non c'è nella definizione del modulo)
		vca.setCv(env_out / 1.2f); // Set CV-Control Voltage normalize envelope to [0,1]
		float final_output = vca.process(lpf_out1);	// Apply envelope to final output using VCA


		// Set outputs
		outputs[OUT1].setVoltage(out_osc1);	
		outputs[OUT2].setVoltage(out_osc2);
		outputs[OUT_MIDDLE].setVoltage(Y_in_the_middle);
		outputs[OUT_LPF].setVoltage(lpf_out1);	// Final LPF output
		outputs[OUTPUT_FINAL].setVoltage(final_output); // Final output after VCA with envelope
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
		box.size = Vec(35*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

		{
			ATitle * title = new ATitle(box.size.x);
			title->setText("MySinth");
			addChild(title);
		}

		//-------------------------- LFO --------------------------------
        {
            ATextLabel * lblRate = new ATextLabel(Vec(20, 10));
            lblRate->setText("LFO");
            addChild(lblRate);
        }
        {
            ATextLabel * lblRate = new ATextLabel(Vec(20, 40));
            lblRate->setText("Rate");
            addChild(lblRate);
        }
        addParam(createParam<RoundBlackKnob>(Vec(20, 70), module, MySinth::RATE));
        {
            ATextLabel * lblLFO = new ATextLabel(Vec(10, 110));
            lblLFO->setText("LFO_OUT");
            addChild(lblLFO);
        }
        addOutput(createOutput<PJ3410Port>(Vec(20, 140), module, MySinth::LFO_OUT));


		//-------------------------- OSCILLATORS ------------------------
		{
			ATextLabel * lbl1 = new ATextLabel(Vec(90, 10));
			lbl1->setText("OSCILLATORS");
			addChild(lbl1);
		}
        {
			ATextLabel * lblPitch = new ATextLabel(Vec(85, 40));
			lblPitch->setText("Pitch");
			addChild(lblPitch);
		}
		addParam(createParam<RoundBlackKnob>(Vec(90, 70), module, MySinth::PITCH));
        {
			ATextLabel * lblPitch = new ATextLabel(Vec(125, 40));
			lblPitch->setText("LFO_Amount");
			addChild(lblPitch);
		}
		addParam(createParam<RoundBlackKnob>(Vec(140, 70), module, MySinth::LFO_AMOUNT));

        {//------------- OSC 1 ----------------
			ATextLabel * lbl1 = new ATextLabel(Vec(110, 35+70));
			lbl1->setText("OSC 1");
			addChild(lbl1);
		}
        {
			ATextLabel * lbl1 = new ATextLabel(Vec(70, 45+75+5));
			lbl1->setText("sqrt");
			addChild(lbl1);
		}
        {
			ATextLabel * lbl1 = new ATextLabel(Vec(75, 65+75+5));
			lbl1->setText("saw");
			addChild(lbl1);
		}
		addParam(createParam<CKSS>(Vec(100, 70+75+5), module, MySinth::OSC_WAVE1));
		{
			ATextLabel * lbl1 = new ATextLabel(Vec(160, 60+75));
			lbl1->setText("level");
			addChild(lbl1);
		}
        addParam(createParam<RoundBlackKnob>(Vec(130, 70+75), module, MySinth::LEVEL1));

        //------------- OSC 2 ---------------
        {
			ATextLabel * lbl2 = new ATextLabel(Vec(110, 35+70+70));
			lbl2->setText("OSC 2");
			addChild(lbl2);
		}
        {
			ATextLabel * lbl1 = new ATextLabel(Vec(70, 45+75+75));
			lbl1->setText("sqrt");
			addChild(lbl1);
		}
        {
			ATextLabel * lbl1 = new ATextLabel(Vec(75, 65+75+75));
			lbl1->setText("saw");
			addChild(lbl1);
		}
		addParam(createParam<CKSS>(Vec(100, 70+75+75), module, MySinth::OSC_WAVE2));
        {
			ATextLabel * lbl1 = new ATextLabel(Vec(160, 60+75+70));
			lbl1->setText("level");
			addChild(lbl1);
		}
		addParam(createParam<RoundBlackKnob>(Vec(130, 70+75+70), module, MySinth::LEVEL2));
		{
			ATextLabel * lblDet = new ATextLabel(Vec(160, 60+75+70+40));
			lblDet->setText("Detune");
			addChild(lblDet);
		}
		addParam(createParam<RoundBlackKnob>(Vec(130, 70+75+70+40), module, MySinth::DETUNE));

        // V/Oct input near pitch
        {
			ATextLabel * lblV = new ATextLabel(Vec(115, 70+75+70+40+30));
			lblV->setText("V/Oct");
			addChild(lblV);
		}
		addInput(createInput<PJ3410Port>(Vec(115, 70+75+70+40+60), module, MySinth::VOCT));
		
		//---------------- Outputs last at bottom
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
		//---------------- Output in the middle
		{
			ATextLabel * lblOutMiddle = new ATextLabel(Vec(200+30, 250));
			lblOutMiddle->setText("OUT_MID");
			addChild(lblOutMiddle);
		}
		addOutput(createOutput<PJ3410Port>(Vec(180+60, 280), module, MySinth::OUT_MIDDLE));
	
		//----------------- NOISE ----------------------
		{
			ATextLabel * lblNoise = new ATextLabel(Vec(160+75, 10));
			lblNoise->setText("NOISE");
			addChild(lblNoise);
		}
		{
			ATextLabel * lblNoiseLevel = new ATextLabel(Vec(175+60, 40));
			lblNoiseLevel->setText("Level");
			addChild(lblNoiseLevel);
		}
		addParam(createParam<RoundBlackKnob>(Vec(180+60, 70), module, MySinth::NOISE_LEVEL));
		{
			ATextLabel * lblLfoNoise = new ATextLabel(Vec(170+60, 110));
			lblLfoNoise->setText("LFO Amt");
			addChild(lblLfoNoise);
		}
		addParam(createParam<RoundBlackKnob>(Vec(180+60, 140), module, MySinth::LFO_AMOUNT_NOISE));
		
		//----------------- LPF ----------------------
		{
			ATextLabel * lblLPF = new ATextLabel(Vec(260+90, 10));
			lblLPF->setText("LPF");
			addChild(lblLPF);
		}
		{
			ATextLabel * lblCutoff = new ATextLabel(Vec(250+90, 40));
			lblCutoff->setText("Cutoff");
			addChild(lblCutoff);
		}
		addParam(createParam<RoundBlackKnob>(Vec(260+90, 70), module, MySinth::CUT_OFF));
		{
			ATextLabel * lblResonance = new ATextLabel(Vec(240+90, 110));
			lblResonance->setText("Resonance");
			addChild(lblResonance);
		}
		addParam(createParam<RoundBlackKnob>(Vec(260+90, 140), module, MySinth::RESONANCE));
		{
			ATextLabel * lblCutoffIn = new ATextLabel(Vec(240+90, 180));
			lblCutoffIn->setText("Cutoff CV");
			addChild(lblCutoffIn);
		}
		addInput(createInput<PJ3410Port>(Vec(260+90, 210), module, MySinth::CUT_OFF_IN));
		{
			ATextLabel * lblOutLPF = new ATextLabel(Vec(250+90, 250));
			lblOutLPF->setText("OUT_LPF");
			addChild(lblOutLPF);
		}
		addOutput(createOutput<PJ3410Port>(Vec(260+90, 280), module, MySinth::OUT_LPF));

		//----------------- Envelope Generator - VCA ----------------------
		{
			ATextLabel * lblEnv = new ATextLabel(Vec(340+90, 10));
			lblEnv->setText("EG - VCA");
			addChild(lblEnv);
		}
		{
			ATextLabel * lblAttack = new ATextLabel(Vec(340+100, 40));
			lblAttack->setText("Attack");
			addChild(lblAttack);
		}
		addParam(createParam<RoundBlackKnob>(Vec(350+100, 70), module, MySinth::ATTACK_PARAM));
		{
			ATextLabel * lblDecay = new ATextLabel(Vec(340+105, 110-10));
			lblDecay->setText("Decay");
			addChild(lblDecay);
		}	
		addParam(createParam<RoundBlackKnob>(Vec(350+100, 140-10), module, MySinth::DECAY_PARAM));
		{
			ATextLabel * lblSustain = new ATextLabel(Vec(330+105, 180-20));
			lblSustain->setText("Sustain");
			addChild(lblSustain);
		}
		addParam(createParam<RoundBlackKnob>(Vec(350+100, 210-20), module, MySinth::SUSTAIN_PARAM));
		{
			ATextLabel * lblRelease = new ATextLabel(Vec(340+95, 250-30));
			lblRelease->setText("Release");
			addChild(lblRelease);
		}
		addParam(createParam<RoundBlackKnob>(Vec(350+100, 280-30), module, MySinth::RELEASE_PARAM));
		{
			ATextLabel * lblGateIn = new ATextLabel(Vec(340+65, 320-40));
			lblGateIn->setText("GATE_IN");
			addChild(lblGateIn);
		}
		addInput(createInput<PJ3410Port>(Vec(350+70, 350-40), module, MySinth::GATE_IN));
		{
			ATextLabel * lblOutLPF = new ATextLabel(Vec(340+120, 320-40));
			lblOutLPF->setText("OUT_FINAL");
			addChild(lblOutLPF);
		}
		addOutput(createOutput<PJ3410Port>(Vec(350+120, 350-40), module, MySinth::OUTPUT_FINAL));
	}

	Model *modelMySinth = createModel<MySinth, MySinthWidget>("MySinth");


    //===================== LINEE GUIDA GRAFICA =====================
    // 30 righe fra l'etichetta e la manopola sotto (30 righe fra etichetta e presa jacks)
    // 40 righe fra manopola e l'etichetta sotto
    // fra etichette 30 righe 
