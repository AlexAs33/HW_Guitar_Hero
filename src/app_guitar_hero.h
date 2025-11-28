/*
* //TODO CABECERA
*/

#ifndef GUITAR_HERO_H
#define GUITAR_HERO_H

#define PERIODO_LEDS  300     //ms
#define MARGEN_PULSAR 100       
#define ACIERTO       20      //puntos por acierto
#define FALLO         10      //puntos por fallo
#define PERIODO_WDT   10      // 1 segundos 
#define TAM_PARTITURA 5
#define T_RESET_MS    3000
#define NOTAS_INIT    2

void guitar_hero(unsigned int num_leds);

#endif	//GUITAR_HERO_H
