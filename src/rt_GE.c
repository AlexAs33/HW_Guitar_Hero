/* *****************************************************************************
 * P.H.2025: Gestor de Eventos (rt_GE)
 * Control centralizado de suscripciones y despacho de eventos.
 * Integra las funciones del servicio GE (svc_GE) en este mismo módulo.
 * 
 * FUNCIONALIDAD:
 *  - Gestiona la tabla de suscripciones por evento.
 *  - Lanza callbacks según prioridad.
 *  - Reprograma automáticamente la alarma de inactividad.
 *  - Entra en bajo consumo cuando no hay eventos.
 * ****************************************************************************/

#include "rt_GE.h"
#include "svc_GE.h"
#include "svc_alarma.h"
#include "rt_fifo.h"
#include "drv_consumo.h"
#include "drv_wdt.h"
#include "drv_uart.h"
#include <string.h>
#include <stdint.h>
#include "drv_uart.h"

/* Fila de suscripciones por tipo de evento */
typedef struct {
    uint8_t num_suscritos;                                      // Cantidad actual de callbacks
    void (*callbacks[rt_GE_MAX_SUSCRITOS])(EVENTO_T, uint32_t); // Callbacks registrados
    uint8_t prioridades[rt_GE_MAX_SUSCRITOS];                   // Prioridades asociadas
} GestorEvento;

static GestorEvento tabla_suscripciones[EVENT_TYPES];
static MONITOR_id_t monitor_overflow;
static uint32_t contador_inactividad;  // Contador auxiliar para ev_INACTIVIDAD

void rt_GE_iniciar(MONITOR_id_t M_overflow)
{
    monitor_overflow = M_overflow;
    contador_inactividad = 0;

    // Inicializar tabla de suscripciones
    for (int i = 0; i < EVENT_TYPES; i++) {
        tabla_suscripciones[i].num_suscritos = 0;
        tabla_suscripciones[i].prioridades[0] = 0;
        tabla_suscripciones[i].callbacks[0] = NULL;
    }

    // Suscribir rt_GE_actualizar al evento de inactividad
    svc_GE_suscribir(ev_INACTIVIDAD, 0 , rt_GE_actualizar);

    // Suscribir rt_GE_actualizar a todos los eventos de usuario
    EVENTO_T eventos_usuario[] = ev_USUARIO;
    for (int i = 0; i < ev_NUM_EV_USUARIO; i++) {
        svc_GE_suscribir(eventos_usuario[i], 0, rt_GE_actualizar);
    }
}

void rt_GE_lanzador(void)
{
    EVENTO_T evento;
    uint32_t auxData;
    Tiempo_us_t timestamp;

    // Programar la primera alarma de inactividad
    uint32_t flags_inactividad = svc_alarma_codificar(false, rt_GE_TIMEOUT_INACTIVIDAD_MS, 0);
    svc_alarma_activar(flags_inactividad, ev_INACTIVIDAD, contador_inactividad);

    // Bucle principal del sistema
    while (1) {
        if (rt_FIFO_extraer(&evento, &auxData, &timestamp)) {
					EVENTO_T eventos_usuario[] = ev_USUARIO;
					for (int i = 0; i < ev_NUM_EV_USUARIO; i++) {
							if(evento == eventos_usuario[i] || evento == ev_T_PERIODICO ) {
								drv_wdt_feed();
								i = ev_NUM_EV_USUARIO;
							}
					}
						
            // Hay evento disponible: despacharlo
            GestorEvento *fila = &tabla_suscripciones[evento];

            for (int i = 0; i < fila->num_suscritos; i++) 
                if (fila->callbacks[i] != NULL) 
                    fila->callbacks[i](evento, auxData);
        } 
        else  // Sin eventos: modo bajo consumo
            drv_consumo_esperar();
        
    }
}

void rt_GE_actualizar(EVENTO_T evento, uint32_t auxiliar)
{
		(void)auxiliar;
    switch (evento) {
        case ev_INACTIVIDAD:
            // Timeout de inactividad → entrar en modo dormir profundo
            if (auxiliar == contador_inactividad) {
                drv_consumo_dormir();
            }
            break;
            
        default:
            // Cualquier evento de usuario → reprogramar alarma de inactividad
						UART_LOG_DEBUG("ACTUALIZO INACTIVIDAD!!");
				
						contador_inactividad = contador_inactividad;
            
						uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
						svc_alarma_activar(flags_cancelar, ev_INACTIVIDAD, contador_inactividad);
						contador_inactividad++;
            uint32_t alarma_flags = svc_alarma_codificar(false, rt_GE_TIMEOUT_INACTIVIDAD_MS, 0);
            svc_alarma_activar(alarma_flags, ev_INACTIVIDAD, contador_inactividad);
            break;
    }
}

bool svc_GE_suscribir(EVENTO_T ID_evento, uint8_t prioridad, void (*f_callback)(EVENTO_T, uint32_t))
{
    if (ID_evento >= EVENT_TYPES) 
        return false;  // Evento inválido

    GestorEvento *fila = &tabla_suscripciones[ID_evento];

    // Si no hay espacio, marcar overflow y detener la ejecución
    if (fila->num_suscritos >= rt_GE_MAX_SUSCRITOS) {
        drv_monitor_marcar(monitor_overflow);
        while (1);  
    }
    // Determinar posición de inserción 
    bool encontrada = false;
    int pos_insercion = fila->num_suscritos;
    for (int i = 0; i < fila->num_suscritos && !encontrada; i++) {
        if (prioridad < fila->prioridades[i]) {
            pos_insercion = i;
            encontrada = true;
        }
    }
    // Desplazar elementos para insertar ordenadamente
    for (int i = fila->num_suscritos; i > pos_insercion; i--) {
        fila->callbacks[i] = fila->callbacks[i - 1];
        fila->prioridades[i] = fila->prioridades[i - 1];
    }
    // Insertar nuevo callback
    fila->callbacks[pos_insercion] = f_callback;
    fila->prioridades[pos_insercion] = prioridad;
    fila->num_suscritos++;

    return true;
}

void svc_GE_cancelar(EVENTO_T ID_evento, void (*f_callback)(EVENTO_T, uint32_t))
{
    // Salimos si el evento no existe
    if (ID_evento >= EVENT_TYPES)  return;  

    GestorEvento *fila = &tabla_suscripciones[ID_evento];

    bool cancelada = false;
    for (int i = 0; i < fila->num_suscritos && !cancelada; i++) {
        if (fila->callbacks[i] == f_callback) {
            // Eliminar y compactar
            for (int j = i; j < fila->num_suscritos - 1; j++) {
                fila->callbacks[j] = fila->callbacks[j + 1];
                fila->prioridades[j] = fila->prioridades[j + 1];
            }

            // Limpiar última posición
            fila->callbacks[fila->num_suscritos - 1] = NULL;
            fila->prioridades[fila->num_suscritos - 1] = 0;
            fila->num_suscritos--;
            cancelada = true;
        }
    }
}
