#include "ABC.hpp"
#include "dsp/digital.hpp"

#define TRIG_TIME 1e-3f
#define Fs 44100 //Hz -> Freq di campionamento


struct Integrator1 : Module {
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

	Integrator1() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        }
    
    float xn1= 0.0;
    float yn= 0.0;
	void process(const ProcessArgs &args) override; //richiamiamo il processo
    
};

void Integrator1::process(const ProcessArgs &args) {

//Leggiamo il valore in input
float xn=inputs[IN].getVoltage();

//Integratore 
yn = yn + xn;//*Fs;


outputs[OUT].setVoltage(yn);

}

struct Integrator1Widget : ModuleWidget {
	Integrator1Widget(Integrator1 * module);
};

Integrator1Widget::Integrator1Widget(Integrator1 * module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
	box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		ATitle * title = new ATitle(box.size.x);
		title->setText("Integrator1");
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
	addInput(createInput<PJ3410Port>(Vec(30, 70), module, Integrator1::IN));
	addOutput(createOutput<PJ3410Port>(Vec(30, 210), module, Integrator1::OUT));
}

Model *modelIntegrator1 = createModel<Integrator1, Integrator1Widget>("Integrator1");


//===================== LINEE GUIDA GRAFICA =====================
//1) fra Scritta e connettore -> 30 pixel, così sta proprio sopra
//2) Separazione fra i vari connettori: - fra connettore e scritta lasciare 40 pixel