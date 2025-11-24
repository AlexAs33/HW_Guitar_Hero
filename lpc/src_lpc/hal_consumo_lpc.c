/* *****************************************************************************
 * P.H.2025: HAL de consumo
 */


#include <LPC210x.H> 
#include "hal_consumo.h"

#define STD_MODE        0x00
#define IDLE_MODE       0x01 
#define POWER_DOWN_MODE 0x02 

//definida en Startup.s
extern void switch_to_PLL(void);

// Poner el micro en modo normal
void hal_consumo_iniciar(void){
	PCON = STD_MODE; 
}

// Poner el micro en modo idle
void hal_consumo_esperar(){
	EXTWAKE = 7;   // EXTINT0,1,2 will awake the processor
	PCON |= IDLE_MODE;
}

// Poner el micro en modo power-down
void hal_consumo_dormir(){
	EXTWAKE = 7;
	PCON |= POWER_DOWN_MODE;
	switch_to_PLL(); 	//PLL aranca a 12Mhz cuando volvemos de power down ???????????
}
