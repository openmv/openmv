#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_syscfg.h>
#include <stm32f4xx_misc.h>
#include "ov9650.h"
#include "rgb_led.h"
#include "usart.h"
#include "imlib.h"
#define SNAPSHOT    0x01
#define COLORTRACK  0x02
#define BREAK() __asm__ volatile ("BKPT");

int main(void)
{
    /* init USART */
    usart_init(9600);

    /* init RGB LED module */
    rgb_led_init();

    /* init OV9650 module */
    ov9650_init();

    /* check MID,PID and VER */
    struct ov9650_id_t id={0};
    ov9650_read_id(&id);

    if (id.MIDH != 0x7F ||
        id.MIDL!= 0xA2 || 
        id.VER != 0x52 || id.PID != 0x96) {
        goto error;
    }

    /* Configure image size and format */
    if (ov9650_config(QQVGA_RGB565) != 0) {
        goto error;
    }
 
    /* Set sensor brightness level -3..+3 */
    ov9650_set_brightness(3);

    while (1) {
        switch (usart_recv()) {
            case SNAPSHOT: {
                if (ov9650_snapshot() != 0) {
                   goto error;
                }
                /* send image data w*h*2 */
                int i;
                uint8_t *frame_buffer = get_frame_buffer();
                for (i=0; i<(160*120*2); i++) {
                    usart_send(frame_buffer[i]);
                }
                break;
            }
            case COLORTRACK: {
                struct point point= {0};
                uint8_t *frame_buffer = get_frame_buffer();
                struct image image= {160, 120, 2, frame_buffer};

                /* color struct */
                struct color color= {
                    .h = 0,
                    .s = 70,
                    .v = 25
                };

                if (ov9650_snapshot() != 0) {
                   goto error;
                }
                imlib_color_track(&color, &image, &point, 10);
                /* send coords 0..100 */
                usart_send(point.x*100/160);
                usart_send(point.y*100/120);
                break;
            }
        }
    }

error:
    rgb_led_set_color(LED_RED);
    while (1) {
        /* Do nothing */
    }
}
