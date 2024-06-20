#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "DHT.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// WIFI CREDENTIALS
#define WIFI_SSID "Put_your_own_WIFI_NAME"
#define WIFI_PASSWORD "Dont_Forget_your_own_password"

// Firebase Auth
#define API_KEY "AIzaSyAVOt978AGLZqchsJyRDItmnmYKBoOsCBw"
#define DATABASE_URL "https://orange-center-code-default-rtdb.firebaseio.com/"

// Temperature and Humidity Sensor
#define DHTPIN 4       
#define DHTTYPE DHT11

// RGB_1 LED pins
#define LED_R1 16
#define LED_G1 17
#define LED_B1 18   

// RGB_2 LED pins
#define LED_R2 19      
#define LED_G2 21       
#define LED_B2 22       

DHT dht(DHTPIN, DHTTYPE);

FirebaseData firebaseData;

// Thresholds for turning on LEDs
const float TEMP_THRESHOLD = 30.0;  
const float HUM_THRESHOLD = 50.0;   

float currentTemperature = 0.0;
float currentHumidity = 0.0;

// Unique identifier for each ESP32 i guess?
String deviceId;

void setup() {
  Serial.begin(115200);  
  dht.begin(); 

  
 // Initialize RGB LED pins
  pinMode(LED_R1, OUTPUT);
  pinMode(LED_G1, OUTPUT);
  pinMode(LED_B1, OUTPUT);
  digitalWrite(LED_R1, LOW);
  digitalWrite(LED_G1, LOW);
  digitalWrite(LED_B1, LOW);


  pinMode(LED_R2, OUTPUT);
  pinMode(LED_G2, OUTPUT);
  pinMode(LED_B2, OUTPUT);
  digitalWrite(LED_R2, LOW);
  digitalWrite(LED_G2, LOW);
  digitalWrite(LED_B2, LOW);

  // Connecting to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi");

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  // Get the unique device ID
  deviceId = String((uint32_t)ESP.getEfuseMac(), HEX);

  // Thread Creation for the tasks
  xTaskCreate(sendDataTask,"SendDataTask",4000, NULL,1,NULL);

  // Create a task for LED control
  xTaskCreate(ledControlTask,"LEDControlTask",1000,NULL,1,NULL);
}

void loop() {
  delay(2000);  

}

// Task to send data to Firebase
void sendDataTask(void *parameter) {
  for (;;) {
    float humidity = dht.readHumidity();        // Read humidity
    float temperature = dht.readTemperature();  // Read temperature in Celsius

    // Check if any reads failed and exit early (to try again)
    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      continue;
    }

    // Update current readings
    currentHumidity = humidity;
    currentTemperature = temperature;

    // Prepare JSON data
    FirebaseJson json;
    json.set("humidity", humidity);
    json.set("temperature", temperature);
    json.set("device_id", deviceId);

    // Send data to Firebase Realtime Database
    if (Firebase.set(firebaseData, "/sensorData/" + deviceId, json)) {
      Serial.println("Data sent to Firebase");
    } else {
      Serial.print("Error sending data: ");
      Serial.println(firebaseData.errorReason());
    }

    // Wait a few seconds between measurements
    vTaskDelay(2000 / portTICK_PERIOD_MS);  
  }
}

// Task to control LEDs based on conditions
void ledControlTask(void *parameter) {
  for (;;) {
    if (currentTemperature > TEMP_THRESHOLD) {
      // light up the red
      digitalWrite(LED_R1, HIGH);  
      digitalWrite(LED_G1, LOW);   
      digitalWrite(LED_B1, LOW);   
    } else {
      // light up the green
      digitalWrite(LED_R1, LOW);   
      digitalWrite(LED_G1, HIGH);  
      digitalWrite(LED_B1, LOW);   
    }

     if (currentHumidity > HUM_THRESHOLD) {
      digitalWrite(LED_R2, HIGH);  
      digitalWrite(LED_G2, LOW);   
      digitalWrite(LED_B2, LOW);   
    } else {
      digitalWrite(LED_R2, LOW);   
      digitalWrite(LED_G2, HIGH);  
      digitalWrite(LED_B2, LOW);   
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay for a second
  }
}
