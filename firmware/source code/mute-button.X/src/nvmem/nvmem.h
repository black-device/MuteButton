#ifndef NVMEM7_H
#define	NVMEM7_H


#define PAGE_SIZE               (512)                // # of 32-bit Instructions per Page
#define BYTE_PAGE_SIZE          (4 * PAGE_SIZE)      // Page size in Bytes
#define ROW_SIZE                64                   // # of 32-bit Instructions per Row
#define BYTE_ROW_SIZE           (4 * ROW_SIZE)       // # Row size in Bytes
#define NUM_ROWS_PAGE           (PAGE_SIZE/ROW_SIZE) // Number of Rows per Page 

#define NVMOP_WORD_PGM          0x4001      // Word program operation
#define NVMOP_DBL_WORD_PGM      0x4002      // Double-Word program operation
#define NVMOP_PAGE_ERASE        0x4004      // Page erase operation
#define NVMOP_ROW_PGM           0x4003      // Row program operation
#define NVMOP_NOP               0x4000      // NOP operation


#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdint.h>
#include "kmem.h"


extern uint32_t NVMemWriteUInt64(void* address, uint64_t* data);
extern uint32_t NVMemErasePage(void* address);
extern uint32_t NVMemWriteRow(void* address, void* data);
extern uint32_t NVMemClearError(void);


#ifdef	__cplusplus
}
#endif

#endif	/* NVMEM7_H */

