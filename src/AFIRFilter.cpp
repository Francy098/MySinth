#include "ABC.hpp"
#include "dsp/digital.hpp"

#define TRIG_TIME 1e-3f

struct AFIRFilter : Module {
	enum ParamIds {
		
        FIR_PARAM1, //viene contanto come 0
        FIR_PARAM2, //come 1
		NUM_PARAMS,
	};
	enum InputIds {
	
        IN,
        NUM_INPUTS,
	};
	enum OutputIds {
	
        OUT,
		NUM_OUTPUTS,
	};

	enum LightsIds { //luce estetica
		PULSE_LIGHT,
		NUM_LIGHTS,
	};

	dsp::PulseGenerator pgen;
	float counter, period;

	AFIRFilter() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS); // settiamo le manopole
		configParam(FIR_PARAM1, -2.0f, 2.0f, 0, "B0"); //settiamo l'inizio ed il fine corsa delle manopole
		configParam(FIR_PARAM2, -2.0f, 2.0f, 0, "B1");
        counter = period = 0.f;
	}

    float x_1 = 0;

	void process(const ProcessArgs &args) override;
};

void AFIRFilter::process(const ProcessArgs &args) {

	float b0 = params[FIR_PARAM1].getValue(); //potenziometri o bottoni, restetuisce il valore
	float b1 = params[FIR_PARAM2].getValue(); //potenziometri o bottoni, restetuisce il valore
	
    float x = inputs[IN].getVoltage(); //si aspetta una tensione
	float y = x*b0 + x_1*b1; //x_1 mantiene in memoria il valore vecchio di x
    outputs[OUT].setVoltage(y); //

   x_1 = x; //memorizzo il precedente valore di x

}

struct AFIRFilterWidget : ModuleWidget {
	AFIRFilterWidget(AFIRFilter * module);
};

AFIRFilterWidget::AFIRFilterWidget(AFIRFilter * module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
	box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    //Parte dedicata alla posizione del testo
	{
		ATitle * title = new ATitle(box.size.x);
		title->setText("AFIRFilter");
		addChild(title);
	}

	{
		ATextLabel * title = new ATextLabel(Vec(15, 40)); //Tutto qyesto si moltiplica per 6 e si ottengono i "pixel"
		title->setText("B0");
		addChild(title);
	}
    //Il primo numero è la colonna ed il secondo sono le righe
	{
		ATextLabel * title = new ATextLabel(Vec(55, 40));
		title->setText("B1");
		addChild(title);
	}
	{
		ATextLabel * title = new ATextLabel(Vec(15, 150)); //Tutto qyesto si moltiplica per 6 e si ottengono i "pixel"
		title->setText("IN");
		addChild(title);
	}

	{
		ATextLabel * title = new ATextLabel(Vec(52, 150));
		title->setText("OUT");
		addChild(title);
	}
    //QUI SETTIAMO l'oggetto effettivo
	addParam(createParam<RoundBlackKnob>(Vec(15, 70), module, AFIRFilter::FIR_PARAM1)); //RoundBlakMob è un tipo di manopola che si vuole graficare

	addOutput(createOutput<PJ3410Port>(Vec(50, 70), module, AFIRFilter::FIR_PARAM2)); //Si sceglie il tipo di connettore da vedere, come PJ3410Port

	addInput(createInput<PJ3410Port>(Vec(10, 180), module, AFIRFilter::IN));

    addOutput(createOutput<PJ3410Port>(Vec(50, 180), module, AFIRFilter::OUT));

}

//Indica il titolo dello strumento
Model *modelAFIRFilter = createModel<AFIRFilter, AFIRFilterWidget>("AFIRFilter");

//In linea di massima fra due riche sotto e sopra è bene lasciare 30 pixel 