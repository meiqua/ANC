/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "string.h"

#include <stdlib.h>     /* srand, rand */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef hTimLed;
TIM_OC_InitTypeDef sConfigLed;

/* Capture Compare Register Value.
   Defined as external in stm32f4xx_it.c file */
__IO uint16_t CCR1Val = 16826;              
extern  __IO uint32_t TimeRecBase;                                            
extern __IO uint32_t LEDsState;
extern __IO uint32_t AUDIODataReady, AUDIOBuffOffset;
extern uint16_t WrBuffer[WR_BUFFER_SIZE];
uint16_t halfBuff[WR_BUFFER_SIZE/2]={0};
uint16_t fullBuff[WR_BUFFER_SIZE]={0};
uint16_t outBuff[WR_BUFFER_SIZE/2];

uint16_t randBuff[WR_BUFFER_SIZE/2];
__IO uint32_t getRand = 0;

extern __IO BUFFER_StateTypeDef buffer_offset;
extern uint16_t Audio_Buffer[WR_BUFFER_SIZE*2];

__IO uint32_t CmdIndex = CMD_PLAY;
__IO uint32_t PbPressCheck = 0;

uint16_t cz[WR_BUFFER_SIZE/2];
uint16_t czt[WR_BUFFER_SIZE/2];
uint16_t wz[WR_BUFFER_SIZE/2];
uint16_t rz[WR_BUFFER_SIZE/2];

uint16_t xz[WR_BUFFER_SIZE]={0};
uint16_t ezp[WR_BUFFER_SIZE/2];
uint16_t yz[WR_BUFFER_SIZE]={0};

uint32_t u=300;
uint32_t leaky=2;
UART_HandleTypeDef UartHandle;


/* Private function prototypes -----------------------------------------------*/
static void TIM_LED_Config(void);
static void SystemClock_Config(void);
void convolve(const uint16_t Signal[/* SignalLen */], uint32_t SignalLen,
              const uint16_t Kernel[/* KernelLen */], uint32_t KernelLen,
              uint16_t Result[/* SignalLen - KernelLen*/])
{
  uint32_t n;

  for (n = 0; n < SignalLen-KernelLen; n++)
  {
    uint32_t kmin, kmax, k;

    Result[n] = 0;

    kmin = n + 1;
    kmax = n + KernelLen;

    for (k = kmin; k <= kmax; k++)
    {
      Result[n] += (uint16_t)((((int32_t)Signal[k]-32767) * Kernel[kmax - k]>>16)+32767);
    }
  }
}
void vecSub(const uint16_t Signal1[],
              const uint16_t Signal2[], uint32_t SignalLen,
              uint16_t Result[]){
								uint32_t n;
								for(n=0;n<SignalLen;n++){
									Result[n]=32767+Signal1[n]-Signal2[n];
								}
							}
void update(const uint16_t cz[],
              const uint16_t ezp[], uint32_t u,uint32_t leaky,
               const uint16_t y[],uint16_t cz1[],uint16_t sub){
								uint32_t i,j;
								int32_t sum=0;
								for(i=0;i<WR_BUFFER_SIZE/2;i++){
									for(j=0;j<WR_BUFFER_SIZE/2;j++){
									  sum += (u*((((int32_t)ezp[j]-32767)*((int32_t)y[WR_BUFFER_SIZE/2+j-i]-32767))>>8))>>8;
								  }
									if(sub==1)
									cz1[i] = ((100-leaky)*cz[i])/100 + sum;
									else
										cz1[i] = ((100-leaky)*cz[i])/100 - sum;
                  sum=0;
								}
							}
void printNs(uint16_t cz[], uint32_t size){ //print noise
	            uint32_t sum=0;
	            uint32_t a=0;
							for(uint32_t i=0;i<size;i+=1){
								sum += cz[i];
							}
							 // it's too fast to print cz
							// sum can filter out high freq
							// print slower
							sum = sum/size;
							if(sum>32767){
							sum -= 32767;
								a = 0;
							}else{
							sum = 32767-sum;
								a = 1;
							}
							for(uint32_t i=0;i<sum;i+=1000){
							  printf("-");
							}
							if(a==0)
							printf("<<\n");
							else
						  printf(">>\n");
}							 

int main(void)
{
  HAL_Init();
  
  /* Configure LED3, LED4, LED5 and LED6 */
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  BSP_LED_Init(LED5);
  BSP_LED_Init(LED6);
  
  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
	
	// uart is not used
  UartHandle.Instance          = USARTx;
  
  UartHandle.Init.BaudRate     = 9600;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }
	// uart is not used
  
  /* Turn ON LED4: start of application */
  BSP_LED_On(LED4);
  
  /* Configure TIM4 Peripheral to manage LEDs lighting */
  TIM_LED_Config();
  
  /* Turn OFF all LEDs */
  LEDsState = LEDS_OFF;
  
  /* Configure USER Button */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
  
	//config recorder
	WaveRecorderProcess();
  
	//config player
  WavePlayBack(DEFAULT_AUDIO_IN_FREQ);
	
  while (1){
		  
		//generate white noise
		  if(CmdIndex == CMD_RECORD&&getRand==0){
				uint32_t i=0;
				getRand = 1;
				for(i=0;i<WR_BUFFER_SIZE/2;i++){
					randBuff[i] = rand()%32767;
				}
			}
		

			
		  if(AUDIODataReady == 1)   //record half the buff
      {	
				if(CmdIndex == CMD_RECORD){		
          LEDsState = LED3_TOGGLE;					
					if(TimeRecBase>5000){ //wait 5s for preperation
						getRand = 0;						
						memcpy(fullBuff, (uint16_t*)&fullBuff[WR_BUFFER_SIZE/2], WR_BUFFER_SIZE);
						memcpy((uint16_t*)&fullBuff[WR_BUFFER_SIZE/2], randBuff, WR_BUFFER_SIZE);
						// fullBuff buffer white noise 
						
						convolve(fullBuff, WR_BUFFER_SIZE,
										 cz, WR_BUFFER_SIZE/2,
										 rz);
						vecSub((uint16_t*)&WrBuffer[AUDIOBuffOffset],rz,WR_BUFFER_SIZE/2,ezp);
						
						update(cz,ezp, u,leaky,fullBuff,czt,1);
						
						memcpy(cz, czt, WR_BUFFER_SIZE);
						
						memcpy(outBuff, randBuff, WR_BUFFER_SIZE);
						
			if(buffer_offset == BUFFER_OFFSET_HALF)
      {       
				memcpy(Audio_Buffer, outBuff, WR_BUFFER_SIZE);          
        buffer_offset = BUFFER_OFFSET_NONE;
      }
      
      if(buffer_offset == BUFFER_OFFSET_FULL)
      {
				memcpy((uint16_t*)&Audio_Buffer[WR_BUFFER_SIZE/2], outBuff, WR_BUFFER_SIZE);          
        buffer_offset = BUFFER_OFFSET_NONE;
      }
			// play noise
						
						if(TimeRecBase>15000){ //10s later
						  // end c(z)
							for(uint32_t i=0;i<WR_BUFFER_SIZE/2;i++)
							printf("%d\n",cz[i]>>8);
              CmdIndex = CMD_PLAY;		
					  }
					}
				}else{			
          LEDsState = LED4_TOGGLE;						
					convolve(yz, WR_BUFFER_SIZE,
										 cz, WR_BUFFER_SIZE/2,
										 rz);	
					vecSub((uint16_t*)&WrBuffer[AUDIOBuffOffset],rz,WR_BUFFER_SIZE/2,czt);					
					memcpy(xz, (uint16_t*)&xz[WR_BUFFER_SIZE/2], WR_BUFFER_SIZE);
					memcpy((uint16_t*)&xz[WR_BUFFER_SIZE/2], czt, WR_BUFFER_SIZE);						
					
					
					convolve(xz, WR_BUFFER_SIZE,
										 wz, WR_BUFFER_SIZE/2,
										 rz);	
					memcpy(yz, (uint16_t*)&yz[WR_BUFFER_SIZE/2], WR_BUFFER_SIZE);
					memcpy((uint16_t*)&yz[WR_BUFFER_SIZE/2], rz, WR_BUFFER_SIZE);	
			
      //change here					
			// play origin or anti-noise
			memcpy(outBuff, rz, WR_BUFFER_SIZE);		
	//		memcpy(outBuff, (uint16_t*)&WrBuffer[AUDIOBuffOffset], WR_BUFFER_SIZE);	
					
					
			if(buffer_offset == BUFFER_OFFSET_HALF)
      {       
				memcpy(Audio_Buffer, outBuff, WR_BUFFER_SIZE);          
        buffer_offset = BUFFER_OFFSET_NONE;
      }
      
      if(buffer_offset == BUFFER_OFFSET_FULL)
      {
				memcpy((uint16_t*)&Audio_Buffer[WR_BUFFER_SIZE/2], outBuff, WR_BUFFER_SIZE);          
        buffer_offset = BUFFER_OFFSET_NONE;
      }
			
					
					convolve(xz, WR_BUFFER_SIZE,
										 cz, WR_BUFFER_SIZE/2,
										 rz);	
					memcpy(fullBuff, (uint16_t*)&fullBuff[WR_BUFFER_SIZE/2], WR_BUFFER_SIZE);
					memcpy((uint16_t*)&fullBuff[WR_BUFFER_SIZE/2], rz, WR_BUFFER_SIZE);
					update(wz,(uint16_t*)&WrBuffer[AUDIOBuffOffset], u,leaky,fullBuff,czt,0);	

			if(TimeRecBase>3000){ // print weight every 3s
							for(uint32_t i=0;i<WR_BUFFER_SIZE/2;i++)
	//						printf("%d\n",wz[i]>>8);		
							TimeRecBase = 0;
			}
	
					memcpy(wz, czt, WR_BUFFER_SIZE);					
				}
				printNs((uint16_t*)&WrBuffer[AUDIOBuffOffset],WR_BUFFER_SIZE/2);
      } 
  }
}

static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }  
}


static void TIM_LED_Config(void)
{
  uint16_t prescalervalue = 0;
  uint32_t tmpvalue = 0;

  /* TIM4 clock enable */
  __HAL_RCC_TIM4_CLK_ENABLE();

  /* Enable the TIM4 global Interrupt */
  HAL_NVIC_SetPriority(TIM4_IRQn, 6, 0);  
  HAL_NVIC_EnableIRQ(TIM4_IRQn);
  
  /* -----------------------------------------------------------------------
  TIM4 Configuration: Output Compare Timing Mode:  
    To get TIM4 counter clock at 550 KHz, the prescaler is computed as follows:
    Prescaler = (TIM4CLK / TIM4 counter clock) - 1
    Prescaler = ((f(APB1) * 2) /550 KHz) - 1
  
    CC update rate = TIM4 counter clock / CCR_Val = 32.687 Hz
    ==> Toggling frequency = 16.343 Hz  
  ----------------------------------------------------------------------- */
  
  /* Compute the prescaler value */
  tmpvalue = HAL_RCC_GetPCLK1Freq();
  prescalervalue = (uint16_t) ((tmpvalue * 2) / 550000) - 1;
  
  /* Time base configuration */
  hTimLed.Instance = TIM4;
  hTimLed.Init.Period = 65535;
  hTimLed.Init.Prescaler = prescalervalue;
  hTimLed.Init.ClockDivision = 0;
  hTimLed.Init.CounterMode = TIM_COUNTERMODE_UP;
  if(HAL_TIM_OC_Init(&hTimLed) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
  /* Output Compare Timing Mode configuration: Channel1 */
  sConfigLed.OCMode = TIM_OCMODE_TIMING;
  sConfigLed.OCIdleState = TIM_OCIDLESTATE_SET;
  sConfigLed.Pulse = CCR1Val;
  sConfigLed.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigLed.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigLed.OCFastMode = TIM_OCFAST_ENABLE;
  sConfigLed.OCNIdleState = TIM_OCNIDLESTATE_SET;
  
  /* Initialize the TIM4 Channel1 with the structure above */
  if(HAL_TIM_OC_ConfigChannel(&hTimLed, &sConfigLed, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Start the Output Compare */
  if(HAL_TIM_OC_Start_IT(&hTimLed, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Start Error */
    Error_Handler();
  }
}


void Error_Handler(void)
{
  /* Turn LED3 on */
  BSP_LED_On(LED3);
  while(1)
  {
  }
}


void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  uint32_t capture = 0; 
  
  if (LEDsState == LED3_TOGGLE)
  {
    /* Toggling LED3 */
    BSP_LED_Toggle(LED3);
    BSP_LED_Off(LED6);
    BSP_LED_Off(LED4);
  }
  else if (LEDsState == LED4_TOGGLE)
  {
    /* Toggling LED4 */
    BSP_LED_Toggle(LED4);
    BSP_LED_Off(LED6);
    BSP_LED_Off(LED3);
  }
  else if (LEDsState == LED6_TOGGLE)
  {
    /* Toggling LED6 */
    BSP_LED_Off(LED3);
    BSP_LED_Off(LED4);
    BSP_LED_Toggle(LED6);
  }
  else if (LEDsState == STOP_TOGGLE)
  {
    /* Turn ON LED6 */
    BSP_LED_On(LED6);
  }
  else if (LEDsState == LEDS_OFF)
  {
    /* Turn OFF all LEDs */
    BSP_LED_Off(LED3);
    BSP_LED_Off(LED4);
    BSP_LED_Off(LED5);
    BSP_LED_Off(LED6);
  }
  /* Get the TIM4 Input Capture 1 value */
  capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
  
  /* Set the TIM4 Capture Compare1 Register value */
  __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (CCR1Val + capture));
}

 /**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == GPIO_PIN_0) 
  {
      if (CmdIndex == CMD_RECORD)
      {
        CmdIndex = CMD_PLAY;
      }
      /* Test on the command: Playing */
      else if (CmdIndex == CMD_PLAY)
      {
        /* Switch to Record command */
        CmdIndex = CMD_RECORD;
      }
      else
      {
        CmdIndex = CMD_PLAY; 
      }

  }
} 


