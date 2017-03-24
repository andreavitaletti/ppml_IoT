/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#define TRACE_PRINTF(...) fprintf(stderr, __VA_ARGS__)
#define TRACE_ARRAY(p, a, n) debugDisplayArray(stderr, p, a, n)
#define TRACE_MPI(p, a) mpiDump(stderr, p, a)

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "stm32l4xx_nucleo_32.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "printfDma.h"
#include "errorHandler.h"

#include "ecdsa.h"
#include "ecdh.h"
#include "ec.h"
#include "sha256.h"
#include "yarrow.h"
#include "debug.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void SystemClock_Config(void);

#endif 
