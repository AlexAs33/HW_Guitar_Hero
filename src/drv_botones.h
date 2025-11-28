#ifndef DRV_BOTONES_H
#define DRV_BOTONES_H

#include <stdint.h>
#include "rt_evento_t.h"

/* Tiempos de la FSM antirebote (en ms) */
#define DRV_BOTONES_RETARDO_REBOTE_MS  20
#define DRV_BOTONES_PERIODO_MUESTREO_MS 20

/* Identificadores de eventos internos para alarmas */
typedef enum {
    e_esperando,
    e_rebotes,   
    e_muestreo, 
		e_salida
} Estado_boton;

/**
 * Inicializa el driver de botones
 * @param callback: Función para encolar eventos (normalmente rt_FIFO_encolar)
 */
void drv_botones_iniciar(void (*callback)());

/**
 * Procesa eventos de la FSM antirebote
 * Esta función debe ser llamada por el dispatcher cuando lleguen eventos internos
 * @param evento: Evento recibido
 * @param auxData: Dato auxiliar (ID del botón)
 */
void drv_botones_actualizar(EVENTO_T evento, uint32_t boton_id);

void drv_boton_estado_actualizar(EVENTO_T evento, uint32_t boton_id);

unsigned int drv_botones_cantidad(void);
#endif // DRV_BOTONES_H
