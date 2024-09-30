#include <ESP32Servo.h>

// Pin definitions
const int servoPin = 33;
const int limitSwitchPin = 32;
const int irSensorPin = 18;
const int qrCodeSerialPort = 2; // Use Serial2 for QR scanner

Servo feederServo;
bool standby = true;  // To track feeder state

// Variables for storing states
volatile bool irSensorDetected = false;  // Flag for IR sensor detection
String qrCodeData = "";

// Function declarations
void IRAM_ATTR irSensorISR();  // Interrupt handler for IR sensor
void activateFeeder();  // Function to activate feeder

// Setup function
void setup() {
  // Initialize Serial
  Serial.begin(115200);                  // Main Serial communication with PC
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // QR Scanner Serial communication
  Serial.println("################################# Rebooted! SniffSpace PORT #################################");
  Serial.println();
  Serial.println("****************************************");
  Serial.println("*           Dognosis Corp.             *");
  Serial.println("*          Sniffspace Ports            *");
  Serial.println("*           Firmware v1.1.1            *");
  Serial.println("****************************************");
  Serial.println();  // New line for spacing  

  // Initialize pins
  pinMode(limitSwitchPin, INPUT_PULLUP); // Limit switch
  pinMode(irSensorPin, INPUT);           // IR sensor

  // Attach the servo
  feederServo.attach(servoPin);

  // Setup interrupts for IR sensor
  attachInterrupt(digitalPinToInterrupt(irSensorPin), irSensorISR, FALLING);  // IR sensor interrupt (falling edge)
}

// Main loop
void loop() {
  // QR Code Handling
  if (Serial2.available()) {
    Serial.println("QR DATA Available");
    qrCodeData = Serial2.readStringUntil('\n');
    if (qrCodeData.length() > 0) {
        Serial.println("QR_DATA_RECEIVED:::" + qrCodeData); // new
    }
  }

  // Feeder Control: Activate feeder based on external command
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    if (command == "ACTIVATE_FEEDER") {
      activateFeeder();
    }
  }

  // Check for IR detection in loop (set by ISR)
  if (irSensorDetected) {
    Serial.println("Sniffed");
    irSensorDetected = false;  // Reset flag after detection
  }

  delay(10);  // Small delay for loop stability
}

// ISR for IR sensor
void IRAM_ATTR irSensorISR() {
  irSensorDetected = true;  // Set flag when IR sensor detects an object
}

// Function to activate the feeder
void activateFeeder() {
  Serial.println("Activating Feeder");

  feederServo.attach(servoPin);  // Attach servo

  // Move the servo to 0 degrees to dispense food
  feederServo.write(0);
  delay(100);
  standby = true;

  // Feeder operation logic
  while (digitalRead(limitSwitchPin) == LOW || standby) {
    if (digitalRead(limitSwitchPin) == LOW) {
      standby = false;  // End-stop reached, stop the feeder
    }
  }

  // Stop the servo and detach
  feederServo.write(0);
  feederServo.detach();
  standby = true;

  Serial.println("Feeder operation complete.");
}