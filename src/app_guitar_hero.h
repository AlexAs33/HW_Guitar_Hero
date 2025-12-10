/* ***************************************************************************************
 * P.H.2025: Interfaz del Modo Guitar Hero
 * Define la estructura externa del juego Guitar Hero basado en una máquina de
 * estados. Proporciona los puntos de entrada necesarios para iniciar el modo
 * de juego y para procesar los distintos eventos que recibe el sistema.
 *
 * Funciones:
 * - Inicialización del modo Guitar Hero
 * - Actualización del estado interno según los eventos recibidos
 * - Gestión de las distintas fases del juego mediante máquina de estados
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#ifndef GUITAR_HERO_H
#define GUITAR_HERO_H

#include "rt_evento_t.h"
#include <stdlib.h>

/* 
 * Inicializa las estricturas necesariar para el funcionamiento
 * del juego Guitar Hero. 
 * Como parámetro se le pasa el número de leds disponibles
 * para representar las notas.
 */
void app_guitar_hero_iniciar(unsigned int num_leds);

/*
 * Función principal para controlar la máquina de estados del juego
 * Dependiendo del evento recibido y el estado actual, se realizan 
 * diferentes acciones para avanzar en el Guitar Hero, como mostrar
 * secuenciad de inicio y fin de partida, gestionar la elección de dificultad,
 * mostrar la partitura de notas, y manejar los aciertos y fallos del jugador.
 * El evento recibido vendrá definido en EVENTO_T y auxData proporcionará
 * información adicional necesaria para procesar el evento.
 */
void app_guitar_hero_actualizar(EVENTO_T ev, uint32_t auxData);

#endif	//GUITAR_HERO_H
