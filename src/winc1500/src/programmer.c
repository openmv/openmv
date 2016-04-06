#include <ff.h>
#include <stdio.h>
#include "fb_alloc.h"

#include "programmer/programmer.h"
#include "root_cert/root_setup.h"
#include "spi_flash/include/spi_flash_map.h"

#define FW_PATH             "/firmware/bin/m2m_aio_3a0.bin"
#define FW_DUMP_PATH        "/firmware/bin/fw_dump.bin"
#define CERT_DIGI_PATH      "/firmware/cert/DigiCert_Root.cer"
#define CERT_DIGISHA2_PATH  "/firmware/cert/DigiCertSHA2_Root.cer"
#define CERT_GEOTRUST_PATH  "/firmware/cert/GeoTrustGlobalCA_Root.cer"
#define CERT_RADIUS_PATH    "/firmware/cert/Radius_Root.cer"
#define CERT_VERISIGN_PATH  "/firmware/cert/VeriSign_Root.cer"

#define MIN(a,b) \
    ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

/**
 * Program firmware to WINC1500 memory.
 *
 * return M2M_SUCCESS on success, error code otherwise.
 */
int burn_firmware()
{
    FIL fp;
    uint32_t offset = 0;
    UINT bytes = 0, bytes_out=0;

    int ret = M2M_ERR_FAIL;
    uint8_t	*buf = fb_alloc(FLASH_SECTOR_SZ);

    if (f_open(&fp, FW_PATH, FA_READ|FA_OPEN_EXISTING) != FR_OK) {
        goto error;
    }

    // Firmware image size
    uint32_t size = f_size(&fp);

    while (size) {
        // Read a chuck (max FLASH_SECTOR_SZ bytes).
        bytes = MIN(size, FLASH_SECTOR_SZ);
        if (f_read(&fp, buf, bytes, &bytes_out) != FR_OK || bytes != bytes_out) {
            printf("burn_firmware: file read error!\n");
            goto error;
        }

        // Write firmware sector to the WINC1500 memory.
        if (programmer_write_firmware_image(buf, offset, bytes) != M2M_SUCCESS) {
            printf("burn_firmware: write error!\n");
            goto error;
        }

        size -= bytes;
        offset += bytes;
    }

    ret = M2M_SUCCESS;

error:
    fb_free();
    f_close(&fp);
    return ret;
}

/**
 * Verify WINC1500 firmware 
 * return M2M_SUCCESS on success, error code otherwise.
 */
int verify_firmware()
{
    FIL fp;
    uint32_t offset = 0;
    UINT bytes = 0, bytes_out=0;

    int ret = M2M_ERR_FAIL;
    uint8_t	*file_buf = fb_alloc(FLASH_SECTOR_SZ);
    uint8_t	*flash_buf = fb_alloc(FLASH_SECTOR_SZ);

    if (f_open(&fp, FW_PATH, FA_READ|FA_OPEN_EXISTING) != FR_OK) {
        goto error;
    }

    // Firmware image size
    uint32_t size = f_size(&fp);

    while (size) {
        // Firmware chuck size (max FLASH_SECTOR_SZ bytes).
        bytes = MIN(size, FLASH_SECTOR_SZ);

        if (f_read(&fp, file_buf, bytes, &bytes_out) != FR_OK || bytes_out != bytes) {
            printf("burn_firmware: file read error!\n");
            goto error;
        }

        if (programmer_read_firmware_image(flash_buf, offset, bytes) != M2M_SUCCESS) {
            printf("verify_firmware: read access failed on firmware section!\r\n");
            goto error;
        }

        for (int i=0; i<bytes; i++) {
            if (flash_buf[i] != file_buf[i]) {
                printf("verify_firmware: verification failed! offset:%ld flash:%x file:%x\n", offset+i, flash_buf[i], file_buf[i]);
                goto error;
            }
        }

        size -= bytes;
        offset += bytes;
    }

    ret = M2M_SUCCESS;

error:
    fb_free();
    fb_free();
    f_close(&fp);
    return ret;
}

/**
 * dump WINC1500 firmware
 * return M2M_SUCCESS on success, error code otherwise.
 */
int dump_firmware()
{
    FIL fp;
    uint32_t offset = 0;
    UINT bytes = 0, bytes_out=0;

    int ret = M2M_ERR_FAIL;
    uint8_t	*flash_buf = fb_alloc(FLASH_SECTOR_SZ);

    if (f_open(&fp, FW_DUMP_PATH, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
        goto error;
    }

    // Firmware image size
    uint32_t size = FLASH_4M_TOTAL_SZ;

    while (size) {
        // Firmware chuck size (max FLASH_SECTOR_SZ bytes).
        bytes = MIN(size, FLASH_SECTOR_SZ);

        if (programmer_read_firmware_image(flash_buf, offset, bytes) != M2M_SUCCESS) {
            printf("verify_firmware: read access failed on firmware section!\r\n");
            goto error;
        }

        if (f_write(&fp, flash_buf, bytes, &bytes_out) != FR_OK || bytes_out != bytes) {
            printf("burn_firmware: file read error!\n");
            goto error;
        }

        size -= bytes;
        offset += bytes;
    }

    ret = M2M_SUCCESS;

error:
    fb_free();
    f_close(&fp);
    return ret;
}

static int burn_certificate(const char *cert, const char *path)
{
    FIL fp;
    int ret = M2M_ERR_FAIL;
    // This is big enough for the max certificate
    char *buf = fb_alloc(FLASH_SECTOR_SZ);

    if (f_open(&fp, path, FA_READ|FA_OPEN_EXISTING) != FR_OK) {
        goto error;
    }

    UINT bytes_out=0;
    UINT bytes = f_size(&fp);

    // Read root certificate
    if (f_read(&fp, buf, bytes, &bytes_out) != FR_OK || bytes_out != bytes) {
        printf("burn_certificate: file read error!\n");
        goto error;
    }

    // Write root certificate to the WINC1500 memory.
    ret = WriteRootCertificate(cert, buf, bytes);

error:
    fb_free();
    f_close(&fp);
    return ret;

}

/**
 * Program root certificates to WINC1500 memory.
 *
 * return: M2M_SUCCESS on success, error code otherwise.
 */
int burn_certificates()
{
    const char *root_certs[][2] = {
        {"DigiCert_Root",           CERT_DIGI_PATH    },
        {"DigiCertSHA2_Root",       CERT_DIGISHA2_PATH},
        {"GeoTrustGlobalCA_Root",   CERT_GEOTRUST_PATH},
        {"Radius_Root",             CERT_RADIUS_PATH  },
        {"VeriSign_Root",           CERT_VERISIGN_PATH},
        {0, 0}
    };

    int ret=M2M_SUCCESS;
    for (int i=0; ret==M2M_SUCCESS && root_certs[i][0]; i++) {
        ret = burn_certificate(root_certs[i][0], root_certs[i][1]);
    }

    return ret;
}

/**
 * Verify WINC1500 certificates
 * return M2M_SUCCESS on success, error code otherwise.
 */
int verify_certificates(void)
{
    int ret = M2M_ERR_FAIL;
    uint8_t	*flash_buf = fb_alloc(FLASH_SECTOR_SZ);

    /* Dump entire root certificate memory region. */
    if (programmer_read_cert_image(flash_buf) != M2M_SUCCESS) {
        printf("verify_certificate: read access failed on certificate section!\r\n");
        goto error;
    }

    ret = M2M_SUCCESS;

error:
    fb_free();
    return ret;
}
