#pragma once

#include <stdint.h>
#include <stdbool.h>

#define PIC_PAGE_SIZE            (2048) // PIC32MM has a memory page of 2048 bytes //__(bin_firm_page_size)



typedef struct {
    uint8_t reg_ref;
    uint8_t reg_len;
} NV_Reg_t;


typedef struct {
    uint32_t veeprom_base_addr;
    uint32_t veeprom_end_addr;
    bool cfg_changed;
} NV_Manager_t;


extern NV_Manager_t NV_Manager_t_Default;


/**
 * Library initialization
 * @param _cfg
 * @return true if initialization was right
 */
bool NV_Mngr_Initialize(NV_Manager_t* _cfg);


/**
 * Obtains a requested parameter, copying it in ret parameter
 * @param reg
 * @param ret pointer to structure where to copy the data
 * @return 0 if not found, other value-> the size of the register
 */
int NV_Mngr_Get_Reg(NV_Reg_t reg, void* ret);


/**
 * Set a new value of a configuration parameter
 * @param reg
 * @param new_val
 * @param value_changed
 * @return 
 */
int NV_Mngr_Set_Reg(NV_Reg_t reg, void* new_val, bool* value_changed);


/**
 * Indicates if the configuration has changed and VEEPROM update is needed
 * @return 
 */
bool NV_Mngr_Has_Changed();


/**
 * Updates the configuration in the VEEPROM
 * @return 
 */
bool NV_Mngr_Update_Cfg();
