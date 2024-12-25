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

// WiFi Credentials
const char* ssid = "YOUR WIFI SSID";
const char* password = "YOUR WIFI PASSWORD";

// Spotify credentials
const char* client_id = "YOUR SPOTIFY CLIENT ID";
const char* client_secret = "YOUR SPOTIFY CLIENT SECRET";
String refresh_token = "YOUR REFRESH TOKEN"; 
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

// Function to get currently playing song's cover art
// Add a global variable to track the last imageUrl
String lastImageUrl = "";
String currentImageUrl;

void getCurrentlyPlaying() {
   
    HTTPClient http;
    http.begin(currently_playing_url);
    http.addHeader("Authorization", "Bearer " + access_token);

    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
        String response = http.getString();

        // Parse the JSON response
        DynamicJsonDocument doc(16384); // Increased buffer size
        DeserializationError error = deserializeJson(doc, response);
        if (error) {
            Serial.print("JSON Deserialization failed: ");
            Serial.println(error.f_str());
            return;
        }

        // Extract the URL of the smallest album cover
        const char* imageUrl = doc["item"]["album"]["images"][2]["url"]; // Change the index if you want a larger image
        if (imageUrl) {
            String currentImageUrl = String(imageUrl);
            if (currentImageUrl != lastImageUrl) {
                lastImageUrl = currentImageUrl; // Update the last imageUrl
                Serial.println("Album Cover URL: " + lastImageUrl);
            }
            // No action needed if URL hasn't changed
        } else {
            Serial.println("Image URL not found in the response.");
        }
    } else if (httpResponseCode == 401) {
        Serial.println("Access token expired. Refreshing token...");
        refreshAccessToken(); // Token expired, refresh it
    } else {
        Serial.println("Failed to get currently playing song. HTTP code: " + String(httpResponseCode));
    }
    http.end();
}



// Matrix Configuration
#define PANEL_RES_X 64
#define PANEL_RES_Y 64
#define PANEL_CHAIN 1

// Pin Config
#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13
#define A_PIN 23
#define B_PIN 19
#define C_PIN 5
#define D_PIN 17
#define E_PIN 32
#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 16

// Matrix Panel Object
HUB75_I2S_CFG mxconfig(
  PANEL_RES_X, // Width
  PANEL_RES_Y, // Height
  PANEL_CHAIN  // Number of chained panels
);
MatrixPanel_I2S_DMA matrix;

// Downloaded Image Buffer
uint8_t* jpgBuffer = nullptr;
size_t jpgSize = 0;

// Function to Render Pixels
// Function to Render Pixels
bool tftOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
  // Loop through the rectangular region and draw each pixel
  for (int16_t row = 0; row < h; row++) {
    for (int16_t col = 0; col < w; col++) {
      int16_t pixelX = x + col;
      int16_t pixelY = y + row;
      if (pixelX >= 0 && pixelY >= 0 && pixelX < PANEL_RES_X && pixelY < PANEL_RES_Y) {
        matrix.drawPixel(pixelX, pixelY, bitmap[row * w + col]);
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

    Serial.printf("HTTP GET Response Code: %d\n", httpCode);
    
    http.end();
    return true;
  }

  Serial.println("Failed to download image.");
  http.end();
  return false;
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");

  // Initialize Matrix
  mxconfig.gpio.r1 = R1_PIN;
  mxconfig.gpio.g1 = G1_PIN;
  mxconfig.gpio.b1 = B1_PIN;
  mxconfig.gpio.r2 = R2_PIN;
  mxconfig.gpio.g2 = G2_PIN;
  mxconfig.gpio.b2 = B2_PIN;
  mxconfig.gpio.a = A_PIN;
  mxconfig.gpio.b = B_PIN;
  mxconfig.gpio.c = C_PIN;
  mxconfig.gpio.d = D_PIN;
  mxconfig.gpio.e = E_PIN;
  mxconfig.gpio.lat = LAT_PIN;
  mxconfig.gpio.oe = OE_PIN;
  mxconfig.gpio.clk = CLK_PIN;

  matrix.begin(mxconfig);
  matrix.fillScreen(matrix.color565(0, 0, 0));

  // Initialize JPEG Decoder
  TJpgDec.setCallback(tftOutput);
  TJpgDec.setSwapBytes(false);

  // Refresh the token initially
    refreshAccessToken();

  

}

void loop() {
  
  getCurrentlyPlaying();
  
  
  // Download and Display Image
  if (downloadImage()) {
    TJpgDec.drawJpg(0, 0, jpgBuffer, jpgSize);
    free(jpgBuffer);
  } else {
    Serial.println("Image display failed.");
  }

    

    Serial.printf("Expected JPEG Size: %d\n", jpgSize);
    if (jpgBuffer) {
        Serial.println("JPEG buffer successfully allocated.");
    } else {
        Serial.println("Failed to allocate JPEG buffer!");
    }
  delay(0);
}
