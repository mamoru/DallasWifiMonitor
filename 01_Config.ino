int configPin = D8;
int configPullupPin = D7;

// Start region overal settings
char INTERVAL_IN_SECONDS[16] = "180";
// End region overal settings

// Start region device SharedAccessSignature settings
char IOTHUB_HOST_NAME[256];
char SAS_KEY[64];
char SAS_EXPIRY[16];
// Start region device SharedAccessSignature settings

bool shouldSaveConfig = false;
void ConfigSetup() {
	pinMode(configPullupPin, OUTPUT);
	digitalWrite(configPullupPin, HIGH);

	pinMode(configPin, INPUT);
}

void SaveConfigCallback() {
	Serial.println("Should save config.");
	shouldSaveConfig = true;
}

bool ConfigModeRequested() {
	bool retVal = false;
	if (digitalRead(configPin) == HIGH) {
		Serial.printf("Pin %d is pulled to HIGH. Entering config mode.\n", configPin);
		retVal = true;
	}
	else {
		Serial.printf("Pin %d is LOW. Continue reading data.\n", configPin);
	}

	return retVal;
}

void EnableConfigMode() {
	WiFiManagerParameter custom_IOTHUB_HOST_NAME("IOTHUB_HOST_NAME", "IoT Hub Host Name", IOTHUB_HOST_NAME, 256);
	WiFiManagerParameter custom_SAS_KEY("SAS_KEY", "SAS Key", SAS_KEY, 64);
	WiFiManagerParameter custom_SAS_EXPIRY("SAS_EXPIRY", "SAS Key Expiry", SAS_EXPIRY, 16);
	WiFiManagerParameter custom_INTERVAL_IN_SECONDS("INTERVAL_IN_SECONDS", "Upload Interval", INTERVAL_IN_SECONDS, 16);

	WiFiManager wifiManager;
	wifiManager.setSaveConfigCallback(SaveConfigCallback);

	wifiManager.addParameter(&custom_IOTHUB_HOST_NAME);
	wifiManager.addParameter(&custom_SAS_KEY);
	wifiManager.addParameter(&custom_SAS_EXPIRY);
	wifiManager.addParameter(&custom_INTERVAL_IN_SECONDS);

	String apName = String("ESP") + (String)ESP.getChipId();

	wifiManager.startConfigPortal(apName.c_str());

	if (shouldSaveConfig) {
		strcpy(IOTHUB_HOST_NAME, custom_IOTHUB_HOST_NAME.getValue());
		strcpy(SAS_KEY, custom_SAS_KEY.getValue());
		strcpy(SAS_EXPIRY, custom_SAS_EXPIRY.getValue());
		strcpy(INTERVAL_IN_SECONDS, custom_INTERVAL_IN_SECONDS.getValue());

		SaveConfig();
	}
}

bool LoadConfig() {
	bool retVal = false;
	if (SPIFFS.begin()) {
		Serial.println("Mounted file system.");
		if (SPIFFS.exists("/config.json")) {
			//file exists, reading and loading
			Serial.println("Reading config file.");
			File configFile = SPIFFS.open("/config.json", "r");
			if (configFile) {
				Serial.println("Opened config file.");
				size_t size = configFile.size();
				// Allocate a buffer to store contents of the file.
				std::unique_ptr<char[]> buf(new char[size]);

				configFile.readBytes(buf.get(), size);
				DynamicJsonBuffer jsonBuffer;
				JsonObject& json = jsonBuffer.parseObject(buf.get());
				json.printTo(Serial);
				if (json.success()) {
					Serial.println("\nParsed json.");

					strcpy(IOTHUB_HOST_NAME, json["IOTHUB_HOST_NAME"]);
					strcpy(SAS_KEY, json["SAS_KEY"]);
					strcpy(SAS_EXPIRY, json["SAS_EXPIRY"]);
					strcpy(INTERVAL_IN_SECONDS, json["INTERVAL_IN_SECONDS"]);

					retVal = true;
				}
				else {
					Serial.println("\nFailed to load json config.");
				}
			}
		}
	}
	else {
		Serial.println("Failed to mount FS.");
	}

	if (retVal) {
		Serial.println("Succesfully loaded config.");
	}
	else {
		Serial.println("Could not load config.");
	}

	return retVal;
}

bool SaveConfig() {
	Serial.println("Saving config.");
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();

	json["IOTHUB_HOST_NAME"] = IOTHUB_HOST_NAME;
	json["SAS_KEY"] = SAS_KEY;
	json["SAS_EXPIRY"] = SAS_EXPIRY;
	json["INTERVAL_IN_SECONDS"] = INTERVAL_IN_SECONDS;

	File configFile = SPIFFS.open("/config.json", "w");
	if (!configFile) {
		Serial.println("Failed to open config file for writing.");
	}
	else {
		json.printTo(Serial);
		json.printTo(configFile);
		configFile.close();
	}
}