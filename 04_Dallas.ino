int dallasDataPin = D2;
int dallasPowerPin = D1;
uint8_t resolution = 12;

OneWire oneWire(dallasDataPin);
DallasTemperature dallas(&oneWire);
DeviceAddress dallasDeviceAddress;

bool  dallasSwitchedOn = false;
float dallasTemperature;

// SETTING UP DALLAS

void SetupDallas() {
	pinMode(dallasPowerPin, OUTPUT);

	SwitchDallasOn();

	dallas.begin();
	Serial.printf("Current global resolution: %d.\n", dallas.getResolution());

	dallas.getAddress(dallasDeviceAddress, 0);
	uint8_t currentResolution = dallas.getResolution(dallasDeviceAddress);
	Serial.printf("Current device resolution: %d.\n", currentResolution);

	if (currentResolution != resolution) {
		Serial.printf("Changing device resolution to: %d.\n", resolution);
		dallas.setResolution(dallasDeviceAddress, resolution);
	}

	SwitchDallasOff();
}

void SwitchDallasOn() {
	if (!dallasSwitchedOn) {
		dallasSwitchedOn = true;
		digitalWrite(dallasPowerPin, HIGH);
		delay(100);
		Serial.println("Switched on the Dallas sensor.");
	}
	else {
		Serial.println("Dallas sensor was already switched on.");
	}
}

void ReadDallasValue() {
	SwitchDallasOn();

	dallas.requestTemperaturesByAddress(dallasDeviceAddress);
	dallasTemperature = dallas.getTempC(dallasDeviceAddress);

	Serial.print("Measured temperature: ");
	Serial.print(dallasTemperature);
	Serial.println(".");

	SwitchDallasOff();
}

void SwitchDallasOff() {
	if (dallasSwitchedOn) {
		dallasSwitchedOn = false;
		digitalWrite(dallasPowerPin, LOW);
		Serial.println("Switched off the Dallas sensor.");
	}
	else {
		Serial.println("Dallas sensor was already switched off.");
	}
}