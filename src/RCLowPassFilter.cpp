#include "ABC.hpp"
#include "dsp/digital.hpp"

#define TRIG_TIME 1e-3f
#define Fs 44100 //Hz -> Freq di campionamento


struct RCLowPassFilter : Module {
	enum ParamIds {
        A,
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

	RCLowPassFilter() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS,NUM_LIGHTS);
        configParam(A, 0.0f, 1.0f, 0.5, "a"); // a va da 0 ad 1, partendo da a=0.5
        }
    
    float yn1= 0.0;
	void process(const ProcessArgs &args) override; //richiamiamo il processo
    
};

void RCLowPassFilter::process(const ProcessArgs &args) {

//Leggiamo il valore in input e di a
float xn=inputs[IN].getVoltage();
float a=params[A].getValue();

//FILTRO RC LOW PASS 
float yn = a*yn1 + xn*(1-a);//*Fs;
yn1=yn;
outputs[OUT].setVoltage(yn);

}

struct RCLowPassFilterWidget : ModuleWidget {
	RCLowPassFilterWidget(RCLowPassFilter * module);
};

RCLowPassFilterWidget::RCLowPassFilterWidget(RCLowPassFilter * module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
	box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		ATitle * title = new ATitle(box.size.x);
		title->setText("RCLowPassFilter");
		addChild(title);
	}

	{   //Centro è a 30
		ATextLabel * title = new ATextLabel(Vec(30, 40)); //1 colonne e 2 riche
		title->setText("IN");
		addChild(title);
	}

	{
		ATextLabel * title = new ATextLabel(Vec(30, 110));
		title->setText("a");
		addChild(title);
	}

    {
	    ATextLabel * title = new ATextLabel(Vec(30, 180));
        title->setText("OUT");
	    addChild(title);
	}
	addInput(createInput<PJ3410Port>(Vec(30, 70), module, RCLowPassFilter::IN));
    addParam(createParam<RoundBlackKnob>(Vec(30, 140), module, RCLowPassFilter::A));
	addOutput(createOutput<PJ3410Port>(Vec(30, 210), module, RCLowPassFilter::OUT));
}

Model *modelRCLowPassFilter = createModel<RCLowPassFilter, RCLowPassFilterWidget>("RCLowPassFilter");


//===================== LINEE GUIDA GRAFICA =====================
//1) fra Scritta e connettore -> 30 pixel, così sta proprio sopra
//2) Separazione fra i vari connettori: - fra connettore e scritta lasciare 40 pixel