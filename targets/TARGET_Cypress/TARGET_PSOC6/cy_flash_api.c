/* mbed Microcontroller Library
 * Copyright (c) 2019, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "flash_api.h"
#include "cyhal_flash.h"

#if DEVICE_FLASH

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TARGET_CYTFM_064B0S2_4343W
#include "region_defs.h"

/* Flash layout for App on TARGET_CYTFM_064B0S2_4343W
 *   +--------------------------+
 *   |                          |
 *   |         KVSTORE          |  }+ MBED_CONF_STORAGE_TDB_INTERNAL_INTERNAL_SIZE
 *   |                          |
 *   +--------------------------+
 *   |                          |
 *   |                          |
 *   |  NS partition for App    |  }+ NS_PARTITION_SIZE
 *   |                          |
 *   |                          |
 *   +--------------------------+ <-+ NS_PARTITION_START
 */

static const cyhal_flash_block_info_t CYTFM_FLASH_BLOCKS[2] =
{
    // Main Flash
    {
        .start_address = NS_PARTITION_START,
        .size = NS_PARTITION_SIZE + MBED_CONF_STORAGE_TDB_INTERNAL_INTERNAL_SIZE,
        .sector_size = CY_FLASH_SIZEOF_ROW,
        .page_size = CY_FLASH_SIZEOF_ROW,
        .erase_value = 0x00U,
    },
    // Working Flash
    {
        .start_address = CY_EM_EEPROM_BASE,
        .size = CY_EM_EEPROM_SIZE,
        .sector_size = CY_FLASH_SIZEOF_ROW,
        .page_size = CY_FLASH_SIZEOF_ROW,
        .erase_value = 0x00U,
    },
};

#endif /* TARGET_CYTFM_064B0S2_4343W */

int32_t flash_init(flash_t *obj)
{
    if (CY_RSLT_SUCCESS != cyhal_flash_init(&(obj->flash))) {
        return -1;
    }
#ifdef TARGET_CYTFM_064B0S2_4343W
    obj->info.block_count =
         sizeof(CYTFM_FLASH_BLOCKS) / sizeof(cyhal_flash_block_info_t);
    obj->info.blocks = CYTFM_FLASH_BLOCKS;
#else /* TARGET_CYTFM_064B0S2_4343W */
    cyhal_flash_get_info(&(obj->flash), &(obj->info));
#endif /* TARGET_CYTFM_064B0S2_4343W */

    return 0;
}

int32_t flash_free(flash_t *obj)
{
    cyhal_flash_free(&(obj->flash));
    return 0;
}

int32_t flash_erase_sector(flash_t *obj, uint32_t address)
{
    return CY_RSLT_SUCCESS == cyhal_flash_erase(&(obj->flash), address) ? 0 : -1;
}

int32_t flash_read(flash_t *obj, uint32_t address, uint8_t *data, uint32_t size)
{
    return CY_RSLT_SUCCESS == cyhal_flash_read(&(obj->flash), address, data, size) ? 0 : -1;
}

int32_t flash_program_page(flash_t *obj, uint32_t address, const uint8_t *data, uint32_t size)
{
    MBED_ASSERT(0 == (address % obj->info.blocks[0].page_size));
    MBED_ASSERT(0 == (size % obj->info.blocks[0].page_size));
    for (uint32_t offset = 0; offset < size; offset += obj->info.blocks[0].page_size) {
        if (CY_RSLT_SUCCESS != cyhal_flash_program(&(obj->flash), address + offset, (const uint32_t *)(data + offset))) {
            return -1;
        }
    }
    return 0;
}

uint32_t flash_get_sector_size(const flash_t *obj, uint32_t address)
{
    if (address < obj->info.blocks[0].start_address || address >= obj->info.blocks[0].start_address + obj->info.blocks[0].size) {
        return MBED_FLASH_INVALID_SIZE;
    }
    return obj->info.blocks[0].sector_size;
}

uint32_t flash_get_page_size(const flash_t *obj)
{
    return obj->info.blocks[0].page_size;
}

uint32_t flash_get_start_address(const flash_t *obj)
{
    return obj->info.blocks[0].start_address;
}

uint32_t flash_get_size(const flash_t *obj)
{
    return obj->info.blocks[0].size;
}

uint8_t flash_get_erase_value(const flash_t *obj)
{
    return obj->info.blocks[0].erase_value;
}

#ifdef __cplusplus
}
#endif

#endif
