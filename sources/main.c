/************************************************************************/
/*																		*/
/*	main.c	--	ZYBO Base System demonstration	 						*/
/*																		*/
/************************************************************************/
/*	Author: Sam Bobrowicz												*/
/*	Copyright 2014, Digilent Inc.										*/
/************************************************************************/
/*  Module Description: 												*/
/*																		*/
/*		This file contains code for running a demonstration of the		*/
/*		Video output and audio capabilities of the ZYBO.				*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/* 																		*/
/*		2/25/2014(SamB): Created										*/
/*																		*/
/************************************************************************/

/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

#include "timer_ps.h"
#include "xparameters.h"
#include "xuartps.h"
#include "audio_fun.h"
#include "xgpio.h"

/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */

/*
 * XPAR redefines
 */
#define AUDIO_IIC_ID XPAR_XIICPS_0_DEVICE_ID
#define AUDIO_CTRL_BASEADDR XPAR_AXI_I2S_ADI_0_S00_AXI_BASEADDR
#define SCU_TIMER_ID XPAR_SCUTIMER_DEVICE_ID
#define UART_BASEADDR XPAR_PS7_UART_1_BASEADDR
#define INPUT_BASEADDR XPAR_AXI_GPIO_0_BASEADDR
#define OUTPUT_BASEADDR XPAR_AXI_GPIO_1_BASEADDR

// Declare a XIicPs structure for the II interface

// Declare 2 XGpio structures (input, output) for the GPIO interfaces

// Declare 2 variables to store the buttons and switches states

// Declare 2 variables to store the Left and Right audio data

/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */

int main(void)
{
	xil_printf("\x1B[H"); //Set cursor to top left of terminal
	xil_printf("\x1B[2J"); //Clear terminal

	// GPIO initialization
	// Look for the initialization function in the xgpio.h file
	// Initialize input XGpio variable
	// Initialize output XGpio variable

	xil_printf("GPIO initialized\n\r");

	// Audio initialization
	// Look for the initialization function in the audio_fun.h file
	// Initialize Audio codec

	xil_printf("Audio initialized\n\r");

	// At this point, the Codec is configured in Bypass mode.
	// Comment everything below to use it that way.

	// Set register to use DAC path (and disable Bypass) with an IIC command (AudioRegSet)

	// Resetting and enabling TX/RX Fifos
	Xil_Out32(AUDIO_CTRL_BASEADDR + I2S_RESET_REG, 0b110); //Reset RX/TX Fifo
	Xil_Out32(AUDIO_CTRL_BASEADDR + I2S_CTRL_REG, 0b011); //Enable RX Fifo, disable mute

	// Variable to keep track of the state of the switches
	static int previous_state=0;

	while(1){
		
		// Use the XGpio_DiscreteRead() function to get switches and buttons data

		// Use the XGpio_DiscreteWrite() function to light the leds according to the switches

		//Print message dependent on whether one or more buttons are pressed
		
		// Use sw0 state to change the audio volume through a IIC command

		// Main audio rec/playback routine
		// Use the I2SFifoRead() function to store the left and right audio data
		// Use the I2SFifoWrite() function to play the left and right audio data

	}

	return 0;
}


