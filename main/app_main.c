#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_softap.h"

#define MANIFEST_URL "https://raw.githubusercontent.com/RbotJ/WebsiteIOT/main/firmware/c6-wroom-1/manifest.json"
char current_version[] = "1.0.2";
static const char *TAG = "OTA";


bool check_for_update(char *latest_version, char *firmware_url) {
    esp_http_client_config_t config = {
        .url = MANIFEST_URL,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int len = esp_http_client_get_content_length(client);
        char *buffer = malloc(len + 1);
        esp_http_client_read(client, buffer, len);
        buffer[len] = '\0';

        cJSON *root = cJSON_Parse(buffer);
        if (root) {
            const cJSON *ver = cJSON_GetObjectItemCaseSensitive(root, "version");
            const cJSON *url = cJSON_GetObjectItemCaseSensitive(root, "firmware_url");
            if (ver && url) {
                strcpy(latest_version, ver->valuestring);
                strcpy(firmware_url, url->valuestring);
                cJSON_Delete(root);
                free(buffer);
                esp_http_client_cleanup(client);
                return strcmp(current_version, latest_version) != 0;
            }
            cJSON_Delete(root);
        }
        free(buffer);
    }

    esp_http_client_cleanup(client);
    return false;
}

void do_ota_update(const char *url) {
    esp_http_client_config_t ota_config = {
        .url = url,
        .cert_pem = NULL,
    };
    if (esp_https_ota(&ota_config) == ESP_OK) {
        ESP_LOGI(TAG, "OTA successful, restarting...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA failed");
    }
}


void wifi_prov_event_handler(void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data) {
if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_START) {
ESP_LOGI(TAG, "Provisioning started");
} else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_CRED_SUCCESS) {
ESP_LOGI(TAG, "Provisioning successful");
} else if (event_base == WIFI_PROV_EVENT && event_id == WIFI_PROV_END) {
wifi_prov_mgr_deinit();
}
}

void start_wifi_provisioning() {
ESP_ERROR_CHECK(esp_netif_init());
ESP_ERROR_CHECK(esp_event_loop_create_default());
esp_netif_create_default_wifi_ap();

wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
ESP_ERROR_CHECK(esp_wifi_init(&cfg));
ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &wifi_prov_event_handler, NULL));
ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_prov_event_handler, NULL));
ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_prov_event_handler, NULL));

wifi_prov_mgr_config_t config = {
.scheme = wifi_prov_scheme_softap,
.scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
};
ESP_ERROR_CHECK(wifi_prov_mgr_init(config));

bool provisioned = false;
wifi_prov_mgr_is_provisioned(&provisioned);

if (!provisioned) {
wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
const char *pop = "12345678";
const char *service_name = "iot-ctrlr";
ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, pop, service_name, NULL));
} else {
ESP_LOGI(TAG, "Already provisioned, starting WiFi STA");
wifi_config_t wifi_cfg;
ESP_ERROR_CHECK(wifi_prov_mgr_get_wifi_config(&wifi_cfg));
ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
ESP_ERROR_CHECK(esp_wifi_start());
}
}


void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());
    start_wifi_provisioning();
    vTaskDelay(pdMS_TO_TICKS(5000));

    char latest_version[32];
    char firmware_url[256];
    if (check_for_update(latest_version, firmware_url)) {
        ESP_LOGI(TAG, "Update found: %s", latest_version, firmware_url);
        do_ota_update(firmware_url);
    } else {
        ESP_LOGI(TAG, "Already up to date");
    }
}