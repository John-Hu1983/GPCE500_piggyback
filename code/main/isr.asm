//==========================================================================
// File Name   : ISR.asm
// Description : Interrupt Service Routine
// Written by  : Jerry Hsu
// Last modified date:
//              2023/12/20
// Note: 
// 1. Interrupts must be declared in address 0x8000~0xffff.
// 2. For FIQ, IRQ1 ~ IRQ7, user needs to clear P_INT_Clear before 
//    exiting interrupt routine
// 3. 
// 	_FIQ:	FIQ								// Fast interrupt entry
//  _IRQ0:	TimerA							// interrupt entry
//  _IRQ1:	TimerB							// interrupt entry
//  _IRQ2:	TimerC							// interrupt entry
//  _IRQ3:	DMA1, DMA2, ADC, SPI1, SPI2		// interrupt entry
//  _IRQ4:	KEY								// interrupt entry
//  _IRQ5:	SPU, EXT1, EXT2					// interrupt entry
//  _IRQ6:	CTS, 512Hz, 2KHz, 4KHz			// interrupt entry     
//  _IRQ7:	UART, 2Hz, 16Hz, 64Hz			// interrupt entry
//  _Break: Software interrupt              // interrupt entry
//==========================================================================

//**************************************************************************
// Header File Included Area
//**************************************************************************
.include GPCE36_CE5.inc

//**************************************************************************
// Contant Defintion Area
//**************************************************************************


//**************************************************************************
// Variable Publication Area
//**************************************************************************

//**************************************************************************
// Function Call Publication Area
//**************************************************************************
.public _BREAK;

.public _FIQ;

.public _IRQ0;

.public _IRQ1;

.public _IRQ2;

.public _IRQ3;

.public _IRQ4;

.public _IRQ5;

.public _IRQ6;

.public _IRQ7;


//**************************************************************************
// External Variable Declaration
//**************************************************************************


//**************************************************************************
// External Function Declaration
//**************************************************************************
.external _uart_service_irq
.external _spi_service_irq
.external _rtc_service_irq
.external _spu_service_irq

//**************************************************************************
// RAM Definition Area
//**************************************************************************
.RAM


//**************************************************************************
// CODE Definition Area
//**************************************************************************
.TEXT

_BREAK:
	push R1, R5 to [SP]
	
	
	pop R1, R5 from [SP]
	reti;

//****************************************************************
// _FIQ
//****************************************************************
_FIQ:
	push R1, R5 to [SP]

	
	pop R1, R5 from [SP]
	reti;
	
//****************************************************************
// _IRQ0
//****************************************************************
_IRQ0:
	push R1, R5 to [SP]
	
	
	pop R1, R5 from [SP]
	reti;

//****************************************************************
// _IRQ1
//****************************************************************
_IRQ1:
	push R1, R5 to [SP]
	

	pop R1, R5 from [SP]
	reti;

//****************************************************************
// _IRQ2
//****************************************************************
_IRQ2:
	push R1, R5 to [SP]


	pop R1, R5 from [SP]
	reti;

//****************************************************************
// _IRQ3
//****************************************************************
_IRQ3:
	push R1, R5 to [SP]
	push SR to [SP]

	call _uart_service_irq
	call _spi_service_irq

	pop SR from [SP]
	pop R1, R5 from [SP]
	reti;

//****************************************************************
// _IRQ4
//****************************************************************
_IRQ4:
	push R1, R5 to [SP]
	
	pop R1, R5 from [SP]
	reti;

//****************************************************************
// _IRQ5
//****************************************************************
_IRQ5:
	push R1, R5 to [SP]
	push SR to [SP]

	call _spu_service_irq

	pop SR from [SP]
	pop R1, R5 from [SP]
	reti;

//****************************************************************
// _IRQ6
//****************************************************************
_IRQ6:
	push R1, R5 to [SP]
	push SR to [SP]

	R1 = C_IRQ6_512Hz | C_IRQ6_2048Hz | C_IRQ6_4096Hz
	push R1 to [SP]
	call _rtc_service_irq
	SP += 1

	pop SR from [SP]
	pop R1, R5 from [SP]
	reti;

//****************************************************************
// _IRQ7
//****************************************************************
_IRQ7:
	push R1, R5 to [SP]
	push SR to [SP]
	call _uart_service_irq

	R1 = C_IRQ7_2Hz | C_IRQ7_16Hz | C_IRQ7_64Hz
	push R1 to [SP]
	call _rtc_service_irq
	SP += 1

	pop SR from [SP]
	pop R1, R5 from [SP]
	reti;


//========================================================================================        
// End of isr.asm
//========================================================================================
