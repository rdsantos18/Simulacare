#include <string.h>
#include <driver/i2c.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "VL6180X_I2C.h"
#include "sdkconfig.h"
#include "lwip/sockets.h"
#include "nvs_flash.h"
#include "esp_spp_api.h"

#define TAG "VL6180X"

// 1 Placa Final 0 placa Prototipo
#define PLACA_FINAL 1

#define	CS0			GPIO_NUM_19		// CS0
#define	CS1			GPIO_NUM_18		// CS1

#define SDA_PIN 21
#define SCL_PIN 22
//static const gpio_num_t SDA_PIN = GPIO_NUM_21;
//static const gpio_num_t SCL_PIN = GPIO_NUM_22;

#define ADDRESS_VL6180X 0x29

#define ADDRESS_COMPRESSAO 0x29
#define ADDRESS_RESPIRACAO 0x54

// To try different scaling factors, change the following define.
// Valid scaling factors are 1, 2, or 3.
#define SCALING 1

// Limites para Envio do Comando.
#define THRESHOLD_C 8
#define THRESHOLD_V 8

uint16_t compressao_data[10];
uint16_t respiracao_data[10];
uint16_t rasc_c[20];
uint16_t rasc_r[20];
uint16_t calibracao[10];
uint16_t LimiteSuperiorCompressao = 53;
uint16_t LimiteInferiorCompressao = 9;
uint16_t LimiteSuperiorRespiracao = 69;
uint16_t LimiteInferiorRespiracao = 17;
uint16_t value;
uint16_t respi;
uint16_t minc = 0;
uint16_t maxc = 0;
uint16_t minv = 0;
uint16_t maxv = 0;
uint16_t erro_c = 0;
uint16_t erro_v = 0;
uint32_t lasteventc;
uint32_t lasteventr;
uint32_t tmr_c;
uint32_t tmr_r;
uint8_t enable_compressao = 0;
uint8_t enable_respiracao = 0;
uint8_t status_compressao = 0;
uint8_t status_respiracao = 0;
uint8_t haste = 0;
uint8_t c_ctrl_20ms = 0;
uint8_t c_ctrl_100ms = 0;
uint8_t r_ctrl_20ms = 0;
uint8_t r_ctrl_100ms = 0;
extern uint8_t PosicaoMao;
extern int SocketClient;
extern uint8_t flag_sensor;
extern uint16_t limite1;
extern uint16_t limite2;
extern uint16_t limite3;
extern uint16_t limite4;
extern uint8_t flag_teste;
extern uint32_t bl_spp_handle;
extern uint8_t modo_comunicacao;
extern uint8_t bt_connected;

char msg[50];
uint8_t msg_bl[50];
int ret;

static char tag[] = "vl6180x";

struct VL6180X_data compressao;
struct VL6180X_data respiracao;

esp_err_t vl6180x_save_calibr()
{
	nvs_handle handle;
	esp_err_t esp_err;

	// Open
	esp_err = nvs_open("storage", NVS_READWRITE, &handle);
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

void print_status_range(char* msg, uint8_t status)
{
	if  ((status >= VL6180X_ERROR_SYSERR_1) && (status <= VL6180X_ERROR_SYSERR_5)) {
	    printf("%s System error\n" , msg);
	}
	else if (status == VL6180X_ERROR_ECEFAIL) {
		printf("%s ECE failure\n", msg);
	}
	else if (status == VL6180X_ERROR_NOCONVERGE) {
		printf("%s No convergence\n", msg);
	}
	else if (status == VL6180X_ERROR_RANGEIGNORE) {
		printf("%s Ignoring range\n", msg);
	}
	else if (status == VL6180X_ERROR_SNR) {
		printf("%s Signal/Noise error\n", msg);
	}
	else if (status == VL6180X_ERROR_RAWUFLOW) {
		printf("%s Raw reading underflow\n", msg);
	}
	else if (status == VL6180X_ERROR_RAWOFLOW) {
		printf("%s Raw reading overflow\n", msg);
	}
	else if (status == VL6180X_ERROR_RANGEUFLOW) {
		printf("%s Range reading underflow\n", msg);
	}
	else if (status == VL6180X_ERROR_RANGEOFLOW) {
		printf("%s Range reading overflow\n", msg);
	}
}

uint16_t vl6180x_max_min_compressao(uint16_t *data)
{
	uint8_t x;
	minc = data[0];
	maxc = 0;

	for(x = 0; x < 5; x++) {
		if(data[x] > maxc) maxc = data[x];
		else if(data[x] < minc) minc = data[x];
	}
	return 0;
}

uint16_t vl6180x_max_min_respiracao(uint16_t *data)
{
	uint8_t x;
	minv = data[0];
	maxv = 0;

	for(x = 0; x < 5; x++) {
		if(data[x] > maxv) maxv = data[x];
		else if(data[x] < minv) minv = data[x];
	}
	return 0;
}


void vl6180x_calibracao(uint16_t *data, uint8_t state)
{
   memcpy(calibracao, data, 10);
   //vl6180x_max_min(&calibracao[0]);
   switch(state){
   	   case 0:			// Retorna
   		   break;
   	   case 1:			// Calibra Compressao Superior
   		   break;
   	   case 2:	   		// Calibra Compressao Inferior
   		   break;
   	   case 3:			// Calibra Respiracao Superior
   		   break;
   	   case 4:			// Calibra Respiracao Inferior
   		   break;
   	   default:
   		   break;
   }
}

void vl6180x_task(void *ignore)
{
	ESP_LOGI(tag, ">> vl6180x");

	c_ctrl_20ms = 0;
	c_ctrl_100ms = 0;

	r_ctrl_20ms = 0;
	r_ctrl_100ms = 0;

	respiracao.i2c_address = 0x29;
	respiracao.ptp_offset = 0;
	respiracao.io_timeout = 500;
	respiracao.scaling = 1;
	respiracao.did_timeout = false;

	compressao.i2c_address = 0x29;
	compressao.ptp_offset = 0;
	compressao.io_timeout = 500;
	compressao.scaling = 1;
	compressao.did_timeout = false;

	// Inicia GPIO
	gpio_set_direction(CS0, GPIO_MODE_OUTPUT);			// CS0
	gpio_set_direction(CS1, GPIO_MODE_OUTPUT);			// CS1
	gpio_set_level(CS0, 0);
	gpio_set_level(CS1, 0);

	printf("Bus I2C init.\n");
	i2c_config_t conf;	
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = SDA_PIN;
	conf.scl_io_num = SCL_PIN;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = 400000;
	i2c_param_config(I2C_NUM_0, &conf);

	i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

	printf("Inicia Sensores VL6180X\n");
	erro_v = 0;
	erro_c = 0;

#if PLACA_FINAL
    printf("2 Sensores VL6180X\n");
#else
    printf("1 Sensor VL6180X\n");
#endif
    // Inicia Sensor VL6180X - Respiracao U1
	printf("VL6180X - Respiracao\n");
    gpio_set_level(CS0, 1);
	vTaskDelay(200 / portTICK_PERIOD_MS);

	VL6180X_init(&respiracao);
	VL6180X_configureDefault(&respiracao);
	VL6180X_setScaling(&respiracao, SCALING, 500);
	VL6180X_setAddress(&respiracao, ADDRESS_RESPIRACAO);
	printf("Sensor Respiracao: 0x%.2X Struct I2C: 0x%.2X\n", VL6180X_readReg(ADDRESS_RESPIRACAO, I2C_SLAVE_DEVICE_ADDRESS), respiracao.i2c_address);

	// Faz a Leitura de Identificação VL6180X
	respiracao.id = VL6180X_readReg(respiracao.i2c_address, IDENTIFICATION_MODEL_ID);
	respiracao.idModelRevMajor = VL6180X_readReg(respiracao.i2c_address, IDENTIFICATION_MODEL_REV_MAJOR);
	respiracao.idModelRevMinor  = VL6180X_readReg(respiracao.i2c_address, IDENTIFICATION_MODEL_REV_MINOR);
	respiracao.idModuleRevMajor = VL6180X_readReg(respiracao.i2c_address, IDENTIFICATION_MODULE_REV_MAJOR);
	respiracao.idModuleRevMinor = VL6180X_readReg(respiracao.i2c_address, IDENTIFICATION_MODULE_REV_MINOR);
	respiracao.idDate = VL6180X_readReg16Bit(respiracao.i2c_address, IDENTIFICATION_DATE);
	respiracao.idTime = VL6180X_readReg16Bit(respiracao.i2c_address, IDENTIFICATION_TIME);
	printf("Respiracao ID = 0x%.2X\n", respiracao.id);
	printf("Respiracao Model Rev = %d.%d\n", respiracao.idModelRevMajor, respiracao.idModelRevMinor);
	printf("Respiracao Module Rev = %d.%.d\n", respiracao.idModuleRevMajor, respiracao.idModuleRevMinor  );
	printf("Respiracao Manufacture Date = %d/%d/1%d Phase: %d\n", ((respiracao.idDate >> 3) & 0x001F), ((respiracao.idDate >> 8) & 0x000F), ((respiracao.idDate >> 12) & 0x000F), (respiracao.idDate & 0x0007));
	printf("Respiracao Manufacture Time (s)= %d\n", respiracao.idTime * 2);
	printf("Respiracao Data: %d\n", readRangeSingleMillimeters(&respiracao));
	if(respiracao.id != 0xB4) {
		printf("Sensor Respiração: Com Falha!!!\n");
	}
	printf("\n");

#if PLACA_FINAL
	// Inicia Sensor VL6180X - Compressao U2
	printf("VL6180X - Compressao\n");
	gpio_set_level(CS1, 1);
	vTaskDelay(200 / portTICK_PERIOD_MS);

	VL6180X_init(&compressao);
	VL6180X_configureDefault(&compressao);
	VL6180X_setScaling(&compressao, SCALING, 500);
	VL6180X_setAddress(&compressao, ADDRESS_COMPRESSAO);
	printf("Sensor Compressao: 0x%.2X Struct I2C: 0x%.2X\n", VL6180X_readReg(ADDRESS_COMPRESSAO, I2C_SLAVE_DEVICE_ADDRESS), compressao.i2c_address);
	// Faz a Leitura de Identificação VL6180X
	compressao.id = VL6180X_readReg(compressao.i2c_address, IDENTIFICATION_MODEL_ID);
	compressao.idModelRevMajor = VL6180X_readReg(compressao.i2c_address, IDENTIFICATION_MODEL_REV_MAJOR);
	compressao.idModelRevMinor  = VL6180X_readReg(compressao.i2c_address, IDENTIFICATION_MODEL_REV_MINOR);
	compressao.idModuleRevMajor = VL6180X_readReg(compressao.i2c_address, IDENTIFICATION_MODULE_REV_MAJOR);
	compressao.idModuleRevMinor = VL6180X_readReg(compressao.i2c_address, IDENTIFICATION_MODULE_REV_MINOR);
	compressao.idDate = VL6180X_readReg16Bit(compressao.i2c_address, IDENTIFICATION_DATE);
	compressao.idTime = VL6180X_readReg16Bit(compressao.i2c_address, IDENTIFICATION_TIME);
	printf("Compressao ID = 0x%.2X\n", compressao.id);
	printf("Compressao Model Rev = %d.%d\n", compressao.idModelRevMajor, compressao.idModelRevMinor);
	printf("Compressao Module Rev = %d.%.d\n", compressao.idModuleRevMajor, compressao.idModuleRevMinor  );
	printf("Compressao Manufacture Date = %d/%d/1%d Phase: %d\n", ((compressao.idDate >> 3) & 0x001F), ((compressao.idDate >> 8) & 0x000F), ((compressao.idDate >> 12) & 0x000F), (compressao.idDate & 0x0007));
	printf("Compressao Manufacture Time (s)= %d\n", compressao.idTime * 2);
    printf("Compressao Data: %d\n", readRangeSingleMillimeters(&compressao));
    if(compressao.id != 0xB4) {
		printf("Sensor Compressao: Com Falha!!!\n");
	}
    printf("\n");
#endif

	value = 0;
	respi = 0;
	while(1)
	{
	   if(flag_sensor != 0) {
#if PLACA_FINAL
		   // Compressao
		   if(compressao.id == 0xB4) {
			   value = readRangeSingleMillimeters(&compressao);
			   status_compressao = VL6180X_getrangestatus(&compressao);
			   if (status_compressao == VL6180X_ERROR_NONE) {
				   rasc_c[c_ctrl_20ms] = value;
				   c_ctrl_20ms++;
				   if(c_ctrl_20ms == 5) {
					   c_ctrl_20ms = 0;
				           compressao_data[c_ctrl_100ms] = ((rasc_c[0] + rasc_c[1] + rasc_c[2] +
				    		                                 rasc_c[3] + rasc_c[4]) / 10);
					   c_ctrl_100ms++;
					   if(c_ctrl_100ms == 5) {
						   c_ctrl_100ms = 0;
						   c_ctrl_20ms = 0;
						   enable_compressao = 255;
						   vl6180x_max_min_compressao(&compressao_data[0]);
						   if((maxc  - minc) > THRESHOLD_V) {
							   haste = 0;
							   lasteventc = xTaskGetTickCount() * portTICK_PERIOD_MS;
						   }
					   }
				   }
			   }
			   else {
				   print_status_range("Compressao-", status_compressao);
			   }
		   }
#endif
		   // Respiracao
		   if(respiracao.id == 0xB4) {
			   respi = readRangeSingleMillimeters(&respiracao);
			   status_respiracao = VL6180X_getrangestatus(&respiracao);
			   if(status_respiracao == VL6180X_ERROR_NONE) {
				   rasc_r[r_ctrl_20ms] = respi;
				   r_ctrl_20ms++;
				   if(r_ctrl_20ms == 5) {
					   r_ctrl_20ms = 0;
					   respiracao_data[r_ctrl_100ms] = ((rasc_r[0] + rasc_r[1] + rasc_r[2] +
							                             rasc_r[3] + rasc_r[4]) / 10);
					   r_ctrl_100ms++;
					   if(r_ctrl_100ms == 5) {
						   r_ctrl_100ms = 0;
						   r_ctrl_20ms = 0;
						   enable_respiracao = 255;
						   vl6180x_max_min_respiracao(&respiracao_data[0]);
						   if((maxv - minv) > THRESHOLD_V) {
							   haste = 255;
							   lasteventr = xTaskGetTickCount() * portTICK_PERIOD_MS;
						   }
					   }
				   }
			   }
			   else {
				   print_status_range("Respiracao-", status_respiracao);
			   }
		   }
		   // Transmite para  APP.
#if PLACA_FINAL
		   if(haste == 0 && enable_compressao) {
			   tmr_c = xTaskGetTickCount() * portTICK_PERIOD_MS;
			   printf("C,%d,%d,%d,%d,%d,%d,%d,%d,F\n",compressao_data[0], compressao_data[1],
					                                  compressao_data[2], compressao_data[3],
                                                      compressao_data[4], PosicaoMao, maxc, minc);
			   sprintf(msg, "C,%d,%d,%d,%d,%d,%d,F",compressao_data[0], compressao_data[1],
						                            compressao_data[2], compressao_data[3],
                                                    compressao_data[4], PosicaoMao);
			   if(modo_comunicacao) {
				   send(SocketClient, msg, strlen(msg), 0);
			   }
			   else {
				   if(bt_connected == 255) {
					   memcpy(msg_bl, msg, strlen(msg));
					   esp_spp_write(bl_spp_handle, strlen(msg), msg_bl);
				   }
			   }
			   enable_compressao = 0;
		   }
#endif
		   if( haste != 0 && enable_respiracao ) {
			   tmr_r = xTaskGetTickCount() * portTICK_PERIOD_MS;
			   printf("%d-V,%d,%d,%d,%d,%d;%d,%d,F\n",tmr_r, respiracao_data[0], respiracao_data[1],
		                                    	             respiracao_data[2], respiracao_data[3],
							                                 respiracao_data[4], maxv, minv);
			   sprintf(msg, "V,%d,%d,%d,%d,%d,F",respiracao_data[0], respiracao_data[1],
							                     respiracao_data[2], respiracao_data[3],
							                     respiracao_data[4]);
			   if(modo_comunicacao) {
				   send(SocketClient, msg, strlen(msg), 0);
			   }
			   else {
				   if(bt_connected == 255) {
					   memcpy(msg_bl, msg, strlen(msg));
					   esp_spp_write(bl_spp_handle, strlen(msg), msg_bl);
				   }
			   }
			   enable_respiracao = 0;
		   }
	   }	// End Flag_Sensor
	    vTaskDelay(20 / portTICK_PERIOD_MS);
	}	// End While
    vTaskDelete(NULL);
}
