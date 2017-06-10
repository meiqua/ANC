/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "waverecorder.h" 
#include "string.h"
#include "pdm_filter.h"

extern __IO uint32_t LEDsState;

uint16_t WrBuffer[WR_BUFFER_SIZE]; //4096

static uint16_t RecBuf[PCM_OUT_SIZE*2];/* PCM stereo samples are saved in RecBuf */
//16*2
static uint16_t InternalBuffer[INTERNAL_BUFF_SIZE];
__IO uint32_t ITCounter = 0;

/* Temporary data sample */
__IO uint32_t AUDIODataReady = 0, AUDIOBuffOffset = 0;

/**
  * @brief  Stop Audio recording.
  * @param  None
  * @retval None
  */
uint32_t WaveRecorderStop(void)
{
  return BSP_AUDIO_IN_Stop();
}

/**
  * @brief  Update the recorded data. 
  * @param  None
  * @retval None
  */
void WaveRecorderProcess(void)
{     
  LEDsState = LEDS_OFF;
	
	//useful
  BSP_AUDIO_IN_Init(DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR);
	// 16K 16bit mono
  BSP_AUDIO_IN_Record((uint16_t*)&InternalBuffer[0], INTERNAL_BUFF_SIZE);
	// buffer size: 128
  
  LEDsState = LED3_TOGGLE;
}

/**
  * @brief  Calculates the remaining file size and new position of the pointer.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
  /* PDM to PCM data convert */
  BSP_AUDIO_IN_PDMToPCM((uint16_t*)&InternalBuffer[INTERNAL_BUFF_SIZE/2], (uint16_t*)&RecBuf[0]);
  
  /* Copy PCM data in internal buffer *///counted in bytes, so *4
  memcpy((uint16_t*)&WrBuffer[ITCounter * (PCM_OUT_SIZE*2)], RecBuf, PCM_OUT_SIZE*4);
  
  if(ITCounter == (WR_BUFFER_SIZE/(PCM_OUT_SIZE*4))-1)
  {
    AUDIODataReady = 1;
    AUDIOBuffOffset = 0;
    ITCounter++;
  }
  else if(ITCounter == (WR_BUFFER_SIZE/(PCM_OUT_SIZE*2))-1)
  {
    AUDIODataReady = 1;
    AUDIOBuffOffset = WR_BUFFER_SIZE/2;
    ITCounter = 0;
  }
  else
  {
    ITCounter++;
  }
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{ 
  /* PDM to PCM data convert */
  BSP_AUDIO_IN_PDMToPCM((uint16_t*)&InternalBuffer[0], (uint16_t*)&RecBuf[0]);
  
  /* Copy PCM data in internal buffer */
  memcpy((uint16_t*)&WrBuffer[ITCounter * (PCM_OUT_SIZE*2)], RecBuf, PCM_OUT_SIZE*4);
  
  if(ITCounter == (WR_BUFFER_SIZE/(PCM_OUT_SIZE*4))-1)
  {
    AUDIODataReady = 1;
    AUDIOBuffOffset = 0;
    ITCounter++;
  }
  else if(ITCounter == (WR_BUFFER_SIZE/(PCM_OUT_SIZE*2))-1)
  {
    AUDIODataReady = 1;
    AUDIOBuffOffset = WR_BUFFER_SIZE/2;
    ITCounter = 0;
  }
  else
  {
    ITCounter++;
  }
}
