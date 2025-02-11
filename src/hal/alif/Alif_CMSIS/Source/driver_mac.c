/* Copyright (C) 2022 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

/**************************************************************************//**
 * @file     driver_mac.c
 * @author   Silesh C V
 * @email    silesh@alifsemi.com
 * @version  V1.0.0
 * @date     22-Mar-2022
 * @brief    CMSIS driver for ETH MAC.
 * @bug      None.
 * @Note     None
 ******************************************************************************/

#include "driver_mac.h"
#include "sys_ctrl_eth.h"

static MAC_DEV MAC0 = {
    .regs = (volatile MAC_REGS *) ETH_BASE,
    .flags  = 0,
    .cb_event = NULL,
    .frame_end = NULL,
    .irq = (IRQn_Type) ETH_SBD_IRQ_IRQn,
    .irq_priority = RTE_ETH_MAC_IRQ_PRIORITY,
};

/* area for descriptors */
static DMA_DESC dma_descs[RX_DESC_COUNT + TX_DESC_COUNT]__attribute__((section("eth_buf"))) __attribute__((aligned(16)));
static uint32_t rx_buffers[RX_DESC_COUNT][ETH_BUF_SIZE >> 2]__attribute__((section("eth_buf")));
static uint32_t tx_buffers[TX_DESC_COUNT][ETH_BUF_SIZE >> 2]__attribute__((section("eth_buf")));

#define ARM_ETH_MAC_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0) /* driver version */

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_ETH_MAC_API_VERSION,
    ARM_ETH_MAC_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_ETH_MAC_CAPABILITIES DriverCapabilities = {
    0,        /* IPv4 header checksum verified on receive */
    0,        /* IPv6 checksum verification supported on receive */
    0,        /* UDP payload checksum verified on receive */
    0,        /* TCP payload checksum verified on receive */
    0,        /* ICMP payload checksum verified on receive */
    0,        /* IPv4 header checksum generated on transmit */
    0,        /* IPv6 checksum generation supported on transmit */
    0,        /* UDP payload checksum generated on transmit */
    0,        /* TCP payload checksum generated on transmit */
    0,        /* ICMP payload checksum generated on transmit */
    ARM_ETH_INTERFACE_RMII,   /* Ethernet Media Interface type */
    0,        /* driver provides initial valid MAC address */
    1,        /* callback event \ref ARM_ETH_MAC_EVENT_RX_FRAME generated */
    1,        /* callback event \ref ARM_ETH_MAC_EVENT_TX_FRAME generated */
    0,        /* wakeup event \ref ARM_ETH_MAC_EVENT_WAKEUP generated */
    0,        /* Precision Timer supported */
    0       /* Reserved (must be zero) */
};

/**
  \fn          void setup_rxdesc(MAC_DEV *dev, uint32_t desc_id)
  \brief       Setup a single Rx DMA descriptor.
  \param[in]   dev        Pointer to the MAC device instance
  \param[in]   desc_id    The descriptor id to setup
  \return      none.
*/
static void setup_rxdesc(MAC_DEV *dev, uint32_t desc_id)
{
    DMA_DESC *desc = &dev->rx_descs[desc_id];

    SCB_InvalidateDCache_by_Addr((uint32_t *)desc, sizeof(DMA_DESC));

    desc->des0 = (uint32_t) LocalToGlobal(&rx_buffers[desc_id][0]);
    desc->des3 = RDES3_OWN | RDES3_INT_ON_COMPLETION_EN |
                    RDES3_BUFFER1_VALID_ADDR;

    SCB_CleanDCache_by_Addr((uint32_t *) desc, sizeof(DMA_DESC));
}

/**
  \fn          void init_rx_descs(MAC_DEV *dev)
  \brief       Initialize Rx DMA descriptors.
  \param[in]   dev        Pointer to the MAC device instance
  \return      none.
*/
static void init_rx_descs(MAC_DEV *dev)
{
    uint32_t i, last_rx_desc;

    for (i = 0; i < RX_DESC_COUNT; i++)
        setup_rxdesc(dev, i);

    dev->regs->DMA_CH0_RX_BASE_ADDR = (uint32_t) LocalToGlobal(dev->rx_descs);
    dev->regs->DMA_CH0_RX_RING_LEN = RX_DESC_COUNT - 1;

    last_rx_desc = (uint32_t) LocalToGlobal(&(dev->rx_descs[RX_DESC_COUNT - 1]));

    dev->regs->DMA_CH0_RX_END_ADDR = last_rx_desc;
}

/**
  \fn          void init_tx_desc (MAC_DEV *dev)
  \brief       Initialize Tx DMA descriptors.
  \param[in]   dev        Pointer to the MAC device instance
  \return      none.
*/
static void init_tx_descs(MAC_DEV *dev)
{
    uint32_t i;

    for (i = 0; i < TX_DESC_COUNT; i++)
        dev->tx_descs[i] = (DMA_DESC) {0, 0, 0, 0};

    dev->regs->DMA_CH0_TX_BASE_ADDR = (uint32_t) LocalToGlobal(dev->tx_descs);
    dev->regs->DMA_CHO_TX_RING_LEN = TX_DESC_COUNT - 1;

    SCB_CleanDCache_by_Addr((uint32_t *) dev->tx_descs,
                                     TX_DESC_COUNT * sizeof(DMA_DESC));
}

/**
  \fn          void init_descriptors(MAC_DEV *dev)
  \brief       Initialize DMA descriptors.
  \param[in]   dev        Pointer to the MAC device instance
  \return      none.
*/
static void init_descriptors(MAC_DEV *dev)
{
    dev->descs = dma_descs;
    dev->tx_descs = (DMA_DESC *) dev->descs;
    dev->rx_descs = (dev->tx_descs + TX_DESC_COUNT);

    init_rx_descs(dev);
    init_tx_descs(dev);
}

/**
  \fn          static int32_t mac_hw_init(MAC_DEV *dev)
  \brief       Initialize the MAC hardware.
  \param[in]   dev        Pointer to the MAC device instance
  \return      \ref execution_status.
*/
static int32_t mac_hw_init(MAC_DEV *dev)
{
    uint32_t val;
    uint32_t timeout = 100;
#ifdef MTL_HAS_MULTIPLE_QUEUES
    uint32_t tx_fifo_sz, rx_fifo_sz, tqs, rqs;
#endif

    /* Soft reset the logic */
    dev->regs->DMA_BUS_MODE |= DMA_BUS_MODE_SFT_RESET;

    do {
        osDelay(1);
        timeout--;
    } while ((dev->regs->DMA_BUS_MODE & DMA_BUS_MODE_SFT_RESET) && timeout);

    if (!timeout)
        return ARM_DRIVER_ERROR;

    /* Configure MTL Tx Q0 Operating mode */
    dev->regs->MTL_TXQ0_OP_MODE &= ~MTL_OP_MODE_TXQEN_MASK;
    dev->regs->MTL_TXQ0_OP_MODE |= MTL_OP_MODE_TXQEN | MTL_OP_MODE_TSF;

    /* Configure MTL RX Q0 operating mode */
    dev->regs->MTL_RXQ0_OP_MODE |= (MTL_OP_MODE_RSF |
                                    MTL_OP_MODE_FEP |
                                    MTL_OP_MODE_FUP);

#ifdef MTL_HAS_MULTIPLE_QUEUES
    /*
     *  configure the MTL T/Rx Q0 sizes by finding out the configured
     * Tx RX FIFO sizes. Note that this is not needed if the IP is
     * configured to have only one queue (each for Tx and Rx).
     */
    val = dev->regs->MAC_HW_FEATURE_1;

    tx_fifo_sz = (val >> MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT) &
                    MAC_HW_FEATURE1_TXFIFOSIZE_MASK;

    rx_fifo_sz = (val >> MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT) &
                    MAC_HW_FEATURE1_RXFIFOSIZE_MASK;

    /*
     * FIFO sizes are encoded as log2(fifo_size) - 7 in the above field
     * and tqs and rqs needs to be programmed in blocks of 256 bytes.
     */
    tqs = (128 << tx_fifo_sz) / 256 - 1;
    rqs = (128 << rx_fifo_sz) / 256 - 1;

    dev->regs->MTL_TXQ0_OP_MODE &= ~(MTL_TXQ0_OPERATION_MODE_TQS_MASK <<
                                     MTL_TXQ0_OPERATION_MODE_TQS_SHIFT);
    dev->regs->MTL_TXQ0_OP_MODE |= (tqs <<  MTL_TXQ0_OPERATION_MODE_TQS_SHIFT);

    dev->regs->MTL_RXQ0_OP_MODE &= ~(MTL_RXQ0_OPERATION_MODE_RQS_MASK <<
                                     MTL_TXQ0_OPERATION_MODE_TQS_SHIFT);
    dev->regs->MTL_RXQ0_OP_MODE |= (rqs << MTL_TXQ0_OPERATION_MODE_TQS_SHIFT);
#endif

    /* Enable MAC RXQ */
    dev->regs->MAC_RXQ_CTRL_0 &= ~(MAC_RXQ_CTRL0_RXQ0EN_MASK <<
                                    MAC_RXQ_CTRL0_RXQ0EN_SHIFT);
    dev->regs->MAC_RXQ_CTRL_0 |= (MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB <<
                                    MAC_RXQ_CTRL0_RXQ0EN_SHIFT);

    /* Configure RXQ control1 for routing multicast/broadcast queues
     * Note that by default, Q0 will be used.
     */
    dev->regs->MAC_RXQ_CTRL_1 |= MAC_RXQCTRL_MCBCQEN;

    /* Configure tx and rx flow control */
    dev->regs->MAC_Q0_TX_FLOW_CTRL |= 0xffff << MAC_Q0_TX_FLOW_CTRL_PT_SHIFT |
                                      MAC_Q0_TX_FLOW_CTRL_TFE;
    dev->regs->MAC_RX_FLOW_CTRL |= MAC_RX_FLOW_CTRL_RFE;

    dev->regs->MAC_CONFIG &= ~MAC_CONFIG_JE;
    dev->regs->MAC_CONFIG |= (MAC_CONFIG_FES | MAC_CONFIG_DM);

    dev->regs->MAC_PACKET_FILTER |= MAC_PACKET_FILTER_PM;

    /* Configure the DMA block */
    dev->regs->DMA_CH0_TX_CTRL |= (16 << DMA_CH0_TX_CONTROL_TXPBL_SHIFT);

    dev->regs->DMA_CH0_RX_CTRL |= ((16 << DMA_CH0_RX_CONTROL_RXPBL_SHIFT) |
                                   (2048 << DMA_CH0_RX_CONTROL_RBSZ_SHIFT));

    val = dev->regs->DMA_SYS_BUS_MODE;
    val |= DMA_SYSBUS_MODE_BLEN4 | DMA_SYSBUS_MODE_BLEN8 |
                DMA_SYSBUS_MODE_BLEN16;
    val |= 3 << DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT;
    val |= 1 << DMA_SYSBUS_MODE_WR_OSR_LMT_SHIFT;
    val |= DMA_SYSBUS_MODE_ONEKBBE;
    dev->regs->DMA_SYS_BUS_MODE = val;

    init_descriptors(dev);

    /* Enable DMA Channel 0 interrupts, rx and tx */
    dev->regs->DMA_CH0_INT_ENABLE |= (DMA_CHAN_INTR_ENA_RIE |
                                      DMA_CHAN_INTR_ENA_NIE |
                                      DMA_CHAN_INTR_ENA_TIE);

    dev->regs->DMA_CH0_TX_CTRL |= DMA_CH0_TX_CONTROL_ST;
    dev->regs->DMA_CH0_RX_CTRL |= DMA_CH0_RX_CONTROL_SR;

    dev->regs->MAC_CONFIG |= (MAC_CONFIG_RE | MAC_CONFIG_TE);

    return 0;
}

/**
  \fn          uint32_t crc32_8bit_rev (uint32_t crc32, uint8_t val)
  \brief       Calculate 32-bit CRC (Polynom: 0x04C11DB7, data bit-reversed).
  \param[in]   crc32  CRC initial value
  \param[in]   val    Input value
  \return      Calculated CRC value
*/
static uint32_t crc32_8bit_rev (uint32_t crc32, uint8_t val)
{
    uint32_t n;

    crc32 ^= __RBIT (val);
    for (n = 8; n; n--) {
        if (crc32 & 0x80000000) {
            crc32 <<= 1;
            crc32  ^= 0x04C11DB7;
        } else {
            crc32 <<= 1;
        }
    }
    return crc32;
}

/**
  \fn          uint32_t crc32_data (const uint8_t *data, uint32_t len)
  \brief       Calculate standard 32-bit Ethernet CRC.
  \param[in]   data  Pointer to buffer containing the data
  \param[in]   len   Data length in bytes
  \return      Calculated CRC value
*/
static uint32_t crc32_data (const uint8_t *data, uint32_t len)
{
    uint32_t crc;

    for (crc = 0xFFFFFFFF; len; len--) {
        crc = crc32_8bit_rev (crc, *data++);
    }
    return (crc ^ 0xFFFFFFFF);
}

/* Ethernet Driver functions */

/**
  \fn          ARM_DRIVER_VERSION GetVersion (void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
static ARM_DRIVER_VERSION ETH_MAC_GetVersion(void)
{
    return DriverVersion;
}

/**
  \fn          ARM_ETH_MAC_CAPABILITIES GetCapabilities (void)
  \brief       Get driver capabilities.
  \return      \ref ARM_ETH_MAC_CAPABILITIES
*/
static ARM_ETH_MAC_CAPABILITIES ETH_MAC_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
  \fn          int32_t Initialize (ARM_ETH_MAC_SignalEvent_t cb_event, MAC_DEV *dev)
  \brief       Initialize Ethernet MAC Device.
  \param[in]   cb_event  Pointer to \ref ARM_ETH_MAC_SignalEvent
  \param[in]   dev       Pointer to the MAC device instance
  \return      \ref execution_status
*/
static int32_t Initialize(ARM_ETH_MAC_SignalEvent_t cb_event, MAC_DEV *dev)
{
    if (!cb_event)
        return ARM_DRIVER_ERROR_PARAMETER;

    dev->flags |=  ETH_INIT;
    dev->cb_event = cb_event;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t Uninitialize (MAC_DEV *dev)
  \brief       De-initialize Ethernet MAC Device.
  \param[in]   dev       Pointer to the MAC device instance
  \return      \ref execution_status
*/
static int32_t Uninitialize(MAC_DEV *dev)
{
    dev->flags &= ~ETH_INIT;
    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t PowerControl (ARM_POWER_STATE state, MAC_DEV *dev)
  \brief       Control Ethernet MAC Device Power.
  \param[in]   state  Power state
  \param[in]   dev       Pointer to the MAC device instance
  \return      \ref execution_status
*/
static int32_t PowerControl(ARM_POWER_STATE state, MAC_DEV *dev)
{
    int32_t ret, val;

    switch (state) {
    case ARM_POWER_OFF:
        /* Disable Ethernet peripheral clock */
        disable_eth_periph_clk();

        NVIC_DisableIRQ(dev->irq);

        dev->flags &= ~ETH_POWER;
        break;

    case ARM_POWER_LOW:
        break;

    case ARM_POWER_FULL:
        if (!(dev->flags & ETH_INIT))
            return ARM_DRIVER_ERROR;

        if (dev->flags & ETH_POWER)
            return ARM_DRIVER_OK;

        /* Enable Ethernet peripheral clock */
        enable_eth_periph_clk();

        ret = mac_hw_init(dev);

        if (ret < 0)
            return ARM_DRIVER_ERROR;

        NVIC_ClearPendingIRQ(dev->irq);
        NVIC_SetPriority(dev->irq, dev->irq_priority);
        NVIC_EnableIRQ(dev->irq);

        dev->flags |= ETH_POWER;
        break;
    default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t GetMacAddress (ARM_ETH_MAC_ADDR *ptr_addr, MAC_DEV *dev)
  \brief       Get Ethernet MAC Address.
  \param[in]   ptr_addr  Pointer to address
  \param[in]   dev       Pointer to the MAC device instance
  \return      \ref execution_status
*/
static int32_t GetMacAddress(ARM_ETH_MAC_ADDR *ptr_addr, MAC_DEV *dev)
{
    uint32_t hi, lo;

    if (!ptr_addr)
        return ARM_DRIVER_ERROR_PARAMETER;

    if (!(dev->flags & ETH_POWER))
        return ARM_DRIVER_ERROR;

    lo = dev->regs->MAC_ADDR_LOW_0;
    hi = dev->regs->MAC_ADDR_HIGH_0 & 0xFFFF;

    ptr_addr->b[0] = (lo >>  0) & 0xff;
    ptr_addr->b[1] = (lo >>  8) & 0xff;
    ptr_addr->b[2] = (lo >> 16) & 0xff;
    ptr_addr->b[3] = (lo >> 24) & 0xff;
    ptr_addr->b[4] = (hi >>  0) & 0xff;
    ptr_addr->b[5] = (hi >>  8) & 0xff;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SetMacAddress (const ARM_ETH_MAC_ADDR *ptr_addr, MAC_DEV *dev)
  \brief       Set Ethernet MAC Address.
  \param[in]   ptr_addr  Pointer to address
  \param[in]   dev       Pointer to MAC device instance
  \return      \ref execution_status
*/
static int32_t SetMacAddress(const ARM_ETH_MAC_ADDR *ptr_addr, MAC_DEV *dev)
{
    uint32_t hi, lo;

    if (!ptr_addr)
        return ARM_DRIVER_ERROR_PARAMETER;

    if (!(dev->flags & ETH_POWER))
        return ARM_DRIVER_ERROR;

    hi = ((ptr_addr->b[5] <<  8) |  ptr_addr->b[4]) | MAC_ADDR_HIGH_AE;

    lo = (ptr_addr->b[3] << 24) | (ptr_addr->b[2] << 16) |
         (ptr_addr->b[1] <<  8) |  ptr_addr->b[0];

    dev->regs->MAC_ADDR_LOW_0 = lo;
    dev->regs->MAC_ADDR_HIGH_0 = hi;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SetAddressFilter (const ARM_ETH_MAC_ADDR *ptr_addr,
                                               uint32_t          num_addr)
  \brief       Configure Address Filter.
  \param[in]   ptr_addr  Pointer to addresses
  \param[in]   num_addr  Number of addresses to configure
  \param[in]   dev       Pointer to MAC device instance
  \return      \ref execution_status
*/
static int32_t SetAddressFilter(const ARM_ETH_MAC_ADDR *ptr_addr, uint32_t num_addr,
    MAC_DEV *dev)
{
    uint32_t crc;

    if (!ptr_addr)
        return ARM_DRIVER_ERROR_PARAMETER;

    if (!(dev->flags & ETH_POWER))
        return ARM_DRIVER_ERROR;

    dev->regs->MAC_PACKET_FILTER &= ~(MAC_PACKET_FILTER_HPF | MAC_PACKET_FILTER_HMC);

    dev->regs->MAC_HASH_TAB_0 = 0x0;
    dev->regs->MAC_HASH_TAB_1 = 0x0;

    if (num_addr == 0) {
        return ARM_DRIVER_OK;
    }

    /* Calculate 64-bit Hash table for MAC addresses */
    for ( ; num_addr; ptr_addr++, num_addr--) {
        crc = crc32_data (&ptr_addr->b[0], 6) >> 26;
        if (crc & 0x20) {
            dev->regs->MAC_HASH_TAB_1 |= (1 << (crc & 0x1F));
        } else {
            dev->regs->MAC_HASH_TAB_0 |= (1 << crc);
        }
    }
    /* Enable perfect and hash address filtering */
    dev->regs->MAC_PACKET_FILTER |= (MAC_PACKET_FILTER_HPF | MAC_PACKET_FILTER_HMC);

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SendFrame (const uint8_t *frame, uint32_t len, uint32_t flags,
                                    MAC_DEV *dev)
  \brief       Send Ethernet frame.
  \param[in]   frame  Pointer to frame buffer with data to send
  \param[in]   len    Frame buffer length in bytes
  \param[in]   flags  Frame transmit flags (see ARM_ETH_MAC_TX_FRAME_...)
  \param[in]   dev    Pointer to the MAC device instance
  \return      \ref execution_status
*/
static int32_t SendFrame(const uint8_t *frame, uint32_t len, uint32_t flags,
    MAC_DEV *dev)
{
    uint32_t cur_idx;
    uint8_t *dst;
    uint32_t buffer_len = len;
    DMA_DESC *desc;

    if (!frame || !len)
        return ARM_DRIVER_ERROR_PARAMETER;

    if (!(dev->flags & ETH_POWER))
        return ARM_DRIVER_ERROR;

    cur_idx = dev->tx_desc_id;
    desc = &dev->tx_descs[cur_idx];

    SCB_InvalidateDCache_by_Addr(desc, sizeof(DMA_DESC));

    dst = dev->frame_end;

    if (dst == NULL) {
        /* new frame */
        if (desc->des3 & TDES3_OWN) {
            return ARM_DRIVER_ERROR_BUSY;
        }
        dst = (uint8_t *) &tx_buffers[cur_idx][0];
        desc->des2 = TDES2_INTERRUPT_ON_COMPLETION | buffer_len;
    } else {
        /* frame fragment */
        desc->des2 += len;
    }

    /* Copy the frame to the buffer */
    for ( ; len > 7; dst += 8, frame += 8, len -= 8) {
        ((uint32_t *) dst)[0] = ((uint32_t *) frame)[0];
        ((uint32_t *) dst)[1] = ((uint32_t *) frame)[1];
    }
    /* Copy remaining 7 bytes */
    for ( ; len > 1; dst += 2, frame += 2, len -= 2)
        ((uint16_t *) dst)[0] = ((uint16_t *) frame)[0];

    if (len > 0)
        dst++[0] = frame++[0];

    SCB_CleanDCache_by_Addr((uint32_t *) &tx_buffers[cur_idx][0], ETH_BUF_SIZE);

    if (flags & ARM_ETH_MAC_TX_FRAME_FRAGMENT) {
        /* More data to come, remember current write position */
        dev->frame_end = dst;
        return ARM_DRIVER_OK;
    }

    dev->tx_desc_id++;
    dev->tx_desc_id %= TX_DESC_COUNT;
    desc->des0 = (uint32_t)LocalToGlobal(&tx_buffers[cur_idx][0]);

    dev->frame_end = NULL;

    desc->des3 = TDES3_OWN | TDES3_LAST_DESCRIPTOR | TDES3_FIRST_DESCRIPTOR |
                    buffer_len;

    SCB_CleanDCache_by_Addr((uint32_t *) desc, sizeof(DMA_DESC));

    dev->regs->DMA_CH0_TX_END_ADDR = (uint32_t) LocalToGlobal(&(dev->tx_descs[dev->tx_desc_id]));

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t ReadFrame (uint8_t *frame, uint32_t len, MAC_DEV *dev)
  \brief       Read data of received Ethernet frame.
  \param[in]   frame  Pointer to frame buffer for data to read into
  \param[in]   len    Frame buffer length in bytes
  \param[in]   dev    Pointer to the MAC device instance
  \return      number of data bytes read or execution status
                 - value >= 0: number of data bytes read
                 - value < 0: error occurred, value is execution status as defined with \ref execution_status
*/
static int32_t ReadFrame(uint8_t *frame, uint32_t len, MAC_DEV *dev)
{
    uint32_t cur_idx;
    int32_t buffer_len = (int32_t) len;
    uint8_t *src;

    if (!frame || !len)
        return ARM_DRIVER_ERROR_PARAMETER;

    if (!(dev->flags & ETH_POWER))
        return ARM_DRIVER_ERROR;

    cur_idx = dev->rx_desc_id;
    src = (uint8_t *) &rx_buffers[cur_idx][0];

    SCB_InvalidateDCache_by_Addr(src, ETH_BUF_SIZE);

    /* copy data to the buffer */
    for ( ; len > 7; frame += 8, src += 8, len -= 8) {
        ((uint32_t *) frame)[0] = ((uint32_t *) src)[0];
        ((uint32_t *) frame)[1] = ((uint32_t *) src)[1];
    }

    /* copy remaining 7 bytes */
    for ( ; len > 1; frame += 2, src += 2, len -= 2)
        ((uint16_t *) frame)[0] = ((uint16_t *)src)[0];

    if (len > 0)
        frame[0] = src[0];

    /* refresh the descriptor */
    setup_rxdesc(dev, cur_idx);

    dev->regs->DMA_CH0_RX_END_ADDR = (uint32_t) LocalToGlobal(&(dev->rx_descs[cur_idx]));

    dev->rx_desc_id++;
    dev->rx_desc_id %= RX_DESC_COUNT;

    return buffer_len;
}

/**
  \fn          uint32_t GetRxFrameSize (MAC_DEV *dev)
  \param[in]   dev    Pointer to the MAC device instance
  \brief       Get size of received Ethernet frame.
  \return      number of bytes in received frame
*/
static uint32_t GetRxFrameSize(MAC_DEV *dev)
{
    DMA_DESC *desc;

    if (!(dev->flags & ETH_POWER))
        return 0;

    desc = &dev->rx_descs[dev->rx_desc_id];

    SCB_InvalidateDCache_by_Addr(desc, sizeof(DMA_DESC));

    if (desc->des3 & RDES3_OWN)
        return 0;

    return (desc->des3 & 0x7fff) - 4;
}

/**
  \fn          int32_t GetRxFrameTime (ARM_ETH_MAC_TIME *time, MAC_DEV *dev)
  \brief       Get time of received Ethernet frame.
  \param[in]   time  Pointer to time structure for data to read into
  \param[in]   dev    Pointer to the MAC device instance
  \return      \ref execution_status
*/
static int32_t GetRxFrameTime(ARM_ETH_MAC_TIME *time, MAC_DEV *dev)
{
    DMA_DESC *desc, *next_desc;
    uint32_t cur_idx = dev->rx_desc_id, next_idx;
    uint32_t own, ctxt;

    if (!time)
        return ARM_DRIVER_ERROR_PARAMETER;

    if (!(dev->flags & ETH_POWER))
        return ARM_DRIVER_ERROR;

    desc = &dev->rx_descs[cur_idx];
    next_idx = (cur_idx + 1) % RX_DESC_COUNT;
    next_desc = &dev->rx_descs[next_idx];

    SCB_InvalidateDCache_by_Addr(desc, sizeof(DMA_DESC));

    if (desc->des3 & RDES3_OWN)
        return ARM_DRIVER_ERROR_BUSY;

    /* get preliminary status from current normal w/b descriptor */
    if (!(desc->des3 & RDES3_RDES1_VALID))
        return ARM_DRIVER_ERROR;

    if (!(desc->des1 & RDES1_TIMESTAMP_AVAILABLE))
        return ARM_DRIVER_ERROR;

    /* OK, move on to the context descriptor */
    own = next_desc->des3 & RDES3_OWN;
    ctxt = ((next_desc->des3 & RDES3_CONTEXT_DESCRIPTOR)
                >> RDES3_CONTEXT_DESCRIPTOR_SHIFT);

    if (!own && ctxt) {
        if ((next_desc->des0 == 0xffffffff) && (next_desc->des1 == 0xffffffff)) {
            /* Corrupted timestamp */
            return ARM_DRIVER_ERROR;
        }

        time->ns = next_desc->des0;
        time->sec = next_desc->des1;
        return ARM_DRIVER_OK;
    }

    return ARM_DRIVER_ERROR;
}

/**
  \fn          int32_t GetTxFrameTime (ARM_ETH_MAC_TIME *time, MAC_DEV *dev)
  \brief       Get time of transmitted Ethernet frame.
  \param[in]   time  Pointer to time structure for data to read into
  \param[in]   dev   Pointer to the MAC device instance
  \return      \ref execution_status
*/
static int32_t GetTxFrameTime(ARM_ETH_MAC_TIME *time, MAC_DEV *dev)
{
    DMA_DESC *desc;

    if (!time)
        return ARM_DRIVER_ERROR_PARAMETER;

    if (!(dev->flags & ETH_POWER))
        return ARM_DRIVER_ERROR;

    desc = &dev->tx_descs[dev->tx_desc_id];

    SCB_InvalidateDCache_by_Addr(desc, sizeof(DMA_DESC));

    if (desc->des3 & TDES3_OWN)
        return ARM_DRIVER_ERROR_BUSY;

    if (desc->des3 & TDES3_CONTEXT_TYPE)
        return ARM_DRIVER_ERROR;

    if (desc->des3 & TDES3_TIMESTAMP_STATUS) {
        time->ns = desc->des0;
        time->sec = desc->des1;
        return ARM_DRIVER_OK;
    }

    return ARM_DRIVER_ERROR;
}

/**
  \fn          int32_t Control (uint32_t control, uint32_t arg, MAC_DEV *dev)
  \brief       Control Ethernet Interface.
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \param[in]   dev      Pointer to the MAC device instance
  \return      \ref execution_status
*/
static int32_t Control(uint32_t control, uint32_t arg, MAC_DEV *dev)
{
    uint32_t val, reg;

    if (!(dev->flags & ETH_POWER))
        return ARM_DRIVER_ERROR;

    switch (control) {
    case ARM_ETH_MAC_CONFIGURE:
        val = dev->regs->MAC_CONFIG &
              ~(MAC_CONFIG_FES |
              MAC_CONFIG_LM |
              MAC_CONFIG_DM);

        switch (arg & ARM_ETH_MAC_SPEED_Msk) {
        case ARM_ETH_MAC_SPEED_10M:
            /* Nothing to do here as FES is already cleared */
            break;
        case ARM_ETH_SPEED_100M:
            val |= MAC_CONFIG_FES;
            break;
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        }

        switch (arg & ARM_ETH_MAC_DUPLEX_Msk) {
        case ARM_ETH_MAC_DUPLEX_FULL:
            val |= MAC_CONFIG_DM;
            break;
        }

        if (arg & ARM_ETH_MAC_LOOPBACK)
            val |= MAC_CONFIG_LM;

        if ((arg & ARM_ETH_MAC_CHECKSUM_OFFLOAD_RX) ||
                    (arg & ARM_ETH_MAC_CHECKSUM_OFFLOAD_TX))
            return ARM_DRIVER_ERROR_UNSUPPORTED;

        dev->regs->MAC_CONFIG = val;

        val = (dev->regs->MAC_PACKET_FILTER) & ~(MAC_PACKET_FILTER_PR |
                MAC_PACKET_FILTER_PM |
                MAC_PACKET_FILTER_DBF);

        /* Disable broadcast frame reception */
        if (!(arg & ARM_ETH_MAC_ADDRESS_BROADCAST))
            val |= MAC_PACKET_FILTER_DBF;

        /* Enable multicast frame reception */
        if (arg & ARM_ETH_MAC_ADDRESS_MULTICAST)
            val |= MAC_PACKET_FILTER_PM;

        /* Promiscuous mode */
        if (arg & ARM_ETH_MAC_ADDRESS_ALL)
            val |= MAC_PACKET_FILTER_PR;

        dev->regs->MAC_PACKET_FILTER = val;
        break;

    case ARM_ETH_MAC_CONTROL_TX:
        val = dev->regs->MAC_CONFIG;
        reg = dev->regs->DMA_CH0_TX_CTRL;
        if (arg != 0) {
            val |= MAC_CONFIG_TE;
            reg |= DMA_CONTROL_ST;
            dev->regs->MAC_CONFIG = val;
            dev->regs->DMA_CH0_TX_CTRL = reg;
        } else {
            reg &= ~DMA_CONTROL_ST;
            val &= ~MAC_CONFIG_TE;
            dev->regs->DMA_CH0_TX_CTRL = reg;
            dev->regs->MAC_CONFIG = val;
        }
        break;

    case ARM_ETH_MAC_CONTROL_RX:
        val = dev->regs->MAC_CONFIG;
        reg = dev->regs->DMA_CH0_RX_CTRL;
        if (arg != 0) {
            val |= MAC_CONFIG_RE;
            reg |= DMA_CONTROL_SR;
            dev->regs->MAC_CONFIG = val;
            dev->regs->DMA_CH0_RX_CTRL = reg;
        } else {
            reg &= ~DMA_CONTROL_SR;
            val &= ~MAC_CONFIG_RE;
            dev->regs->DMA_CH0_RX_CTRL = reg;
            dev->regs->MAC_CONFIG = val;
        }
        break;

    case ARM_ETH_MAC_FLUSH:
        if (arg & ARM_ETH_MAC_FLUSH_RX) {
            val = dev->regs->DMA_CH0_RX_CTRL;
            reg = val;
            val &= ~DMA_CONTROL_SR;
            dev->regs->DMA_CH0_RX_CTRL = val;
            init_rx_descs(dev);
            dev->regs->DMA_CH0_RX_CTRL = reg;
        }
        if (arg & ARM_ETH_MAC_FLUSH_TX) {
            val = dev->regs->DMA_CH0_TX_CTRL;
            reg = val;
            val &= ~DMA_CONTROL_ST;
            dev->regs->DMA_CH0_TX_CTRL = val;
            init_tx_descs(dev);
            dev->regs->DMA_CH0_TX_CTRL = reg;
        }
        break;

    case ARM_ETH_MAC_SLEEP:
        if (arg != 0) {
            dev->regs->DMA_CH0_TX_CTRL &= ~DMA_CONTROL_ST;
            dev->regs->MAC_CONFIG &= ~(MAC_CONFIG_TE | MAC_CONFIG_RE);
            dev->regs->MAC_INT_ENABLE |= MAC_INT_EN_PMTIE;
            dev->regs->MAC_CONFIG |= MAC_CONFIG_RE;
            dev->regs->MAC_PMT_CTRL_STS |= (MAC_PMT_CTRL_STS_MGKPKTEN | MAC_PMT_CTRL_STS_PWRDWN);
        } else {
            dev->regs->MAC_INT_ENABLE &= ~MAC_INT_EN_PMTIE;
            dev->regs->MAC_PMT_CTRL_STS = 0x0;
        }
        break;
  default:
        return ARM_DRIVER_ERROR_UNSUPPORTED;
  }
    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t ControlTimer (uint32_t control, ARM_ETH_MAC_TIME *time, MAC_DEV *dev)
  \brief       Control Precision Timer.
  \param[in]   control  Operation
  \param[in]   time     Pointer to time structure
  \param[in]   dev      Pointer to the MAC device instance
  \return      \ref ARM_DRIVER_ERROR_UNSUPPORTED
*/
static int32_t ControlTimer(uint32_t control, ARM_ETH_MAC_TIME *time, MAC_DEV *dev)
{
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

/**
  \fn          int32_t PHY_Read (uint8_t phy_addr, uint8_t reg_addr, uint16_t *data, MAC_DEV *dev)
  \brief       Read Ethernet PHY Register through Management Interface.
  \param[in]   phy_addr  5-bit device address
  \param[in]   reg_addr  5-bit register address
  \param[out]  data      Pointer where the result is written to
  \param[in]   dev       Pointer to the MAC device instance
  \return      \ref execution_status
*/
static int32_t PHY_Read(uint8_t phy_addr, uint8_t reg_addr, uint16_t *data,
    MAC_DEV *dev)
{
    uint32_t val, timeout = 5;

    if (!data)
        return ARM_DRIVER_ERROR_PARAMETER;

    if (!(dev->flags & ETH_POWER))
        return ARM_DRIVER_ERROR;

    val = (phy_addr << MAC_MDIO_ADDR_PA_SHIFT) |
            (reg_addr << MAC_MDIO_ADDR_RDA_SHIFT) |
            (MAC_MDIO_ADDR_GOC_READ << MAC_MDIO_ADDR_GOC_SHIFT) |
            (MAC_MDIO_ADDR_CR_150_250 << MAC_MDIO_ADDR_CR_SHIFT)|
            MAC_MDIO_ADDR_GB;

    dev->regs->MAC_MDIO_ADDR = val;

    do {
        if (!(dev->regs->MAC_MDIO_ADDR & MAC_MDIO_ADDR_GB)) {
            *data = dev->regs->MAC_MDIO_DATA;
            break;
        }
        osDelay(1);
        timeout--;
    } while(timeout);

    if (!timeout)
        return ARM_DRIVER_ERROR_TIMEOUT;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t PHY_Write (uint8_t phy_addr, uint8_t reg_addr, uint16_t data,
                                                    MAC_DEV *dev)
  \brief       Write Ethernet PHY Register through Management Interface.
  \param[in]   phy_addr  5-bit device address
  \param[in]   reg_addr  5-bit register address
  \param[in]   data      16-bit data to write
  \param[in]   dev       Pointer to the MAC device instance
  \return      \ref execution_status
*/
static int32_t PHY_Write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data,
    MAC_DEV *dev)
{
    uint32_t val, timeout = 5;

    if (!(dev->flags & ETH_POWER))
        return ARM_DRIVER_ERROR;

    val = (phy_addr << MAC_MDIO_ADDR_PA_SHIFT) |
            (reg_addr << MAC_MDIO_ADDR_RDA_SHIFT) |
            (MAC_MDIO_ADDR_GOC_WRITE << MAC_MDIO_ADDR_GOC_SHIFT) |
            (MAC_MDIO_ADDR_CR_150_250 << MAC_MDIO_ADDR_CR_SHIFT) |
            MAC_MDIO_ADDR_GB;

    dev->regs->MAC_MDIO_DATA = data;
    dev->regs->MAC_MDIO_ADDR = val;

    do {
        if (!(dev->regs->MAC_MDIO_ADDR & MAC_MDIO_ADDR_GB))
            break;
        osDelay(1);
        timeout--;
    } while(timeout);


    if (!timeout)
        return ARM_DRIVER_ERROR_TIMEOUT;

    return ARM_DRIVER_OK;
}

/**
  \fn          void ETH_IRQHandler (MAC_DEV *dev)
  \param[in]   dev       Pointer to the MAC device instance
  \brief       MAC instance specific part of Ethernet Interrupt handler.
*/
static void ETH_IRQHandler(MAC_DEV *dev)
{
    uint32_t ch0_stat, event = 0;

    if (dev->regs->DMA_STATUS & DMA_STATUS_CHAN0) {
        ch0_stat = dev->regs->DMA_CH0_STATUS;

        dev->regs->DMA_CH0_STATUS =
		ch0_stat & (DMA_CHAN_STATUS_NIS | DMA_CHAN_STATUS_RI | DMA_CHAN_STATUS_TI);

        if (ch0_stat & DMA_CHAN_STATUS_RI)
            event |= ARM_ETH_MAC_EVENT_RX_FRAME;

        if (ch0_stat & DMA_CHAN_STATUS_TI)
            event |= ARM_ETH_MAC_EVENT_TX_FRAME;

        if (event && dev->cb_event)
            dev->cb_event(event);
    }
}

/**
  \fn          void ETH_SBD_IRQHandler (void)
  \brief       Ethernet Interrupt handler.
*/
void ETH_SBD_IRQHandler(void)
{
    ETH_IRQHandler(&MAC0);
}

/**
  \fn          int32_t ETH_MAC0_Initialize (ARM_ETH_MAC_SignalEvent_t cb_event)
  \brief       Initialize Ethernet MAC Device.
  \param[in]   cb_event  Pointer to \ref ARM_ETH_MAC_SignalEvent
  \return      \ref execution_status
*/
static int32_t ETH_MAC0_Initialize(ARM_ETH_MAC_SignalEvent_t cb_event)
{
    return Initialize(cb_event, &MAC0);
}

/**
  \fn          int32_t ETH_MAC0_Uninitialize (void)
  \brief       De-initialize Ethernet MAC Device.
  \return      \ref execution_status
*/
static int32_t ETH_MAC0_Uninitialize(void)
{
    return Uninitialize(&MAC0);
}

/**
  \fn          int32_t ETH_MAC0_PowerControl (ARM_POWER_STATE state)
  \brief       Control Ethernet MAC Device Power.
  \param[in]   state  Power state
  \return      \ref execution_status
*/
static int32_t ETH_MAC0_PowerControl(ARM_POWER_STATE state)
{
    return PowerControl(state, &MAC0);
}

/**
  \fn          int32_t ETH_MAC0_GetMacAddress (ARM_ETH_MAC_ADDR *ptr_addr)
  \brief       Get Ethernet MAC Address.
  \param[in]   ptr_addr  Pointer to address
  \return      \ref execution_status
*/
static int32_t ETH_MAC0_GetMacAddress(ARM_ETH_MAC_ADDR *ptr_addr)
{
    return GetMacAddress(ptr_addr, &MAC0);
}

/**
  \fn          int32_t ETH_MAC0_SetMacAddress (const ARM_ETH_MAC_ADDR *ptr_addr)
  \brief       Set Ethernet MAC Address.
  \param[in]   ptr_addr  Pointer to address
  \return      \ref execution_status
*/
static int32_t ETH_MAC0_SetMacAddress(const ARM_ETH_MAC_ADDR *ptr_addr)
{
    return SetMacAddress(ptr_addr, &MAC0);
}

/**
  \fn          int32_t ETH_MAC0_SetAddressFilter (const ARM_ETH_MAC_ADDR *ptr_addr,
                                               uint32_t          num_addr)
  \brief       Configure Address Filter.
  \param[in]   ptr_addr  Pointer to addresses
  \param[in]   num_addr  Number of addresses to configure
  \return      \ref execution_status
*/
static int32_t ETH_MAC0_SetAddressFilter(const ARM_ETH_MAC_ADDR *ptr_addr, uint32_t num_addr)
{
    return SetAddressFilter(ptr_addr, num_addr, &MAC0);
}

/**
  \fn          int32_t ETH_MAC0_SendFrame (const uint8_t *frame, uint32_t len, uint32_t flags)
  \brief       Send Ethernet frame.
  \param[in]   frame  Pointer to frame buffer with data to send
  \param[in]   len    Frame buffer length in bytes
  \param[in]   flags  Frame transmit flags (see ARM_ETH_MAC_TX_FRAME_...)
  \return      \ref execution_status
*/
static int32_t ETH_MAC0_SendFrame(const uint8_t *frame, uint32_t len, uint32_t flags)
{
    return SendFrame(frame, len, flags, &MAC0);
}

/**
  \fn          int32_t ETH_MAC0_ReadFrame (uint8_t *frame, uint32_t len)
  \brief       Read data of received Ethernet frame.
  \param[in]   frame  Pointer to frame buffer for data to read into
  \param[in]   len    Frame buffer length in bytes
  \return      number of data bytes read or execution status
                 - value >= 0: number of data bytes read
                 - value < 0: error occurred, value is execution status as defined with \ref execution_status
*/
static int32_t ETH_MAC0_ReadFrame(uint8_t *frame, uint32_t len)
{
    return ReadFrame(frame, len, &MAC0);
}

/**
  \fn          uint32_t ETH_MAC0_GetRxFrameSize (void)
  \brief       Get size of received Ethernet frame.
  \return      number of bytes in received frame
*/
static uint32_t ETH_MAC0_GetRxFrameSize(void)
{
    return GetRxFrameSize(&MAC0);
}

/**
  \fn          int32_t ETH_MAC0_GetRxFrameTime (ARM_ETH_MAC_TIME *time)
  \brief       Get time of received Ethernet frame.
  \param[in]   time  Pointer to time structure for data to read into
  \return      \ref execution_status
*/
static int32_t ETH_MAC0_GetRxFrameTime(ARM_ETH_MAC_TIME *time)
{
    return GetRxFrameTime(time, &MAC0);
}

/**
  \fn          int32_t ETH_MAC0_Control (uint32_t control, uint32_t arg)
  \brief       Control Ethernet Interface.
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \return      \ref execution_status
*/
static int32_t ETH_MAC0_GetTxFrameTime(ARM_ETH_MAC_TIME *time)
{
    return GetTxFrameTime(time, &MAC0);
}

/**
  \fn          int32_t ETH_MAC0_Control (uint32_t control, uint32_t arg)
  \brief       Control Ethernet Interface.
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \return      \ref execution_status
*/
static int32_t ETH_MAC0_Control(uint32_t control, uint32_t arg)
{
    return Control(control, arg, &MAC0);
}

/**
  \fn          int32_t ETH_MAC0_ControlTimer (uint32_t control, ARM_ETH_MAC_TIME *time)
  \brief       Control Precision Timer.
  \param[in]   control  Operation
  \param[in]   time     Pointer to time structure
  \return      \ref execution_status
*/
static int32_t ETH_MAC0_ControlTimer(uint32_t control, ARM_ETH_MAC_TIME *time)
{
    return ControlTimer(control, time, &MAC0);
}

/**
  \fn          int32_t ETH_MAC0_PHY_Read (uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
  \brief       Read Ethernet PHY Register through Management Interface.
  \param[in]   phy_addr  5-bit device address
  \param[in]   reg_addr  5-bit register address
  \param[out]  data      Pointer where the result is written to
  \return      \ref execution_status
*/
static int32_t ETH_MAC0_PHY_Read(uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
{
    return PHY_Read(phy_addr, reg_addr, data, &MAC0);
}

/**
  \fn          int32_t ETH_MAC0_PHY_Write (uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
  \brief       Write Ethernet PHY Register through Management Interface.
  \param[in]   phy_addr  5-bit device address
  \param[in]   reg_addr  5-bit register address
  \param[in]   data      16-bit data to write
  \return      \ref execution_status
*/
static int32_t ETH_MAC0_PHY_Write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
{
    return PHY_Write(phy_addr, reg_addr, data, &MAC0);
}

ARM_DRIVER_ETH_MAC Driver_ETH_MAC0 =
{
    ETH_MAC_GetVersion,
    ETH_MAC_GetCapabilities,
    ETH_MAC0_Initialize,
    ETH_MAC0_Uninitialize,
    ETH_MAC0_PowerControl,
    ETH_MAC0_GetMacAddress,
    ETH_MAC0_SetMacAddress,
    ETH_MAC0_SetAddressFilter,
    ETH_MAC0_SendFrame,
    ETH_MAC0_ReadFrame,
    ETH_MAC0_GetRxFrameSize,
    ETH_MAC0_GetRxFrameTime,
    ETH_MAC0_GetTxFrameTime,
    ETH_MAC0_ControlTimer,
    ETH_MAC0_Control,
    ETH_MAC0_PHY_Read,
    ETH_MAC0_PHY_Write
};

