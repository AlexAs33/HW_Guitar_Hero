/* *****************************************************************************
 * P.H.2025: Implementación del Driver de Monitores
 * Funciones:
 * - Inicializar todos los monitores como salidas desmarcadas
 * - Marcar un monitor (ponerlo a nivel lógico activo)
 * - Desmarcar un monitor (ponerlo a nivel lógico inactivo)
 *
 * Autores:
 * - Pablo Plumed
 * - Alex Asensio
 ***************************************************************************** */

#include "drv_monitor.h"

#if MONITORS_NUMBER > 0
    static const HAL_GPIO_PIN_T s_monitor_list[MONITORS_NUMBER] = MONITORS_LIST;
#endif

/* Helpers ------------------------------------------------------------------ */
static inline int monitor_id_valido(MONITOR_id_t id) {
#if MONITORS_NUMBER > 0
    return (id >= 1 && id <= (MONITOR_id_t)MONITORS_NUMBER);
#else
    (void)id;
    return 0;
#endif
}

static inline uint32_t hw_level_from_state(int marcado) {
#if MONITORS_NUMBER > 0
    /* Si activo-alto: marcado -> 1; si activo-bajo: marcado -> 0 */
    return (MONITORS_ACTIVE_STATE ? (marcado != 0) : (marcado == 0));
#else
    (void)marcado;
    return 0;
#endif
}

/* API ---------------------------------------------------------------------- */

uint32_t drv_monitor_iniciar(void) {
#if MONITORS_NUMBER > 0
    for (MONITOR_id_t i = 1; i <= (MONITOR_id_t)MONITORS_NUMBER; ++i) {
        hal_gpio_sentido(s_monitor_list[i - 1], HAL_GPIO_PIN_DIR_OUTPUT);
        hal_gpio_escribir(s_monitor_list[i - 1], hw_level_from_state(0));
    }
    return (uint32_t)MONITORS_NUMBER;
#else
    return 0;
#endif
}

int drv_monitor_marcar(MONITOR_id_t id) {
#if MONITORS_NUMBER > 0
    if (!monitor_id_valido(id)) return 0;
    hal_gpio_escribir(s_monitor_list[id - 1], hw_level_from_state(1));
    return 1;
#else
    (void)id;
    return 0;
#endif
}

int drv_monitor_desmarcar(MONITOR_id_t id) {
#if MONITORS_NUMBER > 0
    if (!monitor_id_valido(id)) return 0;
    hal_gpio_escribir(s_monitor_list[id - 1], hw_level_from_state(0));
    return 1;
#else
    (void)id;
    return 0;
#endif
}
