/*
 *  VERSI�N NO FUNCIONAL, SIMPLEMENTE ILUSTRA UNA IMPLEMENTACI�N 
 *  DEL JUEGO GUITAR HERO CON UNA M�QUINA DE ESTADOS
 */ 

#ifndef GUITAR_HERO_H
#define GUITAR_HERO_H

#include "rt_evento_t.h"
#include <stdlib.h>


void app_guitar_hero_actualizar(EVENTO_T ev, uint32_t aux);

void app_guitar_hero_iniciar(unsigned int num_leds);

#endif	//GUITAR_HERO_H
