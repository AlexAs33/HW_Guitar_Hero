/* ***************************************************************************************
 * P.H.2025: Parámetros de Configuración del Modo Guitar Hero
 * Define todos los valores constantes utilizados por la aplicación Guitar Hero. 
 * 
 * Este fichero centraliza la configuración para facilitar la modificación y
 * mantenimiento del comportamiento del juego sin alterar la lógica principal.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#define T_SECS_INI_FIN  250
#define PERIODO_LEDS    1000     //ms
#define MARGEN_PULSAR   100       
#define ACIERTO         20      //puntos por acierto
#define FALLO           10      //puntos por fallo

#ifdef DEBUG
#define TAM_PARTITURA   5
#else
#define TAM_PARTITURA   30
#endif

#define T_RESET_MS      3000
#define NOTAS_INIT      1
#define TIMEOUT         255

#define MENU_DIFICULTAD \
"PULSA UN BOTON PARA ELEGIR DIFICULTAD:\n" \
"    BOTON 1 - FACILITO\n" \
"    BOTON 2 - STANDARD\n" \
"    BOTON 3 - LA COSA SE PONE SERIA...\n" \
"    BOTON 4 - PARA VERDADEROS BEAT HEROS!\n"
