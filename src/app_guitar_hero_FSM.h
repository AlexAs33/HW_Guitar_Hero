/*
 *  VERSI�N NO FUNCIONAL, SIMPLEMENTE ILUSTRA UNA IMPLEMENTACI�N 
 *  DEL JUEGO GUITAR HERO CON UNA M�QUINA DE ESTADOS
 */ 

#ifndef GUITAR_HERO_H
#define GUITAR_HERO_H

#include "rt_evento_t.h"
#include <stdlib.h>

#define PERIODO_LEDS  400     //ms
#define MARGEN_PULSAR 60       
#define ACIERTO       20      //puntos por acierto
#define FALLO         10      //puntos por fallo
#define PERIODO_WDT   10      // 1 segundos 
#define TAM_PARTITURA 5
#define T_RESET_MS    3000
#define NOTAS_INIT    2

void manejador_botones_guitar_hero(int32_t id_pin, int32_t id_boton);

void app_guitar_hero_actualizar(EVENTO_T ev, uint32_t aux);

void app_guitar_hero_iniciar(unsigned int num_leds);

#endif	//GUITAR_HERO_H
