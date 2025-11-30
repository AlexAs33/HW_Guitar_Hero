/* ***************************************************************************************
 * P.H.2025: Implementacion del juego Guitar Hero
 *
 * - Inicio de partida:
 *      Se muestra una secuencia de luces de arranque y comienza el primer compás
 *      de la partitura.
 *
 * - Desarrollo del juego:
 *      Se siguen reglas de dificultad y puntuación configurables.  
 *      Puede modificarse la temporización y aceleración al finalizar cada partida.
 *
 * - Fin de partida:
 *      Termina al llegar a 30 compases, por puntuación baja, o si el jugador pulsa
 *      el botón 3 (o 4 en nRF).  
 *      Se muestra una secuencia de luces distinta indicando el final.  
 *      En modo depuración pueden enviarse estadísticas por UART.
 *
 * - Nueva partida:
 *      Se inicia otra partitura si el jugador mantiene pulsado el botón 3 (o 4) durante 3 s.
 *
 * - Inactividad:
 *      Si no hay juego durante 10 s, el sistema entra en modo de bajo consumo profundo.
 *      Al despertar, el juego se reinicia desde cero.
 *
 * - Supervisión:
 *      Si el planificador pasa más de 1 s sin procesar eventos, se reinicia el sistema.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#ifndef GUITAR_HERO_H
#define GUITAR_HERO_H

#define PERIODO_LEDS        500     //ms
#define MARGEN_PULSAR       100       
#define ACIERTO             20      //puntos por acierto
#define FALLO               10      //puntos por fallo
#define PERIODO_WDT         10      // 1 segundos 
#define SEC_INI_FIN         500
#define T_RESET_MS          3000
#define T_RESTA_PARTIDA     250
#define MIN_COMPASES        2

#ifdef DEBUG
	#define TAM_PARTITURA     5
#else
	#define TAM_PARTITURA     30
#endif

void guitar_hero(unsigned int num_leds);

#endif	//GUITAR_HERO_H
