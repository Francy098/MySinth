/*--------------------------- ABC ---------------------------------*
 *-----------------------------------------------------------------*/

#include "ABC.hpp"
#include "dsp/digital.hpp"

#define TRIG_TIME 1e-3f
#define N 1 //Numero max di oscillatori
#define Fs 44100 //Hz -> Freq di campionamento

struct Bench_osc : Module {
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

	Bench_osc() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS,NUM_LIGHTS);
		configParam(SPREAD, 0.0f, 2000.0f, 1000.0f, "SPREAD"); //Da 0Hz a 2kHz
   		configParam(F0, 100.0f, 15000.0f, 100.0f,"F0");   // 100Hz <= F0 <= 20kHz
        //         //pow(10,4) = 10^4
        }
 
	float Ts= 1.f/Fs; //Periodo campionamento, incremento asse dei tempi per max risuluzione. 
    float arg=0; //Argomento di sin(arg)
	float resto = 0.00; // =(float)arg - (int)arg
	float t=0; //Rampa dei tempi [0-1]
	
	void process(const ProcessArgs &args) override; //richiamiamo il processo
    
};

void Bench_osc::process(const ProcessArgs &args) {

float f0 = params[F0].getValue();
float spread = params[SPREAD].getValue();
float y=0; //Resettiamo l'OUT

//Provare a fare un check del arg prima di entrare col modulo o i=int(a) poi arg=a-i;

for(int i=0; i<N; i++)
{
	arg = 2*M_PI*((f0/Ts)+spread*i)*t; //Ogni volta mi calcolo l'argomento giusto del sin da aggiungere alla sommatoria
	resto = arg - (int)arg;	// Conversione in INTERO, si prende SOLO la parte prima della virgola 
	
	if((resto>=0)&&(resto< 0.5))	//SIN_POS: 0<= arg < 0.5
	{
		y=y+(1-16*(resto-1/4)*(resto-1/4))*1.08;
	}else{
		y=y-(1-16*(resto-3/4)*(resto-3/4))*1.08;
	}
}

y =y*1.f/N;  //Per attenuare la ampiezza della somma totale delle armoniche

t=t+Ts; //Avanzamento tempo
if(t >= 1){t=t-1;}

outputs[OUT].setVoltage(y);

}

struct Bench_oscWidget : ModuleWidget {
	Bench_oscWidget(Bench_osc * module);
};

Bench_oscWidget::Bench_oscWidget(Bench_osc * module) {

	setModule(module);
	setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ATemplate.svg")));
	box.size = Vec(6*RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		ATitle * title = new ATitle(box.size.x);
		title->setText("Bench_osc");
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
	addParam(createParam<RoundBlackKnob>(Vec(30, 70), module, Bench_osc::F0));  //RoundBlakMob è un tipo di manopola che si vuole graficare
	addParam(createParam<RoundBlackKnob>(Vec(30, 140), module, Bench_osc::SPREAD)); 
	addOutput(createOutput<PJ3410Port>(Vec(30, 210), module, Bench_osc::OUT));
}

Model *modelBench_osc = createModel<Bench_osc, Bench_oscWidget>("Bench_osc");


//===================== LINEE GUIDA GRAFICA =====================
//1) fra Scritta e connettore -> 30 pixel, così sta proprio sopra
//2) Separazione fra i vari connettori: - fra connettore e scritta lasciare 40 pixel