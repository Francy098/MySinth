/*--------------------------- ABC ---------------------------------*
 * 
 * Author: Leonardo Gabrielli <l.gabrielli@univpm.it>
 * License: GPLv3
 *
 * For a detailed guide of the code and functions see the book:
 * "Developing Virtual Synthesizers with VCV Rack" by L.Gabrielli
 *
 * Copyright 2020, Leonardo Gabrielli
 *
 *-----------------------------------------------------------------*/

#include "ABC.hpp"


Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;
	
	//Progetto MySinth
	p->addModel(modelMySinth);
}
