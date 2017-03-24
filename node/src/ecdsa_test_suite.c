//Dependencies
#include <string.h>
#include "crypto.h"
#include "ec.h"
#include "ec_curves.h"
#include "ecdsa.h"
#include "sha1.h"
#include "sha256.h"
#include "sha384.h"
#include "sha512.h"
#include "debug.h"

#include "yarrow.h"
#include "stm32l4xx_hal.h"

RNG_HandleTypeDef RNG_Handle;
uint_t i; 	
uint32_t value;
uint8_t seed[32];

const uint8_t prvKey[] = {0xC9, 0xAF, 0xA9, 0xD8, 0x45, 0xBA, 0x75, 0x16, 0x6B, 0x5C, 0x21, 0x57, 0x67, 0xB1, 0xD6, 0x93, 0x4E, 0x50, 0xC3, 0xDB, 0x36, 0xE8, 0x9B, 0x12, 0x7B, 0x8A, 0x62, 0x2B, 0x12, 0x0F, 0x67, 0x21};
const uint8_t pubKeyX[] = {0x60, 0xFE, 0xD4, 0xBA, 0x25, 0x5A, 0x9D, 0x31, 0xC9, 0x61, 0xEB, 0x74, 0xC6, 0x35, 0x6D, 0x68, 0xC0, 0x49, 0xB8, 0x92, 0x3B, 0x61, 0xFA, 0x6C, 0xE6, 0x69, 0x62, 0x2E, 0x60, 0xF2, 0x9F, 0xB6};
const uint8_t pubKeyY[] = {0x79, 0x03, 0xFE, 0x10, 0x08, 0xB8, 0xBC, 0x99, 0xA4, 0x1A, 0xE9, 0xE9, 0x56, 0x28, 0xBC, 0x64, 0xF2, 0xF1, 0xB2, 0x0C, 0x2D, 0x7E, 0x9F, 0x51, 0x77, 0xA3, 0xC2, 0x94, 0xD4, 0x46, 0x22, 0x99};
const uint8_t prnum[] = {0xD1, 0x6B, 0x6A, 0xE8, 0x27, 0xF1, 0x71, 0x75, 0xE0, 0x40, 0x87, 0x1A, 0x1C, 0x7E, 0xC3, 0x50, 0x01, 0x92, 0xC4, 0xC9, 0x26, 0x77, 0x33, 0x6E, 0xC2, 0x53, 0x7A, 0xCA, 0xEE, 0x00, 0x08, 0xE0};
const uint8_t message[] = "This is a simple message";
	
uint32_t startTime, stopTime;

error_t ecdsaTestSuite(void)
{
   error_t error;
   uint_t i;
   uint_t n;
   uint8_t digest[64];
   EcDomainParameters params;
   Mpi privateKey;
   EcPoint publicKey;
   EcdsaSignature signature;
   Mpi r;
   YarrowContext YarrowContext_t;
   static uint8_t random[128];
   
   TRACE_INFO("************************\r\n");
   TRACE_INFO("** ECDSA Test Vectors **\r\n");
   TRACE_INFO("************************\r\n");
   TRACE_INFO("\r\n");
   
   //Initialize EC domain parameters
   ecInitDomainParameters(&params);

   //Initialize ECDSA private key
   mpiInit(&privateKey);
   //Initialize ECDSA public key
   ecInit(&publicKey);
   //Initialize ECDSA signature (reference)
   ecdsaInitSignature(&signature);
	 
	 //Initialize multiple precision integer
   mpiInit(&r);
	 
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

   //Read ECDSA private key
   mpiReadRaw(&privateKey, prvKey, 32);

	 //Read ECDSA public key
	 mpiReadRaw(&publicKey.x, pubKeyX, 32);
	 mpiReadRaw(&publicKey.y, pubKeyY, 32);
   
	 //Read pseudo-random number
   mpiReadRaw(&r, prnum, 32);

	 //Any error to report?
   if(error)
   {
			//Test result
			TRACE_INFO("  Result:\r\n");
      TRACE_INFO("    FAILED\r\n\r\n");
   }

      //Get the length, in bytes, of the random number
      n = (mpiGetByteLength(&r) + 3) / 4 * 4;

      memset(random, 0, sizeof(random));
      memcpy(random, r.data, n);

      //Initialize pseudo-random number generator
      yarrowPrngAlgo.addEntropy(&YarrowContext_t, 0, random, n + 4, 0);

      //Digest message
      error = SHA256_HASH_ALGO->compute(message, sizeof(message), digest);

      //Any error to report?
      if(error)
      {
         //Test result
         TRACE_INFO("  Result:\r\n");
         TRACE_INFO("    FAILED\r\n\r\n");
      }

      //Get current time
      startTime = HAL_GetTick();
      //ECDSA signature generation
      error = ecdsaGenerateSignature(&params, YARROW_PRNG_ALGO, &YarrowContext_t,
         &privateKey, digest, SHA256_HASH_ALGO->digestSize, &signature);
      //Get current time
      stopTime = HAL_GetTick();
      
      //Debug message
      TRACE_INFO("ECDSA signature generation: %" PRIu32 "\r\n", (uint32_t) (stopTime - startTime));

      //Any error to report?
      if(error)
      {
         //Test result
         TRACE_INFO("  Result:\r\n");
         TRACE_INFO("    FAILED\r\n\r\n");
      }

      //Dump calculated ECDSA signature
      TRACE_INFO("  Calculated signature (r):\r\n");
      TRACE_INFO_MPI("    ", &signature.r);
      TRACE_INFO("  Calculated signature (s):\r\n");
      TRACE_INFO_MPI("    ", &signature.s);

      /*
			//Check resulting ECDSA signature - not working with YARROW
      if(mpiComp(&calcSignature.r, &refSignature.r) || mpiComp(&calcSignature.s, &refSignature.s))
      {
         //Test result
         TRACE_INFO("  Result:\r\n");
         TRACE_INFO("    FAILED\r\n\r\n");
         //Report an error
         error = ERROR_FAILURE;
         //Exit immediately
         break;
      }
			*/

      //Get current time
      startTime = HAL_GetTick();
      //ECDSA signature verification
      error = ecdsaVerifySignature(&params, &publicKey, digest, SHA256_HASH_ALGO->digestSize, &signature);
      //Get current time
      stopTime = HAL_GetTick();

      //Debug message
      TRACE_INFO("ECDSA signature verification: %" PRIu32 "\r\n", (uint32_t) (stopTime - startTime));


      //Any error to report?
      if(error)
      {
         //Test result
         TRACE_INFO("  Result:\r\n");
         TRACE_INFO("    FAILED\r\n\r\n");
      }
			else
			{
				//Test result
				TRACE_INFO("  Result:\r\n");
				TRACE_INFO("    PASSED\r\n\r\n");
			}

      ecFreeDomainParameters(&params);

      
   
   //Release previously allocated resources
   ecFreeDomainParameters(&params);
   mpiFree(&privateKey);
   ecFree(&publicKey);
   ecdsaFreeSignature(&signature);
   mpiFree(&r);

   //Return status code
   return error;
}
