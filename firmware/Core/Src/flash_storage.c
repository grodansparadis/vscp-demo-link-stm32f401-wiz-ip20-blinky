// FILE: flash_storage.c

/* ******************************************************************************
 * 	VSCP (Very Simple Control Protocol)
 * 	https://www.vscp.org
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2000-2026 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ******************************************************************************
 */

#include "flash_storage.h"

/* ------------------------------------------------------------------ */

bool flash_storage_is_valid(void)
{
    flash_storage_header_t hdr;
    flash_storage_read(0U, (uint16_t *)&hdr, sizeof(hdr) / sizeof(uint16_t));
    return (hdr.magic == FLASH_STORAGE_MAGIC);
}

/* ------------------------------------------------------------------ */

HAL_StatusTypeDef flash_storage_write_header(void)
{
    const flash_storage_header_t hdr = {
        .magic   = FLASH_STORAGE_MAGIC,
        .version = FLASH_STORAGE_VERSION,
    };
    return flash_storage_write(0U, (const uint16_t *)&hdr, sizeof(hdr) / sizeof(uint16_t));
}

/* ------------------------------------------------------------------ */

HAL_StatusTypeDef flash_storage_erase(void)
{
    FLASH_EraseInitTypeDef erase_init = {
        .TypeErase    = FLASH_TYPEERASE_SECTORS,
        .Sector       = FLASH_STORAGE_SECTOR,
        .NbSectors    = FLASH_STORAGE_PAGES,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3,  /* 2.7 V – 3.6 V supply */
    };
    uint32_t sector_error = 0U;
    HAL_StatusTypeDef status;

    HAL_FLASH_Unlock();
    status = HAL_FLASHEx_Erase(&erase_init, &sector_error);
    HAL_FLASH_Lock();

    return status;
}

/* ------------------------------------------------------------------ */

HAL_StatusTypeDef flash_storage_write(uint32_t offset, const uint16_t *data, size_t count)
{
    HAL_StatusTypeDef status = HAL_OK;

    if (data == NULL) {
        return HAL_ERROR;
    }

    HAL_FLASH_Unlock();

    for (size_t i = 0U; i < count && status == HAL_OK; i++) {
        uint32_t addr = FLASH_STORAGE_BASE + offset + (i * 2U);

        /* Bounds check */
        if ((addr + 2U) > (FLASH_STORAGE_BASE + FLASH_STORAGE_SIZE)) {
            status = HAL_ERROR;
            break;
        }

        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, (uint64_t)data[i]);
    }

    HAL_FLASH_Lock();

    return status;
}

/* ------------------------------------------------------------------ */

void flash_storage_read(uint32_t offset, uint16_t *data, size_t count)
{
    if (data == NULL) {
        return;
    }

    for (size_t i = 0U; i < count; i++) {
        uint32_t addr = FLASH_STORAGE_BASE + offset + (i * 2U);
        data[i] = *(const volatile uint16_t *)addr;
    }
}

/* ------------------------------------------------------------------ */
/*                          Record storage                            */
/* ------------------------------------------------------------------ */

#define FLASH_STORAGE_WORDS (FLASH_STORAGE_SIZE / 2U)

static uint32_t s_next_free = 0xFFFFFFFFUL; /* Word index of the next free record slot (unknown) */

static uint16_t flash_word(uint32_t word)
{
    return *(const volatile uint16_t *)(FLASH_STORAGE_BASE + (word * 2U));
}

static uint16_t record_checksum(const uint16_t *data, size_t count)
{
    uint32_t sum = (uint32_t)count;
    for (size_t i = 0U; i < count; i++) {
        sum += data[i];
        sum = (sum & 0xFFFFU) + (sum >> 16);
    }
    return (uint16_t)~sum;
}

static uint16_t flash_record_checksum(uint32_t word, size_t count)
{
    uint32_t sum = (uint32_t)count;
    for (size_t i = 0U; i < count; i++) {
        sum += flash_word(word + 2U + i);
        sum = (sum & 0xFFFFU) + (sum >> 16);
    }
    return (uint16_t)~sum;
}

static bool slot_is_blank(uint32_t word, size_t words)
{
    for (size_t i = 0U; i < words; i++) {
        if (0xFFFFU != flash_word(word + i)) {
            return false;
        }
    }
    return true;
}

/* Find the newest valid record (returns its word index or 0xFFFFFFFF) and
   the first blank slot after the used part. */
static uint32_t scan_records(size_t count, uint32_t *pnext_free)
{
    const size_t slot = 3U + count;
    uint32_t newest   = 0xFFFFFFFFUL;
    uint32_t word     = 0U;

    while ((word + slot) <= FLASH_STORAGE_WORDS) {
        if ((FLASH_STORAGE_RECORD_MAGIC == flash_word(word)) && (count == flash_word(word + 1U)) &&
            (flash_record_checksum(word, count) == flash_word(word + 2U + count))) {
            newest = word;
        }
        else if (slot_is_blank(word, slot)) {
            break; /* Records are appended, so the rest is free */
        }
        word += slot; /* Valid, or a damaged/foreign slot: skip it */
    }
    *pnext_free = word;
    return newest;
}

bool flash_storage_load(uint16_t *data, size_t count)
{
    if ((NULL == data) || (0U == count)) {
        return false;
    }

    uint32_t next_free;
    uint32_t newest = scan_records(count, &next_free);
    s_next_free     = next_free;

    if (0xFFFFFFFFUL != newest) {
        for (size_t i = 0U; i < count; i++) {
            data[i] = flash_word(newest + 2U + i);
        }
        return true;
    }

    /* Old layout: header at offset 0 and the data right after it */
    flash_storage_header_t hdr;
    flash_storage_read(0U, (uint16_t *)&hdr, sizeof(hdr) / sizeof(uint16_t));
    if ((FLASH_STORAGE_MAGIC == hdr.magic) && (FLASH_STORAGE_VERSION == hdr.version)) {
        flash_storage_read(FLASH_STORAGE_DATA_OFFSET, data, count);
        return true;
    }

    return false;
}

HAL_StatusTypeDef flash_storage_save(const uint16_t *data, size_t count)
{
    if ((NULL == data) || (0U == count) || ((3U + count) > FLASH_STORAGE_WORDS)) {
        return HAL_ERROR;
    }

    const size_t slot = 3U + count;
    uint32_t word     = s_next_free;

    if (0xFFFFFFFFUL == word) {
        (void)scan_records(count, &word);
    }

    /* Full, or the slot is not blank (damaged): start over in an erased sector */
    if (((word + slot) > FLASH_STORAGE_WORDS) || !slot_is_blank(word, slot)) {
        HAL_StatusTypeDef status = flash_storage_erase();
        if (HAL_OK != status) {
            return status;
        }
        word = 0U;
    }

    const uint16_t head[2]  = { 0xFFFFU, (uint16_t)count };
    const uint16_t checksum = record_checksum(data, count);
    HAL_StatusTypeDef status;

    /* Length, data and checksum first, the magic last */
    status = flash_storage_write((word + 1U) * 2U, &head[1], 1U);
    if (HAL_OK == status) {
        status = flash_storage_write((word + 2U) * 2U, data, count);
    }
    if (HAL_OK == status) {
        status = flash_storage_write((word + 2U + count) * 2U, &checksum, 1U);
    }
    if (HAL_OK == status) {
        const uint16_t magic = FLASH_STORAGE_RECORD_MAGIC;
        status               = flash_storage_write(word * 2U, &magic, 1U);
    }

    s_next_free = (HAL_OK == status) ? (word + slot) : 0xFFFFFFFFUL;
    return status;
}
