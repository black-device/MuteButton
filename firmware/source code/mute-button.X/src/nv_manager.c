#include "nv_manager.h"
#include "crc/crc.h"
#include "app_config.h"
#include "nvmem/kmem.h"
#include "nvmem/nvmem.h"
#include <string.h>
#include <xc.h>



typedef struct {    
    uint16_t crc;
    uint16_t data_len;
    uint8_t data[256];
} cfg_raw_t;


NV_Manager_t NV_Manager_t_Default = {
    .veeprom_base_addr = (__KSEG0_PROGRAM_MEM_BASE + __KSEG0_PROGRAM_MEM_LENGTH - PIC_PAGE_SIZE),
    .veeprom_end_addr = (__KSEG0_PROGRAM_MEM_BASE + __KSEG0_PROGRAM_MEM_LENGTH),
    .cfg_changed = false,
};


static uint16_t VEEPROM_Read(uint32_t addr, void* data, uint16_t data_to_read);
static void VEEPROM_Clear_all();
static uint16_t VEEPROM_Write(uint32_t addr, void* data, uint16_t data_to_write);


static bool check_crc(cfg_raw_t* data);
static void set_default();


static cfg_raw_t cfg;
static NV_Manager_t* lib_cfg;


/**
 * Library initialization
 * @param _cfg
 * @return true if initialization was right
 */
bool NV_Mngr_Initialize(NV_Manager_t* _cfg) {
    lib_cfg = _cfg;
    lib_cfg->cfg_changed = false;
    
    VEEPROM_Read(0, (uint8_t*)&cfg, sizeof(cfg));
    
    if (!check_crc(&cfg)) {
        // CRC error retrieving data from VEEPROM. Set default config
        memset(cfg.data, 0xff, sizeof(cfg.data));
        cfg.data_len = 0;
    
        NV_Mngr_Update_Cfg();
        
        return false;
    }
    
    return true;
}


/**
 * Obtains a requested parameter, copying it in ret parameter
 * @param reg
 * @param ret pointer to structure where to copy the data
 * @return 0 if not found, other value-> the size of the register
 */
int NV_Mngr_Get_Reg(NV_Reg_t reg, void* ret) {
    
    uint16_t pos = 0;
    while(1) {
        if (pos >= cfg.data_len) {
            ret = NULL;
            return 0;
        }
        
        uint8_t field_size = cfg.data[pos + 1];
        
        if (cfg.data[pos] == reg.reg_ref) {
            memcpy(ret, &cfg.data[pos + 2], field_size);
            return field_size;
        }
        
        pos += field_size + 2;
    }
}




/**
 * Set a new value of a configuration parameter
 * @param reg
 * @param new_val
 * @param value_changed
 * @return 
 */
int NV_Mngr_Set_Reg(NV_Reg_t reg, void* new_val, bool* value_changed) {
    uint16_t pos = 0;
    uint8_t* new_val_ptr = (uint8_t*)new_val;
    
    if (value_changed != NULL) *value_changed = false;
    
    while(1) {
        if (pos >= cfg.data_len) break; // end of the buffer
        
        if (cfg.data[pos] != reg.reg_ref) {
            pos += cfg.data[pos + 1] + 2;
            continue;
        }
        
        // found register. Let's update it    
        int reg_size = cfg.data[++pos];
        bool v_changed = false;
        for (int i = 0; i < reg_size; i++) {
            if (cfg.data[++pos] != new_val_ptr[i]) {
                v_changed = true;
                cfg.data[pos] = new_val_ptr[i];
            }
        }

        if (v_changed) {
            lib_cfg->cfg_changed = true;
            // callback
            if (value_changed != NULL) *value_changed = true;
        }
        return reg_size;
    }
    
    // register not found. Let's add it
    pos = cfg.data_len;
    if ((pos + 2 + reg.reg_len) >= sizeof(cfg.data)) return -1; // not enough free space for a new parameter
    
    cfg.data[pos++] = reg.reg_ref;
    cfg.data[pos++] = reg.reg_len;
    memcpy(&cfg.data[pos], new_val, reg.reg_len);
    cfg.data_len = pos + reg.reg_len;
    lib_cfg->cfg_changed = true;
    
    if (value_changed != NULL) *value_changed = true;
    
    return reg.reg_len; 
}


/**
 * Indicates if the configuration has changed and VEEPROM update is needed
 * @return 
 */
bool NV_Mngr_Has_Changed() { return lib_cfg->cfg_changed; }


/**
 * Updates the configuration in the VEEPROM
 * @return 
 */
bool NV_Mngr_Update_Cfg() {
    int cnt = 4;
        
    while (cnt--) {
        cfg.crc = get_new_crc16(&cfg.data, cfg.data_len);
        VEEPROM_Write(0, (uint8_t*)&cfg, sizeof(cfg));
        
        cfg_raw_t tmp;
        VEEPROM_Read(0, &tmp, sizeof(tmp));
        if (check_crc(&tmp)) {
            lib_cfg->cfg_changed = false;
            return true;
        }
    }
    
    return false;
}



static bool check_crc(cfg_raw_t* data) {
    if (data->data_len > sizeof(data->data)) return false;
    
    uint16_t crc_calc = get_new_crc16(&data->data, data->data_len);
    
    return (crc_calc == data->crc);
}



/**
 * Read data from Virtual EEPROM
 * @param addr
 * @param data
 * @param data_to_read
 * @return Number of bytes read
 */
static uint16_t VEEPROM_Read(uint32_t addr, void* data, uint16_t data_to_read) {
    if (addr > PIC_PAGE_SIZE) return 0;
    
    uint16_t size = ((addr + data_to_read) > PIC_PAGE_SIZE) ? (PIC_PAGE_SIZE - addr) : data_to_read;
    
    memcpy(data, (void*)KVA0_TO_KVA1(lib_cfg->veeprom_base_addr + addr), size);
    
    return size;
}


/**
 * Clears all VEEPROM
 */
static void VEEPROM_Clear_all() {
    // erase flash page
    NVMemErasePage((void*)lib_cfg->veeprom_base_addr);
}


/**
 * Saves data in the VEEPROM
 * @param addr
 * @param data
 * @param data_to_write
 * @return bytes saved
 */
static uint16_t VEEPROM_Write(uint32_t addr, void* data, uint16_t data_to_write) {
    if (addr > PIC_PAGE_SIZE) return 0;
    
    // copy to RAM the VEEPROM
    uint8_t buff[PIC_PAGE_SIZE];
    memcpy(buff, (void*)KVA0_TO_KVA1(lib_cfg->veeprom_base_addr), PIC_PAGE_SIZE);
    
    // copy data to RAM buffer
    uint16_t size = ((addr + data_to_write) > PIC_PAGE_SIZE) ? (PIC_PAGE_SIZE - addr) : data_to_write;
    memcpy((buff + addr), data, size);
    
    // erase flash page
    NVMemErasePage((void*)lib_cfg->veeprom_base_addr);
    
    // copy buffer to flash
    for (int i = 0; i < PIC_PAGE_SIZE; i += sizeof(uint64_t)) {
        NVMemWriteUInt64((void*)(lib_cfg->veeprom_base_addr + i), (uint64_t*)(buff + i));
    }
    
    return size;
}