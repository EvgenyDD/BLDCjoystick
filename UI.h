/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _UI_H
#define _UI_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_bkp.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
enum{
	REG_ON_OFF_TIMES = BKP_DR1,
	REG_UPTIME_TOTAL_30secQuant = BKP_DR2,
	REG_UPTIME_SINCE_CHRG = BKP_DR3,
	REG_POWERACCU_CAPACITY = BKP_DR4
};


/* Exported functions ------------------------------------------------------- */
void MenuDispatcher();
void ProcessEncButton();
void Menu_processBackupData();
void ShutDown();
void ProcessEncoder();

#endif //_UI_H
