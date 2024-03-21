#include <WiFi.h>
#include <HTTPClient.h>

const char WIFI_SSID[] = "CMOICTD";
const char WIFI_PASSWORD[] = "@CMOictd2022";
const int soundPin = 34;   // GPIO pin connected to the sound sensor
const int ledPin = 2;      // ESP32 onboard LED pin
int soundVal;              // sound sensor readings

unsigned long lastRequestTime = 0;  // Variable to store the last time HTTP request was sent
unsigned long lastBelowThresholdTime = 0; // Variable to store the last time sound value was below threshold
bool thresholdExceeded = false;  // Flag to indicate if threshold is exceeded

String HOST_NAME = "http://afas.atwebpages.com"; // change to your PC's IP address
String PATH_NAME   = "/dashboard/db/alert.php?";
String LATITUDE             = "latitude=";
String LATITUDE_VALUE       = "6.07349&";
String LONGITUDE            = "&longitude=";
String LONGITUDE_VALUE      = "125.13165&";
String LABEL                = "&label=";
String LABEL_VALUE          = "Placeda,%20Calumpang&";
String BARANGAY_CODE        = "&barangay_code=";
String BARANGAY_CODE_VALUE  = "13&";

void setup() {
  pinMode(soundPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Read the sound sensor
  soundVal = analogRead(soundPin);

  // Print sound value to serial
  Serial.println(soundVal);

  // If sound level is above a threshold, indicate a clap
  if (soundVal > 300 && !thresholdExceeded) {
    thresholdExceeded = true; // Set the flag to true
    lastRequestTime = millis(); // Record the time
  }

  // Check if the threshold was exceeded for 5 seconds
  if (thresholdExceeded && millis() - lastRequestTime >= 1000) {
    digitalWrite(ledPin, HIGH); // Turn ON ESP32 onboard LED
    
    // Send coordinates to the web server
    HTTPClient http;
    http.begin(HOST_NAME + PATH_NAME + LATITUDE + LATITUDE_VALUE + LONGITUDE + LONGITUDE_VALUE + LABEL + LABEL_VALUE + BARANGAY_CODE + BARANGAY_CODE_VALUE);
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
      } else {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    
    // Reset the flag
    thresholdExceeded = false;
  } 
  
  // If the sound level is below the threshold
  if (!thresholdExceeded) {
    // Record the time when it was last below threshold
    lastBelowThresholdTime = millis();
  }
  
  // If it has been below threshold for 5 seconds, turn off LED
  if (!thresholdExceeded && millis() - lastBelowThresholdTime >= 5000) {
    digitalWrite(ledPin, LOW);
  }
}