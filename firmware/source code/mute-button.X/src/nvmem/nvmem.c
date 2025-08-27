/*********************************************************************
 *
 *                  PIC32 Boot Loader
 *
 *********************************************************************
 * FileName:        NVMem.c
 * Dependencies:
 * Processor:       PIC32
 *
 * Compiler:        MPLAB C32
 *                  MPLAB IDE
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PIC32 Microcontroller is intended
 * and supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PIC32 Microcontroller products.
 * The software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *
 * $Id:  $
 * $Name: $
 *
 **********************************************************************/

#include "nvmem.h"
#include "kmem.h"
#include "../hardware.h"
#include <xc.h>



const uint32_t countPerMicroSec = ((SYS_FREQ/1000000)/2);
#define NVMemIsError()    (NVMCON & (_NVMCON_WRERR_MASK | _NVMCON_LVDERR_MASK))








/********************************************************************
* Function: 	NVMemOperation()
*
* Precondition: 
*
* Input: 		NV operation
*
* Output:		NV eror
*
* Side Effects:	This function must generate MIPS32 code only and 
				hence the attribute (nomips16)
*
* Overview:     Performs reuested operation.
*
*			
* Note:		 	None.
********************************************************************/
static uint32_t __attribute__((nomips16)) NVMemOperation(uint32_t nvmop) {

    INT_Global_Disable();

    // Enable Flash Write/Erase Operations
    //NVMCON = NVMCON_WREN | nvmop;
    //NVMCON = _NVMCON_WREN_MASK | nvmop;
    NVMCON = nvmop;
    // Data sheet prescribes 6us delay for LVD to become stable.
    // To be on the safer side, we shall set 7us delay.
    Delay_us(7);

    NVMKEY 		= 0xAA996655;
    NVMKEY 		= 0x556699AA;
    NVMCONSET 	= _NVMCON_WR_MASK;//NVMCON_WR;

    // Wait for WR bit to clear
    //while(NVMCON & NVMCON_WR);
    //while(NVMCON & _NVMCON_WR_MASK);
    while (NVMCONbits.WR);
    
    // Disable Flash Write/Erase operations
    //NVMCONCLR = NVMCON_WREN;  
    NVMCONCLR = _NVMCON_WREN_MASK;  

	INT_Global_Enable();

	// Return Error Status
    return(NVMemIsError());
}


/*********************************************************************
 * Function:        unsigned int NVMErasePage(void* address)
 *
 * Description:     A page erase will erase a single page of program flash,
 *                  which equates to 1k instructions (4KBytes). The page to
 *                  be erased is selected using NVMADDR. The lower bytes of
 *                  the address given by NVMADDR are ignored in page selection.
 *
 * PreCondition:    None
 *
 * Inputs:          address:  Destination page address to Erase.
 *
 * Output:          '0' if operation completed successfully.
 *
 * Example:         NVMemErasePage((void*) 0xBD000000)
 ********************************************************************/
uint32_t NVMemErasePage(void* address) {
    uint32_t res;

    // Convert Address to Physical Address
	NVMADDR = KVA_TO_PA((uint32_t)address);

	// Unlock and Erase Page
	res = NVMemOperation(NVMOP_PAGE_ERASE);

	// Return WRERR state.
	return res;

}


uint32_t NVMemWriteUInt64(void* address, uint64_t* data) {
    uint32_t res;

    NVMADDR = KVA_TO_PA((uint32_t)address);

    // Load data into NVMDATA register
    uint32_t* ptr = (uint32_t*)data;
    NVMDATA0 = *ptr;
    NVMDATA1 = *(ptr + 1);

    // Unlock and Write Word
    res = NVMemOperation(NVMOP_DBL_WORD_PGM);

	return res;
}




/*********************************************************************
 * Function:        unsigned int NVMWriteRow(void* address, void* data)
 *
 * Description:     The largest block of data that can be programmed in
 *                  a single operation is 1 row 128 instructions (512 Bytes).  The row at
 *                  the location pointed to by NVMADDR is programmed with the data buffer
 *                  pointed to by NVMSRCADDR.
 *
 * PreCondition:    The row of data must be pre-loaded into a buffer in RAM.
 *
 * Inputs:          address:  Destination Row address to write.
 *                  data:  Location of data to write.
 *
 * Output:          '0' if operation completed successfully.
 *
 * Example:         NVMWriteRow((void*) 0xBD000000, (void*) 0xA0000000)
 ********************************************************************/
//u32 NVMemWriteRow(void* address, void* data)
//{
//    u32 res;
//
//    // Set NVMADDR to Address of row t program
//    NVMADDR = KVA_TO_PA((u32)address);
//
//    // Set NVMSRCADDR to the SRAM data buffer Address
//    NVMSRCADDR = KVA_TO_PA((u32)data);
//
//    // Unlock and Write Row
//    res = NVMemOperation(NVMOP_ROW_PGM);
//
//    return res;
//}



/*********************************************************************
 * Function:        unsigned int NVMClearError(void)
 *
 * Description:		Clears the NVMCON error flag.
 *
 * PreCondition:    None
 *
 * Inputs:			None
 *
 * Output:          '0' if operation completed successfully.
 *
 * Example:			NMVClearError()
 ********************************************************************/
uint32_t NVMemClearError(void) {
    uint32_t res;

    res = NVMemOperation(NVMOP_NOP);

    return res;
}





/***********************End of File*************************************************************/
