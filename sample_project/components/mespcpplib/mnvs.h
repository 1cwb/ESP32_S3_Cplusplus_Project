#pragma once

#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

class MNvsBase
{
public:
    static MNvsBase* getINstance()
    {
        static MNvsBase mnvs;
        return &mnvs;
    }

private:
    MNvsBase()
    {
        // Initialize NVS
        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            // NVS partition was truncated and needs to be erased
            // Retry nvs_flash_init
            ESP_ERROR_CHECK(nvs_flash_erase());
            err = nvs_flash_init();
            ESP_ERROR_CHECK( err );
        }
    }
    ~MNvsBase()
    {
        esp_err_t err = nvs_flash_deinit();
        ESP_ERROR_CHECK( err );
    }
    MNvsBase(const MNvsBase&) = delete;
    MNvsBase(MNvsBase&&) = delete;
    MNvsBase& operator=(const MNvsBase& ) = delete;
    MNvsBase& operator=(MNvsBase&& ) = delete;
};

class MNvs
{
public:
    MNvs()
    {
        MNvsBase::getINstance();
    }
    ~MNvs()
    {
    }
    MNvs(const MNvs&) = delete;
    MNvs(MNvs&&) = delete;
    MNvs& operator=(const MNvs& ) = delete;
    MNvs& operator=(MNvs&& ) = delete;
    bool open(const char* namespaceName, nvs_open_mode_t open_mode = NVS_READWRITE)
    {
        esp_err_t err = nvs_open(namespaceName, open_mode, &myHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    void close()
    {
        nvs_close(myHandle_);
    }
    bool eraseKey(const char* key)
    {
        esp_err_t err = nvs_erase_key(myHandle_, key);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool eraseAll()
    {
        esp_err_t err = nvs_erase_all(myHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool seti8(const char* key, int8_t value)
    {
        esp_err_t err = nvs_set_i8(myHandle_, key, value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool setu8(const char* key, uint8_t value)
    {
        esp_err_t err = nvs_set_u8(myHandle_, key, value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool seti16(const char* key, int16_t value)
    {
        esp_err_t err = nvs_set_i16(myHandle_, key, value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool setu16(const char* key, uint16_t value)
    {
        esp_err_t err = nvs_set_u16(myHandle_, key, value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }    
    bool seti32(const char* key, int32_t value)
    {
        esp_err_t err = nvs_set_i32(myHandle_, key, value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool setu32(const char* key, uint32_t value)
    {
        esp_err_t err = nvs_set_u32(myHandle_, key, value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool seti64(const char* key, int64_t value)
    {
        esp_err_t err = nvs_set_i64(myHandle_, key, value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool setu64(const char* key, uint64_t value)
    {
        esp_err_t err = nvs_set_u64(myHandle_, key, value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool commit()
    {
        esp_err_t err = nvs_commit(myHandle_);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool setStr(const char* key, const char* value)
    {
        esp_err_t err = nvs_set_str(myHandle_, key, value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool setBlob(const char* key, const void* value, size_t length)
    {
        esp_err_t err = nvs_set_blob(myHandle_, key, value, length);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool geti8(const char* key, int8_t* out_value)
    {
        esp_err_t err = nvs_get_i8(myHandle_, key, out_value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool getu8(const char* key, uint8_t* out_value)
    {
        esp_err_t err = nvs_get_u8(myHandle_, key, out_value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool geti16(const char* key, int16_t* out_value)
    {
        esp_err_t err = nvs_get_i16(myHandle_, key, out_value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool getu16(const char* key, uint16_t* out_value)
    {
        esp_err_t err = nvs_get_u16(myHandle_, key, out_value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool geti32(const char* key, int32_t* out_value)
    {
        esp_err_t err = nvs_get_i32(myHandle_, key, out_value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool getu32(const char* key, uint32_t* out_value)
    {
        esp_err_t err = nvs_get_u32(myHandle_, key, out_value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool geti64(const char* key, int64_t* out_value)
    {
        esp_err_t err = nvs_get_i64(myHandle_, key, out_value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool getu64(const char* key, uint64_t* out_value)
    {
        esp_err_t err = nvs_get_u64(myHandle_, key, out_value);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool getStr(const char* key, char* out_value, size_t* length)
    {
        esp_err_t err =  nvs_get_str(myHandle_, key, out_value, length);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool getBlob(const char* key, void* out_value, size_t* length)
    {
        esp_err_t err =  nvs_get_blob(myHandle_, key, out_value, length);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool getStats(const char* part_name, nvs_stats_t* nvs_stats)
    {
        esp_err_t err = nvs_get_stats(part_name, nvs_stats);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool getUsedEntryCount(size_t* used_entries)
    {
        esp_err_t err = nvs_get_used_entry_count(myHandle_, used_entries);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
private:
    nvs_handle_t myHandle_;
};