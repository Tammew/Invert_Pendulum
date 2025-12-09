/* main.h (ESP32 Version) */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include "Arduino.h"

/* Exported Global Variables (extern) ----------------------------------------*/

// Global CAN handles
extern void MX_TWAI_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */