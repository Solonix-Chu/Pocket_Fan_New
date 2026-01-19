#include "hal_pocketfan/hal_pocketfan.h"
#include <lvgl.h> // Include LVGL
#include "nvs_flash.h" // Include NVS flash
#include "app_button.h" // Include app button
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"

static const char *TAG = "app_main";

extern "C" int ets_printf(const char *fmt, ...);
extern "C" void app_update(void);
extern "C" void app_init(lv_display_t* disp);

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    ESP_LOGE(TAG, "Stack overflow detected (task=%s)", pcTaskName ? pcTaskName : "unknown");
    // Avoid continuing in a corrupted state
    esp_restart();
}

extern "C" void vApplicationMallocFailedHook(void)
{
    ESP_LOGE(TAG, "Heap malloc failed");
    // Avoid continuing in a corrupted/unstable state
    esp_restart();
}

extern "C" void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    printf("free_heap_size = %d\n", (int)esp_get_free_heap_size());

    // usr_nimble_init();
    // app_button_init();

    ESP_LOGI(TAG, "LCD test_display");

    // Developed-by-Solonix-Chu Morse Code
    ESP_LOGI(TAG, "-.. . ...- . .-.. --- .--. . -.. -....- -... -.-- -....- ... --- .-.. --- -. .. -..- -....- -.-. .... ..- "); 
    
    auto hal = new HAL_PocketFan();
    HAL::Inject(hal);

    // Declare the external image descriptor
    // extern const lv_image_dsc_t _Page1_RGB565A8_128x64;

    // Create an image object to display the asset
    // lv_obj_t * img = lv_image_create(lv_screen_active());
    // lv_image_set_src(img, &_Page1_RGB565A8_128x64);
    // lv_obj_center(img);

    // Initialize the application
    app_init(NULL);

    printf("free_heap_size = %d\n", (int)esp_get_free_heap_size());

    uint32_t last_heap_log_ms = 0;

    while (1) {
        app_update();
        HAL::Get()->feedTheDog();
        uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000);
        if ((now_ms - last_heap_log_ms) >= 10000) {
            last_heap_log_ms = now_ms;
            ESP_LOGI(TAG, "heap free=%u min=%u largest=%u",
                     (unsigned)esp_get_free_heap_size(),
                     (unsigned)esp_get_minimum_free_heap_size(),
                     (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
        }
        // Yield CPU to avoid starving other tasks / idle (improves stability and WDT behavior)
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
