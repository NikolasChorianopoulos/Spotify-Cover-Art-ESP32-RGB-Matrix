#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <TJpg_Decoder.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>
#include <Wire.h>
#include <SPI.h>
#include "SPIFFS.h"
#include <SD.h>

MatrixPanel_I2S_DMA matrix;


// WiFi Credentials
const char* ssid = "Your Wifi SSID";
const char* password = "Your Wifi Password";



void wifiSetup(){
  WiFi.begin(ssid, password);
  int timeout = 10;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
  delay(500);
  Serial.print(".");
  timeout--;
  }

  if (WiFi.status() != WL_CONNECTED){
    Serial.println("WiFi connection failed.");
    matrix.fillScreen(matrix.color565(250, 250, 250));
    uint16_t borderColor = matrix.color565(0, 0, 0);
    matrix.drawRect(2, 2, 60, 60, borderColor); //(x, y ,h ,w color)

    matrix.setCursor(21, 21);
    matrix.setTextColor(matrix.color565(0, 0, 0));
    matrix.setTextSize(1);
    matrix.print("WiFi"); 
    matrix.setCursor(17, 31); 
    matrix.print("Error");
    delay(1000000);
  }
  else {
    Serial.println("WiFi connected.");
    matrix.fillScreen(matrix.color565(250, 250, 250));
    uint16_t borderColor = matrix.color565(0, 0, 0);
    matrix.drawRect(2, 2, 60, 60, borderColor); //(x, y ,h ,w color)

    matrix.setCursor(21, 20);
    matrix.setTextColor(matrix.color565(0, 0, 0));
    matrix.setTextSize(1);
    matrix.print("WiFi"); 
    matrix.setCursor(5, 31); 
    matrix.print("Connected");
   
  }

  
}

// Spotify credentials
const char* client_id = "Your Client ID";
const char* client_secret = "Your Client Secret";
String refresh_token = "Your Refresh Token"; // Obtain this from initial authorization
String access_token;

// Spotify API URLs
const char* token_url = "https://accounts.spotify.com/api/token";
const char* currently_playing_url = "https://api.spotify.com/v1/me/player/currently-playing";

// Function to refresh Spotify access token
void refreshAccessToken() {
    Serial.println("Refreshing access token...");
    HTTPClient http;
    http.begin(token_url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String requestBody = "grant_type=refresh_token&refresh_token=" + refresh_token +
                         "&client_id=" + client_id + "&client_secret=" + client_secret;

    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode == 200) {
        String response = http.getString();
        Serial.println("Access token refreshed");

        // Parse JSON to extract the new access token
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, response);
        access_token = doc["access_token"].as<String>();
    } else {
        Serial.println("Failed to refresh token. HTTP code: " + String(httpResponseCode));
    }
    http.end();
}


//global variable to track the last imageUrl
String lastImageUrl = "";
String currentImageUrl;




void displaySetup() {
  // Matrix Panel Object
  HUB75_I2S_CFG mxconfig(
    64, // Width
    64, // Height
    1  // Number of chained panels
  );

    mxconfig.clkphase = false;

    //Pin Configuration, Initialize Matrix, Text Settings
    mxconfig.gpio.r1 = 25;
    mxconfig.gpio.g1 = 26;
    mxconfig.gpio.b1 = 27;
    mxconfig.gpio.r2 = 14;
    mxconfig.gpio.g2 = 12;
    mxconfig.gpio.b2 = 13;
    mxconfig.gpio.a = 23;
    mxconfig.gpio.b = 19;
    mxconfig.gpio.c = 5;
    mxconfig.gpio.d = 17;
    mxconfig.gpio.e = 32;
    mxconfig.gpio.lat = 4;
    mxconfig.gpio.oe = 15;
    mxconfig.gpio.clk = 16;
  
    
    matrix.begin(mxconfig);
    matrix.fillScreen(matrix.color565(0, 0, 0));
    matrix.setBrightness(150);
    

}

// Downloaded Image Buffer
uint8_t* jpgBuffer = nullptr;
size_t jpgSize = 0;

// Function to Render Pixels
bool tftOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  for (int16_t row = 0; row < h; row++) {
    for (int16_t col = 0; col < w; col++) {
      int16_t pixelX = x + col;
      int16_t pixelY = y + row;
      if (pixelX >= 0 && pixelY >= 0 && pixelX < 64 && pixelY < 64) {
        uint16_t color = bitmap[row * w + col];
        

        matrix.drawPixel(pixelX, pixelY, color);
      }
    }
  }
  return true;
}

// Download Image from URL
bool downloadImage() {
  
  HTTPClient http;
  http.begin(lastImageUrl);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    jpgSize = http.getSize();
    jpgBuffer = (uint8_t*)malloc(jpgSize);
    if (!jpgBuffer) {
      Serial.println("Failed to allocate memory for image.");
      return false;
    }

    WiFiClient* stream = http.getStreamPtr();
    size_t bytesRead = 0;
    while (http.connected() && bytesRead < jpgSize) {
      size_t available = stream->available();
      if (available) {
        bytesRead += stream->readBytes(jpgBuffer + bytesRead, available);
      }
    }

       
    http.end();
    return true;
  }

  Serial.println("Failed to download image.");
  http.end();
  return false;
}

//No Song Playing Image
void noSongPlaying(){

  matrix.fillScreen(matrix.color565(250, 250, 250));
  uint16_t borderColor = matrix.color565(0, 0, 0);
  matrix.drawRect(2, 2, 60, 60, borderColor); //(x, y ,h ,w color)

  matrix.setCursor(11, 20);
  matrix.setTextColor(matrix.color565(0, 0, 0));
  matrix.setTextSize(1);
  matrix.print("No Song"); 
  matrix.setCursor(11, 31); 
  matrix.print("Playing");
}

//get currently playing track json info -> img url
bool getCurrentlyPlaying() {
  HTTPClient http;
  http.begin(currently_playing_url);
  http.addHeader("Authorization", "Bearer " + access_token);

  int httpResponseCode = http.GET();
  if (httpResponseCode != 200) {
      Serial.println("Failed to fetch song. Showing 'No Song Playing'.");
      return false;  // API error 
  }

  String response = http.getString();
  DynamicJsonDocument doc(2048); // Reduced buffer size to prevent memory issues
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
      Serial.println("JSON Parse Error. Showing 'No Song Playing'.");
      return false;  // JSON issue → Show "No Song Playing"
  }

  //Check if song is playing
  bool isPlaying = doc["is_playing"];
  if (!isPlaying) {
      Serial.println("Spotify is paused or no song is playing.");
      return false;
  }

  // Extract album cover URL if available
  const char* imageUrl = doc["item"]["album"]["images"][2]["url"];  
  if (!imageUrl) {
      Serial.println("No album cover found. Showing 'No Song Playing'.");
      return false;
  }

  // Update album cover only if changed
  String newImageUrl = String(imageUrl);
  if (newImageUrl != lastImageUrl) {
      lastImageUrl = newImageUrl;
      Serial.printf("HTTP GET Response Code: %d\n", HTTP_CODE_OK);
      Serial.println("New album cover found: " + lastImageUrl);
  }
  return true; 
}


void setup() {
  Serial.begin(115200);

  //Initiate Display
  displaySetup();

  // Connect to WiFi
  wifiSetup();  
  
  
  // Refresh the token initially
  refreshAccessToken();  

  // Initialize JPEG Decoder
  TJpgDec.setCallback(tftOutput);
  TJpgDec.setSwapBytes(false);
    

}

void loop() {
 
  bool songPlaying = getCurrentlyPlaying();

  if (songPlaying) {
      
      if (downloadImage()) {
          TJpgDec.drawJpg(0, 0, jpgBuffer, jpgSize);
          free(jpgBuffer);
      }
  } else {
      noSongPlaying();
  }

  delay(1000);  
 

}
