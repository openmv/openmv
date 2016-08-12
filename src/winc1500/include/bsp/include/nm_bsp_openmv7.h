#ifndef _NM_BSP_OPENMV7_H_
#define _NM_BSP_OPENMV7_H_

#include "conf_winc.h"

#define NM_EDGE_INTERRUPT   (1)

#define NM_DEBUG            CONF_WINC_DEBUG
#define NM_BSP_PRINTF       CONF_WINC_PRINTF

#define WINC_PIN_CS         (pin_I0)
#define WINC_PIN_EN         (pin_D5)
#define WINC_PIN_RST        (pin_G9)
#define WINC_PIN_IRQ        (pin_H15)

#endif /* _NM_BSP_OPENMV7_H_ */
