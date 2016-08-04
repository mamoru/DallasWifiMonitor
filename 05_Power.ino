ADC_MODE(ADC_VCC);

int powerVoltage;

void ReadPowerValue() {
	powerVoltage = ESP.getVcc();
	Serial.printf("Measured mV: %d.\n", powerVoltage);
}
