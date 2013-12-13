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
#include "sensor.h"
#include "ov9650_regs.h"

#define USB_VID        (0x0483) /* vendor id    */
#define USB_PID        (0x5740) /* product id   */
#define EP_IN          (0x81)   /* IN endpoint  */
#define EP_OUT         (0x01)   /* OUT endpoint */
#define TIMEOUT        (1000)      /* I/O transfer timeout */
#define DEBUG_LEVEL    (1)      /* libusb debug level   */
#define LOCK_SURFACE(surface) \
        if (SDL_LockSurface(surface) < 0 ) {    \
            fprintf(stderr, "failed to lock surface - %s\n", SDL_GetError());\
            return -1;\
        }\

const char *optstring = "f:r:s:c:p:h";
const char *usage =  "usage: openmv <args>\n"                    \
                     "-f  set pixel format yuv422/rgb565/grayscale\n"\
                     "-r  set frame rate  2/8/15/30/60\n"\
                     "-s  set frame size  QQVGA\n"\
                     "-c  send command to camera SNAPSHOT/COLOR_TRACK\n"\
                     "-p  probe register, pass reg addr in hex and use -/+ to change value\n"\
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

void write_reg(reg, val)
{
    uint8_t cmd_buf[3];
    cmd_buf[0] = CMD_WRITE_REGISTER;
    cmd_buf[1] = reg;
    cmd_buf[2] = val;
    bulk_xfr(EP_OUT, cmd_buf, 3);
}

int main (int argc, char **argv) 
{
    char opt;
    uint8_t cmd_buf[32];

    /*sigaction struct*/
    struct sigaction act = {
      .sa_sigaction = sig_hdlr,
      .sa_flags     = SA_SIGINFO|SA_NOCLDSTOP,
    };

    enum sensor_command sensor_cmd = CMD_SNAPSHOT;

    struct sensor_dev sensor = {
        .pixformat = PIXFORMAT_RGB565,
        .framesize = FRAMESIZE_QQVGA,
        .framerate = FRAMERATE_30FPS,
        .frame_buffer = {160, 120, 2, {0}},
    };
    
    uint8_t enable_probe=0;
    uint8_t reg_addr=0;
    uint8_t reg_val=0; /* register value */   
    struct frame_buffer *fb = &sensor.frame_buffer;

    /*parse command line args*/
    while ((opt = getopt(argc, argv, optstring)) > 0) {
        switch (opt) {
            case 'c':
                if (strcmp(optarg, "SNAPSHOT")==0) {
                    sensor_cmd = CMD_SNAPSHOT;
                } else if (strcmp(optarg, "COLOR_TRACK")==0) {
                    sensor_cmd = CMD_COLOR_TRACK;
                } else if (strcmp(optarg, "FACE_DETECTION")==0) {
                    sensor_cmd = CMD_FACE_DETECTION;
                } else {
                    fprintf(stderr, "unsupported command <%s>\n%s\n", optarg, usage);
                    exit(1);
                }
                break;

            case 'f':
                if (strcmp(optarg, "yuv422")==0) {
                    fb->bpp =2;
                    sensor.pixformat = PIXFORMAT_YUV422;
                } else if (strcmp(optarg, "rgb565")==0) {
                    fb->bpp =2;
                    sensor.pixformat = PIXFORMAT_RGB565;
                } else if (strcmp(optarg, "grayscale")==0) {
                    fb->bpp =1;
                    sensor.pixformat = PIXFORMAT_GRAYSCALE;
                } else {
                    fprintf(stderr, "unsupported pixformat <%s>\n%s\n", optarg, usage);
                    exit(1);
                }
                break;

            case 's':
                if (strcmp(optarg, "QQCIF")==0) {
                    fb->width  = 88;
                    fb->height = 72;
                    sensor.framesize = FRAMESIZE_QQCIF;
                } else if (strcmp(optarg, "QQVGA")==0) {
                    fb->width  = 160;
                    fb->height = 120;
                    sensor.framesize = FRAMESIZE_QQVGA;
                } else if (strcmp(optarg, "QCIF")==0) {
                    fb->width  = 176;
                    fb->height = 144;
                    sensor.framesize = FRAMESIZE_QCIF;
                } else {
                    fprintf(stderr, "unsupported framesize <%s>\n%s\n", optarg, usage);
                    exit(1);
                }
                break;

            case 'r':
                switch (atoi(optarg)) {
                    case 2:
                        sensor.framerate=FRAMERATE_2FPS;
                        break;
                    case 8:
                        sensor.framerate=FRAMERATE_8FPS;
                        break;
                    case 15:
                        sensor.framerate=FRAMERATE_15FPS;
                        break;
                    case 30:
                        sensor.framerate=FRAMERATE_30FPS;
                        break;
                    case 60:
                        sensor.framerate=FRAMERATE_60FPS;
                        break;
                    default:
                        fprintf(stderr, "unsupported framerate <%s>\n%s\n", optarg, usage);
                        exit(1);
                }
                break;

            case 'p':
                enable_probe = 1;
                reg_addr = (uint8_t) strtol(optarg, NULL, 16);
                break;

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
    libusb_reset_device(dev);

    /* claim interace zero */
    if (libusb_claim_interface(dev, 0) != 0) {
        fprintf(stderr, "Failed to claim interface\n");
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

    /* reset all registers to their default values */
    cmd_buf[0] = CMD_RESET_SENSOR;
    bulk_xfr(EP_OUT, cmd_buf, 1);

    /* set pixelformat */
    cmd_buf[0] = CMD_SET_PIXFORMAT;
    cmd_buf[1] = sensor.pixformat;
    bulk_xfr(EP_OUT, cmd_buf, 2);

    /* set framesize */
    cmd_buf[0] = CMD_SET_FRAMESIZE;
    cmd_buf[1] = sensor.framesize;
    bulk_xfr(EP_OUT, cmd_buf, 2);

    /* set framerate */
    cmd_buf[0] = CMD_SET_FRAMERATE;
    cmd_buf[1] = sensor.framerate;
    bulk_xfr(EP_OUT, cmd_buf, 2);

    /* Controlled by AWB */
    /*Blue/Red Channels amplifiers*/
//    write_reg(REG_RED,  0x80);
//    write_reg(REG_BLUE, 0x80);

    /* set gain ceiling */
    write_reg(REG_COM9, 0x30); 

    /* set brightness */
    cmd_buf[0] = CMD_SET_BRIGHTNESS;
    cmd_buf[1] = 3;
    bulk_xfr(EP_OUT, cmd_buf, 2);

    sleep(1);

    SDL_Event event;
    long t_start, t_elapsed=0, t_total=0, frames=0;

    /* allocate frame buffer */
    fb->pixels = malloc(fb->width*fb->height*fb->bpp);

    char text_buf[64];

    while(1) {
        t_start = get_time_ms();

        /* request frame */
        cmd_buf[0] = sensor_cmd;
        bulk_xfr(EP_OUT, cmd_buf, 1);

        int ret, len = 0;
        int frame_size = (fb->width*fb->height*fb->bpp);
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

        if (enable_probe) {
            char buf[9]={0};
            for (i=0; i<8; i++) {
                buf[i]=(((reg_val<<i)&0x80)>>7)+48;
            }
            write_reg(reg_addr, reg_val);
            sprintf(text_buf, "REG 0x%.2X 0x%.2X %s", reg_addr, reg_val, buf);
        } else {
            sprintf(text_buf, "FPS %.2f", 1000/(float)(t_total / frames));
        }
   
        SDL_PollEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                exit(0);     
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_0:
                        reg_val=0;
                        break;
                    case SDLK_KP_PLUS:
                        reg_val++;
                        break;
                    case SDLK_KP_MINUS:
                        reg_val--;
                        break;
                    case SDLK_LEFT:
                        reg_val = reg_val<<1 | reg_val>>7;
                        break;
                    case SDLK_RIGHT:
                        reg_val = reg_val>>1 | reg_val<<7;
                        break;
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


