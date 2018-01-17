/*
 * bateria.c
 *
 *  Created on: 7 de nov de 2017
 *      Author: rinaldo
 */

#include <string.h>
#include <errno.h>
#include "esp_err.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_adc_cal.h"
#include <esp_log.h>
#include "driver/gpio.h"
#include "lwip/sockets.h"
#include "esp_spp_api.h"
#include "kalman.h"
#include "kalman2.h"

extern "C" {
	float Le_Bateria(void);
	uint32_t Le_VoltageCarregador(void);
	int Le_Carregador(void);
	uint8_t State_Carregador(void);
	void Protege_Bateria(void);
	void bateria_task(void *pvParameters);
	extern void play_off(void);
	extern void play_bat_1(void);
	extern void play_bat_2(void);
}

#define VBAT		ADC1_CHANNEL_0	// Tensao Bateria
#define CHARGE		ADC1_CHANNEL_3	// Status Carga Bateria
#define	POWEROFF	GPIO_NUM_32		// POWEROFF
#define BATERIA_LEVEL	3200

#define TAG "BATERIA"

SimpleKalmanFilter FilterBateria(5, 5, 0.1);

uint8_t Carregador;
float BateriaVoltage;
uint32_t CarregadorVoltage;
uint8_t BateriaLevel;

int SensorCharge;
int ret;
uint8_t flag_bat1;
uint8_t flag_bat2;

extern uint8_t BuzzerState;
extern int SocketClient;
extern uint8_t flag_sensor;
extern uint32_t bl_spp_handle;
extern uint8_t modo_comunicacao;
extern uint8_t bt_connected;
extern char msg[];
extern uint8_t msg_bl[];

esp_adc_cal_characteristics_t vbat;
esp_adc_cal_characteristics_t charge;

float Le_Bateria(void)
{
	return FilterBateria.updateEstimate(adc1_to_voltage(VBAT, &vbat) * 2);
//	return (adc1_to_voltage(VBAT, &vbat) * 2);
}

uint32_t Le_VoltageCarregador(void)
{
	return (adc1_to_voltage(CHARGE, &charge));
}

int Le_Carregador(void)
{
	return (adc1_get_raw(CHARGE));
}

uint8_t State_Carregador(void)
{
	int value;
	uint8_t ret;

	value = Le_Carregador();
	if (value <= 10) ret = 0;								// ret = 0; Não Conectado
	else if ((value > 10) && (value <= 1000)) ret = 1;		// ret = 1; Carregando
	else if ((value > 1000) && (value <= 2000)) ret = 2;	// ret = 2; Falha Bateria
	else
		ret = 3;											// ret = 3; Bateria Carregada

	return ret;
}

void Protege_Bateria(void)
{
	// Define Niveis de Proteção de Bateria
	if ((BateriaLevel >= 10) && (BateriaLevel <= 25)) {
		// Bateria <= 25% Alarma
		if(flag_bat1 != 255) {
			printf("Bateria V:%.0f P:%d  Alarme 1\n", BateriaVoltage, BateriaLevel);
			play_bat_1();
			flag_bat1 = 255;
		}
	}
	if ((BateriaLevel > 5) && (BateriaLevel < 10)) {
		// Protege Bateria <= 10% Alarme Segundo Estagio
		if(flag_bat2 != 255) {
			printf("Bateria V:%.0f P:%d  Alarme 2\n", BateriaVoltage, BateriaLevel);
			printf("Desliga POWER OFF\n");
			play_bat_2();
		}
	}
	if (BateriaLevel <= 5) {
		printf("Desliga POWER OFF\n");
		play_off();
		gpio_set_level(POWEROFF, 1);
		while(1);		// Aguarda Desligar....
	}
	if (BateriaLevel > 25) {
		flag_bat1 = 0;
		flag_bat2 = 0;
	}
}

void bateria_task(void *pvParameters)
{
	// Init ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    esp_adc_cal_get_characteristics(1100, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, &vbat);
    // channel 3
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
    esp_adc_cal_get_characteristics(1100, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, &charge);
	//
	Carregador = 0;
    BateriaVoltage = 0;
	CarregadorVoltage = 0;
	BateriaLevel = 0;
    SensorCharge = 0;
    flag_bat1 = 0;
    flag_bat2 = 0;

	while(1)
	{
		Carregador = State_Carregador();
		SensorCharge = Le_Carregador();
		BateriaVoltage = Le_Bateria();
		BateriaLevel = (BateriaVoltage - BATERIA_LEVEL) / 10;
		if(BateriaLevel >= 100) BateriaLevel = 100;
		if(BateriaLevel <= 0) BateriaLevel = 0;
		Protege_Bateria();

		if(flag_sensor != 0) {
			printf("B,%d,%.0f,%d,F\n", Carregador, BateriaVoltage, BateriaLevel);
			sprintf(msg, "B,%d,%d,F",Carregador, BateriaLevel);
			if(modo_comunicacao) {
				send(SocketClient, msg, strlen(msg), 0);
			}
			else {
				if(bt_connected == 255) {
					memcpy(msg_bl, msg, strlen(msg));
					esp_spp_write(bl_spp_handle, strlen(msg), msg_bl);
				}
			}
		}
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}
