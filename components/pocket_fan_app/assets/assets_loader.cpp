#include "assets_loader.h"
#include "assets.h"
#include <cstring>
#include <esp_log.h>

// Extern the map data from the .c file

static const char* TAG = "AssetsLoader";

void LoadStaticAssets()
{
    ESP_LOGI(TAG, "Loading static assets...");

    // Allocate on heap (PSRAM if available/configured)
    // Note: StaticAsset_t is large, ensure sufficient heap.
    StaticAsset_t* assets = new StaticAsset_t;
    
    if (!assets) {
        ESP_LOGE(TAG, "Failed to allocate StaticAsset_t");
        return;
    }

    // Load Startup Image
    // 124 * 22 * 3 bytes (RGB565 + Alpha8) = 8184 bytes
    // memcpy(assets->Image.Startup.logo, _start_124X22_RGB565A8_124x22_map, 8184);

    // Inject into AssetPool
    if (AssetPool::InjectStaticAsset(assets)) {
        ESP_LOGI(TAG, "Static assets injected successfully");
    } else {
        ESP_LOGE(TAG, "Failed to inject static assets");
        delete assets;
    }
}
