/* *****************************************************************************
 * P.H.2025: Temporizadores en nRF52840
 * Implementaci�n para cumplir hal_tiempo.h, equivalente al LPC2105
 *
 * - Timer1 ? contador libre (tick de 64 bits, con desbordes acumulados)
 * - Timer0 ? reloj peri�dico por IRQ
 * 
 * Equivalente funcional al c�digo del LPC2105:
 *   - hal_tiempo_iniciar_tick()
 *   - hal_tiempo_actual_tick64()
 *   - hal_tiempo_periodico_config_tick()
 *   - hal_tiempo_periodico_set_callback()
 *   - hal_tiempo_periodico_enable()
 *
 * ****************************************************************************/

#include <nrf.h>                       /* nRF 0x definitions */


#include "hal_tiempo.h"

#define PCLK_MHZ          16u
#define TICKS_PER_US_T1   16
#define TICKS_PER_US_T0   1
#define COUNTER_BITS      32u
#define COUNTER_MAX       (0xFFFFFFFFu)

/* ---- Tick libre con TIMER1 ------------------------------------------------ */
static volatile uint64_t irq_count_t1 = 0;


/* IRQ Handler: igual que T1_ISR del LPC */
void TIMER1_IRQHandler(void)
{
	  volatile uint32_t dummy;
    if (NRF_TIMER1->EVENTS_COMPARE[0]) {
        NRF_TIMER1->EVENTS_COMPARE[0] = 0;
        irq_count_t1++;
				// leerlo de nuevo para asegurar que est� a 0 antes de salir de la IRQ
				dummy = NRF_TIMER1->EVENTS_COMPARE[0];
			  dummy; // para que no salga warning
    }
		irq_count_t1++;
}


/* *****************************************************************************
 * Inicializar TIMER1 como un timer que genera irq's */
 
void hal_tiempo_iniciar_tick(hal_tiempo_info_t *out_info)
{
	// Inicializaci�n de info
	out_info->counter_bits = COUNTER_BITS;
	out_info->counter_max = COUNTER_MAX;
	out_info->ticks_per_us = TICKS_PER_US_T1;
	
	// Parar y limpiar el timer antes de empezar	
	NRF_TIMER1->TASKS_STOP = 1;
  NRF_TIMER1->TASKS_CLEAR = 1;
	
	// Inicializar timer como 32 bits
	NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
	NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
	
  // Timer1 = 16 MHz -> 16 ticks / us 
	NRF_TIMER1->PRESCALER = 0;
	
	// Cada COUNTER_MAX, generar EVENTS_COMPARE[0]
	NRF_TIMER1->CC[0] = COUNTER_MAX;
	
	// Activar interrupciones con EVENTS_COMPARE[0]
	NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;
	
	// Reinicio de contador al llegar a CC[0]
	// no usamos shortcuts the clear porq vamos hasta overflow
	NRF_TIMER1->SHORTS = 0; 
	
	// A�adir IRQ handle al VIC
	NVIC_EnableIRQ(TIMER1_IRQn);
	
	// Activar timer
	NRF_TIMER1->TASKS_START = 1;
}

/* *****************************************************************************
 * Lectura del n�mero de irq generadas*/
 
uint64_t hal_tiempo_actual_tick64(void)
{
	 uint32_t hi1, lo1, hi2, lo2;
	
		NRF_TIMER1 -> TASKS_CAPTURE[3] = 1;
    lo1 = NRF_TIMER1->CC[3];
    hi1 = irq_count_t1;
		NRF_TIMER1 -> TASKS_CAPTURE[3] = 1;
    lo2 = NRF_TIMER1->CC[3];
	
	  // si se ha desbordado
    if (lo2 < lo1) {
        /* ocurri� wrap entre lecturas: usar hi actualizado */
        hi2 = irq_count_t1;
        return (((uint64_t)hi2) * ((uint64_t)((uint64_t)COUNTER_MAX + 1u))) + lo2;
    }
    return (((uint64_t)hi1) * ((uint64_t)(COUNTER_MAX + 1u))) + lo2;
}



/* ---- Reloj peri�dico con TIMER0 ------------------------------------------ */
static void (*s_cb)(void) = 0;

/* IRQ Handler: igual que T0_ISR del LPC */
/**
 * RSI del timer 1, vector definido en : arm_startup_nrf52840.s
 * This IRQ handler will trigger every DELAY_ms ms
 */
void TIMER0_IRQHandler(void)
{
		volatile uint32_t dummy;
    if (NRF_TIMER0->EVENTS_COMPARE[0]) {
				// clear del flag
        NRF_TIMER0->EVENTS_COMPARE[0] = 0;
				// leerlo de nuevo para asegurar que est� a 0 antes de salir de la IRQ
				dummy = NRF_TIMER0->EVENTS_COMPARE[0];
			  dummy; // para que no salga warning
			
			  if (s_cb) s_cb();
    }
}

/* *****************************************************************************
 * Registra el callback del reloj peri�dico
 */
void hal_tiempo_periodico_set_callback(void (*cb)(void)) {
    s_cb = cb;
}


	void hal_iniciar_T0(hal_tiempo_info_t *out_info){
		out_info->ticks_per_us   = TICKS_PER_US_T0;
    out_info->counter_bits   = COUNTER_BITS;
    out_info->counter_max    = COUNTER_MAX;
	}

void hal_tiempo_periodico_config_tick(uint32_t periodo_en_tick) {
	// Resetear y desactivar timer	
	NRF_TIMER0->TASKS_CLEAR = 1;
	NRF_TIMER0->TASKS_STOP = 1;
	
	// Si el periodo es nulo se retorna directamente
	if (periodo_en_tick == 0) return;
	
	// Inicializar timer como 32 bits
	NRF_TIMER0->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
	NRF_TIMER0->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
	
    // Timer1 = 16 MHz -> 16 / 2^4 -> 1 ticks / us 
	NRF_TIMER0->PRESCALER = 4 << TIMER_PRESCALER_PRESCALER_Pos;
	
	// Cada DELAY_ms us, generar EVENTS_COMPARE[0]
	NRF_TIMER0->CC[0] = periodo_en_tick - 1;
	
	// Activar interrupciones con EVENTS_COMPARE[0]
	NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;
	
	// Reinicio de contador al llegar a CC[0]
	NRF_TIMER0->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;
}

void hal_tiempo_periodico_enable(bool enable) {
	if (enable) {
		// A�adir IRQ handle al VIC
		NVIC_EnableIRQ(TIMER0_IRQn);
		
		// Resetear y activar timer	
		NRF_TIMER0->TASKS_CLEAR = 1;
		NRF_TIMER0->TASKS_START = 1;
	}
	else {
		// Parar timer y desactivar interrupciones
		NRF_TIMER0->TASKS_STOP = 1;
		NVIC_DisableIRQ(TIMER0_IRQn);
	}
}

void hal_tiempo_reloj_periodico_tick(uint32_t periodo_en_tick, void(*funcion_callback_drv)()){
	bool hay_periodo = (periodo_en_tick != 0);
	hal_tiempo_periodico_config_tick(periodo_en_tick);
	// Solo hacemos set de la funcion si hay un periodo v�lido
	if(hay_periodo)
		hal_tiempo_periodico_set_callback(funcion_callback_drv);
	hal_tiempo_periodico_enable(hay_periodo);
}

