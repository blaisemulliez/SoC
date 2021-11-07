/************************************************************************/
/*																		*/
/*	audio_fun.c	--	ZYBO Audio demonstration 						    */
/*																		*/
/************************************************************************/

/*  Module Description: 												*/
/*																		*/
/*		This file contains code for running the	SSM2603 Audio codec	    */
/*		the ZYBO.														*/
/************************************************************************/

/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */

#include "audio_fun.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "xparameters.h"
#include "xil_types.h"
#include "xil_io.h"
#include "xil_printf.h"
#include "timer_ps.h"
//#include "xiicps.h"
#include "xuartps.h"

/* Redefine the XPAR constants */
#define IIC_DEVICE_ID		XPAR_XIICPS_0_DEVICE_ID
#define I2S_ADDRESS 		XPAR_AXI_I2S_ADI_0_BASEADDR
#define TIMER_DEVICE_ID 	XPAR_SCUTIMER_DEVICE_ID

/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */

XIicPs Iic;		/* Instance of the IIC Device */

/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */
/***	AudioInitialize(u16 timerID,  u16 iicID, u32 i2sAddr)
**
**	Parameters:
**		timerID - DEVICE_ID for the SCU timer
**		iicID 	- DEVICE_ID for the PS IIC controller connected to the SSM2603
**		i2sAddr - Physical Base address of the I2S controller
**
**	Return Value: int
**		XST_SUCCESS if successful
**
**	Errors:
**
**	Description:
**		Initializes the Audio demo. Must be called once and only once before calling
**		AudioRunDemo
**
*/
int AudioInitialize(u16 timerID,  u16 iicID, u32 i2sAddr)
{
	int Status;
	XIicPs_Config *Config;
	u32 i2sClkDiv;

	TimerInitialize(timerID);

	/*
	 * Initialize the IIC driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XIicPs_LookupConfig(iicID);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XIicPs_SelfTest(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the IIC serial clock rate.
	 */
	Status = XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Write to the SSM2603 audio codec registers to configure the device. Refer to the
	 * SSM2603 Audio Codec data sheet for information on what these writes do.
	 */
	Status = AudioRegSet(&Iic, 15, 0b000000000); //Perform Reset
	TimerDelay(75000);
	Status |= AudioRegSet(&Iic, 6, 0b000110000); // Power up with DAC disabled to avoid parasitic noise
	Status |= AudioRegSet(&Iic, 0, 0b000010111); // Registers setting
	Status |= AudioRegSet(&Iic, 1, 0b000010111); // cont'd
	Status |= AudioRegSet(&Iic, 2, 0b001111001); // cont'd
	Status |= AudioRegSet(&Iic, 3, 0b001111001); // cont'd
	Status |= AudioRegSet(&Iic, 4, 0b000001010); // Bypass activation
	Status |= AudioRegSet(&Iic, 5, 0b000000000); // cont'd
	Status |= AudioRegSet(&Iic, 7, 0b000001010); // Word length is 24
	Status |= AudioRegSet(&Iic, 8, 0b000000000); // No CLKDIV2
	TimerDelay(75000);
	Status |= AudioRegSet(&Iic, 9, 0b000000001); // Digital core activated
	Status |= AudioRegSet(&Iic, 6, 0b000100000); // DAC activation

	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	i2sClkDiv = 1; //Set the BCLK to be MCLK / 4
	i2sClkDiv = i2sClkDiv | (31 << 16); //Set the LRCLK's to be BCLK / 64

	Xil_Out32(i2sAddr + I2S_CLK_CTRL_REG, i2sClkDiv); //Write clock div register

	return XST_SUCCESS;
}

/* ------------------------------------------------------------ */
/***	AudioRegSet(XIicPs *IIcPtr, u8 regAddr, u16 regData)
**
**	Parameters:
**		IIcPtr - Pointer to the initialized XIicPs struct
**		regAddr - Register in the SSM2603 to write to
**		regData - Data to write to the register (lower 9 bits are used)
**
**	Return Value: int
**		XST_SUCCESS if successful
**
**	Errors:
**
**	Description:
**		Writes a value to a register in the SSM2603 device over IIC.
**
*/
int AudioRegSet(XIicPs *IIcPtr, u8 regAddr, u16 regData)
{
	int Status;
	u8 SendBuffer[2];

	SendBuffer[0] = regAddr << 1;
	SendBuffer[0] = SendBuffer[0] | ((regData >> 8) & 0b1);

	SendBuffer[1] = regData & 0xFF;

	Status = XIicPs_MasterSendPolled(IIcPtr, SendBuffer,
				 2, IIC_SLAVE_ADDR);
	if (Status != XST_SUCCESS) {
		xil_printf("IIC send failed\n\r");
		return XST_FAILURE;
	}
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(IIcPtr)) {
		/* NOP */
	}
	return XST_SUCCESS;
}

/* ------------------------------------------------------------ */
/***	I2SFifoWrite (u32 i2sBaseAddr, u32 audioData)
**
**	Parameters:
**		i2sBaseAddr - Physical Base address of the I2S controller
**		audioData - Audio data to be written to FIFO
**
**	Return Value: none
**
**	Errors:
**
**	Description:
**		Blocks execution until space is available in the I2S TX fifo, then
**		writes data to it.
**
*/
void I2SFifoWrite (u32 i2sBaseAddr, u32 audioData)
{
	while ((Xil_In32(i2sBaseAddr + I2S_FIFO_STS_REG)) & 0b0010)
	{}
	Xil_Out32(i2sBaseAddr + I2S_TX_FIFO_REG, audioData);
}

/* ------------------------------------------------------------ */
/***	I2SFifoRead (u32 i2sBaseAddr)
**
**	Parameters:
**		i2sBaseAddr - Physical Base address of the I2S controller
**
**	Return Value: u32
**		Audio data from the I2S RX FIFO
**
**	Errors:
**
**	Description:
**		Blocks execution until data is available in the I2S RX fifo, then
**		reads it out.
**
*/
u32 I2SFifoRead (u32 i2sBaseAddr)
{
	while ((Xil_In32(i2sBaseAddr + I2S_FIFO_STS_REG)) & 0b0100)
	{}
	return Xil_In32(i2sBaseAddr + I2S_RX_FIFO_REG);
}
/* ------------------------------------------------------------ */

/************************************************************************/
