/*******************************************************************************
  File Name:
    nmasic.c

  Summary:
    This module contains WINC1500 ASIC specific internal APIs.

  Description:
    This module contains WINC1500 ASIC specific internal APIs.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*
Copyright (C) 2022, Microchip Technology Inc., and its subsidiaries. All rights reserved.

The software and documentation is provided by microchip and its contributors
"as is" and any express, implied or statutory warranties, including, but not
limited to, the implied warranties of merchantability, fitness for a particular
purpose and non-infringement of third party intellectual property rights are
disclaimed to the fullest extent permitted by law. In no event shall microchip
or its contributors be liable for any direct, indirect, incidental, special,
exemplary, or consequential damages (including, but not limited to, procurement
of substitute goods or services; loss of use, data, or profits; or business
interruption) however caused and on any theory of liability, whether in contract,
strict liability, or tort (including negligence or otherwise) arising in any way
out of the use of the software and documentation, even if advised of the
possibility of such damage.

Except as expressly permitted hereunder and subject to the applicable license terms
for any third-party software incorporated in the software and any applicable open
source software license terms, no license or other rights, whether express or
implied, are granted under any patent or other intellectual property rights of
Microchip or any third party.
*/

#include "nm_common.h"
#include "nmbus.h"
#include "nm_bsp.h"
#include "nmasic.h"
#include "m2m_types.h"

#define NMI_GLB_RESET_0             (NMI_PERIPH_REG_BASE + 0x400)
#define NMI_INTR_REG_BASE           (NMI_PERIPH_REG_BASE + 0xa00)
#define NMI_PIN_MUX_0               (NMI_PERIPH_REG_BASE + 0x408)
#define NMI_INTR_ENABLE             (NMI_INTR_REG_BASE)
#define GET_UINT32(X,Y)             (X[0+Y] + ((uint32_t)X[1+Y]<<8) + ((uint32_t)X[2+Y]<<16) +((uint32_t)X[3+Y]<<24))

#define CORT_HOST_COMM              (0x10)
#define HOST_CORT_COMM              (0x0b)
#define WAKE_CLK_REG                (0x1)
#define CLOCKS_EN_REG               (0xf)



#define TIMEOUT                     (0x2000ul)
#define WAKUP_TRAILS_TIMEOUT        (4)

int8_t chip_apply_conf(uint32_t u32Conf)
{
    int8_t ret = M2M_SUCCESS;
    uint32_t val32 = u32Conf;

#ifdef __ENABLE_PMU__
    val32 |= rHAVE_USE_PMU_BIT;
#endif
#ifdef __ENABLE_SLEEP_CLK_SRC_RTC__
    val32 |= rHAVE_SLEEP_CLK_SRC_RTC_BIT;
#elif defined __ENABLE_SLEEP_CLK_SRC_XO__
    val32 |= rHAVE_SLEEP_CLK_SRC_XO_BIT;
#endif
#ifdef __ENABLE_EXT_PA_INV_TX_RX__
    val32 |= rHAVE_EXT_PA_INV_TX_RX;
#endif
#ifdef __ENABLE_LEGACY_RF_SETTINGS__
    val32 |= rHAVE_LEGACY_RF_SETTINGS;
#endif
#ifdef __DISABLE_FIRMWARE_LOGS__
    val32 |= rHAVE_LOGS_DISABLED_BIT;
#endif
#if defined CONF_WINC_XO_XTALGM2_DIS
    val32 |= rHAVE_XO_XTALGM2_DIS_BIT;
#endif

    val32 |= rHAVE_RESERVED1_BIT;
    do  {
        nm_write_reg(rNMI_GP_REG_1, val32);
        if(val32 != 0) {
            uint32_t reg = 0;
            ret = nm_read_reg_with_ret(rNMI_GP_REG_1, &reg);
            if(ret == M2M_SUCCESS) {
                if(reg == val32)
                    break;
            }
        } else {
            break;
        }
    } while(1);

    return M2M_SUCCESS;
}
void chip_idle(void)
{
    uint32_t reg = 0;
    nm_read_reg_with_ret(WAKE_CLK_REG, &reg);
    if(reg & NBIT1)
    {
        reg &=~ NBIT1;
        nm_write_reg(WAKE_CLK_REG, reg);
    }
}

int8_t enable_interrupts(void)
{
    uint32_t reg = 0;
    int8_t ret = M2M_SUCCESS;
    /**
    interrupt pin mux select
    **/
    ret = nm_read_reg_with_ret(NMI_PIN_MUX_0, &reg);
    if (M2M_SUCCESS != ret) goto ERR1;

    reg |= ((uint32_t) 1 << 8);
    ret = nm_write_reg(NMI_PIN_MUX_0, reg);
    if (M2M_SUCCESS != ret) goto ERR1;

    /**
    interrupt enable
    **/
    ret = nm_read_reg_with_ret(NMI_INTR_ENABLE, &reg);
    if (M2M_SUCCESS != ret) goto ERR1;

    reg |= ((uint32_t) 1 << 16);
    ret = nm_write_reg(NMI_INTR_ENABLE, reg);
    if (M2M_SUCCESS != ret) goto ERR1;
ERR1:
    return ret;
}

int8_t cpu_start(void) {
    uint32_t reg = 0;
    int8_t ret;

    /**
    reset regs
    */
    ret = nm_write_reg(BOOTROM_REG,0);
    ret += nm_write_reg(NMI_STATE_REG,0);
    ret += nm_write_reg(NMI_REV_REG,0);
    /**
    Go...
    **/
    ret += nm_read_reg_with_ret(0x1118, &reg);
    reg |= (1 << 0);
    ret += nm_write_reg(0x1118, reg);
    ret += nm_read_reg_with_ret(NMI_GLB_RESET_0, &reg);
    if ((reg & (1ul << 10)) == (1ul << 10)) {
        reg &= ~(1ul << 10);
        ret += nm_write_reg(NMI_GLB_RESET_0, reg);
    }
    reg |= (1ul << 10);
    ret += nm_write_reg(NMI_GLB_RESET_0, reg);
    nm_sleep(1);
    return ret;
}

uint32_t nmi_get_chipid(void)
{
    static uint32_t chipid = 0;

    if (chipid == 0) {
        uint32_t rfrevid;

        if((nm_read_reg_with_ret(0x1000, &chipid)) != M2M_SUCCESS) {
            chipid = 0;
            return 0;
        }
        if((nm_read_reg_with_ret(0x13f4, &rfrevid)) != M2M_SUCCESS) {
            chipid = 0;
            return 0;
        }

        if (chipid == 0x1002a0)  {
            if (rfrevid == 0x1) { /* 1002A0 */
            } else /* if (rfrevid == 0x2) */ { /* 1002A1 */
                chipid = 0x1002a1;
            }
        } else if(chipid == 0x1002b0) {
            if(rfrevid == 3) { /* 1002B0 */
            } else if(rfrevid == 4) { /* 1002B1 */
                chipid = 0x1002b1;
            } else /* if(rfrevid == 5) */ { /* 1002B2 */
                chipid = 0x1002b2;
            }
        }else if(chipid == 0x1000F0) {
            if((nm_read_reg_with_ret(0x3B0000, &chipid)) != M2M_SUCCESS) {
            chipid = 0;
            return 0;
            }
        }else {

        }
//#define PROBE_FLASH
#ifdef PROBE_FLASH
        if(chipid) {
            UWORD32 flashid;

            flashid = probe_spi_flash();
            if(flashid == 0x1230ef) {
                chipid &= ~(0x0f0000);
                chipid |= 0x050000;
            }
            if(flashid == 0xc21320c2) {
                chipid &= ~(0x0f0000);
                chipid |= 0x050000;
            }
        }
#else
        /*M2M is by default have SPI flash*/
        chipid &= ~(0x0f0000);
        chipid |= 0x050000;
#endif /* PROBE_FLASH */
    }
    return chipid;
}

uint32_t nmi_get_rfrevid(void)
{
    uint32_t rfrevid;
    if((nm_read_reg_with_ret(0x13f4, &rfrevid)) != M2M_SUCCESS) {
        rfrevid = 0;
        return 0;
    }
    return rfrevid;
}

void restore_pmu_settings_after_global_reset(void)
{
    /*
    * Must restore PMU register value after
    * global reset if PMU toggle is done at
    * least once since the last hard reset.
    */
    if(REV(nmi_get_chipid()) >= REV_2B0) {
        nm_write_reg(0x1e48, 0xb78469ce);
    }
}

void nmi_update_pll(void)
{
    uint32_t pll;

    pll = nm_read_reg(0x1428);
    pll &= ~0x1ul;
    nm_write_reg(0x1428, pll);
    pll |= 0x1ul;
    nm_write_reg(0x1428, pll);

}
void nmi_set_sys_clk_src_to_xo(void)
{
    uint32_t val32;

    /* Switch system clock source to XO. This will take effect after nmi_update_pll(). */
    val32 = nm_read_reg(0x141c);
    val32 |= (1 << 2);
    nm_write_reg(0x141c, val32);

    /* Do PLL update */
    nmi_update_pll();
}
int8_t chip_sleep(void)
{
    uint32_t reg;
    int8_t ret = M2M_SUCCESS;

    while(1)
    {
        ret = nm_read_reg_with_ret(CORT_HOST_COMM,&reg);
        if(ret != M2M_SUCCESS) goto ERR1;
        if((reg & NBIT0) == 0) break;
    }

    /* Clear bit 1 */
    ret = nm_read_reg_with_ret(WAKE_CLK_REG, &reg);
    if(ret != M2M_SUCCESS)goto ERR1;
    if(reg & NBIT1)
    {
        reg &=~NBIT1;
        ret = nm_write_reg(WAKE_CLK_REG, reg);
        if(ret != M2M_SUCCESS)goto ERR1;
    }

    ret = nm_read_reg_with_ret(HOST_CORT_COMM, &reg);
    if(ret != M2M_SUCCESS)goto ERR1;
    if(reg & NBIT0)
    {
        reg &= ~NBIT0;
        ret = nm_write_reg(HOST_CORT_COMM, reg);
        if(ret != M2M_SUCCESS)goto ERR1;
    }

ERR1:
    return ret;
}
int8_t chip_wake(void)
{
    int8_t ret = M2M_SUCCESS;
    uint32_t clk_status_reg = 0,trials = 0;

    ret = nm_write_reg(HOST_CORT_COMM, NBIT0);
    if(ret != M2M_SUCCESS)goto _WAKE_EXIT;

    ret = nm_write_reg(WAKE_CLK_REG, NBIT1);
    if(ret != M2M_SUCCESS) goto _WAKE_EXIT;

    nm_sleep(3);
    
    do
    {
        ret = nm_read_reg_with_ret(CLOCKS_EN_REG, (uint32_t *)&clk_status_reg);
        if(ret != M2M_SUCCESS) {
            M2M_ERR("Bus error (5).%d %" PRIx32 "\r\n",ret,clk_status_reg);
            goto _WAKE_EXIT;
        }
        if(clk_status_reg & NBIT2) {
            break;
        }
        nm_sleep(2);
        trials++;
        if(trials > WAKUP_TRAILS_TIMEOUT)
        {
            M2M_ERR("Failed to wake up the chip\r\n");
            ret = M2M_ERR_TIME_OUT;
            goto _WAKE_EXIT;
        }
    }while(1);

    /*workaround sometimes SPI fail to read clock regs after reading/writing clockless registers*/
    nm_bus_reset();

_WAKE_EXIT:
    return ret;
}
int8_t cpu_halt(void)
{
    int8_t ret;
    uint32_t reg = 0;
    ret = nm_read_reg_with_ret(0x1118, &reg);
    reg |= (1 << 0);
    ret += nm_write_reg(0x1118, reg);
    ret += nm_read_reg_with_ret(NMI_GLB_RESET_0, &reg);
    if ((reg & (1ul << 10)) == (1ul << 10)) {
        reg &= ~(1ul << 10);
        ret += nm_write_reg(NMI_GLB_RESET_0, reg);
        ret += nm_read_reg_with_ret(NMI_GLB_RESET_0, &reg);
    }
    return ret;
}

int8_t chip_reset(void)
{
    int8_t ret = M2M_SUCCESS;

    ret = nm_write_reg(NMI_GLB_RESET_0, 0);
    nm_sleep(50);
    return ret;
}

int8_t wait_for_bootrom(uint8_t arg)
{
    int8_t ret = M2M_SUCCESS;
    uint32_t reg = 0, cnt = 0;
    uint32_t u32GpReg1 = 0;
    uint32_t u32DriverVerInfo = M2M_MAKE_VERSION_INFO(M2M_RELEASE_VERSION_MAJOR_NO,\
                M2M_RELEASE_VERSION_MINOR_NO, M2M_RELEASE_VERSION_PATCH_NO,\
                M2M_MIN_REQ_DRV_VERSION_MAJOR_NO, M2M_MIN_REQ_DRV_VERSION_MINOR_NO,\
                M2M_MIN_REQ_DRV_VERSION_PATCH_NO);


    reg = 0;
    while(1) {
        reg = nm_read_reg(0x1014);  /* wait for efuse loading done */
        if (reg & 0x80000000) {
            break;
        }
        nm_sleep(1); /* TODO: Why bus error if this delay is not here. */
    }
    reg = nm_read_reg(M2M_WAIT_FOR_HOST_REG);
    reg &= 0x1;

    /* check if waiting for the host will be skipped or not */
    if(reg == 0)
    {
        reg = 0;
        while(reg != M2M_FINISH_BOOT_ROM)
        {
            nm_sleep(1);
            reg = nm_read_reg(BOOTROM_REG);

            if(++cnt > TIMEOUT)
            {
                M2M_DBG("failed to load firmware from flash.\r\n");
                ret = M2M_ERR_INIT;
                goto ERR2;
            }
        }
    }

    if(M2M_WIFI_MODE_ATE_HIGH == arg) {
        nm_write_reg(NMI_REV_REG, M2M_ATE_FW_START_VALUE);
        nm_write_reg(NMI_STATE_REG, NBIT20);
    }else if(M2M_WIFI_MODE_ATE_LOW == arg) {
        nm_write_reg(NMI_REV_REG, M2M_ATE_FW_START_VALUE);
        nm_write_reg(NMI_STATE_REG, 0);
    }else if(M2M_WIFI_MODE_ETHERNET == arg){
        u32GpReg1 = rHAVE_ETHERNET_MODE_BIT;
        nm_write_reg(NMI_STATE_REG, u32DriverVerInfo);
    } else {
        /*bypass this step*/
        nm_write_reg(NMI_STATE_REG, u32DriverVerInfo);
    }

    if(REV(nmi_get_chipid()) >= REV_3A0){
        chip_apply_conf(u32GpReg1 | rHAVE_USE_PMU_BIT);
    } else {
        chip_apply_conf(u32GpReg1);
    }
    M2M_INFO("DriverVerInfo: 0x%08lx\r\n", u32DriverVerInfo);

    nm_write_reg(BOOTROM_REG,M2M_START_FIRMWARE);

#ifdef __ROM_TEST__
    rom_test();
#endif /* __ROM_TEST__ */

ERR2:
    return ret;
}

int8_t wait_for_firmware_start(uint8_t arg)
{
    int8_t ret = M2M_SUCCESS;
    uint32_t reg = 0, cnt = 0;
    uint32_t u32Timeout = TIMEOUT;
    volatile uint32_t regAddress = NMI_STATE_REG;
    volatile uint32_t checkValue = M2M_FINISH_INIT_STATE;

    if((M2M_WIFI_MODE_ATE_HIGH == arg)||(M2M_WIFI_MODE_ATE_LOW == arg)) {
        regAddress = NMI_REV_REG;
        checkValue = M2M_ATE_FW_IS_UP_VALUE;
    } else {
        /*bypass this step*/
    }


    while (checkValue != reg)
    {
        nm_sleep(2); /* TODO: Why bus error if this delay is not here. */
        M2M_DBG("%x %x %x\r\n",(unsigned int)nm_read_reg(0x108c),(unsigned int)nm_read_reg(0x108c),(unsigned int)nm_read_reg(0x14A0));
        reg = nm_read_reg(regAddress);
        if(++cnt >= u32Timeout)
        {
            M2M_DBG("Time out for wait firmware Run\n");
            ret = M2M_ERR_INIT;
            goto ERR;
        }
    }
    if(M2M_FINISH_INIT_STATE == checkValue)
    {
        nm_write_reg(NMI_STATE_REG, 0);
    }
ERR:
    return ret;
}

int8_t chip_deinit(void)
{
    uint32_t reg = 0;
    int8_t ret;

    /**
    stop the firmware, need a re-download
    **/
    ret = nm_read_reg_with_ret(NMI_GLB_RESET_0, &reg);
    if (ret != M2M_SUCCESS) {
        M2M_ERR("failed to de-initialize\r\n");
        goto ERR1;
    }
    reg &= ~(1 << 10);
    ret = nm_write_reg(NMI_GLB_RESET_0, reg);
    if (ret != M2M_SUCCESS) {
        M2M_ERR("failed to de-initialize\r\n");
        goto ERR1;
    }

ERR1:
    return ret;
}

int8_t set_gpio_dir(uint8_t gpio, uint8_t dir)
{
    uint32_t val32;
    int8_t ret;

    ret = nm_read_reg_with_ret(0x20108, &val32);
    if(ret != M2M_SUCCESS) goto _EXIT;

    if(dir) {
        val32 |= (1ul << gpio);
    } else {
        val32 &= ~(1ul << gpio);
    }

    ret = nm_write_reg(0x20108, val32);

_EXIT:
    return ret;
}
int8_t set_gpio_val(uint8_t gpio, uint8_t val)
{
    uint32_t val32;
    int8_t ret;

    ret = nm_read_reg_with_ret(0x20100, &val32);
    if(ret != M2M_SUCCESS) goto _EXIT;

    if(val) {
        val32 |= (1ul << gpio);
    } else {
        val32 &= ~(1ul << gpio);
    }

    ret = nm_write_reg(0x20100, val32);

_EXIT:
    return ret;
}

int8_t get_gpio_val(uint8_t gpio, uint8_t* val)
{
    uint32_t val32;
    int8_t ret;

    ret = nm_read_reg_with_ret(0x20104, &val32);
    if(ret != M2M_SUCCESS) goto _EXIT;

    *val = (uint8_t)((val32 >> gpio) & 0x01);

_EXIT:
    return ret;
}

int8_t pullup_ctrl(uint32_t pinmask, uint8_t enable)
{
    int8_t s8Ret;
    uint32_t val32;
    s8Ret = nm_read_reg_with_ret(0x142c, &val32);
    if(s8Ret != M2M_SUCCESS) {
        M2M_ERR("[pullup_ctrl]: failed to read\r\n");
        goto _EXIT;
    }
    if(enable) {
        val32 &= ~pinmask;
        } else {
        val32 |= pinmask;
    }
    s8Ret = nm_write_reg(0x142c, val32);
    if(s8Ret  != M2M_SUCCESS) {
        M2M_ERR("[pullup_ctrl]: failed to write\r\n");
        goto _EXIT;
    }
_EXIT:
    return s8Ret;
}

int8_t nmi_get_otp_mac_address(uint8_t *pu8MacAddr,  uint8_t * pu8IsValid)
{
    int8_t ret;
    uint32_t    u32RegValue;
    uint8_t mac[6];
    tstrGpRegs strgp = {0};

    ret = nm_read_reg_with_ret(rNMI_GP_REG_2, &u32RegValue);
    if(ret != M2M_SUCCESS) goto _EXIT_ERR;

    ret = nm_read_block(u32RegValue|0x30000,(uint8_t*)&strgp,sizeof(tstrGpRegs));
    if(ret != M2M_SUCCESS) goto _EXIT_ERR;
    u32RegValue = strgp.u32Mac_efuse_mib;

    if(!EFUSED_MAC(u32RegValue)) {
        M2M_DBG("Default MAC\r\n");
        memset(pu8MacAddr, 0, 6);
        goto _EXIT_ERR;
    }

    M2M_DBG("OTP MAC\r\n");
    u32RegValue >>= 16;
    ret = nm_read_block(u32RegValue|0x30000, mac, 6);
    memcpy(pu8MacAddr,mac,6);
    if(pu8IsValid) *pu8IsValid = 1;
    return ret;

_EXIT_ERR:
    if(pu8IsValid) *pu8IsValid = 0;
    return ret;
}

int8_t nmi_get_mac_address(uint8_t *pu8MacAddr)
{
    int8_t ret;
    uint32_t    u32RegValue;
    uint8_t mac[6];
    tstrGpRegs strgp = {0};

    ret = nm_read_reg_with_ret(rNMI_GP_REG_2, &u32RegValue);
    if(ret != M2M_SUCCESS) goto _EXIT_ERR;

    ret = nm_read_block(u32RegValue|0x30000,(uint8_t*)&strgp,sizeof(tstrGpRegs));
    if(ret != M2M_SUCCESS) goto _EXIT_ERR;
    u32RegValue = strgp.u32Mac_efuse_mib;

    u32RegValue &=0x0000ffff;
    ret = nm_read_block(u32RegValue|0x30000, mac, 6);
    memcpy(pu8MacAddr, mac, 6);

    return ret;

_EXIT_ERR:
    return ret;
}

//DOM-IGNORE-END
