/* ***************************************************************************************
 * P.H.2025: Implementacion de la Hardware Abstraction Layer de servicio de 
						 interrupciones externas para el procesador nRF52840
 *************************************************************************************** */

#include "hal_ext_int.h"
#include "hal_gpio.h"
#include "board.h"
#include <stdbool.h>
#include <nrf.h>

#ifdef DEBUG
    #include "svc_estadisticas.h"
#endif

// Máximo número de canales GPIOTE disponibles
#define GPIOTE_MAX_SIZE 8

// Pines relativos a las interrupciones externas (puedes ampliarlos)
#define EINT0_PIN 11 
#define EINT1_PIN 12
#define EINT2_PIN 24
#define EINT3_PIN 25

// Array con los pines usados por las EINTS
static uint32_t pines_eint[] = {EINT0_PIN, EINT1_PIN, EINT2_PIN, EINT3_PIN};
// Número de EINTS deifinidas (variable si se añaden nuevas)
static uint32_t num_eint = sizeof(pines_eint) / sizeof(pines_eint[0]);
// Función de callback llamada por las EINTS
static void (*f_cb)() = 0;

/* Convierte pin GPIO a número de canal GPIOTE */
static int8_t pin_a_eint(uint32_t pin)
{
	
#ifdef DEBUG 
				svc_estadisticas_set_tmp(e_LANZA_IRQ);
#endif
	
    switch (pin) {
        case EINT0_PIN: return 0;
        case EINT1_PIN: return 1;
        case EINT2_PIN: return 2;
        case EINT3_PIN: return 3;
        default:        return -1;
    }
}

// ISR para las interrupciones externas
void GPIOTE_IRQHandler(void)
{
    for (int i = 0; i < num_eint; i++)
    {
        int canal = pin_a_eint(pines_eint[i]);
        if (canal >= 0 && NRF_GPIOTE->EVENTS_IN[canal])
        {
						// Llamamos a funcion de callback
            if (f_cb) f_cb(pines_eint[i]); 
						// Informamos que la eirq ha sido tomada
            hal_ext_int_clear_flag(pines_eint[i]);
        }
    }
}

// Inicialización del subsistema de interrupciones externas
void hal_ext_int_iniciar(void (*callback)())
{
    f_cb = callback;
    __disable_irq();

    for (int i = 0; i < num_eint; i++) {
        NRF_GPIO->PIN_CNF[pines_eint[i]] =
            (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos)  |
            (GPIO_PIN_CNF_DRIVE_S0S1     << GPIO_PIN_CNF_DRIVE_Pos)|
            (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos)|
            (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos) |
            (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
    }

    // Limpia eventos previos y desactiva IRQs
    NRF_GPIOTE->INTENCLR = 0xFFFFFFFF;
    for (int i = 0; i < num_eint && i < GPIOTE_MAX_SIZE; i++)
        NRF_GPIOTE->EVENTS_IN[i] = 0;

    NVIC_DisableIRQ(GPIOTE_IRQn);
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);

    __enable_irq();
}

// Habilita una interrupción externa
void hal_ext_int_habilitar_int(uint32_t eint_pin)
{
    int canal = pin_a_eint(eint_pin);
    if (canal < 0) return;

    NRF_GPIOTE->CONFIG[canal] =
        (GPIOTE_CONFIG_MODE_Event      << GPIOTE_CONFIG_MODE_Pos) |
        (eint_pin                      << GPIOTE_CONFIG_PSEL_Pos) |
        (GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos);

    NRF_GPIOTE->EVENTS_IN[canal] = 0;
    NRF_GPIOTE->INTENSET = (GPIOTE_INTENSET_IN0_Msk << canal);

    static bool nvic_habilitado = false;
    if (!nvic_habilitado) {
        NVIC_ClearPendingIRQ(GPIOTE_IRQn);
        NVIC_EnableIRQ(GPIOTE_IRQn);
        nvic_habilitado = true;
    }
}

// Deshabilita una interrupción externa
void hal_ext_int_deshabilitar_int(uint32_t eint_pin)
{ 
    int canal = pin_a_eint(eint_pin);
    if (canal < 0) return;

    NRF_GPIOTE->EVENTS_IN[canal] = 0;
    NRF_GPIOTE->INTENCLR = (1UL << canal);
}

// Habilita detección de despertar
void hal_ext_int_habilitar_despertar(uint32_t eint_pin)
{
    NRF_GPIO->PIN_CNF[eint_pin] &= ~(GPIO_PIN_CNF_SENSE_Msk);
    NRF_GPIO->PIN_CNF[eint_pin] |= (GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos);
}

// Deshabilita detección de despertar
void hal_ext_int_deshabilitar_despertar(uint32_t eint_pin)
{
    NRF_GPIO->PIN_CNF[eint_pin] &= ~(GPIO_PIN_CNF_SENSE_Msk);
    NRF_GPIO->PIN_CNF[eint_pin] |= (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
}

// Limpia el flag de interrupción pendiente
void hal_ext_int_clear_flag(uint32_t eint_pin)
{
    int canal = pin_a_eint(eint_pin);
    if (canal >= 0)
        NRF_GPIOTE->EVENTS_IN[canal] = 0;
}

// Limpia todos los flags de interrupción pendientes
void hal_ext_int_clear(void)
{
    NRF_GPIOTE->INTENCLR = 0xFFFFFFFF;
}

// Funciones implementadas por compatibilidad
void pin_to_gpio(uint32_t pin) {}
void pin_to_eint(uint32_t pin) {}
