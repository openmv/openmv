#include "mp.h"
#include "pincfg.h"
#include "spi.h"
#include "py_spi.h"
#include "py_assert.h"
#include "py_image.h"
#include "imlib.h"

#define SPI_TIMEOUT         (500)  /* in ms */

#define SPIx_DMA_RX_IRQn    DMA2_Stream3_IRQn
#define SPIx_RX_DMA_STREAM  DMA2_Stream3
#define SPIx_RX_DMA_CHANNEL DMA_CHANNEL_5

#define SPIx_DMA_TX_IRQn    DMA2_Stream4_IRQn
#define SPIx_TX_DMA_STREAM  DMA2_Stream4
#define SPIx_TX_DMA_CHANNEL DMA_CHANNEL_5

SPI_HandleTypeDef SPIHandle;
static DMA_HandleTypeDef hdma_tx;
static DMA_HandleTypeDef hdma_rx;

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

    if (HAL_SPI_Receive_DMA(&SPIHandle, bufinfo.buf, bufinfo.len) != HAL_OK) {
        return -1;
    }

    // wait for transfer to finish
    while (HAL_SPI_GetState(&SPIHandle) != HAL_SPI_STATE_READY) {
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

    if (bufinfo.len > 1) {
        // start DMA transfer
        if (HAL_SPI_Transmit_DMA(&SPIHandle, bufinfo.buf, bufinfo.len) != HAL_OK) {
            return -1;
        }
        // wait for transfer to finish
        while (HAL_SPI_GetState(&SPIHandle) != HAL_SPI_STATE_READY) {
        }
    } else {
        // don't use DMA for small buffers
        if (HAL_SPI_Transmit(&SPIHandle, bufinfo.buf, bufinfo.len, SPI_TIMEOUT) != HAL_OK) {
            return -1;
        }
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

static void config_dma_stream(SPI_HandleTypeDef *hspi,
        DMA_HandleTypeDef *hdma, DMA_Stream_TypeDef *instance, uint32_t channel)
{
    hdma->Instance                 = instance;
    hdma->Init.Channel             = channel;
    hdma->Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma->Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma->Init.MemInc              = DMA_MINC_ENABLE;
    hdma->Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma->Init.Mode                = DMA_NORMAL;
    hdma->Init.Priority            = DMA_PRIORITY_LOW; //SET to HIGH for rx
    hdma->Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma->Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    hdma->Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma->Init.PeriphBurst         = DMA_PBURST_SINGLE;

    /* This clears any pending flags, for soft-resets */
    hdma->State = HAL_DMA_STATE_RESET;
    HAL_DMA_DeInit(hdma);

    /* Init and associate DMA stream handle */
    HAL_DMA_Init(hdma);
}

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
    SPIHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    SPIHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SPIHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
    SPIHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    SPIHandle.Init.CRCPolynomial     = 7;

    /* Configure TX DMA stream */
    config_dma_stream(&SPIHandle, &hdma_tx, SPIx_TX_DMA_STREAM, SPIx_TX_DMA_CHANNEL);
    __HAL_LINKDMA(&SPIHandle, hdmatx, hdma_tx);

    /* Configure RX DMA stream */
    config_dma_stream(&SPIHandle, &hdma_rx, SPIx_RX_DMA_STREAM, SPIx_RX_DMA_CHANNEL);
    __HAL_LINKDMA(&SPIHandle, hdmarx, hdma_rx);

    /* Configure TX DMA stream IRQ */
    HAL_NVIC_SetPriority(SPIx_DMA_TX_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(SPIx_DMA_TX_IRQn);

    /* Configure RX DMA stream IRQ */
    HAL_NVIC_SetPriority(SPIx_DMA_RX_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPIx_DMA_RX_IRQn);

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
