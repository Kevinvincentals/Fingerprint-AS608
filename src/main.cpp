/**
 * ESP32 Fingerprint Sensor Project
 * 
 * This code is designed for integrating a fingerprint sensor with an ESP32. 
 * It includes functionality for enrolling new fingerprints and identifying 
 * registered fingerprints. The Adafruit Fingerprint Sensor Library is used 
 * for interfacing with the fingerprint sensor.
 * 
 * Author: Kevin Vincent Als
 * Date: 2024-01-06
 */

#include <Adafruit_Fingerprint.h>

// Hardware serial 1 for communication with the fingerprint sensor
HardwareSerial mySerial1(1);

// Pin configuration
uint8_t RX_PIN = 16;  // RX pin of ESP32 connected to TX of the sensor
uint8_t TX_PIN = 17;  // TX pin of ESP32 connected to RX of the sensor

// Initialize fingerprint sensor
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial1);

// Function declarations
void enrollFingerprint(uint8_t id);
uint8_t getFingerprintIDez();

void setup() {
  Serial.begin(9600);
  while (!Serial);  // Wait for Serial to be ready
  
  Serial.println("Adafruit Fingerprint sensor enrollment and identification");

  // Start communication with the fingerprint sensor
  mySerial1.begin(57600, SERIAL_8N1, RX_PIN, TX_PIN);
  
  // Check if the fingerprint sensor is connected
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  // Check and enroll fingerprints if none are stored
  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    Serial.println("No fingerprints enrolled yet. Starting enrollment.");
    enrollFingerprint(1);  // Enroll a fingerprint with ID 1
  } else {
    Serial.print(finger.templateCount);
    Serial.println(" fingerprint(s) found. Skipping enrollment.");
    Serial.println("Waiting for valid finger...");
  }
}

void loop() {
  // Continuously check for a valid fingerprint
  getFingerprintIDez();
  delay(50);  // Short delay between scans
}

void enrollFingerprint(uint8_t id) {
  uint8_t p = -1;
  Serial.print("Waiting for valid finger to enroll as ID ");
  Serial.println(id);

  // Wait for a valid finger to start enrollment
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        delay(50);
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        return;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        return;
      default:
        Serial.println("Unknown error");
        return;
    }
  }

  // Convert the image to a fingerprint model
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error in image conversion");
    return;
  }

  Serial.println("Remove finger");
  delay(2000);
  Serial.println("Place same finger again");

  // Repeat process for a second scan of the same finger
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      Serial.print(".");
      delay(50);
    }
  }

  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) {
    Serial.println("Error in second image conversion");
    return;
  }

  // Create a model from the two images
  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    Serial.println("Error in model creation");
    return;
  }

  // Store the model with the given ID
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Fingerprint enrolled successfully!");
  } else {
    Serial.print("Error in storing fingerprint model, Error code: ");
    Serial.println(p);
  }
}

uint8_t getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return p;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return p;

  // Search for a fingerprint match
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Found a match with ID #");
    Serial.println(finger.fingerID);
    delay(2000);  // Delay after a successful match
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("No match found");
    delay(500);  // Delay if no match is found
  } else {
    Serial.println("Unknown error");
    delay(500);  // Delay on error
  }

  return finger.fingerID;
}
