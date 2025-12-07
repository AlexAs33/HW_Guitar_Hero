#include "svc_alarma.h"

#include "drv_tiempo.h"
#include "drv_consumo.h"
#include "rt_GE.h"
#include "drv_uart.h"
#include <stdio.h>

#include <string.h>

/* Máscaras para codificación de alarma_flags */
#define svc_ALARMA_PERIODICA_MASK   0x80000000U  // Bit 31
#define svc_ALARMA_FLAGS_MASK       0x7F000000U  // Bits 30-24
#define svc_ALARMA_RETARDO_MASK     0x00FFFFFFU  // Bits 23-0

/* Variables estáticas */
static svc_alarma_t alarmas[svc_ALARMAS_MAX];
static MONITOR_id_t monitor_overflow;
static void (*f_cb)(EVENTO_T, uint32_t);
EVENTO_T evento;
//static Tiempo_ms_t periodo_evento_ms;

/* Inicialización */
void svc_alarma_iniciar(MONITOR_id_t M_overflow, void (*cb_a_llamar)(EVENTO_T, uint32_t), EVENTO_T ev_a_notificar)
{
    monitor_overflow = M_overflow;
    f_cb = cb_a_llamar;
		evento = ev_a_notificar;
	
    //periodo_evento_ms = rt_GE_TIMEOUT_INACTIVIDAD_MS;

    for (int i = 0; i < svc_ALARMAS_MAX; i++) alarmas[i].activa = false;

    // Cada vez que se llegue al periodo, se encolará ev_T_PERIODICO
	  drv_tiempo_periodico_ms(svc_ALARMAS_PERIODO_MS, cb_a_llamar, ev_a_notificar);
    //drv_tiempo_periodico_ms(periodo_evento_ms, cb_a_llamar, ev_a_notificar);

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


// Calculo del minimo comun divisor
/*
static Tiempo_ms_t mcd(Tiempo_ms_t a, Tiempo_ms_t b) {
    while (b != 0) {
        Tiempo_ms_t tmp = b;
        b = a % b;
        a = tmp;
    }
    return a;
}

// Calculo del minimo comun divisor entre el periodo de todas las alarmas
static Tiempo_ms_t calcular_mcd_global(void)
{
    Tiempo_ms_t m = 0;
    for (int i = 0; i < svc_ALARMAS_MAX; i++) {
        if (alarmas[i].activa) {
            if (m == 0)
                m = alarmas[i].contador_ms;
            else
                m = mcd(m, alarmas[i].contador_ms);
        }
    }
    return (m == 0) ? svc_ALARMAS_PERIODO_MS : m;
}
*/

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
<<<<<<< Updated upstream
=======
			}
>>>>>>> Stashed changes
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

            // se actualiza el periodo global del ev_T_PERIODICO
						//periodo_evento_ms = calcular_mcd_global();
						//drv_tiempo_periodico_ms(periodo_evento_ms, f_cb, evento);
					
#ifdef DEBUG
//						char buf[64];
//						sprintf(buf, "El nuevo periodo es %d", periodo_evento_ms);
//						UART_LOG_DEBUG(buf);
#endif

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
//           	if (alarmas[i].contador_ms >= periodo_evento_ms)
//                    alarmas[i].contador_ms -= periodo_evento_ms;
            if (alarmas[i].contador_ms >= svc_ALARMAS_PERIODO_MS)
                    alarmas[i].contador_ms -= svc_ALARMAS_PERIODO_MS;						
            else {
                    if (f_cb) {
                        f_cb(alarmas[i].evento, alarmas[i].auxData);
                    }

                    if (alarmas[i].periodica)
                        alarmas[i].contador_ms = alarmas[i].retardo_ms;
                    else
                        alarmas[i].activa = false;
<<<<<<< Updated upstream
=======
										}
>>>>>>> Stashed changes
            }
			}
    }
}

