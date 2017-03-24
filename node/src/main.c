#include "main.h"

void SystemClock_Config(void);
void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng);
void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng);

const uint8_t prnum[] = {0xD1, 0x6B, 0x6A, 0xE8, 0x27, 0xF1, 0x71, 0x75, 0xE0, 0x40, 0x87, 0x1A, 0x1C, 0x7E, 0xC3, 0x50, 0x01, 0x92, 0xC4, 0xC9, 0x26, 0x77, 0x33, 0x6E, 0xC2, 0x53, 0x7A, 0xCA, 0xEE, 0x00, 0x08, 0xE0};

error_t error;
uint_t n;
uint8_t digest[64];
EcDomainParameters params;
Mpi privateKey;
EcPoint publicKey;
EcdsaSignature signature;
Mpi r;
YarrowContext YarrowContext_t;
static uint8_t random[128];

RNG_HandleTypeDef RNG_Handle;
uint_t i; 	
uint32_t value;
uint8_t seed[32];
uint8_t buffer[64];
uint8_t genericCounter = 0;
	
uint8_t message[128];

extern uint16_t rxBufferIndex;
extern uint8_t aRxBuffer[PRINTFDMA_RXBUFFERSIZE];

void printMpi(const Mpi *a)
{
   uint_t i;

   for(i = 0; i < a->size; i++)
		if(a->data[a->size - 1 - i] != 0)
      printf("%08X", a->data[a->size - 1 - i]);
}

void printArray(const void *data, size_t length)
{
   uint_t i;

   for(i = 0; i < length; i++)
		printf("%02X", *((uint8_t *) data + i));

}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{  
		/* STM32L4xx HAL library initialization:
				 - Configure the Flash prefetch
				 - Systick timer is configured by default as source of time base, but user 
					 can eventually implement his proper time base source (a general purpose 
					 timer for example or other time source), keeping in mind that Time base 
					 duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
					 handled in milliseconds basis.
				 - Set NVIC Group Priority to 4
				 - Low Level Initialization
			 */
		HAL_Init();  
		
		/* Configure the system clock = 80 MHz */
		SystemClock_Config();

		/* Init printf */
		PrintfDmaInit();
		
		/* Init LED */
		BSP_LED_Init(LED3);
		BSP_LED_Off(LED3);
		
		/* Welcome message */
		printf("STM32L432 Nucleo\r\nECDSA PhD\r\n\r\n");
		
		uint16_t rcvLength = 0;
		while(rcvLength < 32)
		{
			rcvLength = PrintfDmaGetData(buffer);
			HAL_Delay(100);
		}
	
		//Initialize EC domain parameters
    ecInitDomainParameters(&params);
		
		//Initialize ECDSA private key
		mpiInit(&privateKey);
	 
		//Enable RNG peripheral clock
		__HAL_RCC_RNG_CLK_ENABLE();
		//Initialize RNG
		RNG_Handle.Instance = RNG;
		HAL_RNG_Init(&RNG_Handle);
		
		//Generate a random seed
		for(i = 0; i < 32; i += 4)
		{
			//Get 32-bit random value
			HAL_RNG_GenerateRandomNumber(&RNG_Handle, &value);

			//Copy random value
			seed[i] = value & 0xFF;
			seed[i + 1] = (value >> 8) & 0xFF;
			seed[i + 2] = (value >> 16) & 0xFF;
			seed[i + 3] = (value >> 24) & 0xFF;
		}
		
		//PRNG initialization
   error = yarrowInit(&YarrowContext_t);
   //Any error to report?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize PRNG!\r\n");
   }

   //Properly seed the PRNG
   error = yarrowSeed(&YarrowContext_t, seed, sizeof(seed));
   //Any error to report?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to seed PRNG!\r\n");
   }
   
   //Load EC domain parameters
   ecLoadDomainParameters(&params, SECP256R1_CURVE);
	 HAL_Delay(100);

   //Read ECDSA private key
   mpiReadRaw(&privateKey, buffer, 32);
	 //TRACE_DEBUG_MPI("    ", &privateKey);
	 printMpi(&privateKey);
	 
	 //Read pseudo-random number
   mpiReadRaw(&r, prnum, 32);
	 
	 //Get the length, in bytes, of the random number
   n = (mpiGetByteLength(&r) + 3) / 4 * 4;

   memset(random, 0, sizeof(random));
   memcpy(random, r.data, n);

   //Initialize pseudo-random number generator
   yarrowPrngAlgo.addEntropy(&YarrowContext_t, 0, random, n + 4, 0);
	 
	 printf("Initialization Done\r\n");
	 
		/* Catch error */
		while (1)
		{
			sprintf(message, "%d", genericCounter);
			SHA256_HASH_ALGO->compute(message, sizeof(message), digest);
			ecdsaGenerateSignature(&params, YARROW_PRNG_ALGO, &YarrowContext_t, &privateKey, digest, SHA256_HASH_ALGO->digestSize, &signature);
			printf("%s ", message);
			printArray(&digest, SHA256_HASH_ALGO->digestSize);
			printf(" ");
			printMpi(&signature.r);
			printMpi(&signature.s);
			printf("\r\n");
			
			genericCounter++;
			BSP_LED_Toggle(LED3);
			HAL_Delay(500);
		}
}


/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 80000000
  *            HCLK(Hz)                       = 80000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 40
  *            PLL_R                          = 2
  *            PLL_P                          = 7
  *            PLL_Q                          = 4
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* MSI is enabled after System reset, activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}

/**
  * @brief RNG MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  * @param hrng: RNG handle pointer
  * @retval None
  */
void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
{  

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  /*Select PLLQ output as RNG clock source */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
  PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_PLL;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

  /* RNG Peripheral clock enable */
  __HAL_RCC_RNG_CLK_ENABLE();

}

/**
  * @brief RNG MSP De-Initialization
  *        This function freeze the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  * @param hrng: RNG handle pointer
  * @retval None
  */
void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng)
{
  /* Enable RNG reset state */
  __HAL_RCC_RNG_FORCE_RESET();

  /* Release RNG from reset state */
  __HAL_RCC_RNG_RELEASE_RESET();
} 

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
