#include <WiFi.h>
#include "ThingSpeak.h"
WiFiClient  client;

unsigned long myChannelNumber = 1067599;
const char * myWriteAPIKey = "ERKFD0J8CPJR2O9X";
const char* ssid = "automate";
const char* password = "success123!";
int vibrationsensor = 4;
int vibrationvalue=0;
int sumvibrationvalue;
unsigned long lastTime = 0;
unsigned long timerDelay = 20000;

void setup() {
  Serial.begin(115200);
    pinMode(vibrationsensor, INPUT);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
}

void loop() {
  // Send an HTTP GET request
  if ((millis() - lastTime) > timerDelay) {
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
         sumvibrationvalue=0;
          for (int i = 0; i <= 255; i++) {
           vibrationvalue=digitalRead(vibrationsensor);
           sumvibrationvalue +=vibrationvalue ;
                Serial.println(vibrationvalue);

          }
          //sumvibrationvalue=vibrationvalue;
           Serial.println("sumvibrationvalue");
              Serial.println(sumvibrationvalue);
    delay(10000);

           ThingSpeak.setField(1, sumvibrationvalue);

          
        int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
        if(x == 200){
          Serial.println("Channel update successful.");
        }
        else{
          Serial.println("Problem updating channel. HTTP error code " + String(x));
        }
       Serial.println("vibrationvalue");
      Serial.println(sumvibrationvalue);
  
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}
