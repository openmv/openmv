/* Copyright (C) 2023 Alif Semiconductor - All Rights Reserved.
 * Use, distribution and modification of this code is permitted under the
 * terms stated in the Alif Semiconductor Software License Agreement
 *
 * You should have received a copy of the Alif Semiconductor Software
 * License Agreement with this file. If not, please write to:
 * contact@alifsemi.com, or visit: https://alifsemi.com/license
 *
 */

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

#ifndef DMA_CONFIG_H_
#define DMA_CONFIG_H_

// <o> DMA Destination Cache Control <0-7>
// <i> State of AWCACHE[3,1:0] when the DMAC write the destination data
//      Bit [2] 0 = AWCACHE[3] is LOW
//              1 = AWCACHE[3] is HIGH.
//      Bit [1] 0 = AWCACHE[1] is LOW
//              1 = AWCACHE[1] is HIGH.
//      Bit [0] 0 = AWCACHE[0] is LOW
//              1 = AWCACHE[0] is HIGH.
//      Setting AWCACHE[3,1]=0b10 violates the AXI protocol
// <i> Default: 0x2
#define DMA_DEST_CACHE_CTRL     0x2

// <o> DMA Source Cache Control <0-7>
// <i> State of ARCACHE[2:0] when the DMAC reads the source data
//      Bit [2] 0 = ARCACHE[2] is LOW
//              1 = ARCACHE[2] is HIGH.
//      Bit [1] 0 = ARCACHE[1] is LOW
//              1 = ARCACHE[1] is HIGH.
//      Bit [0] 0 = ARCACHE[0] is LOW
//              1 = ARCACHE[0] is HIGH.
//      Setting ARCACHE[2:1]=0b10 violates the AXI protocol
// <i> Default: 0x2
#define DMA_SRC_CACHE_CTRL      0x2

// <o> DMA Source Protection Control <0-7>
// <i> State of ARPROT[2:0] when the DMAC reads the source data
//      Bit [2] 0 = ARPROT[2] is LOW
//              1 = ARPROT[2] is HIGH.
//      Bit [1] 0 = ARPROT[1] is LOW
//              1 = ARPROT[1] is HIGH.
//      Bit [0] 0 = ARPROT[0] is LOW
//              1 = ARPROT[0] is HIGH.
//      Only DMA channels in the Secure state can program ARPROT[1] LOW
// <i> Default: 0x0
#define DMA_SRC_PROT_CTRL      0x0

// <o> DMA Destination Protection Control <0-7>
// <i> State of AWPROT[2:0] when the DMAC write the destination data
//      Bit [2] 0 = AWPROT[2] is LOW
//              1 = AWPROT[2] is HIGH.
//      Bit [1] 0 = AWPROT[1] is LOW
//              1 = AWPROT[1] is HIGH.
//      Bit [0] 0 = AWPROT[0] is LOW
//              1 = AWPROT[0] is HIGH.
//      Only DMA channels in the Secure state can program AWPROT[1] LOW
// <i> Default: 0x0
#define DMA_DEST_PROT_CTRL      0x0

// <o> DMA Microcode size
// <i> Defines Default memory size(bytes) to hold the microcode for a channel
// <i> Default: 128
#define DMA_MICROCODE_SIZE      128

#endif /* DMA_CONFIG_H_ */
