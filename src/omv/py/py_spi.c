#include "mp.h"
#include "pincfg.h"
#include "spi.h"
#include "py_spi.h"
#include "py_assert.h"
#include "py_image.h"
#include "imlib.h"
#define SPI_TIMEOUT         (500)  /* in ms */
static SPI_HandleTypeDef SPIHandle;

static inline uint8_t spi_xfer(uint8_t out)
{
    if (HAL_SPI_TransmitReceive(&SPIHandle, &out, &out, 1, SPI_TIMEOUT) != HAL_OK) {
        //BREAK();
    }
    return out;
}

static machine_int_t py_spi_read(mp_obj_t obj)
{
    mp_buffer_info_t bufinfo;

    if (MP_OBJ_IS_INT(obj)) {
        bufinfo.len = mp_obj_get_int(obj);
        bufinfo.typecode = 'B';
        mp_obj_str_builder_start(&mp_type_bytes, bufinfo.len, (byte**)&(bufinfo.buf));
    } else {
        mp_get_buffer_raise(obj, &bufinfo, MP_BUFFER_WRITE);
    }

    if (HAL_SPI_Receive(&SPIHandle, bufinfo.buf, bufinfo.len, SPI_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return bufinfo.len;
}

static machine_int_t py_spi_write(mp_obj_t obj)
{
    byte buf[1];
    mp_buffer_info_t bufinfo;

    if (MP_OBJ_IS_INT(obj)) {
        buf[0] = mp_obj_get_int(obj);
        bufinfo.buf = buf;
        bufinfo.len = 1;
        bufinfo.typecode = 'B';
    } else {
        mp_get_buffer_raise(obj, &bufinfo, MP_BUFFER_READ);
    }

    if (HAL_SPI_Transmit(&SPIHandle, bufinfo.buf, bufinfo.len, SPI_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return bufinfo.len;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_spi_read_obj,   py_spi_read);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_spi_write_obj,  py_spi_write);

static const mp_map_elem_t globals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_spi) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read),      (mp_obj_t)&py_spi_read_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write),     (mp_obj_t)&py_spi_write_obj },
};

STATIC MP_DEFINE_CONST_DICT(globals_dict, globals_dict_table);

static const mp_obj_module_t py_spi_module = {
    .base = { &mp_type_module },
    .name = MP_QSTR_spi,
    .globals = (mp_obj_t)&globals_dict,
};

const mp_obj_module_t *py_spi_init()
{
    /* SPI configuration */
    SPIHandle.Instance               = USR_SPI;
    SPIHandle.Init.Mode              = SPI_MODE_MASTER;
    SPIHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SPIHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SPIHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    SPIHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SPIHandle.Init.NSS               = SPI_NSS_SOFT;
    SPIHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    SPIHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SPIHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
    SPIHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    SPIHandle.Init.CRCPolynomial     = 7;

    /* Initialize the user SPI */
    if (HAL_SPI_Init(&SPIHandle) != HAL_OK) {
        /* Initialization Error */
        return NULL;
    }

    uint8_t buf[1];
    /* dummy read */
    HAL_SPI_Receive(&SPIHandle, buf, sizeof(buf), SPI_TIMEOUT);

    return &py_spi_module;
}
