/* ***************************************************************************************
 * P.H.2025: Implementación del Driver de botones
 * Proporciona la interfaz necesaria para gestionar los botones con tratamiento
 * de rebotes mediante MSF, interrupciones externas y alarmas periódicas.
 *
 * Funciones:
 * - Inicialización del driver de botones
 * - Actualización del estado interno según eventos recibidos
 * - Gestión de estados individuales de cada botón
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#include "drv_botones.h"
#include "hal_ext_int.h"
#include "svc_alarma.h"
#include "board.h"
#include "drv_uart.h"

// Estructura que define un botón (estado e identificador)
typedef struct {
    Estado_boton estado;
    uint32_t id;
} Boton;

// Array de botones y lista que mapea id's con pines
static Boton botones[BUTTONS_NUMBER];
static const uint32_t lista_botones[BUTTONS_NUMBER] = BUTTONS_LIST;

// Habilitar los botones e inicializar su estado e id.
void drv_botones_iniciar(void (*callback)())
{
    hal_ext_int_iniciar(callback);
    
    svc_alarma_iniciar((MONITOR_id_t)4, rt_FIFO_encolar, ev_T_PERIODICO);
	
    for (int i = 0; i < BUTTONS_NUMBER; i++) {
        botones[i].estado = e_esperando;
        botones[i].id = lista_botones[i];

        // habilitar eirqs y que despierten al sistema
        hal_ext_int_habilitar_int(lista_botones[i]);
        hal_ext_int_habilitar_despertar(lista_botones[i]);
	
        // configurar pines como entradas
        hal_gpio_sentido(lista_botones[i], HAL_GPIO_PIN_DIR_INPUT);

        svc_GE_suscribir((EVENTO_T)(ev_BOTON_DEBOUNCE + i), 0, drv_botones_actualizar);
    }

    //svc_GE_suscribir(ev_PULSAR_BOTON, 0, drv_botones_actualizar);
}

// Retorna el id del índice del botón en el array
static int drv_botones_encontrar_indice(uint32_t boton_id)
{
    for (int i = 0; i < BUTTONS_NUMBER; i++) {
        if (botones[i].id == boton_id) {
            return i;
        }
    }
    return -1;
}

// Actualiza el estado de la MSF según el evento recibido y el estado del botón
void drv_botones_actualizar(EVENTO_T evento, uint32_t auxData)
{
    uint32_t boton_id = auxData;
    int i = drv_botones_encontrar_indice(boton_id);
    if (i < 0) return;
    
    // Botón que ha cambiado su estado
    Boton* boton = &botones[i];
    uint32_t alarma_flags;

    switch (boton->estado)
    {
    case e_esperando:
            hal_ext_int_deshabilitar_int(boton_id);
            alarma_flags = svc_alarma_codificar(false, DRV_BOTONES_RETARDO_REBOTE_MS, 0);
            svc_alarma_activar(alarma_flags, (EVENTO_T)(ev_BOTON_DEBOUNCE + i), boton_id);
            boton->estado = e_rebotes;

        break;
    
    case e_rebotes:
            alarma_flags = svc_alarma_codificar(true, DRV_BOTONES_PERIODO_MUESTREO_MS, 0);
            pin_to_gpio(boton_id);
            svc_alarma_activar(alarma_flags, (EVENTO_T)(ev_BOTON_DEBOUNCE + i), boton_id);
            boton->estado = e_muestreo;
        break;

    case e_muestreo:
			if (hal_gpio_leer_in(boton_id) == 1) {
                UART_LOG_DEBUG("MUESTREO PARA SALIRME");
								// Esto no deberia ser necesario
								//alarma_flags = svc_alarma_codificar(false , 0, 0);
								//svc_alarma_activar(alarma_flags, (EVENTO_T)(ev_BOTON_DEBOUNCE + i), boton_id);
				
                alarma_flags = svc_alarma_codificar(false, DRV_BOTONES_RETARDO_REBOTE_MS, 0);
                svc_alarma_activar(alarma_flags, (EVENTO_T)(ev_BOTON_DEBOUNCE + i), boton_id);
                boton->estado = e_salida;
            }
        break; 

    case e_salida:	
            pin_to_eint(boton_id);
            hal_ext_int_clear_flag(boton_id);
            hal_ext_int_habilitar_int(boton_id);
            if (i == 2) {
                alarma_flags = svc_alarma_codificar(false, 0, 0);
                svc_alarma_activar(alarma_flags, ev_FIN_GUITAR_HERO, 0);
						}
            boton->estado = e_esperando;
        break;

    default:
        break;
    }
}

unsigned int drv_botones_cantidad() {
    return (unsigned int) BUTTONS_NUMBER;
}

