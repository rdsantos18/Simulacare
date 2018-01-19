#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
// Bluetooth
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
//
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
#include "hardware.h"
#include "kalman.h"

extern "C" {
	void app_main(void);
	void play_on(void);
	void play_off(void);
	uint8_t temprature_sens_read();
    void vl6180x_task(void *pvParameters);
	void posmao_task(void *pvParameters);
	void bateria_task(void *pvParameters);
    void buzzer_task(void *pvParameters);
	void socket_task(void *pvParameters);
	void mongooseTask(void *data);
	static void initialise_wifi(void);
	void bt_spp_init(void);
}

#define TAG "MAIN"
#define TIMER_OFF ((1000 * 60) * 5)		// 5 Minutos
#define MODE_STA 	0x00
#define MODE_AP		0x01
#define SPP_TAG "BT_SPP"
#define SPP_SERVER_NAME "SPP_SERVER"
#define SPP_DEVICE_NAME "SimulaCare-RCP"

static ip4_addr_t ip_sta;
uint8_t flag_power = 0;
uint8_t flag_sensor = 0;
uint32_t bl_spp_handle;
extern uint8_t PosicaoMao;
extern uint8_t BuzzerState;
extern int SensorCharge;
extern uint8_t erro_socket;
extern uint32_t lasteventsw1;
extern uint32_t lasteventsw2;
extern uint32_t lasteventsw1sw2;
extern uint32_t lasteventc;
extern uint32_t lasteventr;
extern uint16_t LimiteSuperiorCompressao;
extern uint16_t LimiteInferiorCompressao;
extern uint16_t LimiteSuperiorRespiracao;
extern uint16_t LimiteInferiorRespiracao;
extern uint8_t msg_bl[];
extern char msg[];

uint16_t limite1;
uint16_t limite2;
uint16_t limite3;
uint16_t limite4;
uint8_t mode_wifi;
char ssid[32];
char password[64];
char btname[32];
uint8_t mac_wifi[6];
uint8_t mac_bt[6];
uint8_t modo_comunicacao = 0; // Bluetooth = 0; WiFi = 1
uint8_t bt_connected = 0;

char versao[] = "SimulaCare RCP v.1.1.5 18/01/2018";

static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_NONE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event) {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_INIT_EVT");
        esp_read_mac(mac_bt, ESP_MAC_BT);
	    printf("MAC_BT:%02X:%02X:%02X:%02X:%02X:%02X\n", mac_bt[0],mac_bt[1],mac_bt[2],mac_bt[3],mac_bt[4],mac_bt[5]);
        esp_bt_dev_set_device_name(btname);
        esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        esp_spp_start_srv(sec_mask,role_slave, 0, SPP_SERVER_NAME);
        break;
    case ESP_SPP_DISCOVERY_COMP_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_DISCOVERY_COMP_EVT");
        break;
    case ESP_SPP_OPEN_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_OPEN_EVT");
        break;
    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CLOSE_EVT");
        bt_connected = 0;
        break;
    case ESP_SPP_START_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_START_EVT");
        break;
    case ESP_SPP_CL_INIT_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CL_INIT_EVT");
        break;
    case ESP_SPP_DATA_IND_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_DATA_IND_EVT len=%d handle=%d",
                 param->data_ind.len, param->data_ind.handle);
        esp_log_buffer_hex("",param->data_ind.data,param->data_ind.len);
        break;
    case ESP_SPP_CONG_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CONG_EVT");
        break;
    case ESP_SPP_WRITE_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_WRITE_EVT");
        break;
    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_SRV_OPEN_EVT");
        bl_spp_handle = param->srv_open.handle;
        bt_connected = 255;
        // Envia Versao
        memcpy(msg_bl, versao, sizeof(versao));
        esp_spp_write(bl_spp_handle, sizeof(versao), msg_bl);
        // Envia Limites
        sprintf(msg, "L,%d,%d,%d,%d,F", LimiteSuperiorCompressao, LimiteInferiorCompressao,
                                        LimiteSuperiorRespiracao, LimiteInferiorRespiracao);
        memcpy(msg_bl, msg, strlen(msg));
        esp_spp_write(bl_spp_handle, strlen(msg), msg_bl);
        break;
    default:
        break;
    }
}

void bt_spp_init()
{
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (esp_bt_controller_init(&bt_cfg) != ESP_OK) {
        ESP_LOGI(SPP_TAG, "%s initialize controller failed\n", __func__);
        return;
    }

    if (esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT) != ESP_OK) {
        ESP_LOGI(SPP_TAG, "%s enable controller failed\n", __func__);
        return;
    }

    if (esp_bluedroid_init() != ESP_OK) {
        ESP_LOGI(SPP_TAG, "%s initialize bluedroid failed\n", __func__);
        return;
    }

    if (esp_bluedroid_enable() != ESP_OK) {
        ESP_LOGI(SPP_TAG, "%s enable bluedroid failed\n", __func__);
        return;
    }

    if (esp_spp_register_callback(esp_spp_cb) != ESP_OK) {
        ESP_LOGI(SPP_TAG, "%s spp register failed\n", __func__);
        return;
    }

    if (esp_spp_init(esp_spp_mode) != ESP_OK) {
        ESP_LOGI(SPP_TAG, "%s spp init failed\n", __func__);
        return;
    }
}

esp_err_t save_config()
{
	nvs_handle handle;
	esp_err_t esp_err;

	// Open
	esp_err = nvs_open("storage", NVS_READWRITE, &handle);
	if (esp_err != ESP_OK) return esp_err;

	esp_err = nvs_set_u8(handle, "bluetooth", modo_comunicacao);
	if (esp_err != ESP_OK) return esp_err;

	esp_err = nvs_set_u8(handle, "mode", mode_wifi);
	if (esp_err != ESP_OK) return esp_err;

	esp_err = nvs_set_blob(handle, "ssid", ssid, 32);
	if (esp_err != ESP_OK) return esp_err;

	esp_err = nvs_set_blob(handle, "password", password, 64);
	if (esp_err != ESP_OK) return esp_err;

	esp_err = nvs_set_blob(handle, "btname", btname, 32);
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
    err = nvs_get_u8(handle, "bluetooth", &modo_comunicacao);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    //printf("Modo_Comunicacao = %d\n", modo_comunicacao);

    err = nvs_get_u8(handle, "mode_wifi", &mode_wifi);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    //printf("Mode_Wifi = %d\n", mode_wifi);

    size_t required_size = 0;
    err = nvs_get_blob(handle, "ssid", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    if(required_size == 0) {
    	printf("Nothing Saved Yet!\n");
    	return ESP_ERR_INVALID_ARG;
    }
    else {
    	required_size = 32;
    	err = nvs_get_blob(handle, "ssid" , ssid, &required_size);
    	if(err != ESP_OK) return err;
    	//else printf("SSID: %s\n", ssid);
    }

    required_size = 0;
    err = nvs_get_blob(handle, "password", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    if(required_size == 0) {
    	printf("Nothing Saved Yet!\n");
    	return ESP_ERR_INVALID_ARG;
    }
    else {
    	required_size = 64;
    	err = nvs_get_blob(handle, "password" , password, &required_size);
        if(err != ESP_OK) return err;
        //else printf("Password: %s\n", password);
    }

    required_size = 0;
    err = nvs_get_blob(handle, "btname", NULL, &required_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;
    if(required_size == 0) {
    	printf("Nothing Saved Yet!\n");
    	return ESP_ERR_INVALID_ARG;
    }
    else {
    	required_size = 32;
    	err = nvs_get_blob(handle, "btname" , btname, &required_size);
        if(err != ESP_OK) return err;
        //else printf("BTNAME: %s\n", btname);
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
		    esp_wifi_get_mac(WIFI_IF_STA, mac_wifi);
		    printf("MAC_WIF:%02X:%02X:%02X:%02X:%02X:%02X\n", mac_wifi[0],mac_wifi[1],mac_wifi[2],mac_wifi[3],mac_wifi[4],mac_wifi[5]);
    		wifi_config_t wifi_config = { };
    		strcpy((char *)wifi_config.sta.ssid, "MONYTEL_C");
    		strcpy((char *)wifi_config.sta.password, "Monytel_Comercial");
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
		    esp_wifi_get_mac(WIFI_IF_AP, mac_wifi);
        	printf("MAC_WIFI:%02X:%02X:%02X:%02X:%02X:%02X\n", mac_wifi[0],mac_wifi[1],mac_wifi[2],mac_wifi[3],mac_wifi[4],mac_wifi[5]);
        	wifi_config_t apConfig = { };
            strcpy((char *)apConfig.ap.ssid, ssid);
            apConfig.ap.ssid_len = 0;
            apConfig.ap.channel = 0;
            apConfig.ap.authmode = WIFI_AUTH_WPA2_PSK;
            strcpy((char *)apConfig.ap.password, password);
            apConfig.ap.ssid_hidden = 0;
            apConfig.ap.max_connection = 4;
            apConfig.ap.beacon_interval = 100;
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
    gpio_set_level(LED_PAINEL, 1);

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
    //printf("ESP32 Temperature: %d °F\n", temprature_sens_read() - 33);
    printf("ESP32 Temperature: %2.1f °C\n", temperatura);
    printf("ESP32 SDK Version: %s\n", esp_get_idf_version());
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    printf("ESP32 MAC:%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    printf("ESP32 Free Heap Size: %08i\n", esp_get_minimum_free_heap_size());

	//modo_comunicacao = 0;
	//mode_wifi = 1;
	//strcpy((char *)ssid, "SimulaCare-RCP");
	//printf("SSID: %s\n", ssid);
	//strcpy((char *)btname, "SimulaCare-RCP");
	//printf("BTNAME: %s\n", btname);
	//strcpy((char *)password, "12345678");
	//printf("Password: %s\n", password);
	//limite1 = limite1;
	//limite2 = limite2;
	//limite3 = limite3;
	//limite4 = limite4;

    err = get_config();
    if (err != ESP_OK) {
    	printf("Grava Programaçao Default!\n");
    	modo_comunicacao = 0;
		mode_wifi = 1;
		strcpy((char *)ssid, "SimulaCare-RCP");
		strcpy((char *)btname, "SimulaCare-RCP");
		strcpy((char *)password, "12345678");
		limite1 = limite1;
		limite2 = limite2;
		limite3 = limite3;
		limite4 = limite4;
    	save_config();
    }
//    else {
//    	printf("Programacao nao Salva!!!\n");
//    }

    printf("Modo_Comunicacao: %d\n", modo_comunicacao);
    printf("Mode_WIFI: %d\n", mode_wifi);
    printf("SSID: %s\n", ssid);
    printf("PASSWD: %s\n", password);
    printf("BTNAME: %s\n", btname);
    printf("Limit1: %d\n", limite1);
    printf("Limit2: %d\n", limite2);
    printf("Limit3: %d\n", limite3);
    printf("Limit4: %d\n", limite4);

    initialise_wifi();
    bt_spp_init();
    
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
        if(((timer_power - lasteventsw1) > TIMER_OFF) &&
           ((timer_power - lasteventsw2) > TIMER_OFF) &&
		   ((timer_power - lasteventsw1sw2) > TIMER_OFF) &&
		   ((timer_power - lasteventc) > TIMER_OFF) &&
		   ((timer_power - lasteventr) > TIMER_OFF)) {
        	// Se Passou 5 Min Sem Atividade Desliga....
            if(flag_power == 0) {
            	printf("Desliga Inatividade 5 min....\n");
            	printf("Var: SW1:%d,SW2:%d,SW:%d,C:%d,R:%d\n", lasteventsw1, lasteventsw2,lasteventsw1sw2
            			                                     , lasteventc, lasteventr);
            	flag_power = 255;
            	//play_off();
            	gpio_set_level(POWEROFF, 1);
            }
        }
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

