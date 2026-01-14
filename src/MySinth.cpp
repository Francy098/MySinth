#include "ABC.hpp"
#include "dsp/digital.hpp"

#define TRIG_TIME 1e-3f
#define Fs 44100 //Hz -> Freq di campionamento

struct MySinth : Module {
	enum ParamIds {
		SPREAD,
        F0,
		NUM_PARAMS,
	};
	enum InputIds {
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
		configParam(SPREAD, 0.0f, 2000.0f, 1000.0f, "SPREAD"); //Da 0Hz a 2kHz
   		configParam(F0, 100.0f, 15000.0f, 100.0f,"F0");   // 100Hz <= F0 <= 20kHz

        }
 
	float Ts= 1.f/Fs; //Periodo campionamento
	
	void process(const ProcessArgs &args) override; //richiamiamo il processo
    
};


void MySinth::process(const ProcessArgs &args) {

    //----------- Dati in input: cosa il sintetizzatore "legge" per rializzare il suono 
    float f0 = params[F0].getValue();
    float spread = params[SPREAD].getValue();
    
    //----------- Elaborazioni interne: calcoli vari per produrre il suono

    //----------- OUTPUT: cosa il sintetizzatore "scrive" per produrre il suono
    outputs[OUT].setVoltage(___);

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

	{   //Centro è a 30
		ATextLabel * title = new ATextLabel(Vec(30, 40)); //1 colonne e 2 riche
		title->setText("F0");
		addChild(title);
	}

	{
		ATextLabel * title = new ATextLabel(Vec(30, 110));
		title->setText("SPREAD");
		addChild(title);
	}

    {
	    ATextLabel * title = new ATextLabel(Vec(30, 180));
        title->setText("OUT");
	    addChild(title);
	}
	addParam(createParam<RoundBlackKnob>(Vec(30, 70), module, MySinth::F0));  //RoundBlakMob è un tipo di manopola che si vuole graficare
	addParam(createParam<RoundBlackKnob>(Vec(30, 140), module, MySinth::SPREAD)); 
	addOutput(createOutput<PJ3410Port>(Vec(30, 210), module, MySinth::OUT));
}

Model *modelMySinth = createModel<MySinth, MySinthWidget>("MySinth");


//===================== LINEE GUIDA GRAFICA =====================
//1) fra Scritta e connettore -> 30 pixel, così sta proprio sopra
//2) Separazione fra i vari connettori: - fra connettore e scritta lasciare 40 pixel