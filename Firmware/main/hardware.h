#include "driver/gpio.h"

// Endereços de Hardware
#define BUZZER		GPIO_NUM_4		// BUZZER
#define	POWEROFF	GPIO_NUM_32		// POWEROFF
#define	LED_ATV		GPIO_NUM_2		// LED ATV
#define	CS0			GPIO_NUM_19		// CS0
#define	CS1			GPIO_NUM_18		// CS1
#define	LED_PAINEL	GPIO_NUM_33		// Led Painel
#define	SW2			GPIO_NUM_16		// SW2
#define	SW1			GPIO_NUM_17		// SW1
#define	SW3			GPIO_NUM_35		// SW3

#define VBAT		ADC1_CHANNEL_0	// Tensao Bateria
#define CHARGE		ADC1_CHANNEL_3	// Status Carga Bateria
