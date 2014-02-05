#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libusb.h>
#include <getopt.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <stdarg.h>
#include "sensor.h"
#include "ov9650_regs.h"

#define USB_VID        (0x0483) /* vendor id    */
#define USB_PID        (0x5740) /* product id   */
#define EP_IN          (0x83)   /* IN endpoint  */
#define EP_OUT         (0x03)   /* OUT endpoint */
#define USB_IFACE      (0x02)   /*  */
#define TIMEOUT        (2000)      /* I/O transfer timeout */
#define DEBUG_LEVEL    (1)      /* libusb debug level   */
#define LOCK_SURFACE(surface) \
        if (SDL_LockSurface(surface) < 0 ) {    \
            fprintf(stderr, "failed to lock surface - %s\n", SDL_GetError());\
            return -1;\
        }\

const char *optstring = "h";
const char *usage =  "OpenMV frambuffer viewer\n"\
                     "usage: openmv <args>\n"\
                     "-h  print this message and exit\n";

libusb_device_handle *dev = NULL;
SDL_Surface *surface, *screen;    
int stop = 0;

static void sig_hdlr(int signum, siginfo_t *siginfo, void *user)
{
    switch (signum) {
        case SIGINT:
            stop =1;
    }
}

const char *err_str(int err)
{
    switch (err) {
        case 0: 
            return "Success";
        case LIBUSB_ERROR_TIMEOUT: 
            return "transfer timed out";
        case LIBUSB_ERROR_PIPE:
            return "endpoint halted";
        case LIBUSB_ERROR_OVERFLOW:
            return "overflow";
        case LIBUSB_ERROR_NO_DEVICE:
            return "device has been disconnected";
        case LIBUSB_ERROR_NOT_FOUND:
            return "the requested interface does not exist";
        default:
            return "other"; 
    };
}

void cleanup(void) 
{
    libusb_close(dev);
    libusb_exit(NULL);
    SDL_FreeSurface(screen);
    SDL_FreeSurface(surface);
    SDL_Quit();
}

int bulk_xfr(int ep, uint8_t *buf, int len)
{
    int ret;
    if ((ret = libusb_bulk_transfer(dev, ep, buf, len, &len, TIMEOUT)) != 0) {
        fprintf(stderr, "I/O error: %s (%d) offset: %d\n", err_str(ret), ret, len);
        return ret;
    }
    return len;
}

int int_xfr(int ep, uint8_t *buf, int len)
{
    int ret;
    if ((ret = libusb_interrupt_transfer(dev, ep, buf, len, &len, TIMEOUT)) != 0) {
        fprintf(stderr, "I/O error: %s offset: %d\n", err_str(ret), len);
        return ret;
    }
    return len;
}

long get_time_ms()
{
    static struct timeval timeval;
    gettimeofday(&timeval, NULL);
    return timeval.tv_sec * 1000 + (long) (timeval.tv_usec * 0.001f);
}

int write_image(char *path, struct frame_buffer *image)
{
    int i;
	FILE *fp = fopen(path, "w");
	if (fp == NULL) {
		printf("Unable to open file %s\n", path);
		return -1;
	}
    /* write header */
    fprintf(fp, "P5\n%d %d\n%d\n", image->width, image->height, 255);
	
    /* write pixels */
	for (i = 0; i < (image->width * image->height); i++) {
		fprintf(fp, "%c", image->pixels[i]);
	}
	fclose(fp);
	return 0;
}

int main (int argc, char **argv) 
{
    char opt;

    /*sigaction struct*/
    struct sigaction act = {
      .sa_sigaction = sig_hdlr,
      .sa_flags     = SA_SIGINFO|SA_NOCLDSTOP,
    };

    struct sensor_dev sensor = {
        .pixformat = PIXFORMAT_RGB565,
        .framesize = FRAMESIZE_QQVGA,
        .framerate = FRAMERATE_30FPS,
        .frame_buffer = {160, 120, 2, {0}},
    };
    
    struct frame_buffer *fb = &sensor.frame_buffer;

    /*parse command line args*/
    while ((opt = getopt(argc, argv, optstring)) > 0) {
        switch (opt) {
            case '?':
            case 'h':
                fprintf(stderr, "%s\n", usage);
                exit(1);
        }
    }

    /*init libusb*/
    libusb_init(NULL);
    
    /*set cleanup to be called at exit*/
    atexit(cleanup);     

    /*install signal handlers*/
    sigaction(SIGINT, &act, NULL);

    /*set debugging level*/
    libusb_set_debug(NULL, DEBUG_LEVEL); 

    if ((dev = libusb_open_device_with_vid_pid(NULL, USB_VID, USB_PID)) == NULL){
        fprintf(stderr, "Device could not be found.\n");
        exit(1);
    }

    /* reset device */
    //libusb_reset_device(dev);

    /* detach kernel driver */
    if (libusb_detach_kernel_driver(dev, USB_IFACE) != 0) {
        fprintf(stderr, "Failed to detach kernel driver\n");
//        exit(1);
    }

    /* claim the framebuffer interace */
    if (libusb_claim_interface(dev, USB_IFACE) != 0) {
        fprintf(stderr, "Failed to claim interface\n");
        exit(1);
    }

    /* set alternate framebuffer interface */
    if (libusb_set_interface_alt_setting(dev, USB_IFACE, 1) != 0) {
        fprintf(stderr, "Failed to set alternate interface \n");
        exit(1);
    }

    Uint32 rmask, gmask, bmask, amask;
    #if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        rmask =  0xFF000000; gmask  = 0x00FF0000; bmask = 0x0000FF00; amask = 0x00000000;    
    #else
        rmask =  0x000000FF; gmask  =  0x0000FF00; bmask = 0x00FF0000; amask = 0x00000000;
    #endif

    /* Init SDL library */
    if (SDL_Init(SDL_INIT_VIDEO)){
        fprintf(stderr, "failed to initialize SDL_VIDEO - %s\n", SDL_GetError());
        exit(1);
    }
 
    /* Init SDL TTF library */
    if(TTF_Init()==-1) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        exit(1);
    }
  
    if ((screen = SDL_SetVideoMode(fb->width, fb->height, 32, SDL_SWSURFACE)) == NULL) {
        fprintf(stderr, "failed to set video mode - %s\n", SDL_GetError());
        exit(1);
    }

    if ((surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
                      fb->width, fb->height, 32, rmask, gmask, bmask, amask)) == NULL) { 
        fprintf(stderr, "failed to create rgb surface - %s\n", SDL_GetError());
        exit(1);
    }
 	
    // load font.ttf at size 16 into font
    TTF_Font *font;
    font=TTF_OpenFont("font.ttf", 12);
    if(!font) {
        fprintf(stderr, "TTF_OpenFontRW: %s\n", TTF_GetError());
        exit(1);
    }    

    SDL_Event event;
    long t_start, t_elapsed=0, t_total=0, frames=0;

    /* allocate frame buffer */
    fb->pixels = malloc(fb->width*fb->height*fb->bpp);

    char text_buf[64];

    while(1) {
        t_start = get_time_ms();

        //cmd_buf[0] = sensor_cmd;
        //int_xfr(EP_OUT, cmd_buf, 1);

        int ret, len = 0;
        int frame_size = (fb->width*fb->height*fb->bpp);

        /* request frame */
        ret = libusb_control_transfer(dev, 0x41, CMD_SNAPSHOT, 0, 2, NULL, 0, TIMEOUT);
        if (ret !=0) {
            fprintf(stderr, "I/O error: %s (%d) offset: %d\n", err_str(ret), ret, len);
            exit(0);
        }

        while (len < frame_size) {
            ret = bulk_xfr(EP_IN, fb->pixels + len, frame_size-len);
            if (stop || ret == LIBUSB_ERROR_PIPE) {
                exit(0);
            }
            len += ret;
        }

        int i;
        uint8_t *pixels = surface->pixels;
        for (i=0; i<frame_size; i+=fb->bpp, pixels+=4) {
            if (fb->bpp == 1) {
                /* map grayscale to RGB888 */
                pixels[0]=fb->pixels[i];
                pixels[1]=fb->pixels[i];
                pixels[2]=fb->pixels[i];
            } else {
                uint8_t p0 = fb->pixels[i];
                uint8_t p1 = fb->pixels[i+1];
                /* map RGB565 to RGB888 */
                #if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                pixels[0] = (uint8_t) (p1&0x1F) * 255/31;
                pixels[1] = (uint8_t) (((p0&0x07)<<3) | (p1>>5)) * 255/63;
                pixels[2] = (uint8_t) (p0>>3) * 255/31;
                #else
                pixels[0] = (uint8_t) (p0>>3) * 255/31;
                pixels[1] = (uint8_t) (((p0&0x07)<<3) | (p1>>5)) * 255/63;
                pixels[2] = (uint8_t) (p1&0x1F) * 255/31;
                #endif
            }
            pixels[3]=0;
        }

        ++frames;
        t_elapsed = get_time_ms() - t_start;
        t_total += t_elapsed;        

        /* print FPS */
        sprintf(text_buf, "FPS %.2f", 1000/(float)(t_total / frames));
   
        SDL_PollEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                exit(0);     
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_s:
                        write_image("snapshot.pgm", fb);
                        printf("snapshot saved...\n");
                        break;
                    case SDLK_ESCAPE:
                        exit(0);     
                    default:
                        break;
                }
                break;
        }

        SDL_Color fg = {255, 0, 0, 0}; 
        SDL_Surface *text = TTF_RenderText_Solid(font, text_buf, fg);
        SDL_Rect rect = {0, 0, 0, 0};
        SDL_BlitSurface(text, NULL, surface, &rect);
        SDL_BlitSurface(surface, NULL, screen, NULL);   
        SDL_FreeSurface(text);

        /*flush*/
        SDL_UpdateRect(screen, 0, 0,  fb->width, fb->height);
    }
    return 0;
}
