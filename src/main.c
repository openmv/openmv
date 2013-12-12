#include <stdlib.h>
#include <string.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_syscfg.h>
#include <stm32f4xx_misc.h>
#include "sensor.h"
#include "rgb_led.h"
#include "usart.h"
#include "imlib.h"
#include "array.h"
#include "systick.h"
#include "usb_generic.h"

#define BREAK() __asm__ volatile ("BKPT");

enum sensor_result run_command(struct sensor_dev *sensor, uint8_t *args)
{
    switch (args[0]) {
        case CMD_RESET_SENSOR:
            sensor_reset(sensor);
            break;
 
        case CMD_READ_REGISTER:
            //sensor_read_reg(sensor, args[1]);
            break;

        case CMD_WRITE_REGISTER:
            sensor_write_reg(sensor, args[1], args[2]);
            break;
   
        case CMD_SET_BRIGHTNESS:
            sensor_set_brightness(sensor, args[1]);
            break;

        case CMD_SET_PIXFORMAT:
            /* Configure image size and format and FPS */
            if (sensor_set_pixformat(sensor, args[1]) != 0) {
                goto error;
            }
            break;

        case CMD_SET_FRAMESIZE:
            /* Configure image size and format and FPS */
            if (sensor_set_framesize(sensor, args[1]) != 0) {
                goto error;
            }
            break;

        case CMD_SET_FRAMERATE:           
            /* Configure framerate */
            if (sensor_set_framerate(sensor, args[1]) != 0) {
                goto error;
            }
            break;

        case CMD_SNAPSHOT: {
            if (sensor_snapshot(sensor) != 0) {
                goto error;
            }
            break;
        }

        case CMD_COLOR_TRACK: {
            struct point point= {0};
            struct frame_buffer *fb = &sensor->frame_buffer;
            #if 0
            struct color hsv;
            hsv.h = usart_recv();
            hsv.s = usart_recv();
            hsv.v = usart_recv();
            #else
            /* red */
            struct color hsv= {.h = 340, .s = 50, .v = 50};
            #endif

            if (sensor_snapshot(sensor) != 0) {
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
            struct frame_buffer *fb = &sensor->frame_buffer;
            uint8_t *background = malloc(fb->width * fb->height * 1);//grayscale

            if (background == NULL) {
                goto error;
            }

            if (sensor->pixformat != PIXFORMAT_YUV422) {
                /* Switch sensor to YUV422 to get 
                   a grayscale image from the Y channel */
//                    if (sensor_config(&sensor, sensor_QQVGA_YUV422, sensor_30FPS) != 0) {
//                       goto error;
//                  }
            }

            if (sensor_snapshot(sensor) != 0) {
                goto error;
            }

            /* Save this frame as background */
            for (i=0; i<(fb->width*fb->height); i++) {
                background[i] = fb->pixels[i*2];
            }

            while (1) {
                systick_sleep(1000);

                if (sensor_snapshot(sensor) != 0) {
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

            if (sensor->framesize > FRAMESIZE_QQVGA) {
                goto error;
            }

 
            if (sensor_snapshot(sensor) != 0) {
                goto error;
            }

            objects = imlib_detect_objects(&cascade, &sensor->frame_buffer);

            int x_pos=0,y_pos=0;
            int objs = array_length(objects);
            if (objs) {
                int i;
                for (i=0; i<objs; i++) {
                    imlib_draw_rectangle(&sensor->frame_buffer, array_at(objects, i));
                }
                struct rectangle *r = array_at(objects, 0);
                x_pos = r->x+r->w/2;
                y_pos = r->y+r->h/2;
                /* Send point coords from 0%..100% */
                usart_send(x_pos*100/sensor->frame_buffer.width);
                usart_send(y_pos*100/sensor->frame_buffer.height);
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
    struct sensor_dev *sensor = user_data;
    struct frame_buffer *fb = &sensor->frame_buffer;
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
    struct sensor_dev *sensor = user_data;   
    struct frame_buffer *fb = &sensor->frame_buffer;

    enum sensor_result ret;
    uint8_t *cmd_buf = ((uint8_t*)buffer);
    ret = run_command(sensor, cmd_buf);

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
    /* sensor handle */
    struct sensor_dev sensor;

    /* USB callback */
    struct usb_user_cb usb_cb = {
        &sensor,
        usb_data_in,
        usb_data_out,
    };

    /* Init SysTick timer */
    systick_init();

    /* init USART */
    usart_init(9600);

    /* init RGB LED module */
    rgb_led_init(LED_BLUE);

    /* init sensor module */
    if (sensor_init(&sensor) != 0) {
        goto error;
    }
  
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
            run_command(&sensor, args);
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
