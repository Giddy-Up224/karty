#include "ESP32_NOW_Serial.h"
#include "MacAddress.h"
#include "WiFi.h"
#include "esp_wifi.h"
#include "pins_conf.h"

// 0: AP mode, 1: Station mode
#define ESPNOW_WIFI_MODE_STATION 1

// Channel to be used by the ESP-NOW protocol
#define ESPNOW_WIFI_CHANNEL 1

#if ESPNOW_WIFI_MODE_STATION          // ESP-NOW using WiFi Station mode
#define ESPNOW_WIFI_MODE WIFI_STA     // WiFi Mode
#define ESPNOW_WIFI_IF   WIFI_IF_STA  // WiFi Interface
#else                                 // ESP-NOW using WiFi AP mode
#define ESPNOW_WIFI_MODE WIFI_AP      // WiFi Mode
#define ESPNOW_WIFI_IF   WIFI_IF_AP   // WiFi Interface
#endif

// Set the MAC address of the device that will receive the data
//                           14:   33:   5C:   03:   69:   D9
const MacAddress peer_mac({0x14, 0x33, 0x5C, 0x03, 0x69, 0xD9});

ESP_NOW_Serial_Class NowSerial(peer_mac, ESPNOW_WIFI_CHANNEL, ESPNOW_WIFI_IF);

void setup() {
  Serial.begin(115200);

  // LEFT
  pinMode(LEFT_PWM, OUTPUT);
  pinMode(LEFT_FWD, OUTPUT);
  pinMode(LEFT_REV, OUTPUT);
  // RIGHT
  pinMode(RIGHT_PWM, OUTPUT);
  pinMode(RIGHT_FWD, OUTPUT);
  pinMode(RIGHT_REV, OUTPUT);

  Serial.print("WiFi Mode: ");
  Serial.println(ESPNOW_WIFI_MODE == WIFI_AP ? "AP" : "Station");
  WiFi.mode(ESPNOW_WIFI_MODE);

  Serial.print("Channel: ");
  Serial.println(ESPNOW_WIFI_CHANNEL);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  while (!(WiFi.STA.started() || WiFi.AP.started())) {
    delay(100);
  }

  Serial.print("MAC Address: ");
  Serial.println(ESPNOW_WIFI_MODE == WIFI_AP ? WiFi.softAPmacAddress() : WiFi.macAddress());

  // Start the ESP-NOW communication
  Serial.println("ESP-NOW communication starting...");
  NowSerial.begin(115200);
  Serial.printf("ESP-NOW version: %d, max data length: %d\n", ESP_NOW.getVersion(), ESP_NOW.getMaxDataLen());
  Serial.println("You can now send data to the peer device using the Serial Monitor.\n");
}

void loop() {
  static uint32_t last_rx_time = 0;
  while (NowSerial.available()) {
    // We're receiving 2 elements from the Master
    static uint8_t commands[2];
    // read both bytes into the array
    for(uint8_t i = 0; i < 2; i++) {
      commands[i] = NowSerial.read();
      Serial.print(commands[i]);
      Serial.print("\t");
    }
    Serial.println();           // newline after printing both values
    last_rx_time = millis();    // reset the timeout timer
    drive_it(commands); // Command the motors
  }
  if(millis() - last_rx_time > 200) {
    all_stop();
  }

  while (Serial.available() && NowSerial.availableForWrite()) {
    if (NowSerial.write(Serial.read()) <= 0) {
      Serial.println("Failed to send data");
      break;
    }
  }

  delay(1);
}


void all_stop() {
  digitalWrite(RIGHT_PWM, LOW);
  digitalWrite(RIGHT_FWD, LOW);
  digitalWrite(RIGHT_REV, LOW);
  digitalWrite(LEFT_PWM, LOW);
  digitalWrite(LEFT_FWD, LOW);
  digitalWrite(LEFT_REV, LOW);
}

bool forward(uint8_t *x_y) {
  x_y++; // got to y in array
  if(*x_y < 80) {
    Serial.printf("rev: %d\n", *x_y);
    return false;
  }
  Serial.printf("fwd: %d\n", *x_y);
  return true;
}

bool left(const uint8_t *x, uint8_t *swing) {
  // go left
  if(*x > 60) {
    *swing = (*x - 60);
    return true;
  }
  *swing = (60 - *x);
  return false;
}

void drive_it(uint8_t *x_y) {
  uint8_t *_y;
  uint8_t *y;
  uint8_t *x;
  _y = x_y;
  y = x_y;
  x = x_y;
  // swing var
  uint8_t swing;
  uint8_t *_swing = &swing;
  y++; // got to y in array
  if(forward(_y)) {
    digitalWrite(LEFT_FWD, HIGH);
    digitalWrite(LEFT_REV, LOW);
    digitalWrite(RIGHT_FWD, HIGH);
    digitalWrite(RIGHT_REV, LOW);
    if(left(x, _swing)) {
      analogWrite(LEFT_PWM, *y - swing);      
      analogWrite(RIGHT_PWM, *y + swing);
    }else if(!left(x, _swing)) {
      analogWrite(LEFT_PWM, *y + swing);      
      analogWrite(RIGHT_PWM, *y - swing);
    }
  } else {
    digitalWrite(LEFT_FWD, LOW);
    digitalWrite(LEFT_REV, HIGH);
    digitalWrite(RIGHT_FWD, LOW);
    digitalWrite(RIGHT_REV, HIGH);
    if(left(x, _swing)) {
      analogWrite(LEFT_PWM, (160 - *y) - swing);
      analogWrite(RIGHT_PWM, (160 - *y) + swing);
    } else if(!left(x, _swing)) {
      analogWrite(LEFT_PWM, (160 - *y) + swing);
      analogWrite(RIGHT_PWM, (160 - *y) - swing);
    }
  }
}