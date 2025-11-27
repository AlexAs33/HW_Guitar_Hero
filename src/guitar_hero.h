/*
* //TODO CABECERA
*/

#ifndef GUITAR_HERO_H
#define GUITAR_HERO_H

#define PERIODO_LEDS  250     //ms
#define MARGEN_PULSAR 100       
#define ACIERTO       20      //puntos por acierto
#define FALLO         10      //puntos por fallo
#define PERIODO_WDT   10      // 1 segundos 
#define SEC_INI_FIN   100
#define TAM_PARTITURA 5
#define T_RESET_MS    3000

void guitar_hero(unsigned int num_leds);

#endif	//GUITAR_HERO_H
