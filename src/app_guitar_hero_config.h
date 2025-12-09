// FICHERO CONFIGURACIÓN PARA ESTABLECER LOS VALORES USADOS EN APP_GUITAR_HERO.C
#define T_SECS_INI_FIN  250
#define PERIODO_LEDS    4000     //ms
#define MARGEN_PULSAR   100       
#define ACIERTO         20      //puntos por acierto
#define FALLO           10      //puntos por fallo

#ifdef DEBUG
#define TAM_PARTITURA   5
#else
#define TAM_PARTITURA		30
#endif

#define T_RESET_MS      3000
#define NOTAS_INIT      2
