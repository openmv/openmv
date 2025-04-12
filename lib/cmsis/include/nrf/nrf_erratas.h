/*

Copyright (c) 2010 - 2018, Nordic Semiconductor ASA All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

3. Neither the name of Nordic Semiconductor ASA nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef NRF_ERRATAS_H__
#define NRF_ERRATAS_H__

#include "nrf.h"

/*lint ++flb "Enter library region */

#if defined(NRF51422_XXAA) ||\
    defined(NRF51422_XXAB) ||\
    defined(NRF51422_XXAC)
    #include "nrf51422_erratas.h"
#elif defined(NRF51801_XXAB)
    #include "nrf51801_erratas.h"
#elif defined(NRF51802_XXAA)
    #include "nrf51802_erratas.h"
#elif defined(NRF51822_XXAA) ||\
    defined(NRF51822_XXAB) ||\
    defined(NRF51822_XXAC)
    #include "nrf51822_erratas.h"
#elif defined(NRF51824_XXAA)
    #include "nrf51824_erratas.h"
#elif defined(NRF51)
    #include "nrf51_erratas.h"
#elif defined(NRF52805_XXAA)
    #include "nrf52805_erratas.h"
#elif defined(NRF52810_XXAA)
    #include "nrf52810_erratas.h"
#elif defined(NRF52811_XXAA)
    #include "nrf52811_erratas.h"
#elif defined(NRF52832_XXAA) ||\
    defined(NRF52832_XXAB)
    #include "nrf52832_erratas.h"
#elif defined(NRF52833_XXAA)
    #include "nrf52833_erratas.h"
#elif defined(NRF52840_XXAA)
    #include "nrf52840_erratas.h"
#elif defined(NRF5340_XXAA_APPLICATION)
    #include "nrf5340_application_erratas.h"
#elif defined(NRF5340_XXAA_NETWORK)
    #include "nrf5340_network_erratas.h"
#elif defined(NRF9160_XXAA)
    #include "nrf9160_erratas.h"
#else
    #error "Device must be defined. See nrf.h."
#endif /* NRF51422_XXAA,
          NRF51422_XXAB,
          NRF51422_XXAC,
          NRF51801_XXAB,
          NRF51802_XXAA,
          NRF51822_XXAA,
          NRF51822_XXAB,
          NRF51822_XXAC,
          NRF51824_XXAA,
          NRF51,
          NRF52805_XXAA,
          NRF52810_XXAA,
          NRF52811_XXAA,
          NRF52832_XXAA,
          NRF52832_XXAB,
          NRF52833_XXAA,
          NRF52840_XXAA,
          NRF5340_XXAA_APPLICATION,
          NRF5340_XXAA_NETWORK,
          NRF9160_XXAA */

/*lint --flb "Leave library region" */

#endif // NRF_ERRATAS_H__
