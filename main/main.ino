#include <Adafruit_NeoPixel.h>
#include <DHT20.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>
#include <LiquidCrystal_I2C.h>

// Define your tasks here
void TaskBlink(void *pvParameters);
void TaskLCDDisplay(void *pvParameters);
void TaskSoilMoistureAndRelay(void *pvParameters);
void TaskLightAndLED(void *pvParameters);
// void TaskTemperatureFan(void *pvParameters);
void TaskMQTT(void *pvParameters);
//Define your components here
Adafruit_NeoPixel pixels3(4, D5, NEO_GRB + NEO_KHZ800);
DHT20 dht20;
LiquidCrystal_I2C lcd(33,16,2);

// WiFi
const char *ssid = "ACLAB";
const char *password = "ACLAB2023";

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "emqx/esp32";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {

  // Initialize serial communication at 115200 bits per second:
  Serial.begin(115200); 
  dht20.begin();
  lcd.init();
  lcd.backlight();
  pixels3.begin();

  // Connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the Wi-Fi network");
  MQTTBroker();

  xTaskCreate( TaskBlink, "Task Blink" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskLCDDisplay, "Task LCD Display" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskSoilMoistureAndRelay, "Task Soild Relay" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate( TaskLightAndLED, "Task Light LED" ,2048  ,NULL  ,2 , NULL);
  // xTaskCreate( TaskTemperatureFan, "Task Temperature Fan", 2048, NULL, 2, NULL);
  xTaskCreate( TaskMQTT, "Task MQTT", 2048, NULL, 2, NULL);

  //Now the task scheduler is automatically started.
  Serial.printf("Basic Multi Threading Arduino Example\n");
}

void loop() {
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  while(1) {                          
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED ON
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED OFF
    delay(200);
  }
}


void TaskLCDDisplay(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  pinMode(A0, INPUT);

  while(1) {                          
    Serial.println("Task LCD Display");
    dht20.read();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nhiet do: ");
    lcd.print(dht20.getTemperature());
    lcd.setCursor(0, 1);
    lcd.print("Do am: ");
    lcd.print(dht20.getHumidity());
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Anh sang: ");
    lcd.print(analogRead(A1));
    lcd.setCursor(0, 1);
    lcd.print("Do am dat: ");
    lcd.print(analogRead(A0));
    delay(1000);
  }
}

void TaskSoilMoistureAndRelay(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  pinMode(D3, OUTPUT);

  while(1) {                          
    Serial.println("Task Soild and Relay");
    
    if(analogRead(A0) > 50){
      digitalWrite(D3, LOW);
      // analogWrite(D7, 250);
    }
    if(analogRead(A0) < 50){
      digitalWrite(D3, HIGH);
      // analogWrite(D7, 0);
    }
    delay(1000);
  }
}


void TaskLightAndLED(void *pvParameters) {  // This is a task.
  //uint32_t blink_delay = *((uint32_t *)pvParameters);

  while(1) {                          
    Serial.println("Task Light and LED");
    if(analogRead(A1) < 350){
      pixels3.setPixelColor(0, pixels3.Color(255,0,0));
      pixels3.setPixelColor(1, pixels3.Color(255,0,0));
      pixels3.setPixelColor(2, pixels3.Color(255,0,0));
      pixels3.setPixelColor(3, pixels3.Color(255,0,0));
      pixels3.show();
    }
    if(analogRead(A1) > 550){
      pixels3.setPixelColor(0, pixels3.Color(0,0,0));
      pixels3.setPixelColor(1, pixels3.Color(0,0,0));
      pixels3.setPixelColor(2, pixels3.Color(0,0,0));
      pixels3.setPixelColor(3, pixels3.Color(0,0,0));
      pixels3.show();
    }
    delay(1000);
  }
}

// void TaskTemperatureFan(void *pvParameters) {  // This is a task.
// //uint32_t blink_delay = *((uint32_t *)pvParameters);

//   while(1) {                          
//     // Serial.println("Task Temperature Fan");
//     // Serial.println(dht20.getTemperature());
//     if ((dht20.getTemperature() > 28)) {
//         analogWrite(D9, 250);
//     } else {
//         analogWrite(D9, 0);
//     }
//   }
//   delay(1000);
// }

void TaskMQTT(void *pvParameters) {  // This is a task.
//uint32_t blink_delay = *((uint32_t *)pvParameters);
    pinMode(D7, OUTPUT);
    while(1) {
      Serial.println("Task MQTT");                  
      client.loop();
      delay(1000);
    }
}

void MQTTBroker() {
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    // Publish and subscribe
    client.publish(topic, "Hi, I'm ESP32 ^^");
    client.subscribe(topic);
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    char *buf = (char*)malloc(20*sizeof(char));
    for (int i = 0; i < length; i++) {
        buf[i]=(char)payload[i];
        Serial.print(buf[i]);
    }
    Serial.println();
    Serial.println("-----------------------");
    if(!strcmp(buf,"on")){
      Serial.println("On");
      analogWrite(D7, 250);
    }else if(!strcmp(buf,"off")){
      Serial.println("Off");
      analogWrite(D7, 0);
    }
    free(buf);
}


