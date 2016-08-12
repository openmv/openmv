#ifndef _NM_BSP_OPENMV2_H_
#define _NM_BSP_OPENMV2_H_

#include "conf_winc.h"

#define NM_EDGE_INTERRUPT   (1)

#define NM_DEBUG            CONF_WINC_DEBUG
#define NM_BSP_PRINTF       CONF_WINC_PRINTF

#define WINC_PIN_CS         (pin_B12)
#define WINC_PIN_EN         (pin_A5)
#define WINC_PIN_RST        (pin_D12)
#define WINC_PIN_IRQ        (pin_D13)

#endif /* _NM_BSP_OPENMV2_H_ */
