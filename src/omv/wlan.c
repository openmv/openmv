#include "std.h"
#include "led.h"
#include "systick.h"
#include "libcc3k.h"
#include "imlib.h"
#include "sensor.h"

#define NULL (0)
#define bzero(p, l) memset(p, 0, l);
#define MAX_PACKET_LENGTH (1024)
int __errno;
static int connected = 0;
static int ipAddressObtained = 0;
unsigned char patchVer[2];

void sWlanCallback(long lEventType, char * data, unsigned char length)
{
    switch (lEventType) {
        case HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE:
            break;
        case HCI_EVNT_WLAN_UNSOL_CONNECT:
            connected = 1;
            break;
        case HCI_EVNT_WLAN_UNSOL_DISCONNECT:
            /* Link down */
            connected = 0;
            break;
        case HCI_EVENT_CC3000_CAN_SHUT_DOWN:
            break;
        case HCI_EVNT_WLAN_UNSOL_DHCP:
            ipAddressObtained = 1;
            break;
    }
}

void get_patch_ver()
{
    nvmem_read_sp_version(patchVer);
}

void config_static_ip()
{
    unsigned char pucIP_Addr[4];
    unsigned char pucIP_DefaultGWAddr[4];
    unsigned char pucSubnetMask[4];
    unsigned char pucDNS[4];

    /* 255.255.255.0 */
    pucSubnetMask[0] = 0xFF;
    pucSubnetMask[1] = 0xFF;
    pucSubnetMask[2] = 0xFF;
    pucSubnetMask[3] = 0x0;

    /* 192.168.1.102 */
    pucIP_Addr[0] = 0xC0;
    pucIP_Addr[1] = 0xA8;
    pucIP_Addr[2] = 0x01;
    pucIP_Addr[3] = 0x67;

    /* 192.168.1.254 */
    pucIP_DefaultGWAddr[0] = 0xC0;
    pucIP_DefaultGWAddr[1] = 0xA8;
    pucIP_DefaultGWAddr[2] = 0x01;
    pucIP_DefaultGWAddr[3] = 0xFE;


    // Currently no implementation of DHCP in hte demo
    pucDNS[0] = 0;
    pucDNS[1] = 0;
    pucDNS[2] = 0;
    pucDNS[3] = 0;

    netapp_dhcp((unsigned long *)pucIP_Addr, (unsigned long *)pucSubnetMask, (unsigned long *)pucIP_DefaultGWAddr, (unsigned long *)pucDNS);
}
#define BREAK() __asm__ volatile ("BKPT")

void tcp_echo_server()
{
    int fd, cli_fd, n;
    char buf[255];
    sockaddr_in addr;

    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
        goto error;
    }

    /* sockaddr param */
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(5555);
    addr.sin_addr.s_addr = 0;//htonl(INADDR_ANY);
    bzero(&addr.sin_zero, sizeof(addr.sin_zero));

    /* bind socket */
    if (bind(fd, (sockaddr*) &addr, sizeof(sockaddr_in)) < 0) {
        BREAK();
        goto error;
    }

    /* wait for client connection */
    if (listen(fd, 5) < 0) {
        BREAK();
        goto error;
    }

    socklen_t len = sizeof(sockaddr);
    cli_fd = accept(fd, (sockaddr*) &addr, &len);
    if (cli_fd < 0) {
        BREAK();
        goto error;
    }

    /* recv/send loop */
    while (1) {
        n = recv(fd, buf, sizeof(buf)-1, 0);
        if (n < 0) {
            continue;
            BREAK();
            goto error;
        }

        n = send(fd, buf, n, 0);
        if (n < 0) {
            continue;
            BREAK();
            goto error;
        }
    }

error:
    led_state(LED_RED, 1);
}

void udp_echo_server()
{
    int fd, n;
    char buf[255];
    sockaddr_in addr;
    socklen_t len;

    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
        goto error;
    }

    /* sockaddr param */
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(5555);
    addr.sin_addr.s_addr = 0;//htonl(INADDR_ANY);
    bzero(&addr.sin_zero, sizeof(addr.sin_zero));

    /* bind socket */
    if (bind(fd, (sockaddr*) &addr, sizeof(sockaddr_in)) < 0) {
        BREAK();
        goto error;
    }

    /* recv/send loop */
    while (1) {
        len = sizeof(sockaddr_in);
        bzero(&addr, len);

        n = recvfrom(fd, buf, sizeof(buf)-1, 0, (sockaddr*) &addr, &len);
        if (n < 0 || n > 255) {
            goto error;
        }

        n = sendto(fd, buf, n, 0, (sockaddr*) &addr, len);
        if (n < 0) {
            goto error;
        }
    }
error:
    led_state(LED_RED, 1);
}

void udp_send_recv()
{
    int fd, n;
    char buf[255];
    sockaddr_in addr;
    socklen_t len;

    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
        goto error;
    }

    /* sockaddr param */
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(5555);
    addr.sin_addr.s_addr = 0;//htonl(INADDR_ANY);
    bzero(&addr.sin_zero, sizeof(addr.sin_zero));

    /* bind socket */
    if (bind(fd, (sockaddr*) &addr, sizeof(sockaddr_in)) < 0) {
        BREAK();
        goto error;
    }

    image_t _image;
    image_t *image = &_image;

    while (1) {
        /* grab one snapshot */
        sensor_snapshot(image);

        len = sizeof(sockaddr_in);
        n = recvfrom(fd, buf, sizeof(buf)-1, 0, (sockaddr*) &addr, &len);
        if (n < 0 || n > 255) {
            BREAK();
            goto error;
        }

        int image_size = (image->w * image->h * image->bpp);
        int c = image_size / MAX_PACKET_LENGTH;
        int r = image_size % MAX_PACKET_LENGTH;

        /* send frame to client */
        for (int i=0; i<c; i++) {
            n = sendto(fd, image->data+(i*MAX_PACKET_LENGTH), MAX_PACKET_LENGTH, 0, (sockaddr*) &addr, sizeof(sockaddr_in));
            if (n < 0) {
                goto error;
            }
            //systick_sleep(1);
        }

        /* send remaining bytes */
        n = sendto(fd, image->data+(c*MAX_PACKET_LENGTH), r, 0, (sockaddr*) &addr, sizeof(sockaddr_in));
        if (n < 0) {
            goto error;
        }
    }

error:
    led_state(LED_RED, 1);
}

void wlan_test()
{
    tNetappIpconfigRetArgs ipconfig;

    led_state(LED_BLUE, 0);
    led_state(LED_GREEN, 1);

    /* Initialize WLAN module */
    wlan_init(sWlanCallback, NULL, NULL, NULL,
            ReadWlanInterruptPin, SpiResumeSpi, SpiPauseSpi, WriteWlanPin);

    /* start WLAN module */
    wlan_start(0);

    /* set connection policy */
    wlan_ioctl_set_connection_policy(0, 0, 0);

    /* Mask out all non-required events from the CC3000 */
    wlan_set_event_mask(HCI_EVNT_WLAN_KEEPALIVE|HCI_EVNT_WLAN_UNSOL_INIT|HCI_EVNT_WLAN_ASYNC_PING_REPORT);

    char *ssid = "mux";
    unsigned char *key = (unsigned char*) "@N2KQZIB*N=@#3@$";
//    unsigned char *bssid = (unsigned char *) "\x54\xE6\xFC\xC4\xC2\x62";

    led_state(LED_GREEN, 0);

    /* connect */
    wlan_connect(WLAN_SEC_WPA2, ssid, 3, 0, key, 16);

    while (!connected || !ipAddressObtained) {
        /* Do nothing */
        led_state(LED_BLUE, 1);
        systick_sleep(500);
        led_state(LED_BLUE, 0);
        systick_sleep(500);
    }

    led_state(LED_BLUE, 1);

    netapp_ipconfig(&ipconfig);
    //config_static_ip();

    /* disable socket timeout */
    netapp_timeout_values(0, 0, 0, 0);

    //udp_echo_server();
    udp_send_recv();
}
