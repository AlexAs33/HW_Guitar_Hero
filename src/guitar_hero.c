
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

//----------DEFINICIONES DE PARTITURA----------
#define partitura \
(uint8_t[]){ \
    0b11, 0b01, 0b10, 0b01, \
    0b10, 0b01, 0b10, 0b01, \
    0b10, 0b01, 0b10, 0b01, \
    0b10, 0b01, 0b10, 0b01, \
    0b10, 0b01, 0b10, 0b01, \
    0b10, 0b01, 0b10, 0b01, \
    0b10, 0b01, 0b10, 0b01, \
    0b10, 0b01 \
}

//----------DEFINICIONES DE STATICS----------
static int notas_restantes = TAM_PARTITURA;
static int leds = 0;
static int periodo_leds = 0;
static int margen_pulsar = 0;
static int en_partida = 0;          // 1 en partida
static int led_1_encendido = 0;
static int led_2_encendido = 0;
static int acierto = 0;
static int fallo = 0;
static int num_partidas = 0;
static int puntuacion = 0;

//----------SECUENCIAS LEDS----------
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

//----------MANEJADORES BOTONES----------
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

//----------AUXILIARES EVENTO DE PARTIDA----------
//devuelve 1 si acierto
int comprobar_acierto(uint32_t boton){
    return (led_1_encendido && boton == 1) || (led_2_encendido && boton == 2);      //logica para recibir un solo boton. 2 aciertos, 2 fallos o 1 y 1 si 2 botones
}

void obtener_notas(uint8_t *l1, uint8_t *l2) {
    uint8_t *p = partitura;
    uint8_t nota = p[TAM_PARTITURA - notas_restantes];
    
    // Extraer el bit más significativo (primer dígito) para l1
    *l1 = (nota >> 1) & 0x01;
    
    // Extraer el bit menos significativo (segundo dígito) para l2
    *l2 = nota & 0x01;
}

//Pre: boton = 255 --> venimos de no haber pulsado boton
void modificar_puntuacion(uint32_t boton){
	//ver acierto / fallo y modificar puntuación
	if(boton == 255 || !comprobar_acierto(boton)){	//FALLO
		puntuacion -= fallo;

    if(puntuacion < 0){ //*QUIZA ampliar logica con notas pulsadas etc
        //GAME OVER //? secuencia especial??
        //notas_restantes = 0;
    }
    //*QUIZA ampliar logica de fallo, muchos fallos mas pierdes yo q se. diferenciar falo de no dar de equivocarse ...
	}
	else{//ACIERTO
			puntuacion += acierto;
			//*QUIZA ampliar logica de acierto, rachas etc
		}
}

//----------PRINCIPAL EVENTO DE PARTIDA----------
void evento_guitar_hero(EVENTO_T evento, uint32_t auxData){

    if(notas_restantes <= 0){
        rt_FIFO_encolar(ev_FIN_GUITAR_HERO, 0);
        return;
    }

    if(evento == ev_TIMEOUT_LED){
        modificar_puntuacion(255);
        //cancelar alarma
        uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
	    svc_alarma_activar(flags_cancelar, ev_TIMEOUT_LED, 0);

        drv_led_establecer(1, LED_OFF); drv_led_establecer(2, LED_OFF);
    }

    else if(auxData == 0){   //toca led
        uint8_t l1, l2;
		obtener_notas(&l1, &l2);

        /*if(num_partidas == 0){
            obtener_notas(&l1, &l2);    //primera partitura codificada
        }
        else{
            //TODO partitura aleatoria
        }*/
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
        
        //alarma para detectar si led pulsado cuando toca
        uint32_t flags_timeout = svc_alarma_codificar(false, periodo_leds - margen_pulsar, 0);  //tienes hasta el siguiente para darle
				svc_alarma_activar(flags_timeout, ev_TIMEOUT_LED, 0);

    }
    else{   //toca boton  //boton solo puede ser 1 o 2
        //cancelar alarma timeout
        uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
	    svc_alarma_activar(flags_cancelar, ev_TIMEOUT_LED, 0);

        uint32_t boton = auxData;
				
		//apagar leds 	//? funcion de apagar y encender??
		drv_led_establecer(1, LED_OFF); drv_led_establecer(2, LED_OFF);
			
		modificar_puntuacion(boton);
    }
}

//----------INICIO Y FIN DE PARTIDA----------
void partida_guitar_hero(){
	//sec_inicio_gh();		//! comentado porque salta el wdt si se pone aqui. salta aunq se alimente despues de establecer
	
    en_partida = 1;
    //activar alarma --> periodo_leds
    uint32_t alarmaGH = svc_alarma_codificar(true, periodo_leds, 0);
    svc_alarma_activar(alarmaGH, ev_LEDS_GUITAR_HERO, 0);

    //suscribir a alarma
    svc_GE_suscribir(ev_LEDS_GUITAR_HERO, 0, evento_guitar_hero);	//Atender a eventos del juego
	
	rt_GE_lanzador(); //guarrada antologica. 1 ge una partida y asi
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
	
	//desuscribir del evento
	svc_GE_cancelar(ev_LEDS_GUITAR_HERO, evento_guitar_hero);

    //Acabar cosas de partida --> //? podría meterse en la funcion de partida en el caso base
    en_partida = 0;
    num_partidas ++;
	notas_restantes = TAM_PARTITURA;

    estadisticas_guitar_hero();  //TODO

    //TODO definir características nueva partida
    partida_guitar_hero();
}

//----------MAIN----------
void guitar_hero(unsigned int num_leds){
    leds = num_leds;
    periodo_leds = PERIODO_LEDS;
    margen_pulsar = MARGEN_PULSAR;
    acierto = ACIERTO;
    fallo = FALLO;
    //Poner en marcha lo necesario del background
    drv_monitor_iniciar();
    drv_consumo_iniciar((MONITOR_id_t)MONITOR_CONSUMO);
    rt_FIFO_inicializar((MONITOR_id_t)MONITOR_FIFO);  
    rt_GE_iniciar((MONITOR_id_t)MONITOR_GE);

    //sec_inicio_gh();	//!como no esta lanzado ge solo enciende 1
		drv_wdt_iniciar(PERIODO_WDT);

    drv_botones_iniciar(manejador_interrupcion_botones_gh);

    svc_GE_suscribir(ev_FIN_GUITAR_HERO, 0, fin_partida_guitar_hero);
		svc_GE_suscribir(ev_TIMEOUT_LED, 0, evento_guitar_hero);

    //? características iniciales en defines, si eso se cambian luego
    partida_guitar_hero();
}
