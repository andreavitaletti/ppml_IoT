#include "main.h"

void Blink_Task(void * pvParameters);
void SystemClock_Config(void);
void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng);
void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng);

RNG_HandleTypeDef RNG_Handle;

uint32_t genericCounter;
EcdhContext EcdhContext_t;
EcDomainParameters EcDomainParameters_t;
YarrowContext YarrowContext_t;
EcdsaSignature EcdsaSignature_t;

uint8_t seed[32];
uint8_t signatureBuffer[256];
uint8_t mpibuffer[128];
uint32_t signatureLength;
uint32_t value;
uint32_t i;
uint32_t returnCode;

const uint8_t InputMessage[] = {"This is a simple message"};
uint8_t MessageDigest[SHA256_DIGEST_SIZE];

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
	Printf_Init();
	
	/* Init LED */
	BSP_LED_Init(LED3);
	BSP_LED_Off(LED3);
	
	/* Welcome message */
	printf("STM32L432 Nucleo\r\nCycloneCrypto test - ECDSA\r\n\r\n");
	
	/* Init counter */
	genericCounter = 0;
	
	/* ECDSA */
	//Enable RNG peripheral clock
  __HAL_RCC_RNG_CLK_ENABLE();
  //Initialize RNG
  RNG_Handle.Instance = RNG;
  HAL_RNG_Init(&RNG_Handle);
	
	returnCode = yarrowInit(&YarrowContext_t);
	//printf("yarrowInit return code: %X\r\n", returnCode);
	ecdhInit(&EcdhContext_t);
	ecdsaInitSignature(&EcdsaSignature_t);

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
	
	// properly seed the PRNG
  returnCode = yarrowSeed(&YarrowContext_t, seed, sizeof(seed));
	//printf("yarrowSeed return code: %X\r\n", returnCode);
	
	// generate keypair
	returnCode = ecLoadDomainParameters(&EcdhContext_t.params, &secp256k1Curve);
	//printf("ecLoadDomainParameters return code: %X\r\n", returnCode);
	
	returnCode = ecdhGenerateKeyPair(&EcdhContext_t, &yarrowPrngAlgo, &YarrowContext_t);
	//printf("ecdhGenerateKeyPair return code: %X\r\n", returnCode);
	
	/*
	// print private key
	printf("Private key: ");
	mpiWriteRaw(&EcdhContext_t.da, mpibuffer, mpiGetLength(&EcdhContext_t.da));
	for(i=0;i<mpiGetLength(&EcdhContext_t.da);i++)
		printf("%02X", mpibuffer[i]);
	printf("\r\n");
	
	// print public key
	printf("Public key: ");
	mpiWriteRaw(&EcdhContext_t.qa.x, mpibuffer, mpiGetLength(&EcdhContext_t.qa.x));
	for(i=0;i<mpiGetLength(&EcdhContext_t.qa.x);i++)
		printf("%02X", mpibuffer[i]);
	printf(" ");
	mpiWriteRaw(&EcdhContext_t.qa.y, mpibuffer, mpiGetLength(&EcdhContext_t.qa.y));
	for(i=0;i<mpiGetLength(&EcdhContext_t.qa.y);i++)
		printf("%02X", mpibuffer[i]);
	printf(" ");
	mpiWriteRaw(&EcdhContext_t.qa.z, mpibuffer, mpiGetLength(&EcdhContext_t.qa.z));
	for(i=0;i<mpiGetLength(&EcdhContext_t.qa.z);i++)
		printf("%02X", mpibuffer[i]);
	printf("\r\n");
	*/
	
	// calculate SHA256 of the message
	returnCode = sha256Compute(&InputMessage, sizeof(InputMessage), MessageDigest);
	//printf("sha256Compute return code: %X\r\n", returnCode);
	
	/*
	printf("SHA256:\t"); 
	for(i = 0; i < sizeof(MessageDigest); i++)
		printf("%02X", MessageDigest[i]);
	printf("\r\n");
	*/

	// sign digest
	
	returnCode = ecdsaGenerateSignature(&EcDomainParameters_t, &yarrowPrngAlgo, &YarrowContext_t, &EcdhContext_t.da, MessageDigest, sizeof(MessageDigest), &EcdsaSignature_t);
	//printf("ecdsaGenerateSignature return code: %X\r\n", returnCode);
	
	// print signature
	returnCode = ecdsaWriteSignature(&EcdsaSignature_t, signatureBuffer, &signatureLength);
	//printf("ecdsaWriteSignature return code: %X\r\n", returnCode);
	
	/*
	printf("Signature:\t");
	for(i = 0; i< signatureLength; i++)
		printf("%02X", signatureBuffer[i]);
	printf("\r\n");
	*/
	
	// verify signature
	returnCode = ecdsaVerifySignature(&EcDomainParameters_t, &EcdhContext_t.qa, MessageDigest, sizeof(MessageDigest), &EcdsaSignature_t);
	//printf("ecdsaVerifySignature return code: %X\r\n", returnCode);
	
	ecdsaFreeSignature(&EcdsaSignature_t);
	//printf("ECDSA - DONE\r\n");
	
	while(1)
	{}
	
	/* Create task */
	xTaskCreate(Blink_Task, "BLINK_TASK", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

  /* Start the FreeRTOS scheduler */
  vTaskStartScheduler();
	
	/* Catch error */
  while (1)
	{
		Error_Handler();
	}
}

void Blink_Task(void * pvParameters)
{
	while(1)
	{
		BSP_LED_Toggle(LED3);
		printf("Counter value: %d\r\n", genericCounter);
		vTaskDelay(500);
		genericCounter++;
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
