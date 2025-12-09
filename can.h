/* can.h (ฉบับปรับปรุงโดยใช้ Forward Declaration) */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include "main.h"         
#include "stm32f1xx_hal_can.h" 

// 💡 Forward Declaration: บอก Compiler ว่ามี Struct ชื่อนี้อยู่ 
// โดยไม่ต้อง Include ไฟล์ CyberGear.h ทั้งหมด
// (สมมติว่าคุณได้กำหนด struct CyberGearDriver_t ใน CyberGear.h)
typedef struct CyberGearDriver_t CyberGearDriver;


/* Exported Global Variables -------------------------------------------------*/

// CAN Handle ที่จะถูกกำหนดใน main.c หรือ can.c
extern CAN_HandleTypeDef hcan;


/* Exported Function Prototypes ----------------------------------------------*/

// ฟังก์ชันเริ่มต้นการทำงานของ CAN
void MX_CAN_Init(void);

// ฟังก์ชันตั้งค่า CAN Filter และ Interrupt
void CAN_Filter_Config(void);

// ฟังก์ชันประมวลผล CAN Message ที่ได้รับ
// ใช้ Forward Declared Type
void CAN_Process_Rx(CyberGearDriver *driver);


#endif /* INC_CAN_H_ */