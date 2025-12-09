#ifndef CYBERGEAR_H
#define CYBERGEAR_H

#include <Arduino.h>
#include <stdbool.h>

// 💡 ESP32 TWAI (CAN) Header
#include "driver/twai.h" 
#include "CyBerGearAddr.h"


// --- โครงสร้างสถานะมอเตอร์ ---
typedef struct {
    float position;
    float speed;
    float torque;
    uint16_t temperature;
} CyberGearStatus;

// --- โครงสร้างคำสั่งควบคุม (ยังคงเป็น Struct) ---
typedef struct {
    float position;
    float speed;
    float kp;
    float kd;
    float torque;
} CyberGearMotionCommand;


// 💡 Class CyberGearDriver (แทน Struct CyberGearDriver)
class CyberGear {
public:
    CyberGear(uint8_t motor_id, uint8_t master_id); //Done

    // Initialization and Control Flow
    bool begin(gpio_num_t tx_pin, gpio_num_t rx_pin, uint32_t baud_rate = 1000000); //Done
    void end();

    // Core Commands (ใช้ Command ID 0x01, 0x02, 0x05)
    void enableMotor(); //Done
    void stopMotor(bool clear_error = false); //Done
    void clearError(); //Done
    void requestStatus(); //Done
    
    uint8_t getRunMode() const { return run_mode; } //Done
    uint8_t getMotorCanID() const { return cybergear_can_id;} //Done
    
    // Set Parameters (ใช้ CMD_RAM_WRITE 0x12)
    void setRunMode(uint8_t mode); //Done

    // Set Limits and Gains
    void setLimitSpeed(float speed); //Done
    void setLimitCurrent(float current); //Done
    void setLimitTorque(float torque); //Done

    // Current Control parameters
    void setCurrentKp(float kp); //Done
    void setCurrentKi(float ki); //Done
    void setCurrentFilterGain(float gain); //Done
    void setCurrentRef(float current); //Done

    // Position control parameters
    void setPositionKp(float kp); //Done
    void setPositionRef(float position); //Done

    // Speed control parameters
    void setSpeedKp(float kp); //Done
    void setSpeedKi(float ki); //Done
    void setSpeedRef(float speed); //Done

    // CAN ID Configuration
    void setMotorCanID(uint8_t can_id); //Done

    // Send Control Command (ใช้ CMD_POSITION)
    void sendMotionControl(const CyberGearMotionCommand& cmd); //Done

    // Get Status
    CyberGearStatus getStatus() const { return status; } //Done
    void processRxMessage(const twai_message_t& rx_message); // สำหรับ TWAI Interrupt/Queue, Done

private:
    uint8_t cybergear_can_id;
    uint8_t master_can_id;
    uint8_t run_mode;
    CyberGearStatus status;

    // Helper Functions
    void sendCanPackage(uint8_t cmd_id, uint16_t option, const uint8_t* data, uint8_t len, uint8_t lenn); //Done
    void sendCanFloatPackage(uint16_t addr, float value, float min, float max); 

    // Conversion Functions
    uint16_t floatToUint(float x, float x_min, float x_max, int bits); //Done
    float uintToFloat(uint16_t x, float x_min, float x_max); //Done
};

#endif // CYBERGEAR_H