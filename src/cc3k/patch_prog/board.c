/*****************************************************************************
*
*  board.c - FRAM board functions
*  Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/


#include "wlan.h" 
#include "evnt_handler.h"    
#include "nvmem.h"
#include "socket.h"
#include "cc3000_common.h"
#include "netapp.h"
#include "spi.h"
#include "hci.h"


#include "inc/hw_types.h"
#include "driverlib/timer.h"
#include "driverlib/rom_map.h"
#include "driverlib/systick.h"
#include "driverlib/fpu.h"
#include "driverlib/debug.h"
#include "grlib/grlib.h"

#include "utils/uartstdio.h"
#include "driverlib/uart.h"
#include "driverlib/ssi.h"

#include "dispatcher.h"
#include "spi_version.h"
#include "board.h"




//
// LMS969B C1 Flash workaround handler - see Errata
//
#define WorkaroundIntHandler    ((void (*)(void))0x881)
//*****************************************************************************
//
// Defines for setting up the system clock.
//
//*****************************************************************************
#define SYSTICK_PER_MS                1000
#define SYSTICK_PER_US                1000000
#define SYSTICK_PER_NS                1000000000
//*****************************************************************************
//
//! pio_init
//!
//! \param  none
//!
//! \return none
//!
//! \brief  Initialize the board's I/O
//
//*****************************************************************************    

void pio_init()
{
					 //  Board Initialization start
				 //
				 //
				 // The FPU should be enabled because some compilers will use floating-
				 // point registers, even for non-floating-point code.  If the FPU is not
				 // enabled this will cause a fault.  This also ensures that floating-
				 // point operations could be added to this application and would work
				 // correctly and use the hardware floating-point unit.  Finally, lazy
				 // stacking is enabled for interrupt handlers.  This allows floating-
				 // point instructions to be used within interrupt handlers, but at the
				 // expense of extra stack usage.
				 //
				 FPUEnable();
				 FPULazyStackingEnable();	
				 
				 //Init the device with 16 MHz clock.
				 initClk();
	
         /* Configure the system peripheral bus that IRQ & EN pin are map to */
					MAP_SysCtlPeripheralEnable( SYSCTL_PERIPH_IRQ_PORT);
					//
					// Disable all the interrupts before configuring the lines
					//
					MAP_GPIOPinIntDisable(SPI_GPIO_IRQ_BASE, 0xFF);
					//
					// Cofigure WLAN_IRQ pin as input
					//
					MAP_GPIOPinTypeGPIOInput(SPI_GPIO_IRQ_BASE, SPI_IRQ_PIN);

					GPIOPadConfigSet(SPI_GPIO_IRQ_BASE, SPI_IRQ_PIN, GPIO_STRENGTH_2MA,
												GPIO_PIN_TYPE_STD_WPU);     
					//
					// Setup the GPIO interrupt for this pin
					//
					MAP_GPIOIntTypeSet(SPI_GPIO_IRQ_BASE, SPI_IRQ_PIN, GPIO_FALLING_EDGE);

					//
					// Disable WLAN chip
					//
					MAP_GPIOPinWrite(SPI_GPIO_IRQ_BASE, SPI_EN_PIN, PIN_LOW);		
					MAP_GPIOPinTypeGPIOOutput(SPI_GPIO_IRQ_BASE, SPI_EN_PIN);
					GPIOPadConfigSet(SPI_GPIO_IRQ_BASE, SPI_EN_PIN, GPIO_STRENGTH_2MA,
												GPIO_PIN_TYPE_STD_WPD);     
					//MAP_GPIOPinWrite(SPI_GPIO_IRQ_BASE, SPI_EN_PIN, PIN_HIGH);
					MAP_GPIOPinWrite(SPI_GPIO_IRQ_BASE, SPI_EN_PIN, PIN_LOW);	
					SysCtlDelay(600000);
					SysCtlDelay(600000);
					SysCtlDelay(600000);
					//
					// Disable WLAN CS with pull up Resistor
					//
					MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_SPI_PORT);	
					MAP_GPIOPinTypeGPIOOutput(SPI_PORT, SPI_CS_PIN);
					GPIOPadConfigSet(SPI_PORT, SPI_CS_PIN, GPIO_STRENGTH_2MA,
												GPIO_PIN_TYPE_STD_WPU);     
					MAP_GPIOPinWrite(SPI_PORT, SPI_CS_PIN, PIN_HIGH);

					// 
					// Enable interrupt for WLAN_IRQ pin
					//
					MAP_GPIOPinIntEnable(SPI_GPIO_IRQ_BASE, SPI_IRQ_PIN);
					// 
					// Clear interrupt status
					//
					SpiCleanGPIOISR();

					MAP_IntEnable(INT_GPIO_SPI);
					
					//init LED
					initLEDs();

}


//*****************************************************************************
//
//! ReadWlanInterruptPin
//!
//! \param  none
//!
//! \return none
//!
//! \brief  return wlan interrup pin
//
//*****************************************************************************

long ReadWlanInterruptPin(void)
{
  return MAP_GPIOPinRead(SPI_GPIO_IRQ_BASE, SPI_IRQ_PIN);
}

//*****************************************************************************
//
//! Enable waln IrQ pin
//!
//! \param  none
//!
//! \return none
//!
//! \brief  Nonr
//
//*****************************************************************************


void WlanInterruptEnable()
{
   MAP_GPIOPinIntEnable(SPI_GPIO_IRQ_BASE, SPI_IRQ_PIN);
}

//*****************************************************************************
//
//! Disable waln IrQ pin
//!
//! \param  none
//!
//! \return none
//!
//! \brief  Nonr
//
//*****************************************************************************


void WlanInterruptDisable()
{
  MAP_GPIOPinIntDisable(SPI_GPIO_IRQ_BASE, SPI_IRQ_PIN);
}


//*****************************************************************************
//
//! WriteWlanPin
//!
//! \param  new val
//!
//! \return none
//!
//! \brief  This functions enables and disables the CC3000 Radio
//
//*****************************************************************************

void WriteWlanPin( unsigned char val )
{
	if(val)
	{
  		MAP_GPIOPinWrite(SPI_GPIO_IRQ_BASE, SPI_EN_PIN,PIN_HIGH);
	}
	else
	{
  		MAP_GPIOPinWrite(SPI_GPIO_IRQ_BASE, SPI_EN_PIN, PIN_LOW);
	}
	
}


//*****************************************************************************
//
// Init SysTick timer.  
// 
//
//*****************************************************************************
void
InitSysTick(void)
{
  // Configure SysTick to occur X times per second, to use as a time
    // reference.  Enable SysTick to generate interrupts.
    //
    unsigned long debug = SysCtlClockGet();
    SysTickPeriodSet(SysCtlClockGet()/SYSTICK_PER_MS);
    SysTickIntEnable();
    SysTickEnable();
}

//*****************************************************************************
//
// The interrupt handler for the SysTick timer.  This handler is called every 1ms
// 
//
//*****************************************************************************
void
SysTickHandler(void)
{
    static unsigned long ulTickCount = 0;
    //
    // Increment the tick counter.
    //
    ulTickCount++;
    //Process after 500 ms
    if(ulTickCount >= 500)
    {
        //
        // Handle any un-solicited event if required - the function shall be triggered 
        // few times in a second
        //
        hci_unsolicited_event_handler();
        ulTickCount = 1;
    }
}

//*****************************************************************************
//
//! init clk
//!
//!  \param  None
//!
//!  \return none
//!
//!  \Init the device with 16 MHz clock.
//
//*****************************************************************************
void initClk(void)
{
	 
	// 16 MHz Crystal on Board. SSI Freq - configure M3 Clock to be ~50 MHz
	//
	MAP_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |SYSCTL_XTAL_16MHZ);	

}

//*****************************************************************************
//
//! Initialize LEDs
//!
//! \param  none
//!
//! \return none
//!
//! \brief  Initializes LED Ports and Pins
//
//*****************************************************************************
void initLEDs()
{
		 // Enable use of PORTG to toggle LED and disable interrupt on this port
		 //
		 MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
		 MAP_GPIOPinIntDisable(GPIO_PORTG_BASE, 0xFF);
		 //
		 // Configure LED4
		 //
		 MAP_GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_2);
		 MAP_GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, 0); 
}
//*****************************************************************************
//
// This function turns the USER LED ON/OFF
// 
//!
//! \param  ledNum is the LED Number
//!
//! \return none
//!
//! \brief  Turns a specific LED Off
//
//*****************************************************************************
void turnLedOn(char trueFalse)
{
		MAP_GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, PIN_HIGH);
}


//*****************************************************************************
//
//! Turn LED Off
//!
//! \param  ledNum is the LED Number
//!
//! \return none
//!
//! \brief  Turns a specific LED Off
//
//*****************************************************************************    
void turnLedOff(char ledNum)
{                     
    	MAP_GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_2, PIN_LOW);
    
}

//*****************************************************************************
//
//! toggleLed
//!
//! \param  ledNum is the LED Number
//!
//! \return none
//!
//! \brief  Toggles a board LED
//
//*****************************************************************************    

void toggleLed(char ledNum)
{
    
   
}


  //*****************************************************************************
// 
//! This function is FLASH workaround interrupt handler
//! 
//! @param   none
//! 
//! @return  none
//! 
//! @brief   This function is FLASH workaround interrupt handler
// 
//*****************************************************************************
void
FlashIntHandler(void)
{
    //
    // Call the PATCH Code Interrupt handler
    //
    WorkaroundIntHandler();
}



