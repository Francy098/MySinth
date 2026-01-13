#include "ABC.hpp"
#include "dsp/digital.hpp"
#include "sin_lut.h"

#define TRIG_TIME 1e-3f
#define N 10 //Numero max di oscillatori
#define Fs 44100 //Hz -> Freq di campionamento

typedef struct{
	int low;
	int high;
} Index ;

struct LUTSinBank : Module {
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

	LUTSinBank() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS,NUM_LIGHTS);
		configParam(SPREAD, 0.0f, 1000.0f, 1000.0f, "SPREAD"); //Da 0Hz a 1kHz
   		configParam(F0, 100.0f, 10000.0f, 100.0f,"F0");   // 100Hz <= F0 <= 10kHz
        }
 
	float Ts= 1.f/Fs; //Periodo campionamento, incremento asse dei tempi per max risuluzione. 
    float teta=0; //Argomento di sin(arg)
	float t=0; //Rampa dei tempi [0-1]
	Index index; //Struct con index.low e index.high

	void process(const ProcessArgs &args) override; //richiamiamo il processo
    
};

Index find_index_for(float teta)
{
	int i=0;
	while(teta > sin_lut[i].angle_rad){i++;}

	if(teta == sin_lut[i].angle_rad){
		return {i,i};
	}else{
		return {i-1,i};
	}
}

void LUTSinBank::process(const ProcessArgs &args) {

float f0 = params[F0].getValue();
float spread = params[SPREAD].getValue();
float y=0; //Resettiamo l'OUT

//Provare a fare un check del arg prima di entrare col modulo o i=int(a) poi arg=a-i;

for(int i=0; i<N; i++)
{
	teta = 2*M_PI*(f0+spread*i)*t; //Ogni volta mi calcolo l'argomento giusto del sin da aggiungere alla sommatoria
	
	index = find_index_for(teta);

	y = y + (sin_lut[index.low].value+sin_lut[index.high].value)*0.5; //Faccio la media fra due valori della LUT
}

y =y*1.f/N;  //Per attenuare la ampiezza della somma totale delle armoniche

t=t+Ts; //Avanzamento tempo
if(t >= 1){t=t-1;}

outputs[OUT].setVoltage(y);

}

struct LUTSinBankWidget : ModuleWidget {
	LUTSinBankWidget(LUTSinBank * module);
};

LUTSinBankWidget::LUTSinBankWidget(LUTSinBank * module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
	box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		ATitle * title = new ATitle(box.size.x);
		title->setText("LUTSinBank");
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
	addParam(createParam<RoundBlackKnob>(Vec(30, 70), module, LUTSinBank::F0));  //RoundBlakMob è un tipo di manopola che si vuole graficare
	addParam(createParam<RoundBlackKnob>(Vec(30, 140), module, LUTSinBank::SPREAD)); 
	addOutput(createOutput<PJ3410Port>(Vec(30, 210), module, LUTSinBank::OUT));
}

Model *modelLUTSinBank = createModel<LUTSinBank, LUTSinBankWidget>("LUTSinBank");


//===================== LINEE GUIDA GRAFICA =====================
//1) fra Scritta e connettore -> 30 pixel, così sta proprio sopra
//2) Separazione fra i vari connettori: - fra connettore e scritta lasciare 40 pixel