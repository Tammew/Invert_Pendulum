//ช่วยตรวจสอบทีครับ ผมทำการ comment ในจุดที่สงสัย และคิดว่าน่าจะมีการผิดพลาด, และช่วยเทียบกับตัว CyBerGear.h ด้วยครับ ขอบคุณครับ
#include "CyBerGear.h"
#include <string.h>

// --- Helper Implementations (Conversion) ---
uint16_t CyberGear::floatToUint(float x, float x_min, float x_max, int bits) {
    if (bits > 16) bits = 16;
    float span = x_max - x_min;
    float offset = x_min;
    if (x > x_max) x = x_max;
    else if (x < x_min) x = x_min;
    return (uint16_t)((x - offset) * ((float)((1 << bits) - 1)) / span);
}

float CyberGear::uintToFloat(uint16_t x, float x_min, float x_max) {
    uint16_t type_max = 0xFFFF;
    float span = x_max - x_min;
    return ((float)x / type_max * span) + x_min;
}

// --- Constructor & TWAI Initialization ---
CyberGear::CyberGear(uint8_t motor_id, uint8_t master_id) 
    : cybergear_can_id(motor_id), master_can_id(master_id) {
    run_mode = MODE_POSITION; 
    memset(&status, 0, sizeof(CyberGearStatus)); // Clear status struct
}

bool CyberGear::begin(gpio_num_t tx_pin, gpio_num_t rx_pin, uint32_t baud_rate) {
    // 1. TWAI Configuration (General) - แก้ไขโดยใช้สมาชิกโดยตรง
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(tx_pin, rx_pin, TWAI_MODE_NORMAL);
    g_config.rx_queue_len = 20; // 💡 แก้ไข: ใช้สมาชิกนี้โดยตรง
    g_config.tx_queue_len = 20; // 💡 แก้ไข: ใช้สมาชิกนี้โดยตรง

    // 2. TWAI Timing - แก้ไขโดยใช้ Macro Default ที่ถูกต้อง
    twai_timing_config_t t_config;
    if (baud_rate == 1000000) {
        t_config = TWAI_TIMING_CONFIG_1MBITS(); // 💡 ใช้ Macro ที่ถูกต้อง
    } else if (baud_rate == 500000) {
        t_config = TWAI_TIMING_CONFIG_500KBITS();
    } else {
        return false;
    }

    // 3. TWAI Filter - แก้ไขโดยใช้ Struct Initialization
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    
    // 4. Install and Start Driver
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) return false;
    if (twai_start() != ESP_OK) return false;
    return true; 
}


// --- Core Send Function (with 5x Pump) ---

void CyberGear::sendCanPackage(uint8_t cmd_id, uint16_t option, const uint8_t* data, uint8_t len, uint8_t lenn) {
    twai_message_t message;
    
    // 1. Build Extended ID
    uint32_t extended_id = (cmd_id << 24) | (option << 8) | cybergear_can_id;

    // 2. Setup Message Header
    message.extd = 1; 
    message.identifier = extended_id;
    message.data_length_code = len;
    message.rtr = 0;
    
    // 3. Copy Data
    memcpy(message.data, data, len);

    for (int i = 0; i < lenn; i++) {
        // twai_transmit will block briefly if the TX queue is full
        twai_transmit(&message, pdMS_TO_TICKS(1)); 
    }
}

// --- High-Level Commands ---

void CyberGear::enableMotor() {
    uint8_t data[8] = {0x00};
    sendCanPackage(CMD_ENABLE, master_can_id, data, 8, 3);
}

void CyberGear::stopMotor(bool clear_error) {
    uint8_t data[8] = {0x00};
    uint8_t command_id = CMD_STOP;

    if (clear_error) {
        // ใช้ 0x05 เพื่อ Clear Error
        command_id = CMD_CLEAR_ERROR; 
    }
    sendCanPackage(command_id, master_can_id, data, 8, 3);
}

void CyberGear::clearError() {
    uint8_t data[8] = {0x00};
    sendCanPackage(CMD_CLEAR_ERROR, master_can_id, data, 8, 2);
}

void CyberGear::requestStatus() {
    uint8_t data[8] = {0x00};
    sendCanPackage(CMD_GET_STATUS, master_can_id, data, 8, 1);
}

// #define MODE_MOTION                  0x00
// #define MODE_POSITION                0x01
// #define MODE_SPEED                   0x02
// #define MODE_CURRENT                 0x03
// Example from CyBerGearAddr.h
void CyberGear::setRunMode(uint8_t mode){
    uint8_t data[8] = {0x00};
    data[0] = ADDR_RUN_MODE & 0x00FF;
    data[1] = ADDR_RUN_MODE >> 8;
    data[4] = mode;
    sendCanPackage(CMD_RAM_WRITE, master_can_id, data, 8, 3);
}


// --- RAM Write (Float Parameter Set) ---

void CyberGear::sendCanFloatPackage(uint16_t addr, float value, float min, float max) {
    uint8_t data[8] = {0x00};
    
    // 1. Prepare Address
    data[0] = addr & 0x00FF;
    data[1] = addr >> 8;
    
    // 2. Clamp Value
    if (value > max) value = max;
    else if (value < min) value = min;

    // 3. Copy Float Value (4 bytes)
    memcpy(&data[4], &value, 4);

    sendCanPackage(CMD_RAM_WRITE, master_can_id, data, 8, 1);
}

// --- Specific Parameter Setters ---
void CyberGear::setLimitSpeed(float speed) {
    sendCanFloatPackage(ADDR_LIMIT_SPEED, speed, 0.0f, V_MAX);
}
void CyberGear::setLimitCurrent(float current){
	sendCanFloatPackage(ADDR_LIMIT_TORQUE, current, 0.0f, I_MAX);
}
void CyberGear::setLimitTorque(float torque){
	sendCanFloatPackage(ADDR_LIMIT_TORQUE, torque, 0.0f, T_MAX);
}

// Current Control parameters
void CyberGear::setCurrentKp(float kp) {
    sendCanFloatPackage(ADDR_CURRENT_KP, kp, KP_MIN, KP_MAX);
}
void CyberGear::setCurrentKi(float ki) {
	sendCanFloatPackage(ADDR_CURRENT_KI, ki, KI_MIN, KI_MAX);
}
void CyberGear::setCurrentFilterGain(float gain) {
	sendCanFloatPackage(ADDR_CURRENT_FILTER_GAIN, gain, 0.0f, 1.0f);
}
void CyberGear::setCurrentRef(float current) {
	sendCanFloatPackage(ADDR_I_REF, current, I_MIN, I_MAX);
}

// Position control parameters
void CyberGear::setPositionKp(float kp) {
    sendCanFloatPackage(ADDR_POSITION_KP, kp, KP_MIN, KP_MAX);
}
void CyberGear::setPositionRef(float position) {
    sendCanFloatPackage(ADDR_POSITION_REF, position, POS_MIN, POS_MAX);
}

// Speed control parameters
void CyberGear::setSpeedKp(float kp) {
    sendCanFloatPackage(ADDR_SPEED_KP, kp, KP_MIN, KP_MAX);
}
void CyberGear::setSpeedKi(float ki) {
    sendCanFloatPackage(ADDR_SPEED_KI, ki, KI_MIN, KI_MAX);
}
void CyberGear::setSpeedRef(float speed) {
    sendCanFloatPackage(ADDR_SPEED_REF, speed, V_MIN, V_MAX);
}

// CAN ID Configuration
void CyberGear::setMotorCanID(uint8_t can_id) {
    uint8_t data[8] = {0x00};
    data[0] = CMD_SET_CAN_ID & 0x00FF;
    data[1] = CMD_SET_CAN_ID >> 8;
    data[4] = can_id;
    sendCanPackage(CMD_SET_CAN_ID, master_can_id, data, 8, 2);
}

// --- RX Message Processing ---
void CyberGear::processRxMessage(const twai_message_t& rx_message) {
    // 💡 Note: This is simplified as the Motor ID check should happen here too
    if (rx_message.extd == 0 || rx_message.data_length_code < 8) return;

    // 1. Extract Raw Data
    uint16_t raw_position = rx_message.data[1] | (rx_message.data[0] << 8);
    uint16_t raw_speed = rx_message.data[3] | (rx_message.data[2] << 8);
    uint16_t raw_torque = rx_message.data[5] | (rx_message.data[4] << 8);
    uint16_t raw_temperature = rx_message.data[7] | (rx_message.data[6] << 8);

    // 2. Convert and Update Status
    status.position = uintToFloat(raw_position, POS_MIN, POS_MAX);
    status.speed = uintToFloat(raw_speed, V_MIN, V_MAX);
    status.torque = uintToFloat(raw_torque, T_MIN, T_MAX);
    status.temperature = raw_temperature;
}

// --- Motion Control Command ---
void CyberGear::sendMotionControl(const CyberGearMotionCommand& cmd) {
    uint8_t data[8] = {0x00};

    uint16_t position = floatToUint(cmd.position, POS_MIN, POS_MAX, 16);
    data[0] = position >> 8;
    data[1] = position & 0x00FF;

    uint16_t speed = floatToUint(cmd.speed, V_MIN, V_MAX, 16);
    data[2] = speed >> 8;
    data[3] = speed & 0x00FF;

    uint16_t kp = floatToUint(cmd.kp, KP_MIN, KP_MAX, 16);
    data[4] = kp >> 8;
    data[5] = kp & 0x00FF;

    uint16_t kd = floatToUint(cmd.kd, KD_MIN, KD_MAX, 16);
    data[6] = kd >> 8;
    data[7] = kd & 0x00FF;

    // 💡 Note: Torque is often sent via the 'option' field in CMD_POSITION, 
    // but here we follow the original structure which passed torque implicitly.
    // If torque is not packed into the data payload, this structure is fine.
    
    // สมมติว่า CMD_POSITION ใช้สำหรับ Motion Control
    // และใช้ T_MAX/T_MIN เป็น Option
    uint16_t torque_option = floatToUint(cmd.torque, T_MIN, T_MAX, 16); 

    // ใช้ master_can_id เป็น option
    sendCanPackage(CMD_POSITION, torque_option, data, 8, 1); 
}