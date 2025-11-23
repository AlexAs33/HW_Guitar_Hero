/*
* //TODO CABECERA
*/

#ifndef GUITAR_HERO_H
#define GUITAR_HERO_H

#define PERIODO_LEDS  1000      //ms
#define RESET         3000      //ms
#define MARGEN_PULSAR 100       
#define ACIERTO       20        //puntos por acierto
#define FALLO         10        //puntos por fallo
#define PERIODO_WDT   1         //1"
#define SEC_INI_FIN   500u
#define TAM_PARTITURA 30

void guitar_hero(unsigned int num_leds);

#endif	//GUITAR_HERO_H
