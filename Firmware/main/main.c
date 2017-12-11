#include <string.h>
#include <stdlib.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_spi_flash.h"
#include "lwip/inet.h"
#include "lwip/ip4_addr.h"

#include "socket_task.h"
#include "Buzzer.h"
#include "VL6180X.h"
#include "SW.h"
#include "bateria.h"
#include "hardware.h"
#include "WebServer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TAG "MAIN"
#define TIMER_OFF ((1000 * 60) * 5)		// 5 Minutos
#define MODE_STA 	0x00
#define MODE_AP		0x01

static ip4_addr_t ip_sta;
uint8_t flag_power = 0;
uint8_t flag_sensor = 0;
extern uint8_t PosicaoMao;
extern uint8_t BuzzerState;
extern int SensorCharge;
extern uint8_t erro_socket;
extern uint32_t lasteventsw1;
extern uint32_t lasteventsw2;
extern uint32_t lasteventsw1sw2;
extern uint16_t LimiteSuperiorCompressao;
extern uint16_t LimiteInferiorCompressao;
extern uint16_t LimiteSuperiorRespiracao;
extern uint16_t LimiteInferiorRespiracao;

uint16_t limite1;
uint16_t limite2;
uint16_t limite3;
uint16_t limite4;
uint8_t mode_wifi;
char ssid[32];
char password[64];

char versao[] = "Simulacare RCP v.1.0.4 07/12/2017";

EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

// Prototipos
uint8_t temprature_sens_read();

esp_err_t save_config()
{
	nvs_handle handle;
	esp_err_t esp_err;

	// Open
	esp_err = nvs_open("storage", NVS_READWRITE, &handle);
	if (esp_err != ESP_OK) return esp_err;

	esp_err = nvs_set_u8(handle, "mode", mode_wifi);
	if (esp_err != ESP_OK) return esp_err;

	esp_err = nvs_set_blob(handle, "ssid", ssid, 32);
	if (esp_err != ESP_OK) return esp_err;

	esp_err = nvs_set_blob(handle, "password", password, 64);
	if (esp_err != ESP_OK) return esp_err;

	limite1 = LimiteSuperiorCompressao;
	esp_err = nvs_set_u16(handle, "limite1", limite1);
	if (esp_err != ESP_OK) return esp_err;

	limite2 = LimiteInferiorCompressao;
	esp_err = nvs_set_u16(handle, "limite2", limite2);
	if (esp_err != ESP_OK) return esp_err;

	limite3 = LimiteSuperiorRespiracao;
	esp_err = nvs_set_u16(handle, "limite3", limite3);
	if (esp_err != ESP_OK) return esp_err;

	limite4 = LimiteInferiorRespiracao;
	esp_err = nvs_set_u16(handle, "limite4", limite4);
	if (esp_err != ESP_OK) return esp_err;

	esp_err = nvs_commit(handle);
	if (esp_err != ESP_OK) return esp_err;

	nvs_close(handle);
	return ESP_OK;
}


esp_err_t get_config(void)
{
	nvs_handle handle;
	esp_err_t err;

	printf("Opening Non-Volatile Storage (NVS) handle... \n");

    err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) {
    	printf("Error (%d) opening NVS handle!\n", err);
    	return err;
    }
    printf("Reading Variables from NVS ... \n");
    err = nvs_get_u8(handle, "mode_wifi", &mode_wifi);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    //printf("Mode_Wifi = %d\n", mode_wifi);

    size_t required_size = 0;
    err = nvs_get_blob(handle, "ssid", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    printf("SSID:\n");
    if(required_size == 0) {
    	printf("Nothing Saved Yet!\n");
    }
    else {
    	required_size = 32;
    	err = nvs_get_blob(handle, "ssid" , ssid, &required_size);
    	if(err != ESP_OK) return err;
    }

    required_size = 0;
    err = nvs_get_blob(handle, "password", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    printf("PASSWORD:\n");
    if(required_size == 0) {
    	printf("Nothing Saved Yet!\n");
    }
    else {
    	required_size = 64;
    	err = nvs_get_blob(handle, "password" , ssid, &required_size);
        if(err != ESP_OK) return err;
    }
    err = nvs_get_u16(handle, "limite1", &limite1);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    //printf("Limite1 = %d\n", limite1);

    err = nvs_get_u16(handle, "limite2", &limite2);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    //printf("Limite2 = %d\n", limite2);

    err = nvs_get_u16(handle, "limite3", &limite3);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    //printf("Limite3 = %d\n", limite3);

    err = nvs_get_u16(handle, "limite4", &limite4);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    //printf("Limite4 = %d\n", limite4);

    nvs_close(handle);
    return ESP_OK;
}

static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
    	printf("SYSTEM_EVENT_STA_GOT_IP\n");
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        ip_sta = event->event_info.got_ip.ip_info.ip;
        printf("Got IP: %s\n", inet_ntoa( ip_sta ));
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_START:
    	xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        printf("Access point started\n");
        break;
    case SYSTEM_EVENT_AP_STOP:
    	xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        printf("Access point SYSTEM_EVENT_AP_STOP\n");
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        printf("Access point SYSTEM_EVENT_AP_STACONNECTED\n");
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
    	printf("Access point SYSTEM_EVENT_AP_STADISCONNECTED\n");
    	xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    	break;
    case SYSTEM_EVENT_AP_PROBEREQRECVED:
    	xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    	printf("Access point SYSTEM_EVENT_AP_PROBEREQRECVED\n");
    	break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    mode_wifi = MODE_AP;

	printf("WIFI_MODE:%d\n", mode_wifi);

	if(mode_wifi == MODE_STA) {
    	tcpip_adapter_init();
    	wifi_event_group = xEventGroupCreate();
    	ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL) );
    	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    	wifi_config_t wifi_config = {
    			.sta = {
    					.ssid = "MONYTEL_C",
						.password = "Monytel_Comercial",
    			},
    	};
    	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    	ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    	ESP_ERROR_CHECK( esp_wifi_start() );
    	ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_NONE) );
    	ESP_LOGI(TAG, "Connecting to \"%s\"", wifi_config.sta.ssid);
    	xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    	ESP_LOGI(TAG, "Connected");
    }
    else {
        tcpip_adapter_init();
        wifi_event_group = xEventGroupCreate();
        ESP_ERROR_CHECK( esp_event_loop_init(wifi_event_handler, NULL) );
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
        ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_AP) );
        wifi_config_t apConfig = {
                .ap = {
                	.ssid = "SimulaCare-RCP",
                    .ssid_len = 0,
        	        .channel = 0,
    				.authmode = WIFI_AUTH_WPA2_PSK,
                    .password = "12345678",
                    .ssid_hidden = 0,
                    .max_connection = 4,
                    .beacon_interval = 100
                }
        };
        ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_AP, &apConfig) );
        ESP_ERROR_CHECK( esp_wifi_start() );
        //xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
    }
}

void app_main(void)
{
	// Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
    	// NVS partition was truncated and needs to be erased
	    // Retry nvs_flash_init
	    ESP_ERROR_CHECK(nvs_flash_erase());
	    err = nvs_flash_init();
	}
    gpio_set_direction(POWEROFF, GPIO_MODE_OUTPUT);			// POWEROFF
    gpio_set_level(LED_PAINEL, 0);

    gpio_set_direction(BUZZER, GPIO_MODE_OUTPUT);			// BUZZER
    gpio_set_direction(POWEROFF, GPIO_MODE_OUTPUT);			// POWEROFF
    gpio_set_direction(LED_ATV, GPIO_MODE_OUTPUT);			// LED ATV
    gpio_set_direction(CS0, GPIO_MODE_OUTPUT);				// CS0
    gpio_set_direction(CS1, GPIO_MODE_OUTPUT);				// CS1
    gpio_set_direction(LED_PAINEL, GPIO_MODE_OUTPUT);		// Led Painel

    gpio_set_direction(GPIO_NUM_25, GPIO_MODE_OUTPUT);		// GPIO_25
    gpio_set_direction(GPIO_NUM_26, GPIO_MODE_OUTPUT);		// GPIO_26
    gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT);		// GPIO_27
    gpio_set_direction(GPIO_NUM_14, GPIO_MODE_OUTPUT);		// GPIO_14
    gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);		// GPIO_12
    gpio_set_direction(GPIO_NUM_23, GPIO_MODE_OUTPUT);		// GPIO_23

    gpio_set_direction(SW2, GPIO_MODE_INPUT);				// SW2
    gpio_set_direction(SW1, GPIO_MODE_INPUT);				// SW1
    gpio_set_direction(SW3, GPIO_MODE_INPUT);				// SW3
    gpio_pullup_en(SW1); gpio_pulldown_dis(SW1);
    gpio_pullup_en(SW2); gpio_pulldown_dis(SW2);
    gpio_pullup_en(SW3); gpio_pulldown_dis(SW3);

    gpio_set_level(LED_ATV, 0);
    gpio_set_level(POWEROFF, 0);
    gpio_set_level(BUZZER, 0);
    gpio_set_level(CS0, 0);
    gpio_set_level(CS1, 0);

    gpio_set_level(GPIO_NUM_25, 0);
    gpio_set_level(GPIO_NUM_26, 0);
    gpio_set_level(GPIO_NUM_27, 0);
    gpio_set_level(GPIO_NUM_14, 0);
    gpio_set_level(GPIO_NUM_12, 0);
    gpio_set_level(GPIO_NUM_23, 0);

    printf("%s\n", versao);
    // Print chip information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("ESP32 chip with %d CPU cores, WiFi%s%s, ",
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    printf("Silicon revision %d, ", chip_info.revision);
    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    float temperatura = ((float)((temprature_sens_read() - 33) - 32 ) / 1.80);
    printf("ESP32 Temperature: %d °F\n", temprature_sens_read() - 33);
    printf("ESP32 Temperature: %2.1f °C\n", temperatura);
    printf("ESP32 SDK Version: %s\n", esp_get_idf_version());
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    printf("ESP32 MAC:%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    printf("ESP32 Free Heap Size: %08i\n", esp_get_minimum_free_heap_size());

    get_config();
    printf("Mode_WIFI: %d\n", mode_wifi);
    printf("SSID: %s\n", ssid);
    printf("PASSWD: %s\n", password);
    printf("Limit1: %d\n", limite1);
    printf("Limit2: %d\n", limite2);
    printf("Limit3: %d\n", limite3);
    printf("Limit4: %d\n", limite4);

    initialise_wifi();

    xTaskCreate(buzzer_task, "buzzer_task", 4096, NULL, 5, NULL);
    xTaskCreate(posmao_task, "posmao_task", 4096, NULL, 5, NULL);
    xTaskCreate(bateria_task, "bateria_task", 4096, NULL, 5, NULL);
    xTaskCreate(vl6180x_task, "vl6180x_task", 4096, NULL, 4, NULL);
    xTaskCreate(socket_task, "socket_task", 4096, NULL, 5, NULL);
    xTaskCreatePinnedToCore(&mongooseTask, "mongooseTask", 20000, NULL, 5, NULL,0);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    //play_on();					// Beep Ligado
    int level = 0;
    printf("W,%s,F\n", versao);
    printf("L,%d,%d,%d,%d,F\n", LimiteSuperiorCompressao, LimiteInferiorCompressao,
    		                    LimiteSuperiorRespiracao, LimiteInferiorRespiracao);

    // Habilita Sensores a Enviar Dados...
    vTaskDelay(50 / portTICK_PERIOD_MS);
    flag_sensor = 255;

    while (true) {
        gpio_set_level(LED_ATV, level);
        level = !level;
        if(erro_socket != 0) {
        	printf("Erro Socket\n");
        	esp_restart();
        }
        uint32_t timer_power = xTaskGetTickCount() * portTICK_PERIOD_MS;
        if(((timer_power - lasteventsw1) > TIMER_OFF) ||
           ((timer_power - lasteventsw2) > TIMER_OFF) ||
		   ((timer_power - lasteventsw1sw2) > TIMER_OFF)) {
        	// Se Passou 5 Min Sem Atividade Desliga....
            if(flag_power == 0) {
            	printf("Desliga Inatividade 5 min....\n");
            	flag_power = 255;
            	gpio_set_level(POWEROFF, 1);
            }
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}
