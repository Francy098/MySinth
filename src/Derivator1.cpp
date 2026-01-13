#include "ABC.hpp"
#include "dsp/digital.hpp"

#define TRIG_TIME 1e-3f
#define Fs 44100 //Hz -> Freq di campionamento


struct Derivator1 : Module {
	enum ParamIds {
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
		NUM_LIGHTS,
	};

	Derivator1() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS,NUM_LIGHTS);
        }
    float xn1= 0.0;
	void process(const ProcessArgs &args) override; //richiamiamo il processo
    
};

void Derivator1::process(const ProcessArgs &args) {

//Leggiamo il valore in input
float xn=inputs[IN].getVoltage();

//Derivatore -> RAPPORTO INCREMENTALE
float yn = (xn - xn1);//*Fs;
xn1=xn;
outputs[OUT].setVoltage(yn);

}

struct Derivator1Widget : ModuleWidget {
	Derivator1Widget(Derivator1 * module);
};

Derivator1Widget::Derivator1Widget(Derivator1 * module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
	box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		ATitle * title = new ATitle(box.size.x);
		title->setText("Derivator1");
		addChild(title);
	}

	{   //Centro è a 30
		ATextLabel * title = new ATextLabel(Vec(30, 40)); //1 colonne e 2 riche
		title->setText("IN");
		addChild(title);
	}

    {
	    ATextLabel * title = new ATextLabel(Vec(30, 180));
        title->setText("OUT");
	    addChild(title);
	}
	addInput(createInput<PJ3410Port>(Vec(30, 70), module, Derivator1::IN));
	addOutput(createOutput<PJ3410Port>(Vec(30, 210), module, Derivator1::OUT));
}

Model *modelDerivator1 = createModel<Derivator1, Derivator1Widget>("Derivator1");


//===================== LINEE GUIDA GRAFICA =====================
//1) fra Scritta e connettore -> 30 pixel, così sta proprio sopra
//2) Separazione fra i vari connettori: - fra connettore e scritta lasciare 40 pixel