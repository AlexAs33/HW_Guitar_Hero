/* ***************************************************************************************
 * P.H.2025: Imoplementacion del Servicio de Estadísticas
 *
 * Funciones:
 * - Inicializar el servicio de estadísticas
 * - Registrar timestamps de eventos del sistema
 * - Calcular tiempos de respuesta de IRQs y usuario
 * - Medir tiempos de permanencia en colas FIFO
 * - Calcular estadísticas (media, máximo) de eventos
 * 
 * Nota:
 *  Las estadísticas se almacenan en una estructura global y se calculan de forma
 *  incremental para minimizar el uso de memoria y procesamiento.
 *  Los tiempos se miden en microsegundos (US) y milisegundos (MS) según el caso.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#include "svc_estadisticas.h"
#include "drv_tiempo.h"
#include <stdint.h>
#include <stdio.h>

// Variable global que almacena todas las estadísticas del sistema
estadisticas_t estadisticas = {0};

/**
 * Inicializa todas las estadísticas del sistema a cero.
 * Esta función prepara la estructura de estadísticas poniendo todos los
 * contadores y temporizadores en su estado inicial.
 */
void svc_estadisticas_iniciar(void) {
    // Inicializar tiempos de IRQ (Interrupciones)
    estadisticas.lanza_irq = 0;
    estadisticas.atiende_irq = 0;
    estadisticas.tiempo_respuesta_irq_US = 0;
    
    // Inicializar tiempos de secuencia de usuario
    estadisticas.termina_secuencia = 0;
    estadisticas.empieza_pulsar = 0;
    estadisticas.tiempo_respuesta_usuario_US = 0;
    
    // Inicializar tiempos de estados del sistema
    estadisticas.tiempo_inicio_despierto = 0;
    estadisticas.tiempo_fin_despierto = 0;
    estadisticas.tiempo_inicio_espera = 0;
    estadisticas.tiempo_fin_espera = 0;
    estadisticas.tiempo_despierto_US = 0;
    estadisticas.tiempo_espera_US = 0;
    
    // Inicializar estadísticas de eventos en cola (FIFO)
    for (unsigned i = 0; i < EVENT_TYPES; i++) {
        estadisticas.eventos[i].tiempo_fifo_max_MS = 0;
        estadisticas.eventos[i].tiempo_fifo_med_MS = 0;
        estadisticas.eventos[i].mediciones = 0;
        estadisticas.eventos[i].tiempo_desencolar = 0;
        estadisticas.eventos[i].tiempo_encolar = 0;
        estadisticas.eventos[i].flag_encolar = 1;  // Listo para encolar
    }
}

/**
 * Esta función captura el momento exacto en que ocurre un evento y calcula
 * automáticamente los tiempos de respuesta cuando se completa un par de eventos.
 */
void svc_estadisticas_set_tmp(estadisticas_evento_t evento) {
    uint64_t tiempo_actual = drv_tiempo_actual_us();
    
    switch (evento) {
        case e_LANZA_IRQ:
            // Marca el inicio de una interrupción
            estadisticas.lanza_irq = tiempo_actual;
            break;
            
        case e_ATIENDE_IRQ:
            // Marca cuando se atiende la interrupción y calcula el tiempo de respuesta
            estadisticas.atiende_irq = tiempo_actual;
            estadisticas.tiempo_respuesta_irq_US = 
                estadisticas.atiende_irq - estadisticas.lanza_irq;
            
            // Limpiar los timestamps temporales
            estadisticas.atiende_irq = 0;
            estadisticas.lanza_irq = 0;
            break;
            
        case e_TERMINA_SECUENCIA:
            // Marca cuando el sistema termina de mostrar una secuencia
            estadisticas.termina_secuencia = tiempo_actual;
            break;
            
        case e_EMPIEZA_PULSAR:
            // Marca cuando el usuario empieza a responder y calcula su tiempo de reacción
            estadisticas.empieza_pulsar = tiempo_actual;
            estadisticas.tiempo_respuesta_usuario_US = 
                estadisticas.empieza_pulsar - estadisticas.termina_secuencia;
            
            // Limpiar los timestamps temporales
            estadisticas.termina_secuencia = 0;
            estadisticas.empieza_pulsar = 0;
            break;
            
        case e_TIEMPO_INICIO_DESPIERTO:
            // Marca cuando el sistema entra en estado activo
            estadisticas.tiempo_inicio_despierto = tiempo_actual;
            break;
            
        case e_TIEMPO_FIN_DESPIERTO:
            // Marca cuando el sistema sale del estado activo y acumula el tiempo total
            estadisticas.tiempo_fin_despierto = tiempo_actual;
            estadisticas.tiempo_despierto_US += 
                estadisticas.tiempo_fin_despierto - estadisticas.tiempo_inicio_despierto;
            
            // Limpiar los timestamps temporales
            estadisticas.tiempo_inicio_despierto = 0;
            estadisticas.tiempo_fin_despierto = 0;
            break;
            
        case e_TIEMPO_INICIO_ESPERA:
            // Marca cuando el sistema entra en estado de espera
            estadisticas.tiempo_inicio_espera = tiempo_actual;
            break;
            
        case e_TIEMPO_FIN_ESPERA:
            // Marca cuando el sistema sale del estado de espera y acumula el tiempo total
            estadisticas.tiempo_fin_espera = tiempo_actual;
            estadisticas.tiempo_espera_US += 
                estadisticas.tiempo_fin_espera - estadisticas.tiempo_inicio_espera;
            
            // Limpiar los timestamps temporales
            estadisticas.tiempo_inicio_espera = 0;
            estadisticas.tiempo_fin_espera = 0;
            break;
            
        case e_TIEMPO_DESENCOLAR:
        case e_TIEMPO_ENCOLAR:
            // Estos eventos se gestionan en la función específica de FIFO
            break;
    }
}

/**
 * Mide cuánto tiempo pasa un evento en la cola desde que se encola hasta que
 * se desencola, calculando tanto el máximo como la media móvil.
 */
void svc_estadisticas_set_tmp_fifo(EVENTO_T evento, 
                                    estadisticas_evento_t desencolar_encolar){
    uint64_t tiempo_actual = drv_tiempo_actual_ms();
    
    // Caso: Se está encolando un evento
    if (desencolar_encolar == e_TIEMPO_ENCOLAR && 
        estadisticas.eventos[evento].flag_encolar) {
        
        // Guardar el momento en que se encola
        estadisticas.eventos[evento].tiempo_encolar = tiempo_actual;
        estadisticas.eventos[evento].flag_encolar = 0;  // Marcar como "encolado"
    } 
    // Caso: Se está desencolando un evento
    else if (desencolar_encolar == e_TIEMPO_DESENCOLAR && 
             !estadisticas.eventos[evento].flag_encolar) {
        
        // Guardar el momento en que se desencola
        estadisticas.eventos[evento].tiempo_desencolar = tiempo_actual;
        
        // Calcular cuánto tiempo estuvo en la cola
        int64_t tiempo_en_cola = 
            estadisticas.eventos[evento].tiempo_desencolar - 
            estadisticas.eventos[evento].tiempo_encolar;
        
        // Limpiar timestamps temporales
        estadisticas.eventos[evento].tiempo_desencolar = 0;
        estadisticas.eventos[evento].tiempo_encolar = 0;
        
        // Actualizar el tiempo máximo si este es mayor
        if (tiempo_en_cola > estadisticas.eventos[evento].tiempo_fifo_max_MS) {
            estadisticas.eventos[evento].tiempo_fifo_max_MS = tiempo_en_cola;
        }
        
        // Marcar como listo para un nuevo encolamiento
        estadisticas.eventos[evento].flag_encolar = 1;
        
        // Incrementar el contador de mediciones
        estadisticas.eventos[evento].mediciones++;
        
        // Guardar la media anterior para el cálculo incremental
        int64_t media_anterior = estadisticas.eventos[evento].tiempo_fifo_med_MS;
        
        // Calcular la nueva media de forma incremental
        if (estadisticas.eventos[evento].mediciones > 1) {
            // Fórmula de media móvil: nueva_media = media_anterior + (nuevo_valor - media_anterior) / n
            estadisticas.eventos[evento].tiempo_fifo_med_MS += 
                (tiempo_en_cola - media_anterior) / estadisticas.eventos[evento].mediciones;
        } else {
            // Primera medición: la media es igual al valor
            estadisticas.eventos[evento].tiempo_fifo_med_MS = tiempo_en_cola;
        }
    }
}
