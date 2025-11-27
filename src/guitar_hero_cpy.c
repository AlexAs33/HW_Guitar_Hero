
#include "guitar_hero.h"

#include "rt_fifo.h"
#include "rt_GE.h"

#include "svc_GE.h"
#include "svc_alarma.h"

#include "drv_botones.h"
#include "drv_consumo.h"
#include "drv_leds.h"
#include "drv_wdt.h"
#include "drv_uart.h"

#include "random.h"

#include <stdio.h>
#include <stdlib.h>

//Monitores Overflow
#define MONITOR_CONSUMO 1
#define MONITOR_FIFO 2
#define MONITOR_GE 3

//----------DEFINICIONES DE STATICS----------
//Control de leds
static int leds = 0;                        //Numero de leds
static int periodo_leds = 0;                //ms cada los que se encienden los leds
static int led_1_encendido = 0;             //1 si led 1 encendido
static int led_2_encendido = 0;             //1 si led 2 encendido
//Control partida
static int notas_restantes = TAM_PARTITURA; //Acordes que tiene una partitura
static int margen_pulsar = 0;               //ms maximos antes del siguiente acorde para pulsar
static int en_partida = 0;                  // 1 en partida
static int num_partidas = 0;                //num partidas jugadas
//Control puntuacion
static int puntos_acierto = 0;                     //puntos por acierto
static int puntos_fallo = 0;                       //puntos por fallo
static int puntuacion = 0;                  //puntuacion en una partida
static int aciertos = 0;                    //aciertos en una partida
static int fallos = 0;                      //fallos en una partida
static int puntuacion_total = 0;            //puntuacion total
static int racha = 0;                       //aciertos encadenados en una partida
//Partitura
static uint8_t partitura[TAM_PARTITURA] = { 
    0b10, 0b01, 0b11, 0b11, 
    0b10, 0b01, 0b10, 0b01,
    0b10, 0b01, 0b10, 0b01,
    0b10, 0b01, 0b10, 0b01,
    0b10, 0b01, 0b10, 0b01,
    0b10, 0b01, 0b10, 0b01,
    0b10, 0b01, 0b10, 0b01,
    0b10, 0b01
};


void evento_guitar_hero(EVENTO_T evento, uint32_t auxData);
//----------SECUENCIAS LEDS----------
static int palpitaciones = 0;
void sec_inicio_gh(EVENTO_T ev, uint32_t auxData){   // 1 --- Numleds
		(void)ev;
		(void)auxData;

    if(palpitaciones < leds){
        drv_led_establecer(palpitaciones +1, LED_ON);
        palpitaciones ++;
    }
    else{
        uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
				svc_alarma_activar(flags_cancelar, ev_SEC_INI_FIN, 0);
				svc_GE_cancelar(ev_SEC_INI_FIN, sec_inicio_gh);
        for(int i = 1; i <= leds; i++)
						drv_led_establecer(i, LED_OFF);
				en_partida = 1;
    //activar alarma --> periodo_leds
    uint32_t alarmaGH = svc_alarma_codificar(true, periodo_leds, 0);
    svc_alarma_activar(alarmaGH, ev_LEDS_GUITAR_HERO, 0);

    //suscribir a alarma
    svc_GE_suscribir(ev_LEDS_GUITAR_HERO, 0, evento_guitar_hero);	//Atender a eventos del juego
    }
}

void sec_fin_gh(EVENTO_T ev, uint32_t auxData){   // Numleds --- 1
    for(int i = leds; i <= 1; i--) {
		drv_led_establecer(i, LED_ON);
		for (int j = 0; j <= SEC_INI_FIN; j++)
				drv_consumo_esperar();
	}
	
	for(int i = 1; i <= leds; i++)
			drv_led_establecer(i, LED_OFF);
}

//Pre modo = 0 --> inicio. 1 --> fin
void main_secuencias_gh(int modo){
    uint32_t alarma_sec_inicio = svc_alarma_codificar(true, SEC_INI_FIN, 0);
    svc_alarma_activar(alarma_sec_inicio, ev_SEC_INI_FIN, 0);
    palpitaciones = 0;
    if(modo == 0) svc_GE_suscribir(ev_SEC_INI_FIN, 0, sec_inicio_gh);
    else if (modo == 1) svc_GE_suscribir(ev_SEC_INI_FIN, 0, sec_fin_gh);
}

//----------MANEJADORES BOTONES----------
void manejador_interrupcion_botones_fin(int32_t id_pin, int32_t id_boton) {
    //TODO boton 3 o 4 pulsados 3 segundos
		EVENTO_T ev; 
		uint32_t auxData;
		sec_fin_gh(ev, auxData);

		while(1);   //! como fin --> wdt nos sacara
}

void manejador_interrupcion_botones_juego(int32_t id_pin, int32_t id_boton) {
    drv_botones_actualizar(ev_PULSAR_BOTON, id_pin);		//Actulaiza el estado a bajo nivel del pin
		rt_FIFO_encolar(ev_ACT_INACTIVIDAD, 0);
	
#ifdef DEBUG
				char buf[64];
        sprintf(buf, "Soy el botón %d", id_boton);
        UART_LOG_DEBUG(buf);
#endif
	
    if(id_boton == 0 || id_boton == 1){	//! mirar bien lo de los nuumeros
        rt_FIFO_encolar(ev_LEDS_GUITAR_HERO, id_boton + 1);	//Produce un evento del juego asociado al botón.
    }
    else{   //admitira 3 en nrf --> //?importante ?????
        notas_restantes = 0;
				EVENTO_T ev; 
				uint32_t auxData;
				sec_fin_gh(ev, auxData);
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
        racha = 0;
        fallos++;
        if(boton == 255){   //Fallo de no pulsar nada cuenta por dos
            puntuacion -= puntos_fallo*2;
        }
        else{
            puntuacion -= puntos_fallo;
        }

       /* if(puntuacion < (puntos_acierto * (TAM_PARTITURA - notas_restantes))/2){    //s no llevamos la mitad de los puntos posibles perdemos
            //GAME OVER
            notas_restantes = 0;
            //?secuencia game over distinta a la de apagar??
        }*/
	}
	else{//ACIERTO
        racha ++;
        if(racha >= 5){
            puntuacion += puntos_acierto * racha;
        }
        else{
            puntuacion += puntos_acierto;
        }
        aciertos ++;    
	}
}

//----------PRINCIPAL EVENTO DE PARTIDA----------
static int led1_estado_previo = 0;
static int led2_estado_previo = 0;
void evento_guitar_hero(EVENTO_T evento, uint32_t auxData){

    if(notas_restantes <= 0){
        rt_FIFO_encolar(ev_FIN_GUITAR_HERO, 0);
        return;
    }

    if(evento == ev_TIMEOUT_LED && notas_restantes < 29){
        modificar_puntuacion(255);
        //cancelar alarma
        uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
				svc_alarma_activar(flags_cancelar, ev_TIMEOUT_LED, 0);

        drv_led_establecer((LED_id_t)3, LED_OFF); drv_led_establecer((LED_id_t)4, LED_OFF);
    }

    else if(auxData == 0){   //toca led
        uint8_t l1, l2;

        if(num_partidas == 0){
            obtener_notas(&l1, &l2);    //primera partitura codificada
        }
        else{
            uint8_t nota = random_value(0, 3);   // notas entre 0 y 3
            l1 = (nota & 0x01) ? '1' : '0';
            l2 = (nota & 0x02) ? '1' : '0';
        }
        notas_restantes --;

        drv_led_establecer((LED_id_t)1, LED_OFF); drv_led_establecer((LED_id_t)2, LED_OFF);

         //encender led(s) que toca. segundo paso
        if(led1_estado_previo){
            drv_led_establecer((LED_id_t)3, LED_ON);
            led_1_encendido = 1;
        }
        else led_1_encendido = 0;

        if(led2_estado_previo){
            drv_led_establecer((LED_id_t)4, LED_ON);
            led_2_encendido = 1;
        }
        else led_2_encendido = 0;

        //encender led(s) que toca. primer paso
        if(l1){
            drv_led_establecer((LED_id_t)1, LED_ON);
            led1_estado_previo = 1;
        }
        else led1_estado_previo = 0;

        if(l2){
            drv_led_establecer((LED_id_t)2, LED_ON);
            led2_estado_previo = 1;
        }
        else led2_estado_previo = 0;

        //alarma para detectar si led pulsado cuando toca
        uint32_t flags_timeout = svc_alarma_codificar(false, periodo_leds - margen_pulsar, 0);  //tienes hasta el siguiente para darle
				svc_alarma_activar(flags_timeout, ev_TIMEOUT_LED, 0);

    }
    else{   //toca boton  //boton solo puede ser 1 o 2
        //cancelar alarma timeout
        uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
				svc_alarma_activar(flags_cancelar, ev_TIMEOUT_LED, 0);

        uint32_t boton = auxData;
			
#ifdef DEBUG
				char buf[64];
        sprintf(buf, "Voy a apagar el led leds %d", boton);
        UART_LOG_DEBUG(buf);
#endif
			
				//apagar led correspondiente
				drv_led_establecer(boton, LED_OFF);
					
				modificar_puntuacion(boton);
    }
}

//----------INICIO Y FIN DE PARTIDA----------
void partida_guitar_hero(){
		main_secuencias_gh(0);		//! comentado porque salta el wdt si se pone aqui. salta aunq se alimente despues de establece
		rt_GE_lanzador(); //guarrada antologica. 1 ge una partida y asi
}

//? solo esto o  separar de puntuacion
void estadisticas_guitar_hero(){
		drv_uart_puts("Tu desempegno en esta partida ha sido el siguiente\r\n");
		drv_uart_puts("Con "); drv_uart_putint(aciertos); drv_uart_puts(" aciertos\r\n");
		drv_uart_puts("Y "); drv_uart_putint(fallos); drv_uart_puts(" fallos\r\n");
		//TODO contar rachas y poner
		drv_uart_puts("Has conseguido una puntuacion de "); drv_uart_putint(puntuacion); drv_uart_puts(" puntos\r\n");
		drv_uart_puts("Lo que en tus "); drv_uart_putint(num_partidas); drv_uart_puts(" partidas, suma un total de "); drv_uart_putint(puntuacion_total); drv_uart_puts(" puntos\r\n");
	
    //TODO hay que poner cosas de estadistica de rendimiento???
}

void fin_partida_guitar_hero(EVENTO_T evento, uint32_t auxData){
    (void) evento;
    (void) auxData;
	
		// matar alarma ev_LEDS_GUITAR_HERO
    uint32_t flags_cancelar = svc_alarma_codificar(false, 0, 0);
		svc_alarma_activar(flags_cancelar, ev_LEDS_GUITAR_HERO, 0);
	
		// desuscribir del evento
		svc_GE_cancelar(ev_LEDS_GUITAR_HERO, evento_guitar_hero);

    // acabar cosas de partida --> //? podría meterse en la funcion de partida en el caso base
    en_partida = 0;
    num_partidas ++;
    puntuacion_total += puntuacion;
		notas_restantes = TAM_PARTITURA;
	
		// cambiar partitura para la siguiente partida
		for (int i = 0; i < TAM_PARTITURA; i++)
			partitura[i] = random_value(0, 3);

    estadisticas_guitar_hero();  //TODO
		aciertos = 0;
		fallos = 0;
		puntuacion = 0;
    //TODO definir características nueva partida
    partida_guitar_hero();
}

//----------MAIN----------
void guitar_hero(unsigned int num_leds){
    leds = num_leds;
    periodo_leds = PERIODO_LEDS;
    margen_pulsar = MARGEN_PULSAR;
    puntos_acierto = ACIERTO;
    puntos_fallo = FALLO;

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
	
		drv_uart_init(9600);

    drv_uart_init(9600);

    //? características iniciales en defines, si eso se cambian luego
    partida_guitar_hero();
}
