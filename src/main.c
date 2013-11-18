#include <stdlib.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_syscfg.h>
#include <stm32f4xx_misc.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_syscfg.h>
#include <stm32f4xx_misc.h>
#include "ov9650.h"
#include "rgb_led.h"
#include "usart.h"
#include "imlib.h"

#define BREAK() __asm__ volatile ("BKPT");

int main(void)
{
    struct ov9650_handle ov9650;

    /* init USART */
    usart_init(9600);

    /* init RGB LED module */
    rgb_led_init();

    /* init OV9650 module */
    ov9650_init(&ov9650);

    /* check MID,PID and VER */
    if (ov9650.id.MIDH != 0x7F ||
        ov9650.id.MIDL!= 0xA2 || 
        ov9650.id.VER != 0x52 ||
        ov9650.id.PID != 0x96) {
        goto error;
    }

    /* Configure image size and format */
    if (ov9650_config(&ov9650, QQVGA_RGB565) != 0) {
        goto error;
    }
 
    /* Set sensor brightness level -3..+3 */
    ov9650_set_brightness(&ov9650, 2);

    while (1) {
        switch (usart_recv()) {
            case SNAPSHOT: {
                int i;
                struct frame_buffer *fb = &ov9650.frame_buffer;
                if (ov9650_snapshot(&ov9650) != 0) {
                    goto error;
                }

                /* send image data w*h*2 */
                for (i=0; i<(fb->width * fb->height * fb->bpp); i++) {
                    usart_send(fb->pixels[i]);
                }               

                break;
            }
            case COLOR_TRACK: {
                struct point point= {0};
                struct frame_buffer *fb = &ov9650.frame_buffer;

                /* Color struct */
                struct color color= {
                    .h = 0,
                    .s = 70,
                    .v = 25
                };
    
                if (ov9650_snapshot(&ov9650) != 0) {
                    goto error;
                }

                imlib_color_track(&ov9650.frame_buffer, &color, &point, 10);

                /* Send point coords from 0%..100% */
                usart_send(point.x*100/fb->width);
                usart_send(point.y*100/fb->height);
                break;
            }
            case MOTION_DETECTION: {
                int i;
                int pixels;
                struct frame_buffer *fb = &ov9650.frame_buffer;
                uint8_t *background = malloc(fb->width * fb->height * 1);//grayscale

                if (background == NULL) {
                    goto error;
                }

                if (ov9650.config != QQVGA_YUV422) {
                    /* Switch sensor to YUV422 to get 
                       a grayscale image from the Y channel */
                    if (ov9650_config(&ov9650, QQVGA_YUV422) != 0) {
                        goto error;
                    }
                }

                if (ov9650_snapshot(&ov9650) != 0) {
                    goto error;
                }

                /* Save this frame as background */
                for (i=0; i<(fb->width*fb->height); i++) {
                    background[i]  = fb->pixels[i*2];
                }

                while (1) {
                    delay(100000);
                    if (ov9650_snapshot(&ov9650) != 0) {
                        goto error;
                    }
                    
                    for (i=0, pixels=0; i<(fb->width*fb->height); i++) {
                        uint8_t y = fb->pixels[i*2];
                        int diff = (y-background[i]) * (y-background[i]);

                        /* consider pixel changed if change more than 25% */
                        if ((diff*100)/(255*255) > 25) {
                            pixels++;
                            /* reuse the frame buffer */
                            fb->pixels[i] = 0xff;
                        } else {
                            fb->pixels[i] = 0x00;
                        }
                    }
                        
                    /* send if more than 10% of the image changed  */
                    if ((pixels*100)/(fb->width*fb->height)>5) {
                        uint8_t kernel[] = {1,1,1,
                                            1,1,1, 
                                            1,1,1};

                        /* free background frame */
                        free(background);

                        /* perform image erosion */
//                        imlib_erosion_filter(fb, kernel, 3);

                        for (i=0; i<(fb->width*fb->height); i++) {
                            /* send twice because lcd expects RGB565 */
                            usart_send(fb->pixels[i]);
                            usart_send(fb->pixels[i]);
                        }

                        break;
                    }
                }
            }
        }
    }

error:
    rgb_led_set_color(LED_RED);
    while (1) {
        /* Do nothing */
    }
}
