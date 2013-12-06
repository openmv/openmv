#include <stdlib.h>
#include <string.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_syscfg.h>
#include <stm32f4xx_misc.h>
#include "ov9650.h"
#include "rgb_led.h"
#include "usart.h"
#include "imlib.h"
#include "array.h"
#include "systick.h"
#include "usb_generic.h"

#define BREAK() __asm__ volatile ("BKPT");

enum cmd_result run_command(struct ov9650_handle *ov9650, uint8_t *args)
{
    switch (args[0]) {
        case CMD_WRITE_REG:
            SCCB_Write(args[1], args[2]);
            break;

        case CMD_RESET_REG:
            ov9650_reset(ov9650);
            break;
    
        case CMD_SET_BRIGHTNESS:
            ov9650_set_brightness(ov9650, args[1]);
            break;

        case CMD_SET_PIXFORMAT:
            /* Configure image size and format and FPS */
            if (ov9650_set_pixformat(ov9650, args[1]) != 0) {
                goto error;
            }
            break;

        case CMD_SET_FRAMESIZE:
            /* Configure image size and format and FPS */
            if (ov9650_set_framesize(ov9650, args[1]) != 0) {
                goto error;
            }
            break;

        case CMD_SET_FRAMERATE:           
            /* Configure framerate */
            if (ov9650_set_framerate(ov9650, args[1]) != 0) {
                goto error;
            }
            break;

        case CMD_SNAPSHOT: {
            if (ov9650_snapshot(ov9650) != 0) {
                goto error;
            }
            break;
        }

        case CMD_COLOR_TRACK: {
            struct point point= {0};
            struct frame_buffer *fb = &ov9650->frame_buffer;
            #if 0
            struct color hsv;
            hsv.h = usart_recv();
            hsv.s = usart_recv();
            hsv.v = usart_recv();
            #else
            /* red */
            struct color hsv= {.h = 340, .s = 50, .v = 50};
            #endif

            if (ov9650_snapshot(ov9650) != 0) {
                goto error;
            }

            imlib_color_track(fb, &hsv, &point, 10);
            if (point.x && point.y) {
                struct rectangle r = {.x=point.x-5, .y=point.y-5, .w=10, .h=10};
                imlib_draw_rectangle(fb, &r);

                /* Send point coords from 0%..100% */
                usart_send(point.x*100/fb->width);
                usart_send(point.y*100/fb->height);
            }

            break;
        }

        case CMD_MOTION_DETECTION: {
            int i;
            int pixels;
            struct frame_buffer *fb = &ov9650->frame_buffer;
            uint8_t *background = malloc(fb->width * fb->height * 1);//grayscale

            if (background == NULL) {
                goto error;
            }

            if (ov9650->pixformat != PIXFORMAT_YUV422) {
                /* Switch sensor to YUV422 to get 
                   a grayscale image from the Y channel */
//                    if (ov9650_config(&ov9650, OV9650_QQVGA_YUV422, OV9650_30FPS) != 0) {
//                       goto error;
//                  }
            }

            if (ov9650_snapshot(ov9650) != 0) {
                goto error;
            }

            /* Save this frame as background */
            for (i=0; i<(fb->width*fb->height); i++) {
                background[i] = fb->pixels[i*2];
            }

            while (1) {
                systick_sleep(1000);

                if (ov9650_snapshot(ov9650) != 0) {
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
                    imlib_erosion_filter(fb, kernel, 3);

                    for (i=0; i<(fb->width*fb->height); i++) {
                        /* send twice because lcd expects RGB565 */
                        usart_send(fb->pixels[i]);
                        usart_send(fb->pixels[i]);
                    }
                    break;
                }
            }
            break;
        }             

        case CMD_FACE_DETECTION: {
            /* detection objects array */
            struct array *objects;

            /* detection parameters */
            struct cascade cascade = {
                .step = 2,
                .n_stages = 12,
                .window = {24, 24},
                .scale_factor = 1.25f,
            };

            if (ov9650->framesize > FRAMESIZE_QQVGA) {
                goto error;
            }

 
            if (ov9650_snapshot(ov9650) != 0) {
                goto error;
            }

            objects = imlib_detect_objects(&cascade, &ov9650->frame_buffer);

            int x_pos=0,y_pos=0;
            int objs = array_length(objects);
            if (objs) {
                int i;
                for (i=0; i<objs; i++) {
                    imlib_draw_rectangle(&ov9650->frame_buffer, array_at(objects, i));
                }
                struct rectangle *r = array_at(objects, 0);
                x_pos = r->x+r->w/2;
                y_pos = r->y+r->h/2;
                /* Send point coords from 0%..100% */
                usart_send(x_pos*100/ov9650->frame_buffer.width);
                usart_send(y_pos*100/ov9650->frame_buffer.height);
            }
            array_free(objects);
            break;
        }
    }

    return CMD_ACK;
error:
    return CMD_NACK;
}

static int frame_tx_bytes;

void usb_data_in(void *buffer, int *length, void *user_data)
{
    int usb_tx_length=64;
    struct ov9650_handle *ov9650 = user_data;
    struct frame_buffer *fb = &ov9650->frame_buffer;
    int size = (fb->width*fb->height*fb->bpp);
    
    if (frame_tx_bytes < size) {
        memcpy(buffer, fb->pixels+frame_tx_bytes, *length);
        *length = usb_tx_length;
        frame_tx_bytes += usb_tx_length;
    } else {
        *length = 0;
    }
}

void usb_data_out(void *buffer, int *length, void *user_data)
{
    int usb_tx_length=64;
    struct ov9650_handle *ov9650 = user_data;   
    struct frame_buffer *fb = &ov9650->frame_buffer;

    enum cmd_result ret;
    uint8_t *cmd_buf = ((uint8_t*)buffer);
    ret = run_command(ov9650, cmd_buf);

    switch (cmd_buf[0]) {
        case CMD_SNAPSHOT:
        case CMD_COLOR_TRACK:
        case CMD_MOTION_DETECTION:
        case CMD_FACE_DETECTION:
            /* send back frame */
            memcpy(buffer, fb->pixels, usb_tx_length);    
            *length = usb_tx_length;
            /* reset bytes counter */
            frame_tx_bytes = usb_tx_length;
            break;
        default:
            /* send back ACK/NACK */
            //*length = 1;
            *length =0; //ignore it for now
            cmd_buf[0] = ret;
    }
}

/* This function loads the .ccm data into the CMM region 
   It's here to avoid modifiying the startup code */
void load_ccm_section () __attribute__ ((section (".init")));
void load_ccm_section (){
    extern char _eidata, _sccm, _eccm;

    char *src = &_eidata;
    char *dst = &_sccm;
    while (dst < &_eccm) {
        *dst++ = *src++;
    }
}

int main(void)
{
    /* OV9650 handle */
    struct ov9650_handle ov9650;

    /* USB callback */
    struct usb_user_cb usb_cb = {
        &ov9650,
        usb_data_in,
        usb_data_out,
    };

    /* Init SysTick timer */
    systick_init();

    /* init USART */
    usart_init(9600);

    /* init RGB LED module */
    rgb_led_init(LED_BLUE);

    /* init OV9650 module */
    ov9650_init(&ov9650);

    /* check MID,PID and VER */
    if (ov9650.id.MIDH != 0x7F ||
        ov9650.id.MIDL!= 0xA2 || 
        ov9650.id.VER != 0x52 ||
        ov9650.id.PID != 0x96) {
        goto error;
    }

    if (ov9650_set_pixformat(&ov9650, PIXFORMAT_RGB565) != 0) {
        goto error;
    }

    /* Configure image size and format and FPS */
    if (ov9650_set_framesize(&ov9650, FRAMESIZE_QQVGA) != 0) {
        goto error;
    }

    /* Configure framerate */
    if (ov9650_set_framerate(&ov9650, FRAMERATE_60FPS) != 0) {
        goto error;
    }
   
    /* Set sensor brightness level -3..+3 */
    ov9650_set_brightness(&ov9650, 3);
  
    /* init usb device */
    usb_dev_init(&usb_cb);

//    systick_sleep(3000);
    rgb_led_set_color(LED_GREEN);

#if 0
    /* FPS test */
    while (1) {
        volatile int fps = 0;
        uint8_t args[]= {CMD_FACE_DETECTION};
        uint32_t ticks = systick_current_millis();
        while ((systick_current_millis()-ticks)<1000) {
            run_command(&ov9650, args);
            fps++;
        }
        BREAK();
    }
#endif    

    while (1) {
    }

error:
    rgb_led_set_color(LED_RED);
    while (1) {
        /* Do nothing */
    }
}
