#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_softap.h"

#define MANIFEST_URL "https://raw.githubusercontent.com/RbotJ/WebsiteIOT/main/firmware/c6-wroom-1/manifest.json"

static const char *TAG = "OTA";

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
static const char current_version[] = TOSTRING(FIRMWARE_VERSION);

char latest_version[32] = {0};
bool update_available = false;

bool check_for_update(char *latest_version_out, char *firmware_url_out) {
    esp_http_client_config_t config = {
        .url = MANIFEST_URL,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (esp_http_client_perform(client) != ESP_OK) {
        ESP_LOGE(TAG, "HTTP request failed");
        esp_http_client_cleanup(client);
        return false;
    }

    int content_length = esp_http_client_get_content_length(client);
    if (content_length <= 0) {
        ESP_LOGE(TAG, "Invalid content length: %d", content_length);
        esp_http_client_cleanup(client);
        return false;
    }

    char *buffer = malloc(content_length + 1);
    if (!buffer) {
        ESP_LOGE(TAG, "Memory allocation failed");
        esp_http_client_cleanup(client);
        return false;
    }

    int read_len = esp_http_client_read(client, buffer, content_length);
    if (read_len != content_length) {
        ESP_LOGE(TAG, "HTTP read error, expected %d, got %d", content_length, read_len);
        free(buffer);
        esp_http_client_cleanup(client);
        return false;
    }
    buffer[content_length] = '\0';

    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (!root) {
        ESP_LOGE(TAG, "JSON parsing failed");
        esp_http_client_cleanup(client);
        return false;
    }

    const cJSON *ver = cJSON_GetObjectItemCaseSensitive(root, "version");
    const cJSON *url = cJSON_GetObjectItemCaseSensitive(root, "firmware_url");

    if (!cJSON_IsString(ver) || !cJSON_IsString(url)) {
        ESP_LOGE(TAG, "Invalid JSON format");
        cJSON_Delete(root);
        esp_http_client_cleanup(client);
        return false;
    }

    snprintf(latest_version_out, 32, "%s", ver->valuestring);
    snprintf(firmware_url_out, 256, "%s", url->valuestring);

    cJSON_Delete(root);
    esp_http_client_cleanup(client);

    int cmp = strcmp(current_version, latest_version_out);
    return cmp < 0;
}

void do_ota_update(const char *url) {
    esp_http_client_config_t http_config = {
        .url = url,
        .cert_pem = NULL, // Replace with GitHub root certificate for production
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    ESP_LOGI(TAG, "Starting OTA update from %s", url);
    if (esp_https_ota(&ota_config) == ESP_OK) {
        ESP_LOGI(TAG, "OTA successful, marking app valid...");
        esp_ota_mark_app_valid_cancel_rollback();
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA failed");
    }
}

esp_err_t version_get_handler(httpd_req_t *req) {
    char response[128];
    snprintf(response, sizeof(response),
             "Current: %s\nLatest: %s\nUpdate Available: %s\n",
             current_version, latest_version, update_available ? "Yes" : "No");
    return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}

esp_err_t restart_post_handler(httpd_req_t *req) {
    httpd_resp_send(req, "Rebooting...\n", HTTPD_RESP_USE_STRLEN);
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
    return ESP_OK;
}

esp_err_t trigger_update_handler(httpd_req_t *req) {
    char firmware_url[256];
    if (check_for_update(latest_version, firmware_url)) {
        update_available = true;
        httpd_resp_send(req, "Updating firmware...\n", HTTPD_RESP_USE_STRLEN);
        vTaskDelay(pdMS_TO_TICKS(500));  // Ensure response is sent before restarting
        do_ota_update(firmware_url);
    } else {
        httpd_resp_send(req, "No update available.\n", HTTPD_RESP_USE_STRLEN);
    }
    return ESP_OK;
}

httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t version_uri = {
            .uri = "/version",
            .method = HTTP_GET,
            .handler = version_get_handler
        };
        httpd_register_uri_handler(server, &version_uri);

        httpd_uri_t update_uri = {
            .uri = "/update",
            .method = HTTP_POST,
            .handler = trigger_update_handler
        };
        httpd_register_uri_handler(server, &update_uri);

        httpd_uri_t restart_uri = {
            .uri = "/restart",
            .method = HTTP_POST,
            .handler = restart_post_handler
        };
        httpd_register_uri_handler(server, &restart_uri);
    }
    return server;
}

void wifi_prov_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data) {
    if (event_base == WIFI_PROV_EVENT) {
        if (event_id == WIFI_PROV_START) {
            ESP_LOGI(TAG, "Provisioning started");
        } else if (event_id == WIFI_PROV_CRED_SUCCESS) {
            ESP_LOGI(TAG, "Provisioning successful");
        } else if (event_id == WIFI_PROV_END) {
            wifi_prov_mgr_deinit();
        }
    }
}

void start_wifi_provisioning(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID, &wifi_prov_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_prov_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_prov_event_handler, NULL));

    wifi_prov_mgr_config_t prov_config = {
        .scheme = wifi_prov_scheme_softap,
        .scheme_event_handler = WIFI_PROV_EVENT_HANDLER_NONE
    };
    ESP_ERROR_CHECK(wifi_prov_mgr_init(prov_config));

    bool provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    if (!provisioned) {
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;
        const char *pop = "12345678";
        const char *service_name = "iot-ctrlr";
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, pop, service_name, NULL));
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting WiFi STA");
        wifi_config_t wifi_cfg;
#ifdef CONFIG_WIFI_PROV_MGR_GET_WIFI_CONFIG
        ESP_ERROR_CHECK(wifi_prov_mgr_get_wifi_config(&wifi_cfg));
#else
        ESP_LOGW(TAG, "wifi_prov_mgr_get_wifi_config not available, loading WiFi config from NVS");
        memset(&wifi_cfg, 0, sizeof(wifi_cfg));
        // TODO: Load your WiFi config from NVS here if you saved it during provisioning.
#endif
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
        ESP_ERROR_CHECK(esp_wifi_start());
    }
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    start_wifi_provisioning();
    vTaskDelay(pdMS_TO_TICKS(5000));

    char firmware_url[256];
    update_available = check_for_update(latest_version, firmware_url);
    if (update_available) {
        ESP_LOGI(TAG, "Update available: %s", latest_version);
    } else {
        ESP_LOGI(TAG, "Already up to date");
    }

    start_webserver();
}
