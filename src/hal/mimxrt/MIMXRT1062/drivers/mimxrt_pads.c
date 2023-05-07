// THIS FILE IS AUTO-GENERATED DO NOT EDIT.

/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2023 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2023 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 */
#include "omv_gpio.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "mimxrt_pads.h"

static omv_gpio_irq_descr_t gpio_irq_descr_gpio1[32] = { NULL };
static omv_gpio_irq_descr_t gpio_irq_descr_gpio2[32] = { NULL };
static omv_gpio_irq_descr_t gpio_irq_descr_gpio3[28] = { NULL };
static omv_gpio_irq_descr_t gpio_irq_descr_gpio4[32] = { NULL };
static omv_gpio_irq_descr_t gpio_irq_descr_gpio5[3] = { NULL };
static omv_gpio_irq_descr_t gpio_irq_descr_gpio10[22] = { NULL };

omv_gpio_irq_descr_t *const gpio_irq_descr_table[11] = {
    [1] = gpio_irq_descr_gpio1,
    [2] = gpio_irq_descr_gpio2,
    [3] = gpio_irq_descr_gpio3,
    [4] = gpio_irq_descr_gpio4,
    [5] = gpio_irq_descr_gpio5,
    [10] = gpio_irq_descr_gpio10,
};

static const imxrt_pad_af_t imxrt_pad_EMC_00_AF[5] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8494, 0 },    // FLEXPWM4
    { 2, 0x8500, 1 },    // LPSPI2
    { 3, 0x860C, 0 },    // XBAR1
    { 4, 0x0000, 0 },    // FLEXIO1
};
const imxrt_pad_t imxrt_pad_EMC_00 = { GPIO4, 0, 0x401F, 0x8014, 0x8204, 5, imxrt_pad_EMC_00_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_01_AF[5] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x0000, 0 },    // FLEXPWM4
    { 2, 0x84FC, 1 },    // LPSPI2
    { 3, 0x8610, 0 },    // XBAR1
    { 4, 0x0000, 0 },    // FLEXIO1
};
const imxrt_pad_t imxrt_pad_EMC_01 = { GPIO4, 1, 0x401F, 0x8018, 0x8208, 5, imxrt_pad_EMC_01_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_02_AF[5] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8498, 0 },    // FLEXPWM4
    { 2, 0x8508, 1 },    // LPSPI2
    { 3, 0x8614, 0 },    // XBAR1
    { 4, 0x0000, 0 },    // FLEXIO1
};
const imxrt_pad_t imxrt_pad_EMC_02 = { GPIO4, 2, 0x401F, 0x801C, 0x820C, 5, imxrt_pad_EMC_02_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_03_AF[5] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x0000, 0 },    // FLEXPWM4
    { 2, 0x8504, 1 },    // LPSPI2
    { 3, 0x8618, 0 },    // XBAR1
    { 4, 0x0000, 0 },    // FLEXIO1
};
const imxrt_pad_t imxrt_pad_EMC_03 = { GPIO4, 3, 0x401F, 0x8020, 0x8210, 5, imxrt_pad_EMC_03_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_04_AF[5] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x849C, 0 },    // FLEXPWM4
    { 2, 0x0000, 0 },    // SAI2
    { 3, 0x861C, 0 },    // XBAR1
    { 4, 0x0000, 0 },    // FLEXIO1
};
const imxrt_pad_t imxrt_pad_EMC_04 = { GPIO4, 4, 0x401F, 0x8024, 0x8214, 5, imxrt_pad_EMC_04_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_05_AF[5] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x0000, 0 },    // FLEXPWM4
    { 2, 0x85C4, 0 },    // SAI2
    { 3, 0x8620, 0 },    // XBAR1
    { 4, 0x0000, 0 },    // FLEXIO1
};
const imxrt_pad_t imxrt_pad_EMC_05 = { GPIO4, 5, 0x401F, 0x8028, 0x8218, 5, imxrt_pad_EMC_05_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_06_AF[5] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8478, 0 },    // FLEXPWM2
    { 2, 0x85C0, 0 },    // SAI2
    { 3, 0x8624, 0 },    // XBAR1
    { 4, 0x0000, 0 },    // FLEXIO1
};
const imxrt_pad_t imxrt_pad_EMC_06 = { GPIO4, 6, 0x401F, 0x802C, 0x821C, 5, imxrt_pad_EMC_06_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_07_AF[5] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8488, 0 },    // FLEXPWM2
    { 2, 0x85B0, 0 },    // SAI2
    { 3, 0x8628, 0 },    // XBAR1
    { 4, 0x0000, 0 },    // FLEXIO1
};
const imxrt_pad_t imxrt_pad_EMC_07 = { GPIO4, 7, 0x401F, 0x8030, 0x8220, 5, imxrt_pad_EMC_07_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_08_AF[5] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x847C, 0 },    // FLEXPWM2
    { 2, 0x85B8, 0 },    // SAI2
    { 3, 0x862C, 0 },    // XBAR1
    { 4, 0x0000, 0 },    // FLEXIO1
};
const imxrt_pad_t imxrt_pad_EMC_08 = { GPIO4, 8, 0x401F, 0x8034, 0x8224, 5, imxrt_pad_EMC_08_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_09_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x848C, 0 },    // FLEXPWM2
    { 2, 0x85BC, 0 },    // SAI2
    { 3, 0x0000, 0 },    // FLEXCAN2
    { 4, 0x0000, 0 },    // FLEXIO1
    { 8, 0x0000, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_09 = { GPIO4, 9, 0x401F, 0x8038, 0x8228, 6, imxrt_pad_EMC_09_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_10_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8480, 0 },    // FLEXPWM2
    { 2, 0x85B4, 0 },    // SAI2
    { 3, 0x8450, 0 },    // FLEXCAN2
    { 4, 0x0000, 0 },    // FLEXIO1
    { 8, 0x0000, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_10 = { GPIO4, 10, 0x401F, 0x803C, 0x822C, 6, imxrt_pad_EMC_10_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_11_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8490, 0 },    // FLEXPWM2
    { 2, 0x84E8, 0 },    // LPI2C4
    { 3, 0x0000, 0 },    // USDHC2
    { 4, 0x0000, 0 },    // FLEXIO1
    { 8, 0x0000, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_11 = { GPIO4, 11, 0x401F, 0x8040, 0x8230, 6, imxrt_pad_EMC_11_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_12_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8640, 0 },    // XBAR1
    { 2, 0x84E4, 0 },    // LPI2C4
    { 3, 0x85D8, 0 },    // USDHC1
    { 4, 0x8454, 1 },    // FLEXPWM1
    { 8, 0x8754, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_12 = { GPIO4, 12, 0x401F, 0x8044, 0x8234, 6, imxrt_pad_EMC_12_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_13_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8650, 1 },    // XBAR1
    { 2, 0x853C, 1 },    // LPUART3
    { 3, 0x0000, 0 },    // MQS
    { 4, 0x8464, 1 },    // FLEXPWM1
    { 8, 0x8740, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_13 = { GPIO4, 13, 0x401F, 0x8048, 0x8238, 6, imxrt_pad_EMC_13_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_14_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8654, 0 },    // XBAR1
    { 2, 0x8538, 1 },    // LPUART3
    { 3, 0x0000, 0 },    // MQS
    { 4, 0x0000, 0 },    // LPSPI2
    { 8, 0x8744, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_14 = { GPIO4, 14, 0x401F, 0x804C, 0x823C, 6, imxrt_pad_EMC_14_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_15_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8634, 0 },    // XBAR1
    { 2, 0x8534, 0 },    // LPUART3
    { 3, 0x0000, 0 },    // SPDIF
    { 4, 0x857C, 0 },    // QTIMER3
    { 8, 0x8748, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_15 = { GPIO4, 15, 0x401F, 0x8050, 0x8240, 6, imxrt_pad_EMC_15_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_16_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8658, 0 },    // XBAR1
    { 2, 0x0000, 0 },    // LPUART3
    { 3, 0x85C8, 1 },    // SPDIF
    { 4, 0x8580, 1 },    // QTIMER3
    { 8, 0x874C, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_16 = { GPIO4, 16, 0x401F, 0x8054, 0x8244, 6, imxrt_pad_EMC_16_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_17_AF[5] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x84A0, 0 },    // FLEXPWM4
    { 2, 0x0000, 0 },    // LPUART4
    { 3, 0x0000, 0 },    // FLEXCAN1
    { 4, 0x8584, 0 },    // QTIMER3
};
const imxrt_pad_t imxrt_pad_EMC_17 = { GPIO4, 17, 0x401F, 0x8058, 0x8248, 5, imxrt_pad_EMC_17_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_18_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x0000, 0 },    // FLEXPWM4
    { 2, 0x0000, 0 },    // LPUART4
    { 3, 0x844C, 1 },    // FLEXCAN1
    { 4, 0x8588, 0 },    // QTIMER3
    { 6, 0x0000, 0 },    // SNVS
};
const imxrt_pad_t imxrt_pad_EMC_18 = { GPIO4, 18, 0x401F, 0x805C, 0x824C, 6, imxrt_pad_EMC_18_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_19_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8474, 1 },    // FLEXPWM2
    { 2, 0x8544, 1 },    // LPUART4
    { 3, 0x8438, 0 },    // ENET
    { 4, 0x856C, 0 },    // QTIMER2
    { 6, 0x0000, 0 },    // SNVS
};
const imxrt_pad_t imxrt_pad_EMC_19 = { GPIO4, 19, 0x401F, 0x8060, 0x8250, 6, imxrt_pad_EMC_19_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_20_AF[5] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8484, 1 },    // FLEXPWM2
    { 2, 0x8540, 1 },    // LPUART4
    { 3, 0x8434, 0 },    // ENET
    { 4, 0x8570, 0 },    // QTIMER2
};
const imxrt_pad_t imxrt_pad_EMC_20 = { GPIO4, 20, 0x401F, 0x8064, 0x8254, 5, imxrt_pad_EMC_20_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_21_AF[5] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x0000, 0 },    // FLEXPWM3
    { 2, 0x84E0, 0 },    // LPI2C3
    { 3, 0x0000, 0 },    // ENET
    { 4, 0x8574, 0 },    // QTIMER2
};
const imxrt_pad_t imxrt_pad_EMC_21 = { GPIO4, 21, 0x401F, 0x8068, 0x8258, 5, imxrt_pad_EMC_21_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_22_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x0000, 0 },    // FLEXPWM3
    { 2, 0x84DC, 0 },    // LPI2C3
    { 3, 0x0000, 0 },    // ENET
    { 4, 0x8578, 0 },    // QTIMER2
    { 8, 0x0000, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_22 = { GPIO4, 22, 0x401F, 0x806C, 0x825C, 6, imxrt_pad_EMC_22_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_23_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8458, 0 },    // FLEXPWM1
    { 2, 0x854C, 0 },    // LPUART5
    { 3, 0x843C, 0 },    // ENET
    { 4, 0x875C, 0 },    // GPT1
    { 8, 0x872C, 1 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_23 = { GPIO4, 23, 0x401F, 0x8070, 0x8260, 6, imxrt_pad_EMC_23_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_24_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8468, 0 },    // FLEXPWM1
    { 2, 0x8548, 0 },    // LPUART5
    { 3, 0x0000, 0 },    // ENET
    { 4, 0x8758, 0 },    // GPT1
    { 8, 0x0000, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_24 = { GPIO4, 24, 0x401F, 0x8074, 0x8264, 6, imxrt_pad_EMC_24_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_25_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x845C, 0 },    // FLEXPWM1
    { 2, 0x8554, 0 },    // LPUART6
    { 3, 0x8448, 0 },    // ENET
    { 4, 0x842C, 0 },    // ENET
    { 8, 0x8750, 1 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_25 = { GPIO4, 25, 0x401F, 0x8078, 0x8268, 6, imxrt_pad_EMC_25_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_26_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x846C, 0 },    // FLEXPWM1
    { 2, 0x8550, 0 },    // LPUART6
    { 3, 0x8440, 0 },    // ENET
    { 4, 0x0000, 0 },    // FLEXIO1
    { 8, 0x8730, 1 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_26 = { GPIO4, 26, 0x401F, 0x807C, 0x826C, 6, imxrt_pad_EMC_26_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_27_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8460, 0 },    // FLEXPWM1
    { 2, 0x0000, 0 },    // LPUART5
    { 3, 0x84F0, 0 },    // LPSPI1
    { 4, 0x0000, 0 },    // FLEXIO1
    { 8, 0x8734, 1 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_27 = { GPIO4, 27, 0x401F, 0x8080, 0x8270, 6, imxrt_pad_EMC_27_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_28_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8470, 0 },    // FLEXPWM1
    { 2, 0x0000, 0 },    // LPUART5
    { 3, 0x84F8, 0 },    // LPSPI1
    { 4, 0x0000, 0 },    // FLEXIO1
    { 8, 0x8738, 1 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_28 = { GPIO4, 28, 0x401F, 0x8084, 0x8274, 6, imxrt_pad_EMC_28_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_29_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x0000, 0 },    // FLEXPWM3
    { 2, 0x0000, 0 },    // LPUART6
    { 3, 0x84F4, 0 },    // LPSPI1
    { 4, 0x0000, 0 },    // FLEXIO1
    { 8, 0x873C, 1 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_EMC_29 = { GPIO4, 29, 0x401F, 0x8088, 0x8278, 6, imxrt_pad_EMC_29_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_30_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x0000, 0 },    // FLEXPWM3
    { 2, 0x0000, 0 },    // LPUART6
    { 3, 0x84EC, 1 },    // LPSPI1
    { 4, 0x0000, 0 },    // CSI
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_EMC_30 = { GPIO4, 30, 0x401F, 0x808C, 0x827C, 6, imxrt_pad_EMC_30_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_31_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x0000, 0 },    // FLEXPWM3
    { 2, 0x855C, 1 },    // LPUART7
    { 3, 0x0000, 0 },    // LPSPI1
    { 4, 0x0000, 0 },    // CSI
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_EMC_31 = { GPIO4, 31, 0x401F, 0x8090, 0x8280, 6, imxrt_pad_EMC_31_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_32_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x0000, 0 },    // FLEXPWM3
    { 2, 0x8558, 1 },    // LPUART7
    { 3, 0x83FC, 4 },    // CCM
    { 4, 0x0000, 0 },    // CSI
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_EMC_32 = { GPIO3, 18, 0x401F, 0x8094, 0x8284, 6, imxrt_pad_EMC_32_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_33_AF[7] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x0000, 0 },    // FLEXPWM3
    { 2, 0x0000, 0 },    // USDHC1
    { 3, 0x8778, 0 },    // SAI3
    { 4, 0x0000, 0 },    // CSI
    { 8, 0x8728, 0 },    // ENET2
    { 9, 0x870C, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_EMC_33 = { GPIO3, 19, 0x401F, 0x8098, 0x8288, 7, imxrt_pad_EMC_33_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_34_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x0000, 0 },    // FLEXPWM3
    { 2, 0x0000, 0 },    // USDHC1
    { 3, 0x877C, 0 },    // SAI3
    { 4, 0x0000, 0 },    // CSI
    { 8, 0x8720, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_EMC_34 = { GPIO3, 20, 0x401F, 0x809C, 0x828C, 6, imxrt_pad_EMC_34_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_35_AF[7] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8630, 0 },    // XBAR1
    { 2, 0x0000, 0 },    // GPT1
    { 3, 0x8774, 0 },    // SAI3
    { 4, 0x0000, 0 },    // CSI
    { 6, 0x85D4, 0 },    // USDHC1
    { 8, 0x8714, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_EMC_35 = { GPIO3, 21, 0x401F, 0x80A0, 0x8290, 7, imxrt_pad_EMC_35_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_36_AF[8] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8638, 0 },    // XBAR1
    { 2, 0x0000, 0 },    // GPT1
    { 3, 0x0000, 0 },    // SAI3
    { 4, 0x0000, 0 },    // CSI
    { 6, 0x85D8, 1 },    // USDHC1
    { 8, 0x8718, 0 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXCAN3
};
const imxrt_pad_t imxrt_pad_EMC_36 = { GPIO3, 22, 0x401F, 0x80A4, 0x8294, 8, imxrt_pad_EMC_36_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_37_AF[8] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x863C, 0 },    // XBAR1
    { 2, 0x0000, 0 },    // GPT1
    { 3, 0x8770, 0 },    // SAI3
    { 4, 0x0000, 0 },    // CSI
    { 6, 0x8608, 0 },    // USDHC2
    { 8, 0x871C, 0 },    // ENET2
    { 9, 0x878C, 0 },    // FLEXCAN3
};
const imxrt_pad_t imxrt_pad_EMC_37 = { GPIO3, 23, 0x401F, 0x80A8, 0x8298, 8, imxrt_pad_EMC_37_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_38_AF[7] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8454, 2 },    // FLEXPWM1
    { 2, 0x8564, 2 },    // LPUART8
    { 3, 0x8780, 0 },    // SAI3
    { 4, 0x0000, 0 },    // CSI
    { 6, 0x0000, 0 },    // USDHC2
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_EMC_38 = { GPIO3, 24, 0x401F, 0x80AC, 0x829C, 7, imxrt_pad_EMC_38_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_39_AF[8] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8464, 2 },    // FLEXPWM1
    { 2, 0x8560, 2 },    // LPUART8
    { 3, 0x8784, 0 },    // SAI3
    { 4, 0x0000, 0 },    // WDOG1
    { 6, 0x85E0, 1 },    // USDHC2
    { 8, 0x8710, 0 },    // ENET2
    { 9, 0x8788, 1 },    // SEMC
};
const imxrt_pad_t imxrt_pad_EMC_39 = { GPIO3, 25, 0x401F, 0x80B0, 0x82A0, 8, imxrt_pad_EMC_39_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_40_AF[7] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8768, 0 },    // GPT2
    { 2, 0x0000, 0 },    // LPSPI1
    { 3, 0x85CC, 1 },    // USB
    { 4, 0x0000, 0 },    // ENET
    { 6, 0x0000, 0 },    // USDHC2
    { 9, 0x0000, 0 },    // SEMC
};
const imxrt_pad_t imxrt_pad_EMC_40 = { GPIO3, 26, 0x401F, 0x80B4, 0x82A4, 7, imxrt_pad_EMC_40_AF };

static const imxrt_pad_af_t imxrt_pad_EMC_41_AF[6] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x8764, 0 },    // GPT2
    { 2, 0x0000, 0 },    // LPSPI1
    { 3, 0x0000, 0 },    // USB
    { 4, 0x8430, 1 },    // ENET
    { 6, 0x0000, 0 },    // USDHC1
};
const imxrt_pad_t imxrt_pad_EMC_41 = { GPIO3, 27, 0x401F, 0x80B8, 0x82A8, 6, imxrt_pad_EMC_41_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_00_AF[7] = {
    { 0, 0x8474, 2 },    // FLEXPWM2
    { 1, 0x8644, 0 },    // XBAR1
    { 2, 0x0000, 0 },    // REF
    { 3, 0x83F8, 0 },    // USB
    { 4, 0x0000, 0 },    // LPI2C1
    { 6, 0x0000, 0 },    // USDHC1
    { 7, 0x8510, 0 },    // LPSPI3
};
const imxrt_pad_t imxrt_pad_AD_B0_00 = { GPIO1, 0, 0x401F, 0x80BC, 0x82AC, 7, imxrt_pad_AD_B0_00_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_01_AF[7] = {
    { 0, 0x8484, 2 },    // FLEXPWM2
    { 1, 0x8648, 0 },    // XBAR1
    { 2, 0x0000, 0 },    // REF
    { 3, 0x83F4, 0 },    // USB
    { 4, 0x0000, 0 },    // LPI2C1
    { 6, 0x0000, 0 },    // EWM
    { 7, 0x8518, 0 },    // LPSPI3
};
const imxrt_pad_t imxrt_pad_AD_B0_01 = { GPIO1, 1, 0x401F, 0x80C0, 0x82B0, 7, imxrt_pad_AD_B0_01_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_02_AF[7] = {
    { 0, 0x0000, 0 },    // FLEXCAN2
    { 1, 0x864C, 0 },    // XBAR1
    { 2, 0x8554, 1 },    // LPUART6
    { 3, 0x0000, 0 },    // USB
    { 4, 0x0000, 0 },    // FLEXPWM1
    { 6, 0x0000, 0 },    // LPI2C1
    { 7, 0x8514, 0 },    // LPSPI3
};
const imxrt_pad_t imxrt_pad_AD_B0_02 = { GPIO1, 2, 0x401F, 0x80C4, 0x82B4, 7, imxrt_pad_AD_B0_02_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_03_AF[7] = {
    { 0, 0x8450, 1 },    // FLEXCAN2
    { 1, 0x862C, 1 },    // XBAR1
    { 2, 0x8550, 1 },    // LPUART6
    { 3, 0x85D0, 0 },    // USB
    { 4, 0x0000, 0 },    // FLEXPWM1
    { 6, 0x0000, 0 },    // REF
    { 7, 0x850C, 0 },    // LPSPI3
};
const imxrt_pad_t imxrt_pad_AD_B0_03 = { GPIO1, 3, 0x401F, 0x80C8, 0x82B8, 7, imxrt_pad_AD_B0_03_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_04_AF[7] = {
    { 0, 0x0000, 0 },    // SRC
    { 1, 0x0000, 0 },    // MQS
    { 2, 0x0000, 0 },    // ENET
    { 3, 0x85C4, 1 },    // SAI2
    { 4, 0x841C, 1 },    // CSI
    { 6, 0x0000, 0 },    // PIT
    { 7, 0x0000, 0 },    // LPSPI3
};
const imxrt_pad_t imxrt_pad_AD_B0_04 = { GPIO1, 4, 0x401F, 0x80CC, 0x82BC, 7, imxrt_pad_AD_B0_04_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_05_AF[7] = {
    { 0, 0x0000, 0 },    // SRC
    { 1, 0x0000, 0 },    // MQS
    { 2, 0x0000, 0 },    // ENET
    { 3, 0x85C0, 1 },    // SAI2
    { 4, 0x8418, 1 },    // CSI
    { 6, 0x862C, 2 },    // XBAR1
    { 7, 0x0000, 0 },    // LPSPI3
};
const imxrt_pad_t imxrt_pad_AD_B0_05 = { GPIO1, 5, 0x401F, 0x80D0, 0x82C0, 7, imxrt_pad_AD_B0_05_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_06_AF[7] = {
    { 0, 0x0000, 0 },    // JTAG
    { 1, 0x0000, 0 },    // GPT2
    { 2, 0x0000, 0 },    // ENET
    { 3, 0x85B4, 1 },    // SAI2
    { 4, 0x8414, 1 },    // CSI
    { 6, 0x8630, 1 },    // XBAR1
    { 7, 0x0000, 0 },    // LPSPI3
};
const imxrt_pad_t imxrt_pad_AD_B0_06 = { GPIO1, 6, 0x401F, 0x80D4, 0x82C4, 7, imxrt_pad_AD_B0_06_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_07_AF[7] = {
    { 0, 0x0000, 0 },    // JTAG
    { 1, 0x0000, 0 },    // GPT2
    { 2, 0x0000, 0 },    // ENET
    { 3, 0x85BC, 1 },    // SAI2
    { 4, 0x8410, 1 },    // CSI
    { 6, 0x8654, 1 },    // XBAR1
    { 7, 0x0000, 0 },    // ENET
};
const imxrt_pad_t imxrt_pad_AD_B0_07 = { GPIO1, 7, 0x401F, 0x80D8, 0x82C8, 7, imxrt_pad_AD_B0_07_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_08_AF[7] = {
    { 0, 0x0000, 0 },    // JTAG
    { 1, 0x0000, 0 },    // GPT2
    { 2, 0x0000, 0 },    // ENET
    { 3, 0x85B8, 1 },    // SAI2
    { 4, 0x840C, 1 },    // CSI
    { 6, 0x8634, 1 },    // XBAR1
    { 7, 0x0000, 0 },    // ENET
};
const imxrt_pad_t imxrt_pad_AD_B0_08 = { GPIO1, 8, 0x401F, 0x80DC, 0x82CC, 7, imxrt_pad_AD_B0_08_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_09_AF[8] = {
    { 0, 0x0000, 0 },    // JTAG
    { 1, 0x8474, 3 },    // FLEXPWM2
    { 2, 0x0000, 0 },    // ENET
    { 3, 0x0000, 0 },    // SAI2
    { 4, 0x8408, 1 },    // CSI
    { 6, 0x8658, 1 },    // XBAR1
    { 7, 0x876C, 0 },    // GPT2
    { 9, 0x8788, 2 },    // SEMC
};
const imxrt_pad_t imxrt_pad_AD_B0_09 = { GPIO1, 9, 0x401F, 0x80E0, 0x82D0, 8, imxrt_pad_AD_B0_09_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_10_AF[9] = {
    { 0, 0x0000, 0 },    // JTAG
    { 1, 0x8454, 3 },    // FLEXPWM1
    { 2, 0x0000, 0 },    // ENET
    { 3, 0x85B0, 1 },    // SAI2
    { 4, 0x8404, 1 },    // CSI
    { 6, 0x8638, 1 },    // XBAR1
    { 7, 0x0000, 0 },    // ENET
    { 8, 0x0000, 0 },    // FLEXCAN3
    { 9, 0x0000, 0 },    // ARM
};
const imxrt_pad_t imxrt_pad_AD_B0_10 = { GPIO1, 10, 0x401F, 0x80E4, 0x82D4, 9, imxrt_pad_AD_B0_10_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_11_AF[9] = {
    { 0, 0x0000, 0 },    // JTAG
    { 1, 0x8464, 3 },    // FLEXPWM1
    { 2, 0x0000, 0 },    // ENET
    { 3, 0x0000, 0 },    // WDOG1
    { 4, 0x8400, 1 },    // CSI
    { 6, 0x863C, 1 },    // XBAR1
    { 7, 0x8444, 1 },    // ENET
    { 8, 0x878C, 2 },    // FLEXCAN3
    { 9, 0x0000, 0 },    // SEMC
};
const imxrt_pad_t imxrt_pad_AD_B0_11 = { GPIO1, 11, 0x401F, 0x80E8, 0x82D8, 9, imxrt_pad_AD_B0_11_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_12_AF[7] = {
    { 0, 0x84E4, 1 },    // LPI2C4
    { 1, 0x83FC, 1 },    // CCM
    { 2, 0x0000, 0 },    // LPUART1
    { 3, 0x0000, 0 },    // WDOG2
    { 4, 0x0000, 0 },    // FLEXPWM1
    { 6, 0x0000, 0 },    // ENET
    { 7, 0x8568, 0 },    // NMI
};
const imxrt_pad_t imxrt_pad_AD_B0_12 = { GPIO1, 12, 0x401F, 0x80EC, 0x82DC, 7, imxrt_pad_AD_B0_12_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_13_AF[7] = {
    { 0, 0x84E8, 1 },    // LPI2C4
    { 1, 0x8760, 0 },    // GPT1
    { 2, 0x0000, 0 },    // LPUART1
    { 3, 0x0000, 0 },    // EWM
    { 4, 0x0000, 0 },    // FLEXPWM1
    { 6, 0x0000, 0 },    // ENET
    { 7, 0x0000, 0 },    // REF
};
const imxrt_pad_t imxrt_pad_AD_B0_13 = { GPIO1, 13, 0x401F, 0x80F0, 0x82E0, 7, imxrt_pad_AD_B0_13_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_14_AF[7] = {
    { 0, 0x85CC, 0 },    // USB
    { 1, 0x8640, 1 },    // XBAR1
    { 2, 0x0000, 0 },    // LPUART1
    { 3, 0x0000, 0 },    // ENET
    { 4, 0x8428, 0 },    // CSI
    { 6, 0x0000, 0 },    // FLEXCAN2
    { 8, 0x0000, 0 },    // FLEXCAN3
};
const imxrt_pad_t imxrt_pad_AD_B0_14 = { GPIO1, 14, 0x401F, 0x80F4, 0x82E4, 7, imxrt_pad_AD_B0_14_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B0_15_AF[8] = {
    { 0, 0x0000, 0 },    // USB
    { 1, 0x8650, 0 },    // XBAR1
    { 2, 0x0000, 0 },    // LPUART1
    { 3, 0x8444, 0 },    // ENET
    { 4, 0x8420, 0 },    // CSI
    { 6, 0x8450, 2 },    // FLEXCAN2
    { 7, 0x0000, 0 },    // WDOG1
    { 8, 0x878C, 1 },    // FLEXCAN3
};
const imxrt_pad_t imxrt_pad_AD_B0_15 = { GPIO1, 15, 0x401F, 0x80F8, 0x82E8, 8, imxrt_pad_AD_B0_15_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_00_AF[9] = {
    { 0, 0x83F8, 1 },    // USB
    { 1, 0x857C, 1 },    // QTIMER3
    { 2, 0x0000, 0 },    // LPUART2
    { 3, 0x84CC, 1 },    // LPI2C1
    { 4, 0x0000, 0 },    // WDOG1
    { 6, 0x85D8, 2 },    // USDHC1
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x0000, 0 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_00 = { GPIO1, 16, 0x401F, 0x80FC, 0x82EC, 9, imxrt_pad_AD_B1_00_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_01_AF[9] = {
    { 0, 0x0000, 0 },    // USB
    { 1, 0x8580, 0 },    // QTIMER3
    { 2, 0x0000, 0 },    // LPUART2
    { 3, 0x84D0, 1 },    // LPI2C1
    { 4, 0x83FC, 2 },    // CCM
    { 6, 0x0000, 0 },    // USDHC1
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x8724, 0 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_01 = { GPIO1, 17, 0x401F, 0x8100, 0x82F0, 9, imxrt_pad_AD_B1_01_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_02_AF[9] = {
    { 0, 0x83F4, 1 },    // USB
    { 1, 0x8584, 1 },    // QTIMER3
    { 2, 0x8530, 1 },    // LPUART2
    { 3, 0x0000, 0 },    // SPDIF
    { 4, 0x0000, 0 },    // ENET
    { 6, 0x85D4, 1 },    // USDHC1
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x876C, 1 },    // GPT2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_02 = { GPIO1, 18, 0x401F, 0x8104, 0x82F4, 9, imxrt_pad_AD_B1_02_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_03_AF[9] = {
    { 0, 0x85D0, 1 },    // USB
    { 1, 0x8588, 1 },    // QTIMER3
    { 2, 0x852C, 1 },    // LPUART2
    { 3, 0x85C8, 0 },    // SPDIF
    { 4, 0x0000, 0 },    // ENET
    { 6, 0x85E0, 0 },    // USDHC2
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x8764, 1 },    // GPT2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_03 = { GPIO1, 19, 0x401F, 0x8108, 0x82F8, 9, imxrt_pad_AD_B1_03_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_04_AF[9] = {
    { 0, 0x84C4, 1 },    // FLEXSPIB
    { 1, 0x0000, 0 },    // ENET
    { 2, 0x8534, 1 },    // LPUART3
    { 3, 0x0000, 0 },    // SPDIF
    { 4, 0x8424, 0 },    // CSI
    { 6, 0x85E8, 1 },    // USDHC2
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x8768, 1 },    // GPT2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_04 = { GPIO1, 20, 0x401F, 0x810C, 0x82FC, 9, imxrt_pad_AD_B1_04_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_05_AF[9] = {
    { 0, 0x84C0, 1 },    // FLEXSPIB
    { 1, 0x8430, 0 },    // ENET
    { 2, 0x0000, 0 },    // LPUART3
    { 3, 0x0000, 0 },    // SPDIF
    { 4, 0x0000, 0 },    // CSI
    { 6, 0x85EC, 1 },    // USDHC2
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x0000, 0 },    // GPT2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_05 = { GPIO1, 21, 0x401F, 0x8110, 0x8300, 9, imxrt_pad_AD_B1_05_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_06_AF[9] = {
    { 0, 0x84BC, 1 },    // FLEXSPIB
    { 1, 0x84E0, 2 },    // LPI2C3
    { 2, 0x853C, 0 },    // LPUART3
    { 3, 0x0000, 0 },    // SPDIF
    { 4, 0x8428, 1 },    // CSI
    { 6, 0x85F0, 1 },    // USDHC2
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x0000, 0 },    // GPT2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_06 = { GPIO1, 22, 0x401F, 0x8114, 0x8304, 9, imxrt_pad_AD_B1_06_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_07_AF[9] = {
    { 0, 0x84B8, 1 },    // FLEXSPIB
    { 1, 0x84DC, 2 },    // LPI2C3
    { 2, 0x8538, 0 },    // LPUART3
    { 3, 0x0000, 0 },    // SPDIF
    { 4, 0x8420, 1 },    // CSI
    { 6, 0x85F4, 1 },    // USDHC2
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x0000, 0 },    // GPT2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_07 = { GPIO1, 23, 0x401F, 0x8118, 0x8308, 9, imxrt_pad_AD_B1_07_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_08_AF[8] = {
    { 0, 0x0000, 0 },    // FLEXSPIA
    { 1, 0x8494, 1 },    // FLEXPWM4
    { 2, 0x0000, 0 },    // FLEXCAN1
    { 3, 0x83FC, 3 },    // CCM
    { 4, 0x841C, 0 },    // CSI
    { 6, 0x85E4, 1 },    // USDHC2
    { 7, 0x0000, 0 },    // KPP
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_08 = { GPIO1, 24, 0x401F, 0x811C, 0x830C, 8, imxrt_pad_AD_B1_08_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_09_AF[8] = {
    { 0, 0x84A4, 1 },    // FLEXSPIA
    { 1, 0x8498, 1 },    // FLEXPWM4
    { 2, 0x844C, 2 },    // FLEXCAN1
    { 3, 0x858C, 1 },    // SAI1
    { 4, 0x8418, 0 },    // CSI
    { 6, 0x85DC, 1 },    // USDHC2
    { 7, 0x0000, 0 },    // KPP
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_09 = { GPIO1, 25, 0x401F, 0x8120, 0x8310, 8, imxrt_pad_AD_B1_09_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_10_AF[9] = {
    { 0, 0x84B4, 1 },    // FLEXSPIA
    { 1, 0x0000, 0 },    // WDOG1
    { 2, 0x8564, 1 },    // LPUART8
    { 3, 0x85A4, 1 },    // SAI1
    { 4, 0x8414, 0 },    // CSI
    { 6, 0x8608, 1 },    // USDHC2
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x0000, 0 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_10 = { GPIO1, 26, 0x401F, 0x8124, 0x8314, 9, imxrt_pad_AD_B1_10_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_11_AF[9] = {
    { 0, 0x84B0, 1 },    // FLEXSPIA
    { 1, 0x0000, 0 },    // EWM
    { 2, 0x8560, 1 },    // LPUART8
    { 3, 0x8590, 1 },    // SAI1
    { 4, 0x8410, 0 },    // CSI
    { 6, 0x0000, 0 },    // USDHC2
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x0000, 0 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_11 = { GPIO1, 27, 0x401F, 0x8128, 0x8318, 9, imxrt_pad_AD_B1_11_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_12_AF[9] = {
    { 0, 0x84AC, 1 },    // FLEXSPIA
    { 1, 0x0000, 0 },    // ACMP
    { 2, 0x850C, 1 },    // LPSPI3
    { 3, 0x8594, 1 },    // SAI1
    { 4, 0x840C, 0 },    // CSI
    { 6, 0x85F8, 1 },    // USDHC2
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x0000, 0 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_12 = { GPIO1, 28, 0x401F, 0x812C, 0x831C, 9, imxrt_pad_AD_B1_12_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_13_AF[9] = {
    { 0, 0x84A8, 1 },    // FLEXSPIA
    { 1, 0x0000, 0 },    // ACMP
    { 2, 0x8514, 1 },    // LPSPI3
    { 3, 0x0000, 0 },    // SAI1
    { 4, 0x8408, 0 },    // CSI
    { 6, 0x85FC, 1 },    // USDHC2
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x0000, 0 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_13 = { GPIO1, 29, 0x401F, 0x8130, 0x8320, 9, imxrt_pad_AD_B1_13_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_14_AF[9] = {
    { 0, 0x84C8, 1 },    // FLEXSPIA
    { 1, 0x0000, 0 },    // ACMP
    { 2, 0x8518, 1 },    // LPSPI3
    { 3, 0x85A8, 1 },    // SAI1
    { 4, 0x8404, 0 },    // CSI
    { 6, 0x8600, 1 },    // USDHC2
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x0000, 0 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_14 = { GPIO1, 30, 0x401F, 0x8134, 0x8324, 9, imxrt_pad_AD_B1_14_AF };

static const imxrt_pad_af_t imxrt_pad_AD_B1_15_AF[9] = {
    { 0, 0x0000, 0 },    // FLEXSPIA
    { 1, 0x0000, 0 },    // ACMP
    { 2, 0x8510, 1 },    // LPSPI3
    { 3, 0x85AC, 1 },    // SAI1
    { 4, 0x8400, 0 },    // CSI
    { 6, 0x8604, 1 },    // USDHC2
    { 7, 0x0000, 0 },    // KPP
    { 8, 0x0000, 0 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_AD_B1_15 = { GPIO1, 31, 0x401F, 0x8138, 0x8328, 9, imxrt_pad_AD_B1_15_AF };

static const imxrt_pad_af_t imxrt_pad_B0_00_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x0000, 0 },    // QTIMER1
    { 2, 0x0000, 0 },    // MQS
    { 3, 0x851C, 0 },    // LPSPI4
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SEMC
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_00 = { GPIO2, 0, 0x401F, 0x813C, 0x832C, 7, imxrt_pad_B0_00_AF };

static const imxrt_pad_af_t imxrt_pad_B0_01_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x0000, 0 },    // QTIMER1
    { 2, 0x0000, 0 },    // MQS
    { 3, 0x8524, 0 },    // LPSPI4
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SEMC
    { 8, 0x8710, 1 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_01 = { GPIO2, 1, 0x401F, 0x8140, 0x8330, 7, imxrt_pad_B0_01_AF };

static const imxrt_pad_af_t imxrt_pad_B0_02_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x0000, 0 },    // QTIMER1
    { 2, 0x0000, 0 },    // FLEXCAN1
    { 3, 0x8528, 0 },    // LPSPI4
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SEMC
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_02 = { GPIO2, 2, 0x401F, 0x8144, 0x8334, 7, imxrt_pad_B0_02_AF };

static const imxrt_pad_af_t imxrt_pad_B0_03_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x856C, 1 },    // QTIMER2
    { 2, 0x844C, 3 },    // FLEXCAN1
    { 3, 0x8520, 0 },    // LPSPI4
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // WDOG2
    { 8, 0x8724, 1 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_03 = { GPIO2, 3, 0x401F, 0x8148, 0x8338, 7, imxrt_pad_B0_03_AF };

static const imxrt_pad_af_t imxrt_pad_B0_04_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x8570, 1 },    // QTIMER2
    { 2, 0x84D4, 1 },    // LPI2C2
    { 3, 0x0000, 0 },    // ARM
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SRC
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_04 = { GPIO2, 4, 0x401F, 0x814C, 0x833C, 7, imxrt_pad_B0_04_AF };

static const imxrt_pad_af_t imxrt_pad_B0_05_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x8574, 1 },    // QTIMER2
    { 2, 0x84D8, 1 },    // LPI2C2
    { 3, 0x0000, 0 },    // ARM
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SRC
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_05 = { GPIO2, 5, 0x401F, 0x8150, 0x8340, 7, imxrt_pad_B0_05_AF };

static const imxrt_pad_af_t imxrt_pad_B0_06_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x857C, 2 },    // QTIMER3
    { 2, 0x8478, 1 },    // FLEXPWM2
    { 3, 0x0000, 0 },    // ARM
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SRC
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_06 = { GPIO2, 6, 0x401F, 0x8154, 0x8344, 7, imxrt_pad_B0_06_AF };

static const imxrt_pad_af_t imxrt_pad_B0_07_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x8580, 2 },    // QTIMER3
    { 2, 0x8488, 1 },    // FLEXPWM2
    { 3, 0x0000, 0 },    // ARM
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SRC
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_07 = { GPIO2, 7, 0x401F, 0x8158, 0x8348, 7, imxrt_pad_B0_07_AF };

static const imxrt_pad_af_t imxrt_pad_B0_08_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x8584, 2 },    // QTIMER3
    { 2, 0x847C, 1 },    // FLEXPWM2
    { 3, 0x853C, 2 },    // LPUART3
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SRC
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_08 = { GPIO2, 8, 0x401F, 0x815C, 0x834C, 7, imxrt_pad_B0_08_AF };

static const imxrt_pad_af_t imxrt_pad_B0_09_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x0000, 0 },    // QTIMER4
    { 2, 0x848C, 1 },    // FLEXPWM2
    { 3, 0x8538, 2 },    // LPUART3
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SRC
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_09 = { GPIO2, 9, 0x401F, 0x8160, 0x8350, 7, imxrt_pad_B0_09_AF };

static const imxrt_pad_af_t imxrt_pad_B0_10_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x0000, 0 },    // QTIMER4
    { 2, 0x8480, 1 },    // FLEXPWM2
    { 3, 0x8598, 1 },    // SAI1
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SRC
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_10 = { GPIO2, 10, 0x401F, 0x8164, 0x8354, 7, imxrt_pad_B0_10_AF };

static const imxrt_pad_af_t imxrt_pad_B0_11_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x0000, 0 },    // QTIMER4
    { 2, 0x8490, 1 },    // FLEXPWM2
    { 3, 0x859C, 1 },    // SAI1
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SRC
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_11 = { GPIO2, 11, 0x401F, 0x8168, 0x8358, 7, imxrt_pad_B0_11_AF };

static const imxrt_pad_af_t imxrt_pad_B0_12_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x0000, 0 },    // XBAR1
    { 2, 0x0000, 0 },    // ARM
    { 3, 0x85A0, 1 },    // SAI1
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SRC
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_12 = { GPIO2, 12, 0x401F, 0x816C, 0x835C, 7, imxrt_pad_B0_12_AF };

static const imxrt_pad_af_t imxrt_pad_B0_13_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x0000, 0 },    // XBAR1
    { 2, 0x0000, 0 },    // ARM
    { 3, 0x858C, 2 },    // SAI1
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SRC
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_13 = { GPIO2, 13, 0x401F, 0x8170, 0x8360, 7, imxrt_pad_B0_13_AF };

static const imxrt_pad_af_t imxrt_pad_B0_14_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x0000, 0 },    // XBAR1
    { 2, 0x0000, 0 },    // ARM
    { 3, 0x85A4, 2 },    // SAI1
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SRC
    { 8, 0x0000, 0 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_14 = { GPIO2, 14, 0x401F, 0x8174, 0x8364, 7, imxrt_pad_B0_14_AF };

static const imxrt_pad_af_t imxrt_pad_B0_15_AF[8] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x0000, 0 },    // XBAR1
    { 2, 0x0000, 0 },    // ARM
    { 3, 0x8590, 2 },    // SAI1
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // SRC
    { 8, 0x8728, 2 },    // ENET2
    { 9, 0x870C, 2 },    // ENET2
};
const imxrt_pad_t imxrt_pad_B0_15 = { GPIO2, 15, 0x401F, 0x8178, 0x8368, 8, imxrt_pad_B0_15_AF };

static const imxrt_pad_af_t imxrt_pad_B1_00_AF[8] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x8644, 1 },    // XBAR1
    { 2, 0x8544, 2 },    // LPUART4
    { 3, 0x8594, 2 },    // SAI1
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x8454, 4 },    // FLEXPWM1
    { 8, 0x8720, 2 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_00 = { GPIO2, 16, 0x401F, 0x817C, 0x836C, 8, imxrt_pad_B1_00_AF };

static const imxrt_pad_af_t imxrt_pad_B1_01_AF[8] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x8648, 1 },    // XBAR1
    { 2, 0x8540, 2 },    // LPUART4
    { 3, 0x0000, 0 },    // SAI1
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x8464, 4 },    // FLEXPWM1
    { 8, 0x8714, 2 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_01 = { GPIO2, 17, 0x401F, 0x8180, 0x8370, 8, imxrt_pad_B1_01_AF };

static const imxrt_pad_af_t imxrt_pad_B1_02_AF[8] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x864C, 1 },    // XBAR1
    { 2, 0x0000, 0 },    // LPSPI4
    { 3, 0x85A8, 2 },    // SAI1
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x8474, 4 },    // FLEXPWM2
    { 8, 0x8718, 2 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_02 = { GPIO2, 18, 0x401F, 0x8184, 0x8374, 8, imxrt_pad_B1_02_AF };

static const imxrt_pad_af_t imxrt_pad_B1_03_AF[8] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x862C, 3 },    // XBAR1
    { 2, 0x0000, 0 },    // LPSPI4
    { 3, 0x85AC, 2 },    // SAI1
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x8484, 3 },    // FLEXPWM2
    { 8, 0x871C, 2 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_03 = { GPIO2, 19, 0x401F, 0x8188, 0x8378, 8, imxrt_pad_B1_03_AF };

static const imxrt_pad_af_t imxrt_pad_B1_04_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x851C, 1 },    // LPSPI4
    { 2, 0x0000, 0 },    // CSI
    { 3, 0x8434, 1 },    // ENET
    { 4, 0x0000, 0 },    // FLEXIO2
    { 8, 0x8760, 1 },    // GPT1
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_04 = { GPIO2, 20, 0x401F, 0x818C, 0x837C, 7, imxrt_pad_B1_04_AF };

static const imxrt_pad_af_t imxrt_pad_B1_05_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x8524, 1 },    // LPSPI4
    { 2, 0x0000, 0 },    // CSI
    { 3, 0x8438, 1 },    // ENET
    { 4, 0x0000, 0 },    // FLEXIO2
    { 8, 0x8758, 1 },    // GPT1
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_05 = { GPIO2, 21, 0x401F, 0x8190, 0x8380, 7, imxrt_pad_B1_05_AF };

static const imxrt_pad_af_t imxrt_pad_B1_06_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x8528, 1 },    // LPSPI4
    { 2, 0x0000, 0 },    // CSI
    { 3, 0x843C, 1 },    // ENET
    { 4, 0x0000, 0 },    // FLEXIO2
    { 8, 0x875C, 1 },    // GPT1
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_06 = { GPIO2, 22, 0x401F, 0x8194, 0x8384, 7, imxrt_pad_B1_06_AF };

static const imxrt_pad_af_t imxrt_pad_B1_07_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x8520, 1 },    // LPSPI4
    { 2, 0x0000, 0 },    // CSI
    { 3, 0x0000, 0 },    // ENET
    { 4, 0x0000, 0 },    // FLEXIO2
    { 8, 0x0000, 0 },    // GPT1
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_07 = { GPIO2, 23, 0x401F, 0x8198, 0x8388, 7, imxrt_pad_B1_07_AF };

static const imxrt_pad_af_t imxrt_pad_B1_08_AF[8] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x0000, 0 },    // QTIMER1
    { 2, 0x0000, 0 },    // CSI
    { 3, 0x0000, 0 },    // ENET
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // FLEXCAN2
    { 8, 0x0000, 0 },    // GPT1
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_08 = { GPIO2, 24, 0x401F, 0x819C, 0x838C, 8, imxrt_pad_B1_08_AF };

static const imxrt_pad_af_t imxrt_pad_B1_09_AF[8] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x8578, 1 },    // QTIMER2
    { 2, 0x0000, 0 },    // CSI
    { 3, 0x0000, 0 },    // ENET
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x8450, 3 },    // FLEXCAN2
    { 8, 0x0000, 0 },    // GPT1
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_09 = { GPIO2, 25, 0x401F, 0x81A0, 0x8390, 8, imxrt_pad_B1_09_AF };

static const imxrt_pad_af_t imxrt_pad_B1_10_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x8588, 2 },    // QTIMER3
    { 2, 0x0000, 0 },    // CSI
    { 3, 0x8448, 1 },    // ENET
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x842C, 1 },    // ENET
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_10 = { GPIO2, 26, 0x401F, 0x81A4, 0x8394, 7, imxrt_pad_B1_10_AF };

static const imxrt_pad_af_t imxrt_pad_B1_11_AF[7] = {
    { 0, 0x0000, 0 },    // LCD
    { 1, 0x0000, 0 },    // QTIMER4
    { 2, 0x0000, 0 },    // CSI
    { 3, 0x8440, 1 },    // ENET
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // LPSPI4
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_11 = { GPIO2, 27, 0x401F, 0x81A8, 0x8398, 7, imxrt_pad_B1_11_AF };

static const imxrt_pad_af_t imxrt_pad_B1_12_AF[6] = {
    { 1, 0x854C, 1 },    // LPUART5
    { 2, 0x8424, 1 },    // CSI
    { 3, 0x8444, 2 },    // ENET
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x85D4, 2 },    // USDHC1
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_12 = { GPIO2, 28, 0x401F, 0x81AC, 0x839C, 6, imxrt_pad_B1_12_AF };

static const imxrt_pad_af_t imxrt_pad_B1_13_AF[8] = {
    { 0, 0x0000, 0 },    // WDOG1
    { 1, 0x8548, 1 },    // LPUART5
    { 2, 0x8428, 2 },    // CSI
    { 3, 0x0000, 0 },    // ENET
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x85D8, 3 },    // USDHC1
    { 8, 0x8788, 3 },    // SEMC
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_13 = { GPIO2, 29, 0x401F, 0x81B0, 0x83A0, 8, imxrt_pad_B1_13_AF };

static const imxrt_pad_af_t imxrt_pad_B1_14_AF[8] = {
    { 0, 0x0000, 0 },    // ENET
    { 1, 0x849C, 1 },    // FLEXPWM4
    { 2, 0x8420, 2 },    // CSI
    { 3, 0x860C, 1 },    // XBAR1
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // USDHC1
    { 8, 0x0000, 0 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_14 = { GPIO2, 30, 0x401F, 0x81B4, 0x83A4, 8, imxrt_pad_B1_14_AF };

static const imxrt_pad_af_t imxrt_pad_B1_15_AF[8] = {
    { 0, 0x8430, 2 },    // ENET
    { 1, 0x84A0, 1 },    // FLEXPWM4
    { 2, 0x0000, 0 },    // CSI
    { 3, 0x8610, 1 },    // XBAR1
    { 4, 0x0000, 0 },    // FLEXIO2
    { 6, 0x0000, 0 },    // USDHC1
    { 8, 0x0000, 0 },    // ENET2
    { 9, 0x0000, 0 },    // FLEXIO3
};
const imxrt_pad_t imxrt_pad_B1_15 = { GPIO2, 31, 0x401F, 0x81B8, 0x83A8, 8, imxrt_pad_B1_15_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B0_00_AF[8] = {
    { 0, 0x0000, 0 },    // USDHC1
    { 1, 0x8458, 1 },    // FLEXPWM1
    { 2, 0x84DC, 1 },    // LPI2C3
    { 3, 0x8614, 1 },    // XBAR1
    { 4, 0x84F0, 1 },    // LPSPI1
    { 6, 0x0000, 0 },    // FLEXSPIA
    { 8, 0x0000, 0 },    // ENET2
    { 9, 0x8788, 0 },    // SEMC
};
const imxrt_pad_t imxrt_pad_SD_B0_00 = { GPIO3, 12, 0x401F, 0x81BC, 0x83AC, 8, imxrt_pad_SD_B0_00_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B0_01_AF[8] = {
    { 0, 0x0000, 0 },    // USDHC1
    { 1, 0x8468, 1 },    // FLEXPWM1
    { 2, 0x84E0, 1 },    // LPI2C3
    { 3, 0x8618, 1 },    // XBAR1
    { 4, 0x84EC, 0 },    // LPSPI1
    { 6, 0x0000, 0 },    // FLEXSPIB
    { 8, 0x8728, 1 },    // ENET2
    { 9, 0x870C, 1 },    // ENET2
};
const imxrt_pad_t imxrt_pad_SD_B0_01 = { GPIO3, 13, 0x401F, 0x81C0, 0x83B0, 8, imxrt_pad_SD_B0_01_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B0_02_AF[7] = {
    { 0, 0x0000, 0 },    // USDHC1
    { 1, 0x845C, 1 },    // FLEXPWM1
    { 2, 0x0000, 0 },    // LPUART8
    { 3, 0x861C, 1 },    // XBAR1
    { 4, 0x84F8, 1 },    // LPSPI1
    { 8, 0x8720, 1 },    // ENET2
    { 9, 0x0000, 0 },    // SEMC
};
const imxrt_pad_t imxrt_pad_SD_B0_02 = { GPIO3, 14, 0x401F, 0x81C4, 0x83B4, 7, imxrt_pad_SD_B0_02_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B0_03_AF[7] = {
    { 0, 0x0000, 0 },    // USDHC1
    { 1, 0x846C, 1 },    // FLEXPWM1
    { 2, 0x0000, 0 },    // LPUART8
    { 3, 0x8620, 1 },    // XBAR1
    { 4, 0x84F4, 1 },    // LPSPI1
    { 8, 0x8714, 1 },    // ENET2
    { 9, 0x0000, 0 },    // SEMC
};
const imxrt_pad_t imxrt_pad_SD_B0_03 = { GPIO3, 15, 0x401F, 0x81C8, 0x83B8, 7, imxrt_pad_SD_B0_03_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B0_04_AF[7] = {
    { 0, 0x0000, 0 },    // USDHC1
    { 1, 0x8460, 1 },    // FLEXPWM1
    { 2, 0x8564, 0 },    // LPUART8
    { 3, 0x8624, 1 },    // XBAR1
    { 4, 0x0000, 0 },    // FLEXSPIB
    { 6, 0x0000, 0 },    // CCM
    { 8, 0x8718, 1 },    // ENET2
};
const imxrt_pad_t imxrt_pad_SD_B0_04 = { GPIO3, 16, 0x401F, 0x81CC, 0x83BC, 7, imxrt_pad_SD_B0_04_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B0_05_AF[7] = {
    { 0, 0x0000, 0 },    // USDHC1
    { 1, 0x8470, 1 },    // FLEXPWM1
    { 2, 0x8560, 0 },    // LPUART8
    { 3, 0x8628, 1 },    // XBAR1
    { 4, 0x0000, 0 },    // FLEXSPIB
    { 6, 0x0000, 0 },    // CCM
    { 8, 0x871C, 1 },    // ENET2
};
const imxrt_pad_t imxrt_pad_SD_B0_05 = { GPIO3, 17, 0x401F, 0x81D0, 0x83C0, 7, imxrt_pad_SD_B0_05_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B1_00_AF[6] = {
    { 0, 0x85F4, 0 },    // USDHC2
    { 1, 0x84C4, 0 },    // FLEXSPIB
    { 2, 0x8454, 0 },    // FLEXPWM1
    { 3, 0x8598, 0 },    // SAI1
    { 4, 0x8544, 0 },    // LPUART4
    { 8, 0x8778, 1 },    // SAI3
};
const imxrt_pad_t imxrt_pad_SD_B1_00 = { GPIO3, 0, 0x401F, 0x81D4, 0x83C4, 6, imxrt_pad_SD_B1_00_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B1_01_AF[6] = {
    { 0, 0x85F0, 0 },    // USDHC2
    { 1, 0x84C0, 0 },    // FLEXSPIB
    { 2, 0x8464, 0 },    // FLEXPWM1
    { 3, 0x859C, 0 },    // SAI1
    { 4, 0x8540, 0 },    // LPUART4
    { 8, 0x0000, 0 },    // SAI3
};
const imxrt_pad_t imxrt_pad_SD_B1_01 = { GPIO3, 1, 0x401F, 0x81D8, 0x83C8, 6, imxrt_pad_SD_B1_01_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B1_02_AF[7] = {
    { 0, 0x85EC, 0 },    // USDHC2
    { 1, 0x84BC, 0 },    // FLEXSPIB
    { 2, 0x8474, 0 },    // FLEXPWM2
    { 3, 0x85A0, 0 },    // SAI1
    { 4, 0x0000, 0 },    // FLEXCAN1
    { 6, 0x0000, 0 },    // CCM
    { 8, 0x8784, 1 },    // SAI3
};
const imxrt_pad_t imxrt_pad_SD_B1_02 = { GPIO3, 2, 0x401F, 0x81DC, 0x83CC, 7, imxrt_pad_SD_B1_02_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B1_03_AF[7] = {
    { 0, 0x85E8, 0 },    // USDHC2
    { 1, 0x84B8, 0 },    // FLEXSPIB
    { 2, 0x8484, 0 },    // FLEXPWM2
    { 3, 0x858C, 0 },    // SAI1
    { 4, 0x844C, 0 },    // FLEXCAN1
    { 6, 0x83FC, 0 },    // CCM
    { 8, 0x8780, 1 },    // SAI3
};
const imxrt_pad_t imxrt_pad_SD_B1_03 = { GPIO3, 3, 0x401F, 0x81E0, 0x83D0, 7, imxrt_pad_SD_B1_03_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B1_04_AF[7] = {
    { 0, 0x85DC, 0 },    // USDHC2
    { 1, 0x0000, 0 },    // FLEXSPIB
    { 2, 0x84CC, 0 },    // LPI2C1
    { 3, 0x85A4, 0 },    // SAI1
    { 4, 0x0000, 0 },    // FLEXSPIA
    { 6, 0x0000, 0 },    // CCM
    { 8, 0x8770, 1 },    // SAI3
};
const imxrt_pad_t imxrt_pad_SD_B1_04 = { GPIO3, 4, 0x401F, 0x81E4, 0x83D4, 7, imxrt_pad_SD_B1_04_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B1_05_AF[6] = {
    { 0, 0x85E4, 0 },    // USDHC2
    { 1, 0x84A4, 0 },    // FLEXSPIA
    { 2, 0x84D0, 0 },    // LPI2C1
    { 3, 0x8590, 0 },    // SAI1
    { 4, 0x0000, 0 },    // FLEXSPIB
    { 8, 0x877C, 1 },    // SAI3
};
const imxrt_pad_t imxrt_pad_SD_B1_05 = { GPIO3, 5, 0x401F, 0x81E8, 0x83D8, 6, imxrt_pad_SD_B1_05_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B1_06_AF[6] = {
    { 0, 0x0000, 0 },    // USDHC2
    { 1, 0x0000, 0 },    // FLEXSPIA
    { 2, 0x0000, 0 },    // LPUART7
    { 3, 0x8594, 0 },    // SAI1
    { 4, 0x84FC, 0 },    // LPSPI2
    { 8, 0x8774, 1 },    // SAI3
};
const imxrt_pad_t imxrt_pad_SD_B1_06 = { GPIO3, 6, 0x401F, 0x81EC, 0x83DC, 6, imxrt_pad_SD_B1_06_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B1_07_AF[5] = {
    { 0, 0x0000, 0 },    // SEMC
    { 1, 0x84C8, 0 },    // FLEXSPIA
    { 2, 0x0000, 0 },    // LPUART7
    { 3, 0x0000, 0 },    // SAI1
    { 4, 0x8500, 0 },    // LPSPI2
};
const imxrt_pad_t imxrt_pad_SD_B1_07 = { GPIO3, 7, 0x401F, 0x81F0, 0x83E0, 5, imxrt_pad_SD_B1_07_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B1_08_AF[6] = {
    { 0, 0x85F8, 0 },    // USDHC2
    { 1, 0x84A8, 0 },    // FLEXSPIA
    { 2, 0x855C, 0 },    // LPUART7
    { 3, 0x85A8, 0 },    // SAI1
    { 4, 0x8508, 0 },    // LPSPI2
    { 6, 0x0000, 0 },    // SEMC
};
const imxrt_pad_t imxrt_pad_SD_B1_08 = { GPIO3, 8, 0x401F, 0x81F4, 0x83E4, 6, imxrt_pad_SD_B1_08_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B1_09_AF[5] = {
    { 0, 0x85FC, 0 },    // USDHC2
    { 1, 0x84AC, 0 },    // FLEXSPIA
    { 2, 0x8558, 0 },    // LPUART7
    { 3, 0x85AC, 0 },    // SAI1
    { 4, 0x8504, 0 },    // LPSPI2
};
const imxrt_pad_t imxrt_pad_SD_B1_09 = { GPIO3, 9, 0x401F, 0x81F8, 0x83E8, 5, imxrt_pad_SD_B1_09_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B1_10_AF[5] = {
    { 0, 0x8600, 0 },    // USDHC2
    { 1, 0x84B0, 0 },    // FLEXSPIA
    { 2, 0x852C, 0 },    // LPUART2
    { 3, 0x84D8, 0 },    // LPI2C2
    { 4, 0x0000, 0 },    // LPSPI2
};
const imxrt_pad_t imxrt_pad_SD_B1_10 = { GPIO3, 10, 0x401F, 0x81FC, 0x83EC, 5, imxrt_pad_SD_B1_10_AF };

static const imxrt_pad_af_t imxrt_pad_SD_B1_11_AF[5] = {
    { 0, 0x8604, 0 },    // USDHC2
    { 1, 0x84B4, 0 },    // FLEXSPIA
    { 2, 0x8530, 0 },    // LPUART2
    { 3, 0x84D4, 0 },    // LPI2C2
    { 4, 0x0000, 0 },    // LPSPI2
};
const imxrt_pad_t imxrt_pad_SD_B1_11 = { GPIO3, 11, 0x401F, 0x8200, 0x83F0, 5, imxrt_pad_SD_B1_11_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_00_AF[0] = {
};
const imxrt_pad_t imxrt_pad_SPI_B0_00 = { GPIO10, 0, 0x401F, 0x865C, 0x86B4, 0, imxrt_pad_SPI_B0_00_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_01_AF[1] = {
    { 0, 0x8754, 1 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B0_01 = { GPIO10, 1, 0x401F, 0x8660, 0x86B8, 1, imxrt_pad_SPI_B0_01_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_02_AF[1] = {
    { 0, 0x8730, 2 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B0_02 = { GPIO10, 2, 0x401F, 0x8664, 0x86BC, 1, imxrt_pad_SPI_B0_02_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_03_AF[1] = {
    { 0, 0x8748, 1 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B0_03 = { GPIO10, 3, 0x401F, 0x8668, 0x86C0, 1, imxrt_pad_SPI_B0_03_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_04_AF[1] = {
    { 0, 0x874C, 1 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B0_04 = { GPIO10, 4, 0x401F, 0x866C, 0x86C4, 1, imxrt_pad_SPI_B0_04_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_05_AF[1] = {
    { 0, 0x0000, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B0_05 = { GPIO10, 5, 0x401F, 0x8670, 0x86C8, 1, imxrt_pad_SPI_B0_05_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_06_AF[1] = {
    { 0, 0x8738, 2 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B0_06 = { GPIO10, 6, 0x401F, 0x8674, 0x86CC, 1, imxrt_pad_SPI_B0_06_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_07_AF[1] = {
    { 0, 0x8744, 1 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B0_07 = { GPIO10, 7, 0x401F, 0x8678, 0x86D0, 1, imxrt_pad_SPI_B0_07_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_08_AF[1] = {
    { 0, 0x8750, 2 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B0_08 = { GPIO10, 8, 0x401F, 0x867C, 0x86D4, 1, imxrt_pad_SPI_B0_08_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_09_AF[1] = {
    { 0, 0x872C, 2 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B0_09 = { GPIO10, 9, 0x401F, 0x8680, 0x86D8, 1, imxrt_pad_SPI_B0_09_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_10_AF[1] = {
    { 0, 0x873C, 2 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B0_10 = { GPIO10, 10, 0x401F, 0x8684, 0x86DC, 1, imxrt_pad_SPI_B0_10_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_11_AF[1] = {
    { 0, 0x8740, 1 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B0_11 = { GPIO10, 11, 0x401F, 0x8688, 0x86E0, 1, imxrt_pad_SPI_B0_11_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_12_AF[1] = {
    { 0, 0x8734, 2 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B0_12 = { GPIO10, 12, 0x401F, 0x868C, 0x86E4, 1, imxrt_pad_SPI_B0_12_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B0_13_AF[0] = {
};
const imxrt_pad_t imxrt_pad_SPI_B0_13 = { GPIO10, 13, 0x401F, 0x8690, 0x86E8, 0, imxrt_pad_SPI_B0_13_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B1_00_AF[1] = {
    { 0, 0x872C, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B1_00 = { GPIO10, 14, 0x401F, 0x8694, 0x86EC, 1, imxrt_pad_SPI_B1_00_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B1_01_AF[1] = {
    { 0, 0x873C, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B1_01 = { GPIO10, 15, 0x401F, 0x8698, 0x86F0, 1, imxrt_pad_SPI_B1_01_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B1_02_AF[1] = {
    { 0, 0x8738, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B1_02 = { GPIO10, 16, 0x401F, 0x869C, 0x86F4, 1, imxrt_pad_SPI_B1_02_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B1_03_AF[1] = {
    { 0, 0x8734, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B1_03 = { GPIO10, 17, 0x401F, 0x86A0, 0x86F8, 1, imxrt_pad_SPI_B1_03_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B1_04_AF[1] = {
    { 0, 0x8730, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B1_04 = { GPIO10, 18, 0x401F, 0x86A4, 0x86FC, 1, imxrt_pad_SPI_B1_04_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B1_05_AF[1] = {
    { 0, 0x8750, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B1_05 = { GPIO10, 19, 0x401F, 0x86A8, 0x8700, 1, imxrt_pad_SPI_B1_05_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B1_06_AF[1] = {
    { 0, 0x0000, 0 },    // FLEXSPI2
};
const imxrt_pad_t imxrt_pad_SPI_B1_06 = { GPIO10, 20, 0x401F, 0x86AC, 0x8704, 1, imxrt_pad_SPI_B1_06_AF };

static const imxrt_pad_af_t imxrt_pad_SPI_B1_07_AF[0] = {
};
const imxrt_pad_t imxrt_pad_SPI_B1_07 = { GPIO10, 21, 0x401F, 0x86B0, 0x8708, 0, imxrt_pad_SPI_B1_07_AF };

static const imxrt_pad_af_t imxrt_pad_WAKEUP_AF[1] = {
    { 7, 0x8568, 1 },    // NMI
};
const imxrt_pad_t imxrt_pad_WAKEUP = { GPIO5, 0, 0x400A, 0x8000, 0x8018, 1, imxrt_pad_WAKEUP_AF };

static const imxrt_pad_af_t imxrt_pad_PMIC_ON_REQ_AF[1] = {
    { 0, 0x0000, 0 },    // SNVS
};
const imxrt_pad_t imxrt_pad_PMIC_ON_REQ = { GPIO5, 1, 0x400A, 0x8004, 0x801C, 1, imxrt_pad_PMIC_ON_REQ_AF };

static const imxrt_pad_af_t imxrt_pad_PMIC_STBY_REQ_AF[1] = {
    { 0, 0x0000, 0 },    // CCM
};
const imxrt_pad_t imxrt_pad_PMIC_STBY_REQ = { GPIO5, 2, 0x400A, 0x8008, 0x8020, 1, imxrt_pad_PMIC_STBY_REQ_AF };
