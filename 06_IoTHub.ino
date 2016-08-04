// Azure IoT Hub Settings
const char* TARGET_URL = "/devices/";
const char* IOT_HUB_END_POINT = "/messages/events?api-version=2015-08-15-preview";

String endPoint;
String fullSas;

int limit = 0; // limiter for response loop
int sendCount = 0;

WiFiClientSecure tlsClient;

const int BufferLen = 256;
char buffer[BufferLen];

String iotHubResponseCode = "NaN";
void PublishData() {
	if (!IsConfigValid()) {
		return;
	}

	InitialiseIotHub();

	int length;
	StaticJsonBuffer<300> jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();

	root["DeviceID"] = (String)ESP.getChipId();
	root["DeviceType"] = "WifiTemperatureSensor";
	root["mV"] = powerVoltage;
	root["Temp"] = dallasTemperature;

	length = root.printTo(buffer, BufferLen);

	PublishToAzure((String)buffer);
}

boolean IsConfigValid() {
	bool retVal = true;

	if (strlen(IOTHUB_HOST_NAME) > 0) {
		Serial.printf("IOTHUB_HOST_NAME = '%s'.\n", IOTHUB_HOST_NAME);
	}
	else {
		Serial.println("IOTHUB_HOST_NAME is not configured.");
		retVal = false;
	}

	if (strlen(SAS_KEY) > 0) {
		Serial.printf("SAS_KEY = '%s'.\n", SAS_KEY);
	}
	else {
		Serial.println("SAS_KEY is not configured.");
		retVal = false;
	}

	if (strlen(SAS_EXPIRY) > 0) {
		Serial.printf("SAS_EXPIRY = '%s'.\n", SAS_EXPIRY);
	}
	else {
		Serial.println("SAS_EXPIRY is not configured.");
		retVal = false;
	}

	return retVal;
}

void PublishToAzure(String data) {
	String request = "POST " + endPoint + " HTTP/1.1\r\n" +
		"Host: " + IOTHUB_HOST_NAME + "\r\n" +
		"Authorization: SharedAccessSignature " + fullSas + "\r\n" +
		"Content-Type: application/atom+xml;type=entry;charset=utf-8\r\n" +
		"Content-Length: " + data.length() + "\r\n\r\n" + data;

	Serial.printf("Trying to send the following message:\n---\n%s\n---\n", request.c_str());

	if (!tlsClient.connected()) {
		ConnectToAzure();
	}

	if (!tlsClient.connected()) {
		Serial.println("Could not connect to azure.");
		iotHubResponseCode = "NC";
	}
	else {
		tlsClient.print(request);

		String response = "";
		String chunk = "";
		limit = 0;

		do {
			if (tlsClient.connected()) {
				delay(5);  // give esp8266 firmware networking some time
				chunk = tlsClient.readStringUntil('\n');
				response += chunk;
			}
			limit++;
		} while (chunk.length() > 0 && limit < 100);

		tlsClient.stop();

		if (response.length() > 12) {
			iotHubResponseCode = response.substring(9, 12);
			Serial.printf("Response code: %s.\n", iotHubResponseCode.c_str());
		}
		else {
			Serial.printf("Response:\n---%s\n---\n", response.c_str());
		}
	}
}

void InitialiseIotHub() {
	String url = UrlEncode(IOTHUB_HOST_NAME) + UrlEncode(TARGET_URL) + (String)ESP.getChipId();
	endPoint = (String)TARGET_URL + (String)ESP.getChipId() + (String)IOT_HUB_END_POINT;

	fullSas = CreateIotHubSas(SAS_KEY, url);
}

void ConnectToAzure() {
	if (WiFi.status() != WL_CONNECTED) { return; }
	if (!tlsClient.connect(IOTHUB_HOST_NAME, 443)) {      // Use WiFiClientSecure class to create TLS connection
		Serial.println("Host connection failed.");
	}
	else {
		Serial.println("Host connected.");
	}
}

String CreateIotHubSas(char *key, String url) {
	String stringToSign = url + "\n" + SAS_EXPIRY;

	// START: Create signature
	// https://raw.githubusercontent.com/adamvr/arduino-base64/master/examples/base64/base64.ino

	int keyLength = strlen(key);

	int decodedKeyLength = base64_dec_len(key, keyLength);
	char* decodedKey = new char[decodedKeyLength];  //allocate char array big enough for the base64 decoded key
	base64_decode(decodedKey, key, keyLength);  //decode key

	Sha256.initHmac((const uint8_t*)decodedKey, decodedKeyLength);
	Sha256.print(stringToSign);
	char* sign = (char*)Sha256.resultHmac();
	delete[] decodedKey;
	// END: Create signature

	// START: Get base64 of signature
	int encodedSignLen = base64_enc_len(HASH_LENGTH);
	char* encodedSign = new char[encodedSignLen];
	base64_encode(encodedSign, sign, HASH_LENGTH);

	// SharedAccessSignature
	String retVal = "sr=" + url + "&sig=" + UrlEncode(encodedSign) + "&se=" + SAS_EXPIRY;
	// END: create SAS 
	delete[] encodedSign;
	return retVal;
}

String UrlEncode(const char* msg)
{
	const char *hex = "0123456789abcdef";
	String encodedMsg = "";

	while (*msg != '\0') {
		if (('a' <= *msg && *msg <= 'z')
			|| ('A' <= *msg && *msg <= 'Z')
			|| ('0' <= *msg && *msg <= '9')) {
			encodedMsg += *msg;
		}
		else {
			encodedMsg += '%';
			encodedMsg += hex[*msg >> 4];
			encodedMsg += hex[*msg & 15];
		}
		msg++;
	}
	return encodedMsg;
}