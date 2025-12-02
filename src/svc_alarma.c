/* ***************************************************************************************
 * P.H.2025: Implementación del Servicio de Alarmas
 *
 * Funciones:
 * - Iniciar el servicio con un monitor de overflow y una función de inactividad asociada
 * - Codificar alarmas con un tiempo específico
 * - Activar alarmas con un veneto asociado
 * - Actualizar alarmas
 * 
 * Nota:
 *  por las características del hardware donde se trabaja el número de alarmas está limitado.
 *  Si se excede el programa se detendrá en un bucle infinito.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#include "svc_alarma.h"

#include "drv_tiempo.h"
#include "drv_consumo.h"
#include "rt_GE.h"
#include "drv_uart.h"

#include <string.h>

#ifdef DEBUG
		#include <stdio.h>
#endif

/* Máscaras para codificación de alarma_flags */
#define svc_ALARMA_PERIODICA_MASK   0x80000000U  // Bit 31
#define svc_ALARMA_FLAGS_MASK       0x7F000000U  // Bits 30-24
#define svc_ALARMA_RETARDO_MASK     0x00FFFFFFU  // Bits 23-0

/* Variables estáticas */
static svc_alarma_t alarmas[svc_ALARMAS_MAX];
static MONITOR_id_t monitor_overflow;
static void (*f_cb)(EVENTO_T, uint32_t);
//static Tiempo_ms_t periodo_evento_ms;

/* Inicialización */
void svc_alarma_iniciar(MONITOR_id_t M_overflow, void (*cb_a_llamar)(EVENTO_T, uint32_t), EVENTO_T ev_a_notificar)
{
    monitor_overflow = M_overflow;
    f_cb = cb_a_llamar;

    for (int i = 0; i < svc_ALARMAS_MAX; i++) alarmas[i].activa = false;

    // Cada vez que se llegue al periodo, se encolará ev_T_PERIODICO
    drv_tiempo_periodico_ms(svc_ALARMAS_PERIODO_MS, cb_a_llamar, ev_a_notificar);

    svc_GE_suscribir(ev_a_notificar, 0, svc_alarma_actualizar);
}

/* Codificación */
uint32_t svc_alarma_codificar(bool periodico, Tiempo_ms_t retardo_ms, uint8_t flags)
{
    uint32_t f = (retardo_ms & svc_ALARMA_RETARDO_MASK);
    f |= ((uint32_t)(flags & 0x7F) << 24);
    if (periodico) f |= svc_ALARMA_PERIODICA_MASK;
    return f;
}

/* Activar o reprogramar una alarma */
void svc_alarma_activar(uint32_t alarma_flags, EVENTO_T ID_EVENTO, uint32_t aux_Data)
{
    bool periodica = (alarma_flags & svc_ALARMA_PERIODICA_MASK) != 0;
    Tiempo_ms_t retardo_ms = alarma_flags & svc_ALARMA_RETARDO_MASK;
    bool reprogramada = false;

    // Si el retardo es 0, desactivar alarma si existe
    if (retardo_ms == 0) {
        for (int i = 0; i < svc_ALARMAS_MAX; i++) {
            if (alarmas[i].activa && alarmas[i].evento == ID_EVENTO && alarmas[i].auxData == aux_Data) 
                alarmas[i].activa = false;
        }
        return;
    }
    // Reprogramar o crear alarma 
    for (int i = 0; i < svc_ALARMAS_MAX && !reprogramada; i++) {
        if (!alarmas[i].activa || (alarmas[i].activa && alarmas[i].evento == ID_EVENTO && alarmas[i].auxData == aux_Data)) 
        {
            alarmas[i].activa = true;
            alarmas[i].periodica = periodica;
            alarmas[i].retardo_ms = retardo_ms;
            alarmas[i].contador_ms = retardo_ms;
            alarmas[i].evento = ID_EVENTO;
            alarmas[i].auxData = aux_Data;

            reprogramada = true;
        }
    }   
    // si no se ha encontrado una alarma en ninguno de los casos, marcamos overflow
    if (!reprogramada) {
        drv_monitor_marcar(monitor_overflow);
				UART_LOG_ERROR("OVERFLOW DE ALARMAS!!");
        while(1); // overflow de alarmas
    }
}


/* Actualizar contadores */
void svc_alarma_actualizar(EVENTO_T evento, uint32_t aux)
{
    for (int i = 0; i < svc_ALARMAS_MAX; i++) {
        if (alarmas[i].activa) {
            if (alarmas[i].contador_ms >= svc_ALARMAS_PERIODO_MS)
                    alarmas[i].contador_ms -= svc_ALARMAS_PERIODO_MS;
            else {
                    if (f_cb) {
                        f_cb(alarmas[i].evento, alarmas[i].auxData);
#ifdef DEBUG
												if (alarmas[i].evento != 1) {
														char buf[64];
														sprintf(buf, "Soy el evento: %d", alarmas[i].evento);
														UART_LOG_DEBUG(buf);
												}
#endif
                    }

                    if (alarmas[i].periodica)
                        alarmas[i].contador_ms = alarmas[i].retardo_ms;
                    else
                        alarmas[i].activa = false;
            }
			}
    }
}

