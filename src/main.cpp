#include <Arduino.h>

#ifndef DEVICE_BOARD_NAME
#  define DEVICE_BOARD_NAME "bcmxcp"
#endif

#include "controlWiFi.h"

#include <esp_task_wdt.h>
#define WDT_TIMEOUT 60

#define LED_BUILTIN 22

#include "bcmxcp_io.h"


void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  esp_reset_reason_t reason = esp_reset_reason();
  switch (reason) {
    case ESP_RST_POWERON:
      Serial.println("Reset due to power-on event");
    break;
  
    case ESP_RST_SW:
      Serial.println("Software reset via esp_restart");
    break;

    case ESP_RST_WDT:
      Serial.println("Rebooted by Watchdog!");
    break;

    default:
    break;
  }

  //Set witchdog timeout for 32 seconds
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);

  while (esp_task_wdt_status(NULL) != ESP_OK) {
    // LED blinks indefinitely
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
  }
  

  Serial.print("Start WiFi on ");
  Serial.println(DEVICE_BOARD_NAME);
  
  initializeWiFi(DEVICE_BOARD_NAME);
  
  establishWiFi();

  // you're connected now, so print out the data
  printWifiStatus();

  //init connection to UPS
  upsdrv_initups();
  
  upsdrv_initinfo();

}

void loop() {
  //upsdrv_updateinfo();
  //esp_task_wdt_reset();
}