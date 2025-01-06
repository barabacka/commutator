/*
 * connections.c
 *
 *  Created on: Jan 2, 2025
 *      Author: Barabacka
 */
#include "main.h"
#include "defs.h"
#include "gpio.h"


typedef enum{
	GPIO_STABLE = 0,
	GPIO_PULSE,
}T_CTRL_TYPE;

typedef enum{
	CTRL_12V_EXT1_ON,
	CTRL_12V_EXT1_OFF,
	CTRL_SND_DAC_ON,
	CTRL_SND_EXT_ON,
	CTRL_PWR_RELAY_ON,
	CTRL_PWR_RELAY_OFF,
	CTRL_TOPC_REL_OFF,
	CTRL_TOPR_REL_ON,
	CTRL_BOTR_REL_ON,
	CTRL_BOTC_REL_OFF,
	CTRL_MAX
}CTRL_NAME_T;

typedef struct{
	CTRL_NAME_T		name;
	T_CTRL_TYPE		type;
	GPIO_TypeDef * 	port;
	uint16_t 		pit;
	GPIO_PinState 	state_on;
	int				delay;
	GPIO_PinState 	state_off;
}CTRL_ENTITY_T;

static const CTRL_ENTITY_T ctrl_dev[CTRL_MAX]={
	{ CTRL_12V_EXT1_ON,		GPIO_STABLE, 	GPIOB,	GPIO_PIN_5,		GPIO_PIN_SET, 	0, 0 },
	{ CTRL_12V_EXT1_OFF,	GPIO_STABLE, 	GPIOB,	GPIO_PIN_5, 	GPIO_PIN_RESET, 0, 0 },
	{ CTRL_SND_DAC_ON,		GPIO_PULSE,		GPIOA,	GPIO_PIN_13,	GPIO_PIN_SET, 	30, GPIO_PIN_RESET },
	{ CTRL_SND_EXT_ON,		GPIO_PULSE, 	GPIOA,	GPIO_PIN_11,	GPIO_PIN_SET, 	30, GPIO_PIN_RESET },
	{ CTRL_PWR_RELAY_ON,	GPIO_STABLE, 	GPIOB,	GPIO_PIN_4,		GPIO_PIN_SET, 	5, 	0 },
	{ CTRL_PWR_RELAY_OFF,	GPIO_STABLE, 	GPIOB,	GPIO_PIN_4, 	GPIO_PIN_RESET, 0, 	0 },
	{ CTRL_TOPC_REL_OFF,	GPIO_PULSE,		GPIOA,	GPIO_PIN_12,	GPIO_PIN_SET, 	50, GPIO_PIN_RESET },
	{ CTRL_TOPR_REL_ON,		GPIO_PULSE,		GPIOA,	GPIO_PIN_14,	GPIO_PIN_SET, 	50, GPIO_PIN_RESET },
	{ CTRL_BOTR_REL_ON,		GPIO_PULSE,		GPIOA,	GPIO_PIN_15,	GPIO_PIN_SET, 	50, GPIO_PIN_RESET },
	{ CTRL_BOTC_REL_OFF,	GPIO_PULSE,		GPIOB,	GPIO_PIN_3,		GPIO_PIN_SET, 	50, GPIO_PIN_RESET },
};

static const CTRL_NAME_T turn_music_on[]={
	CTRL_SND_DAC_ON,
	CTRL_PWR_RELAY_ON,
		CTRL_BOTR_REL_ON,
	CTRL_PWR_RELAY_OFF,
	CTRL_MAX
};

static const CTRL_NAME_T turn_music_off[]={
	CTRL_MAX
};

static const CTRL_NAME_T turn_tv_on[]={
	CTRL_12V_EXT1_ON,
	CTRL_SND_EXT_ON,
	CTRL_PWR_RELAY_ON,
		CTRL_BOTR_REL_ON,
		CTRL_TOPR_REL_ON,
	CTRL_PWR_RELAY_OFF,
	CTRL_MAX
};

static const CTRL_NAME_T turn_tv_off[]={
	CTRL_12V_EXT1_OFF,
	CTRL_PWR_RELAY_ON,
		CTRL_TOPC_REL_OFF,
	CTRL_PWR_RELAY_OFF,
	CTRL_MAX
};

static const CTRL_NAME_T turn_all_off[]={
	CTRL_12V_EXT1_OFF,
	CTRL_PWR_RELAY_ON,
		CTRL_BOTC_REL_OFF,
		CTRL_TOPC_REL_OFF,
	CTRL_PWR_RELAY_OFF,
	CTRL_MAX
};

static const CTRL_NAME_T * sw_ctrl[TURN_MAX]={
	turn_music_on,
	turn_tv_on,
	turn_music_off,
	turn_tv_off,
	turn_all_off
};

void switch_it ( T_SWITCH_CTRL state )
{

	CTRL_NAME_T * sw;
	if ( state < TURN_MAX )
	{
		sw = ( CTRL_NAME_T * )sw_ctrl[state];
		while ( *sw < CTRL_MAX ){
			switch ( ctrl_dev[*sw].type ){
				case GPIO_STABLE:
					HAL_GPIO_WritePin ( ctrl_dev[*sw].port, ctrl_dev[*sw].pit, ctrl_dev[*sw].state_on );
					if ( ctrl_dev[*sw].delay )
						HAL_Delay ( ctrl_dev[*sw].delay );
					break;
				case GPIO_PULSE:
					HAL_GPIO_WritePin ( ctrl_dev[*sw].port, ctrl_dev[*sw].pit, ctrl_dev[*sw].state_on );
					if ( ctrl_dev[*sw].delay )
						HAL_Delay ( ctrl_dev[*sw].delay );
					HAL_GPIO_WritePin ( ctrl_dev[*sw].port, ctrl_dev[*sw].pit, ctrl_dev[*sw].state_off );
					break;
				default:
					break;
			}

			sw++;
		}
	}

}
