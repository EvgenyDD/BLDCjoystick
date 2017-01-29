/* Includes ------------------------------------------------------------------*/
#include "interface.h"
#include "buffer.h"
#include "nRF24L01.h"
#include "hw_hal.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
InterfaceStatusTypedef currentState;

/* Extern variables ----------------------------------------------------------*/
extern int16_t thrLevel, brkLevel;
extern uint32_t sysTickerMs;
extern uint32_t shutdownDelay;

float limDuty = 0.50;
float limCurr = 10.00;
float limSpeed = 15.0;
float limSmartSpeed = 1.0;
uint8_t invertMode = 0;

uint8_t cntcnt;

// Speed[km/h] = pi*Dwheel[km]*ERPM**12/30[gear ratio]*60[rpm->hour]    no (1/7[magnets/2])
#define KMH_TO_RPM (18.946)

mc_control_mode controlMode;

/* Private function prototypes -----------------------------------------------*/
void delay_ms(uint32_t nTime);

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : fmap
*******************************************************************************/
float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
    float out = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;

    if(out>out_max)
        return out_max;
    if(out < out_min)
        return out_min;

    return out;
}


/*******************************************************************************
* Function Name  : InterfaceInit
*******************************************************************************/
void InterfaceInit()
{
	currentState.packetTxSucc = 1;

	NRF_Init();
	delay_ms(50);

	NRF_ConfigChnlPld(115, 18);
	NRF_ConfigRatePwr(NRF_DataRate_250k, NRF_OutputPower_0dBm);
	NRF_WriteRegister(NRF_EN_RXADDR, 0);
	NRF_WriteRegister(NRF_DYNPD, 0);

	NRF_WriteRegister(NRF_SETUP_RETR, (3 << 4) | (7));
	NRF_WriteRegister(NRF_FEATURE, (1 << 2) | (1 << 1) | (1 << 0));

	NRF_WriteRegister(NRF_EN_AA, 1 << 0); 		//pipe 0
	NRF_WriteRegister(NRF_EN_RXADDR, 1 << 0); 	//pipe 0
	NRF_WriteRegister(NRF_DYNPD, 1 << 0); 		//pipe 0

	NRF_PowerUpRx();
}

/*******************************************************************************
* Function Name  : InterfaceCallback
* Description    :
*******************************************************************************/
void InterfaceCallback()
{
	uint8_t dataOut[32];
	int32_t ind = 0;

	dataOut[ind++] = 0x00;

	if(shutdownDelay == 0)
		dataOut[0] |= COM_NRF_OFF;

	if(brkLevel > 10)
	{
		dataOut[0] |= COM_NRF_BRAKE;
		buffer_append_float32(dataOut, fmap(brkLevel, 0.0, 100.0, 0, 20/*A*/), 1000.0, &ind);
	}
	else if(thrLevel > 10)
	{
		if(invertMode)
			dataOut[0] |= COM_NRF_INV;

		switch(controlMode)
		{
		case CONTROL_MODE_DUTY:
			dataOut[0] |= COM_NRF_DUTY;
			buffer_append_float32(dataOut, fmap(thrLevel, 0.0, 100.0, 0, limDuty), 1000.0, &ind);
			break;

		case CONTROL_MODE_SPEED:
			dataOut[0] |= COM_NRF_SPEED;
			buffer_append_float32(dataOut, fmap(thrLevel, 0.0, 100.0, 0, limSpeed * KMH_TO_RPM * 7 * 10), 1000.0, &ind);
			break;

		case CONTROL_MODE_CURRENT:
			dataOut[0] |= COM_NRF_CURR;
			buffer_append_float32(dataOut, fmap(thrLevel, 0.0, 100.0, 0, limCurr), 1000.0, &ind);
			break;

		case CONTROL_MODE_SMART_SPEED:
			dataOut[0] |= COM_NRF_SMRT_SPEED;
			buffer_append_float32(dataOut, fmap(thrLevel, 0.0, 100.0, 0, limSmartSpeed), 1000.0, &ind);
			break;

		default:
			buffer_append_float32(dataOut, 0.0, 1000.0, &ind);
			break;
		}
	}
	else
		buffer_append_float32(dataOut, 0.0, 1000.0, &ind);


	uint8_t failCounter = 0;

again:
	NRF_Send(dataOut, ind);

	while((currentState.packetTransmitStatus = NRF_GetTransmissionStatus()) == NRF_Transmit_Status_Sending);

	failCounter++;
	if(currentState.packetTransmitStatus == NRF_Transmit_Status_Lost && failCounter < 4)
	    goto again;

	NRF_CE_H;

	static uint32_t prev = 0;
	currentState.TxLatency = sysTickerMs - prev;
	prev = sysTickerMs;

	(currentState.packetTransmitStatus == NRF_Transmit_Status_Lost) ? currentState.packetTxFail++ : currentState.packetTxSucc++;

	NRF_PowerUpRx();
}


/*******************************************************************************
* Function Name  : SendPowerOff
*******************************************************************************/
#define RETR 20
void SendPowerOff()
{
	uint8_t dataOut[32];
	int32_t ind = 0;

	dataOut[ind++] = 0x00;

	dataOut[0] |= COM_NRF_OFF;

	buffer_append_float32(dataOut, 0.0, 1000.0, &ind);

	uint8_t failCounter = 0;

again:
	NRF_Send(dataOut, ind);

	while((currentState.packetTransmitStatus = NRF_GetTransmissionStatus()) == NRF_Transmit_Status_Sending);

	failCounter++;
	if(currentState.packetTransmitStatus == NRF_Transmit_Status_Lost && failCounter < RETR)
	{
		delay_ms(5);
	    goto again;
	}

	NRF_CE_H;
}


/*******************************************************************************
* Function Name  : InterfaceProcessInputPacket
*******************************************************************************/
void InterfaceProcessInputPacket(uint8_t *data, uint16_t len)
{
	shutdownDelay = SHDN_DELAY_MS;

    for(int i=0; i<len; i++)
        currentState.datas[i] = data[i];

    currentState.len = len;

	COM_ID_TYPE command = data[0];

	data++;
	len--;
	int32_t ind = 0;

	switch(command)
    {
    case COM_STATUS:

        currentState.speed = (float)buffer_get_int16(data, &ind) / KMH_TO_RPM;

        currentState.currentMtr = (int32_t) buffer_get_int16(data, &ind);
        currentState.currentBat = (int32_t) buffer_get_int16(data, &ind);
        currentState.voltBat = data[ind++];
        currentState.tempMotor = (int8_t) data[ind++];
        currentState.tempMos = (int8_t) data[ind++];
        currentState.faultCode = data[ind++];

        currentState.power = (float) currentState.currentBat * (float) currentState.voltBat / 100.0;

        currentState.capacityDisc = buffer_get_uint16(data, &ind);
        currentState.capacityChrg = buffer_get_uint16(data, &ind);
        break;

    case COM_SHDN:
    	shutdownDelay = 0;
    	break;

	case COM_TELEMETRY:
	    cntcnt++;
		for(uint8_t i = 0; i < 6; i++)
			currentState.voltCell[i] = data[ind++] + 200;

		break;

	default:
		break;
	}
}
