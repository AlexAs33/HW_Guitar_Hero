
#include "guitar_hero.h"

#include "rt_fifo.h"
#include "rt_GE.h"

#include "svc_GE.h"
#include "svc_alarma.h"

#include "drv_botones.h"
#include "drv_consumo.h"
#include "drv_leds.h"
#include "drv_wdt.h"

#include "random.h"

#include <stdio.h>
#include <stdlib.h>

//Monitores Overflow
#define MONITOR_CONSUMO 1
#define MONITOR_FIFO 2
#define MONITOR_GE 3

static int notas_restantes =  5;//30;
static int leds = 0;
static int periodo_leds = 0;
static int en_partida = 0;          // 1 en partida
static int led_1_encendido = 0;
static int led_2_encendido = 0;
static int acierto = 0;
static int fallo = 0;
static int num_partidas = 0;
static int puntuacion = 0;

#define partitura \
(uint8_t[]){ \
    0b10, 0b01, 0b10, 0b01, \
    0b10, 0b01, 0b10, 0b01, \
    0b10, 0b01, 0b10, 0b01, \
    0b10, 0b01, 0b10, 0b01, \
    0b10, 0b01, 0b10, 0b01, \
    0b10, 0b01, 0b10, 0b01, \
    0b10, 0b01, 0b10, 0b01, \
    0b10, 0b01 \
}



void sec_inicio_gh(){   // 1 --- Numleds
    for(int i = 1; i <= leds; i++) {
		drv_led_establecer(i, LED_ON);
		for (int j = 0; j <= SEC_INI_FIN; j++)
			drv_consumo_esperar();
	}
	
	for(int i = 1; i <= leds; i++)
		drv_led_establecer(i, LED_OFF);
}

void sec_fin_gh(){   // Numleds --- 1
    for(int i = leds; i <= 1; i--) {
		drv_led_establecer(i, LED_ON);
		for (int j = 0; j <= SEC_INI_FIN; j++)
			drv_consumo_esperar();
	}
	
	for(int i = 1; i <= leds; i++)
		drv_led_establecer(i, LED_OFF);
}

void manejador_interrupcion_botones_fin(int32_t id_pin, int32_t id_boton) {
    //TODO boton 3 o 4 pulsados 3 segundos

    sec_fin_gh();

    while(1);   //! como fin --> wdt nos sacara
}

void manejador_interrupcion_botones_juego(int32_t id_pin, int32_t id_boton) {
    drv_botones_actualizar(ev_PULSAR_BOTON, id_pin);		//Actulaiza el estado a bajo nivel del pin
    if(id_boton == 0 || id_boton == 1){	//! mirar bien lo de los nuumeros
        rt_FIFO_encolar(ev_LEDS_GUITAR_HERO, id_boton + 1);	//Produce un evento del juego asociado al botón.
    }
    else{   //admitira 3 en nrf --> //?importante ?????
        notas_restantes = 0;
        sec_fin_gh();
    }
}

void manejador_interrupcion_botones_gh(int32_t id_pin, int32_t id_boton) {
    if (id_boton != -1) {
        if(en_partida) manejador_interrupcion_botones_juego(id_pin, id_boton);

        else manejador_interrupcion_botones_fin(id_pin, id_boton);
    }
}

//devuelve 1 si acierto
int comprobar_acierto(uint32_t boton){//! pueden ser dos leds, dos botones --> dos funciones distintas?? o todo en una ???
    return (led_1_encendido && boton == 1) || (led_2_encendido && boton == 2);      //logica para recibir un solo boton
}

void obtener_notas(uint8_t *l1, uint8_t *l2) {
    uint8_t *p = partitura;
    uint8_t nota = p[TAM_PARTITURA - notas_restantes];
    
    // Extraer el bit más significativo (primer dígito) para l1
    *l1 = (nota >> 1) & 0x01;
    
    // Extraer el bit menos significativo (segundo dígito) para l2
    *l2 = nota & 0x01;
}

void evento_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void)evento;

    if(notas_restantes <= 0){
        rt_FIFO_encolar(ev_FIN_GUITAR_HERO, 0);
        return;
    }

    if(auxData == 0){   //toca led
        uint8_t l1, l2;

        if(num_partidas == 0){
            obtener_notas(&l1, &l2);    //primera partitura codificada
        }
        else{
            obtener_notas(&l1, &l2);
            //TODO partitura aleatoria
        }
        notas_restantes --;

        //encender led(s) que toca
        if(l1){
            drv_led_establecer(1, LED_ON);
            led_1_encendido = 1;
        }
        if(l2){
            drv_led_establecer(2, LED_ON);
            led_2_encendido = 1;
        }         
    }
    else{   //toca boton  //boton solo puede ser 1 o 2
        uint32_t boton = auxData;    //! pueden ser dos leds, dos botones --> dos funciones distintas?? o todo en una ???
				
				//apagar leds 	//? funcion de apagar y encender??
				drv_led_establecer(1, LED_OFF); drv_led_establecer(2, LED_OFF);
			
			
        //ver acierto / fallo y modificar puntuación
        if(!comprobar_acierto(boton)){
            puntuacion -= fallo;

            if(puntuacion < 0){ //*QUIZA ampliar logica con notas pulsadas etc
                //GAME OVER //? secuencia especial??
                //notas_restantes = 0;
            }
            //*QUIZA ampliar logica de fallo, muchos fallos mas pierdes yo q se
        }
        else{
            puntuacion += acierto;
            //*QUIZA ampliar logica de acierto, rachas etc
        }
    }
}

void partida_guitar_hero(){
    en_partida = 1;
    //activar alarma --> periodo_leds
    uint32_t alarmaGH = svc_alarma_codificar(true, periodo_leds, 0);
    svc_alarma_activar(alarmaGH, ev_LEDS_GUITAR_HERO, 0);

    //suscribir a alarma
    svc_GE_suscribir(ev_LEDS_GUITAR_HERO, 0, evento_guitar_hero);	//Atender a eventos del juego
	
		rt_GE_lanzador(); //guarrada antologica. 1 ge una partida y asi   //! supuesto que se pueden suscribir eventos post lanzamiento. Sino lanzar cada partida
										//se queda siempre aqui
}

//? solo esto o  separar de puntuacion
void estadisticas_guitar_hero(){
		(void)puntuacion;
    //TODO
}

void fin_partida_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) evento;
    (void) auxData;
	
		//matar alarma ev_LEDS_GUITAR_HERO
    uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
		svc_alarma_activar(flags_cancelar, ev_LEDS_GUITAR_HERO, 0);

    //Acabar cosas de partida --> //? podría meterse en la funcion de partida en el caso base
    en_partida = 0;
    num_partidas ++;

    estadisticas_guitar_hero();  //TODO

    //TODO definir características nueva partida
    partida_guitar_hero();
}

void guitar_hero(unsigned int num_leds){
    leds = num_leds;
    periodo_leds = PERIODO_LEDS;
    acierto = ACIERTO;
    fallo = FALLO;
    //Poner en marcha lo necesario del background
    drv_monitor_iniciar();
    drv_consumo_iniciar((MONITOR_id_t)MONITOR_CONSUMO);
    rt_FIFO_inicializar((MONITOR_id_t)MONITOR_FIFO);  
    rt_GE_iniciar((MONITOR_id_t)MONITOR_GE);

    //sec_inicio_gh();
	drv_wdt_iniciar(PERIODO_WDT);

    drv_botones_iniciar(manejador_interrupcion_botones_gh);

    svc_GE_suscribir(ev_FIN_GUITAR_HERO, 0, fin_partida_guitar_hero);

    //? características iniciales en defines, si eso se cambian luego
    partida_guitar_hero();
}
