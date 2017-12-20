#include "esp_system.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "SW.h"

#define	SW2		GPIO_NUM_16	// SW2
#define	SW1		GPIO_NUM_17	// SW1

uint8_t pos_mao_sw1;
uint8_t pos_mao_sw2;
uint8_t lastState_sw1;
uint8_t lastState_sw2;
uint8_t flagtecla;
uint8_t Count;
uint8_t PosicaoMao;
uint32_t lasteventsw1;
uint32_t lasteventsw2;
uint32_t lasteventsw1sw2;
uint32_t lastDebounceSW1SW2;
extern uint8_t flag_sensor;

void posmao_task(void *pvParameters) {

   pos_mao_sw1 = 0;
   pos_mao_sw2 = 0;
   lastDebounceSW1SW2 = 0;
   flagtecla = 3;
   PosicaoMao = 5;
	   while(1) {
//		  if(flag_sensor != 0){
			  pos_mao_sw1 = gpio_get_level(SW1);	// Le Chave SW1
			  pos_mao_sw2 = gpio_get_level(SW2);	// Le Chave SW2
			  if ((pos_mao_sw1 != lastState_sw1) || (pos_mao_sw2 != lastState_sw2)) {
				  lastDebounceSW1SW2 = xTaskGetTickCount() * portTICK_PERIOD_MS;
			  }
			  if ((xTaskGetTickCount() - lastDebounceSW1SW2) >= 100) {
				  // Retorna a ler a chave depois de 100ms
				  vTaskDelay(100 / portTICK_PERIOD_MS);
				  pos_mao_sw1 = gpio_get_level(SW1);	// Le Chave SW1
				  pos_mao_sw2 = gpio_get_level(SW2);	// Le Chave SW2
				  // Determined by the state SW1 + SW2:
				  if ( (pos_mao_sw1 == 0) && (pos_mao_sw2 == 0) && (flagtecla == 0)) {
					  flagtecla = 3;
					  lasteventsw1sw2 = xTaskGetTickCount() * portTICK_PERIOD_MS;
					  Count++;
					  PosicaoMao = 4;
					  printf("SW1+SW2 LOW , PosMao: %d , Contador %d\n", PosicaoMao, Count);
				  }
				  else if ((pos_mao_sw1 == 1) && (pos_mao_sw2 == 1) && ((flagtecla == 3) ||
                          (flagtecla == 1) || (flagtecla == 2))) {
					  if(flagtecla == 3) {
						  lasteventsw1sw2 = xTaskGetTickCount() * portTICK_PERIOD_MS;
						  PosicaoMao = 5;
						  printf("SW1+SW2 HIGH\n");
					  }
					  if(flagtecla == 1) {
						  lasteventsw1 = xTaskGetTickCount() * portTICK_PERIOD_MS;
						  PosicaoMao = 1;
						  printf("SW1 HIGH\n");
					  }
					  if(flagtecla == 2) {
						  lasteventsw2 = xTaskGetTickCount() * portTICK_PERIOD_MS;
						  PosicaoMao = 3;
						  printf("SW2 HIGH\n");
					  }
					  flagtecla = 0;
				  }
				  else if ((pos_mao_sw1 == 0) && (pos_mao_sw2 == 1) && (flagtecla == 0) ) {
					  flagtecla = 1;
					  lasteventsw1 = xTaskGetTickCount() * portTICK_PERIOD_MS;
					  PosicaoMao = 0;
					  printf("SW1 LOW\n");
				  }
				  else if ((pos_mao_sw2 == 0) && (pos_mao_sw1 == 1) && (flagtecla == 0)) {
					  flagtecla = 2;
					  lasteventsw2 = xTaskGetTickCount() * portTICK_PERIOD_MS;
					  PosicaoMao = 2;
					  printf("SW2 LOW\n");
				  }
			  }
			  lastState_sw1 = pos_mao_sw1;
			  lastState_sw2 = pos_mao_sw2;
//		  }
	   } // while(1)
	   vTaskDelete(NULL);
}
