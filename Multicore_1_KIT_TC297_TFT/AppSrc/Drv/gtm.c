/*
 * gtm.c
 *
 *  Created on: Jun 14, 2017
 *      Author: gurka
 */

#include "gtm.h"

#include "IfxGtm_reg.h"
#include "IfxPort_reg.h"
#include "Ifx_Types.h"
#include "IfxScuCcu.h"

void gtm_global_init()
{
	//volatile unsigned int uwTemp;
	uint16 endinit_pw;
	endinit_pw = IfxScuWdt_getCpuWatchdogPassword(); //Get the password for the current CPU

	//-------------------------------------------------------------------------
	//Clear the Endinit Function to access the protected Registers
	IfxScuWdt_clearCpuEndinit(endinit_pw);
		//A. Enable the GTM Module:
		GTM_CLC.U = 0x00000000;   						    // load clock control register
		while ((GTM_CLC.U & 0x00000002) == 0x00000002);  	//wait until module is enabled
		//SET the Endinit Function after access to the protected Registers
    IfxScuWdt_setCpuEndinit(endinit_pw);
    //-------------------------------------------------------------------------

    //CMU Clock Enable Register (GTM_CMU_CLK_EN)
    //-----------------------------------------
    //CMU_FXCLK (Enable all CMU_FXCLK)
    //-----------------------------------------
    GTM_CMU_CLK_EN.U = (0x2 << 22 /*CMU_FXCLK*/);
    //-----------------------------------------
}

void gtm_tom_init()
{
	//TOM0 Channel 1 Control Register (GTM_TOM0_CH1_CTRL)
	//---------------------------------------------------
	//CLK_SRC_SR (Clock source select for channel) CLK_SRC updated with CLK_SRC_SR
	//TRIGOUT (Trigger output selection)
	//---------------------------------------------------
	GTM_TOM0_CH6_CTRL.U = (0x0 << 12 /*CLK_SRC_SR*/)| // (FXCLKSEL/2^0)
						  (0x1 << 24 /*TRIGOUT*/);	// TRIG_[x] is TRIG_CCU0
	//---------------------------------------------------


	//TOM0 TGC0 Output Enable Control Register (GTM_TOM0_TGC0_OUTEN_CTRL)
	//-------------------------------------------------------------------
	//OUTEN_CTRL6 (Output TOM_OUT(6) enable/disable update value)
	//-------------------------------------------------------------------
	GTM_TOM0_TGC0_OUTEN_CTRL.U = (0x2 << 12 /*OUTEN_CTRL6*/); //Enable TOM0 Ch1 output
	//-------------------------------------------------------------------


	//TOM0 TGC0 Global Control Register (GTM_TOM0_TGC0_GLB_CTRL)
	//----------------------------------------------------------
	//UPEN_CTRL6 (TOM channel 6 enable update of register CM0, CM1, and CLK_SRC from SR0, SR1, and CLK_SRC_SR)
	//----------------------------------------------------------
	GTM_TOM0_TGC0_GLB_CTRL.U = (0x2 << 28 /*UPEN_CTRL1*/);
	//----------------------------------------------------------


	//TOM0 TGC0 Enable/Disable Control Register (GTM_TOM0_TGC0_ENDIS_CTRL)
	//--------------------------------------------------------------------
	//ENDIS_CTRL6 (TOM channel 6 enable/disable update value)
	//Notes: Allow CN0 of channel to count on an update trigger
	//--------------------------------------------------------------------
	GTM_TOM0_TGC0_ENDIS_CTRL.U = (0x2 << 12 /*ENDIS_CTRL6*/);
	//--------------------------------------------------------------------


	//TOM0 Channel 1 CCU0 Compare Shadow Register (GTM_TOM0_CH1_SR0)
	//--------------------------------------------------------------
	//SR0 (TOM channel 1 shadow register SR0 for update of compare register CM0)
	//Notes: Set period.
	//--------------------------------------------------------------
	GTM_TOM0_CH6_SR0.U = (25000 << 0 /*SR0*/);
	//--------------------------------------------------------------


	//TOM0 Channel 1 CCU1 Compare Shadow Register (GTM_TOM0_CH1_SR1)
	//--------------------------------------------------------------
	//SR1 (TOM channel 1 shadow register SR1 for update of compare register CM1)
	//Notes: Set duty cycle.
	//--------------------------------------------------------------
	GTM_TOM0_CH6_SR1.U = (12500 << 0 /*SR1*/);
	//--------------------------------------------------------------


	//ADC Trigger 0 Output Select 0 Register (ADCTRIG0OUT0)
	//-----------------------------------------------------
	//SEL2 (Output Selection for ADC2 GTM Connection)
	//Notes: TOM0 channel output is directly used as triggering signal. SEL0 specifies VADC Group0
	//-----------------------------------------------------
	GTM_ADCTRIG0OUT0.U = (0x1 << 0 /*SEL0*/); //TOM0 Ch6 output is trigger
	//-----------------------------------------------------

	/*Output GTM signal to GPIO*/

//	//Timer Output Select Register 1 (TOUTSEL1)
//	//-----------------------------------------
//	//SEL15 (TOUT31 Output Selection) Select timer connected as TOUT31
//	//Notes: Timer output selection dictated by choice of I/O port below.
//	//-----------------------------------------
//	GTM_TOUTSEL1.U = (0x0 << 30 /*SEL15*/); //Connect Timer A to TOUT31
//	//-----------------------------------------
//
//
//	//Port 33 Input/Output Control Register 8 (P33_IOCR8)
//	//---------------------------------------------------
//	//PC9 (Port Control for Port 33 Pin 9)
//	//Notes: P33.9 selected since it is mapped to TOM0_1
//	//       See 24.9.2 Port Connections
//	//---------------------------------------------------
//	P33_IOCR8.U = (0x11 << 11 /*PC9*/);
//	//---------------------------------------------------


	/*Set trigger request*/
	GTM_TOM0_TGC0_GLB_CTRL.U |= (0x1 << 0 /*HOST_TRIG*/);
}
