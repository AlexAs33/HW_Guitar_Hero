/* ***************************************************************************************
 * P.H.2025: Implementación del Driver para la Sclusion Zone
 
 * Funciones:
 *  - Habilitar las interrupciones al salir de la zona de exclusión.
 *  - Desabilitar las interrupciones para entrar en la zona de exclusión.
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 *************************************************************************************** */

#include "drv_sc.h"

#include "hal_sc.h"

static uint32_t anid_sc = 0;    //anidamiento de sc's
static uint8_t restaurar = 1;  //0 no restauran interr. 1 si

//Deshabilita las interrupciones
void drv_sc_disable(void) {
  anid_sc++;
  restaurar = hal_sc_disable();
}

//Habilita las interrupciones si no hay anidamiento
void drv_sc_enable(void) {
	if(anid_sc > 0){	//sc's activas
		anid_sc--;
	}
  if (anid_sc == 0 && restaurar) { //no sc's activas. RESTAURAR
    hal_sc_restaurar();
  }
}
