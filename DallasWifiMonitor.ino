#include <FS.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "sha256.h"
#include "base64.h"

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include "WiFiManager.h"

void setup() {
	SetupSleep();
	ConfigSetup();

	Serial.begin(115200);
	delay(100);
	Serial.println();
	printInfo();

	if (LoadConfig() == false || ConfigModeRequested()) {
		EnableConfigMode();
		ESP.reset();
	}

	SetupDallas();
}

void loop() {
	ReadPowerValue();

	ConnectWiFi();

	ReadDallasValue();

	PublishData();

	GotoSleep();
}

void printInfo() {
	Serial.print("Chip Id:");
	Serial.println(ESP.getChipId());

	Serial.print("Flash Chip Id:");
	Serial.println(ESP.getFlashChipId());

	Serial.print("Flash Chip Speed:");
	Serial.println(ESP.getFlashChipSpeed());

	Serial.print("Flash Chip Size:");
	Serial.println(ESP.getFlashChipSize());

	Serial.print("Cpu Freq MHz:");
	Serial.println(ESP.getCpuFreqMHz());

	Serial.print("Sdk Version:");
	Serial.println(ESP.getSdkVersion());

	Serial.print("Boot Version:");
	Serial.println(ESP.getBootVersion());

	Serial.print("Boot Mode:");
	Serial.println(ESP.getBootMode());
}