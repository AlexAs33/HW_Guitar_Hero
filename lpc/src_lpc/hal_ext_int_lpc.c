/* ***************************************************************************************
 * P.H.2025: Implementacion de la Hardware Abstraction Layer de servicio de 
						 interrupciones externas para el procesador LPC2105
 *************************************************************************************** */
 
#include "hal_ext_int.h"
#include "hal_gpio.h"
#include "board.h"

#ifdef DEBUG
    #include "svc_estadisticas.h"
#endif

#define CLEAR_FLAG 0x7

// Pines relativos a las interrupciones externas
#define EINT0_PIN 16
#define EINT1_PIN 14
#define EINT2_PIN 15

// Canales correspondientes a las EIRQs en el VIC
#define EINT0_VIC 14
#define EINT1_VIC 15
#define EINT2_VIC 16

// Selección de función alternativa
#define PINSEL0_EINT1_MASK   (3UL << 28)   // bits de P0.14
#define PINSEL0_EINT2_MASK   (3UL << 30)   // bits de P0.15
#define PINSEL1_EINT0_MASK   (3UL << 0)    // bits de P0.16

#define PINSEL0_EINT1_FUNC   (2UL << 28)   // 01 -> EINT1
#define PINSEL0_EINT2_FUNC   (2UL << 30)   // 01 -> EINT2
#define PINSEL1_EINT0_FUNC   (1UL << 0)    // 01 -> EINT0

static void (*f_cb)() = 0;

// Convierte pin GPIO a número de EINT del sistema 
static int8_t pin_a_eint(uint32_t pin)
{
    switch (pin) {
        case EINT0_PIN: return 0;  // P0.16 = EINT0
        case EINT1_PIN: return 1;  // P0.14 = EINT1
        case EINT2_PIN: return 2;  // P0.15	= EINT2
        default: return -1;
    }
}

static int8_t vic_channel(uint32_t pin)
{
    switch (pin) {
        case EINT0_PIN: return EINT0_VIC;
        case EINT1_PIN: return EINT1_VIC;
        case EINT2_PIN: return EINT2_VIC;
        default: return -1;
    }
}

// Prototipos de las ISRs
// ISRs para las interrupciones externas
void EINT_Handler(void) __irq 
{
    uint32_t pin;
    if (EXTINT & (1UL << 0))      pin = EINT0_PIN;
    else if (EXTINT & (1UL << 1)) pin = EINT1_PIN;
    else if (EXTINT & (1UL << 2)) pin = EINT2_PIN;
    else return;

#ifdef DEBUG 
				svc_estadisticas_set_tmp(e_LANZA_IRQ);
#endif
	
    // Limpiar flag de interrupción correspondiente
    hal_ext_int_clear_flag(pin);

    // LLamar a la función de callback asociada con el pin
    if (f_cb) f_cb(pin, pin_a_eint(pin));
	
    // Indicar al VIC que la IRQ se ha atendido
    VICVectAddr = 0;
}

// Inicialización del subsistema de interrupciones externas 
void hal_ext_int_iniciar(void (*callback)())
{
    // Deshabilitar interrupciones globales durante la configuración
    __disable_irq();

    f_cb = callback;

    // Deshabilitar todas las interrupciones externas en el VIC
    VICIntEnClr = (1 << EINT0_VIC) | (1 << EINT1_VIC) | (1 << EINT2_VIC);

    // Limpiar todos los flags de interrupción
    EXTINT = CLEAR_FLAG;

    PINSEL0 &= (~PINSEL0_EINT1_MASK | ~PINSEL0_EINT2_MASK);
    PINSEL0 |= (PINSEL0_EINT1_FUNC | PINSEL0_EINT2_FUNC);

    PINSEL1 &= ~PINSEL1_EINT0_MASK;
    PINSEL1 |= PINSEL1_EINT0_FUNC;
	
    // Configuración del VIC para cada EINT
    VICVectAddr2 = (uint32_t)EINT_Handler; // Asigna direccion de la ISR
    VICVectCntl2 = 0x20 | EINT0_VIC;    // Habilita vector y asigna canal

    VICVectAddr3 = (uint32_t)EINT_Handler;
    VICVectCntl3 = 0x20 | EINT1_VIC;
    
    VICVectAddr4 = (uint32_t)EINT_Handler;
    VICVectCntl4 = 0x20 | EINT2_VIC;

    __enable_irq();
}

/**
 *	Devuelve el estado del pin
 */
int32_t hal_ext_int_leer(uint32_t pin){
	uint32_t estado;
	int8_t eint_id = pin_a_eint(pin);
	if (eint_id < 0) return -1;
	
	estado = (EXTINT & 1 << eint_id) >> eint_id;
	return (int32_t)!estado;
}


// Habilita una interrupción externa 
void hal_ext_int_habilitar_int(uint32_t pin)
{
    if (pin_a_eint(pin) >= 0) 
        VICIntEnable = (1UL << vic_channel(pin));
}

// Deshabilita una interrupción externa 
void hal_ext_int_deshabilitar_int(uint32_t pin)
{ 
    if (pin_a_eint(pin) >= 0) 
        VICIntEnClr = (1UL << vic_channel(pin));
}

// Habilita la interrupción externa para despertar al sistema 
void hal_ext_int_habilitar_despertar(uint32_t eint_id)
{
    int8_t pin = pin_a_eint(eint_id);
    if (pin >= 0) 
        EXTWAKE |= (1UL << pin);   
}

// Deshabilita la interrupción externa para despertar al sistema 
void hal_ext_int_deshabilitar_despertar(uint32_t pin)
{
    int8_t eint_id = pin_a_eint(pin);
    if (eint_id >= 0) 
        EXTWAKE &= ~(1UL << eint_id); 
}

// Limpia el flag de interrupción pendiente 
void hal_ext_int_clear_flag(uint32_t pin)
{
    int8_t eint_id = pin_a_eint(pin);
    if (eint_id >= 0) 
        EXTINT = (1UL << eint_id);
}

// Limpia todos los flags de interrupción pendientes
void hal_ext_int_clear(void) {
		EXTINT = CLEAR_FLAG;
}

void pin_to_gpio(uint32_t pin)
{
    if (pin == EINT0_PIN) {         // EINT0 → PINSEL1 bits 1:0
        PINSEL1 &= ~(3UL << 0);
    } 
    else if (pin == EINT1_PIN) {    // EINT1 → PINSEL0 bits 29:28
        PINSEL0 &= ~(3UL << 28);
    } 
    else if (pin == EINT2_PIN) {    // EINT2 → PINSEL0 bits 31:30
        PINSEL0 &= ~(3UL << 30);
    }
}

void pin_to_eint(uint32_t pin)
{
    if (pin == EINT0_PIN) {         
        PINSEL1 |= (1UL << 0);   // EINT0 = 01
    } 
    else if (pin == EINT1_PIN) {
        PINSEL0 |= (2UL << 28);  // EINT1 = 10
    } 
    else if (pin == EINT2_PIN) {
        PINSEL0 |= (2UL << 30);  // EINT2 = 10
    }
}
