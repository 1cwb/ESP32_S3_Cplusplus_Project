#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "meventhandler.h"

struct stMeventInfo
{
    void* eventHandlerArg;
    esp_event_base_t eventBase;
    int32_t eventId;
    void* eventData;
};

class MEvent
{
public:
    using MEventHandlerCallback = esp_event_handler_t;
    static MEvent* getInstance()
    {
        static MEvent event;
        return &event;
    }

    MEvent(const MEvent&) = delete;
    MEvent(MEvent&&) = delete;
    MEvent& operator=(const MEvent&) =delete;
    MEvent& operator=(MEvent&&) =delete;

    bool registerEventHandlerInstance(esp_event_base_t eventBase,
                                              int32_t eventId,
                                              esp_event_handler_t eventHandler,
                                              void *eventHandlerArg,
                                              esp_event_handler_instance_t *context)
    {
        esp_err_t err = esp_event_handler_instance_register(eventBase, eventId, eventHandler, eventHandlerArg, context);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
    bool unregisterEventHandlerInstance(esp_event_base_t event_base, int32_t event_id, esp_event_handler_t event_handler)
    {
        esp_err_t err = esp_event_handler_unregister(event_base, event_id, event_handler);
        if(err != ESP_OK)
        {
            printf("Error: %s()%d %s\n",__FUNCTION__,__LINE__,esp_err_to_name(err));
            return false;
        }
        return true;
    }
private:
    MEvent()
    {
        if(esp_event_loop_create_default() != ESP_OK)
        {
            printf("Error : esp_event_loop_create_default fail\n");
        }
    }
    ~MEvent()
    {
        esp_event_loop_delete_default();
    }
};