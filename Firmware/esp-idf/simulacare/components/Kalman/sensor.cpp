#include <string.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "lwip/sockets.h"
#include <stdio.h>
#include "sdkconfig.h"
#include "nvs_flash.h"
#include "kalman.h"
//#include "kalman2.h"

extern uint16_t compressao_data[];
extern uint16_t respiracao_data[];
extern uint8_t enable_compressao;
extern uint8_t enable_respiracao;
extern uint8_t PosicaoMao;
extern int SocketClient;
extern char msg[255];
extern uint32_t lasteventc;
extern uint32_t lasteventr;
float compressao_estimate_valor[10];
double compressao_correctedValue[10];
float respiracao_estimate_valor[10];
double respiracao_correctedValue[10];

uint8_t x;
uint8_t flag_teste = 0;
extern uint8_t respiracao_ready;
extern uint8_t compressao_ready;
extern uint8_t haste;


SimpleKalmanFilter compressao_filter(8, 4, 1);
SimpleKalmanFilter respiracao_filter(8, 4, 1);
//KalmanFilter compressao_filter2;
//KalmanFilter respiracao_filter2;

void sensor_task(void *ignore)
{
	//compressao_filter2.setState( 80 );
	//compressao_filter2.setCovariance(0.01);
	//respiracao_filter2.setState( 80 );
	//respiracao_filter2.setCovariance(0.1);
	while(1) {
		if(enable_compressao != 0) {
			for(x = 0; x < 5; x++ ) {
				//compressao_filter2.correct(compressao_data[x]);
				compressao_estimate_valor[x] = compressao_filter.updateEstimate(compressao_data[x]);
				//compressao_correctedValue[x] = compressao_filter2.getState();
				//printf("%d\t%.0f\n", compressao_data[x], compressao_correctedValue[x]);
			}
		}
		if(enable_respiracao != 0) {
			for(x = 0; x < 5; x++ ) {
				//respiracao_filter2.correct(respiracao_data[x]);
				respiracao_estimate_valor[x] = respiracao_filter.updateEstimate(respiracao_data[x]);
				//respiracao_correctedValue[x] = respiracao_filter2.getState();
				//printf("%d\t%.0f\n", respiracao_data[x], respiracao_correctedValue[x]);
			}
		}
		// Trasmite para APP.
		if(haste == 0 && enable_compressao){
			if(enable_compressao != 0) {
				enable_compressao = 0;
				printf("C,%.0f,%.0f,%.0f,%.0f,%.0f,%d,F\n",compressao_estimate_valor[0], compressao_estimate_valor[1],
														   compressao_estimate_valor[2], compressao_estimate_valor[3],
														   compressao_estimate_valor[4], PosicaoMao);
				sprintf(msg, "C,%.0f,%.0f,%.0f,%.0f,%.0f,%d,F",compressao_estimate_valor[0], compressao_estimate_valor[1],
															   compressao_estimate_valor[2], compressao_estimate_valor[3],
															   compressao_estimate_valor[4], PosicaoMao);
				send(SocketClient, msg, strlen(msg), 0);
				enable_compressao = 0;
				lasteventc = xTaskGetTickCount() * portTICK_PERIOD_MS;
			}
		}
		else if(haste !=0 && enable_respiracao){
			   printf("V,%.0f,%.0f,%.0f,%.0f,%.0f,F\n",respiracao_estimate_valor[0], respiracao_estimate_valor[1],
					   	   	   	   	   	   	   	   	   respiracao_estimate_valor[2], respiracao_estimate_valor[3],
													   respiracao_estimate_valor[4]);
			   sprintf(msg, "V,%.0f,%.0f,%.0f,%.0f,%.0f,F",respiracao_estimate_valor[0], respiracao_estimate_valor[1],
					   	   	   	   	   	   	   	   	   	   respiracao_estimate_valor[2], respiracao_estimate_valor[3],
														   respiracao_estimate_valor[4]);
			   send(SocketClient, msg, strlen(msg), 0);
			   enable_respiracao = 0;
			   lasteventr = xTaskGetTickCount() * portTICK_PERIOD_MS;
		}
		vTaskDelay(20 / portTICK_PERIOD_MS);
	}
   vTaskDelete(NULL);		
}
