// Main sketch for ESP32 CyberGear driver
#include "CyBerGear.h"

// Define GPIO pins for TWAI (CAN) - ESP32-S3 pins
#define TWAI_TX_PIN GPIO_NUM_17
#define TWAI_RX_PIN GPIO_NUM_18

// Create CyberGear instance
CyberGear motor(0x7F, 0x00); // Motor ID = 0x7F, Master ID = 0x00

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("Initializing CyberGear...");
    
    // Initialize TWAI (CAN) at 1 Mbps
    if (motor.begin(TWAI_TX_PIN, TWAI_RX_PIN, 1000000)) {
        Serial.println("CyberGear initialized successfully!");
    } else {
        Serial.println("Failed to initialize CyberGear");
        while(1) delay(1000);
    }
    
    // Example: Enable motor
    motor.enableMotor();
    Serial.println("Motor enabled");
}

// Like this right?
void loop() {
    // Request motor status periodically
    motor.requestStatus();
    
    // Get and print status
    CyberGearStatus status = motor.getStatus();
    Serial.print("Position: ");
    Serial.print(status.position);
    Serial.print(" | Speed: ");
    Serial.print(status.speed);
    Serial.print(" | Torque: ");
    Serial.print(status.torque);
    Serial.print(" | Temp: ");
    Serial.println(status.temperature);

    twai_message_t rx_message;
    if (twai_receive(&rx_message, 0) == ESP_OK) {
        motor.processRxMessage(rx_message);
    }
    delay(10);
}
