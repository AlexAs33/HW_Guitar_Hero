/* *****************************************************************************
 * P.H.2025: Driver/Manejador de los temporizadores
 * suministra los servicios independientemente del hardware
 *
 * usa los servicos de hal_tiempo.h: 
 */
 
#include "drv_tiempo.h"
#include "hal_tiempo.h"
#include "svc_alarma.h"


#define TODO 0	//pa que no de error de compilacion con el proyecto vacio, modicicar

#define US2MILIS 1000

static hal_tiempo_info_t info;

/**
 * inicializa el reloj y empieza a contar
 */
bool drv_tiempo_iniciar(void){
	hal_tiempo_iniciar_tick(&info);
	return (info.ticks_per_us != 0);
}

/**
 * tiempo desde que se inicio el temporizador en microsegundos
 */
Tiempo_us_t drv_tiempo_actual_us(void){
	return (Tiempo_us_t)(hal_tiempo_actual_tick64() / info.ticks_per_us);
}

/**
 * tiempo desde que se inicio el temporizador en milisegundos
 */
Tiempo_ms_t drv_tiempo_actual_ms(void){
	return (Tiempo_ms_t)(drv_tiempo_actual_us() / US2MILIS); // pasamos de micros a milis
}

/**
 * retardo: esperar un cierto tiempo en milisegundos
 */
void drv_tiempo_esperar_ms(Tiempo_ms_t ms){
	Tiempo_ms_t espera = drv_tiempo_actual_ms() + ms;
	while(espera > drv_tiempo_actual_ms());
}

/**
 * esperar hasta un determinado tiempo (en ms), devuelve el tiempo actual
 */
Tiempo_ms_t drv_tiempo_esperar_hasta_ms(Tiempo_ms_t ms){
	while(ms > drv_tiempo_actual_ms());
	return drv_tiempo_actual_ms();
}


// En drv_tiempo.c - variables estáticas
static void (*s_callback_usuario)() = 0;
static uint32_t s_ID_evento_guardado = 0;
//static uint32_t s_callback_tick_ms = 0;

// Wrapper interno que llama la ISR
static void callback_wrapper_interno(void) {
    // Llamada al callback usuario si está definido
    if (s_callback_usuario != 0) {
        s_callback_usuario(s_ID_evento_guardado, 0);  
    }
}

void drv_tiempo_periodico_ms(Tiempo_ms_t ms, void(*funcion_callback)(uint32_t, uint32_t), uint32_t ID_evento) 
{
    s_callback_usuario = funcion_callback;
    s_ID_evento_guardado = ID_evento;

    hal_iniciar_T0(&info);
    hal_tiempo_reloj_periodico_tick((uint32_t)ms * US2MILIS * info.ticks_per_us, callback_wrapper_interno);
}
