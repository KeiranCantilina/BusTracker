#include <Adafruit_EPD.h>
#include <Adafruit_MCPSRAM.h>
#include <Adafruit_ThinkInk.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <String>


// --- Configuration ---
const char* ssid = "YOUR_WIFI_SSID";          // Replace with your Wi-Fi network name
const char* password = "YOUR_WIFI_PASSWORD";  // Replace with your Wi-Fi password
const char* url = "https://www.transsee.ca/predict?s=cota.001.NHI1STN";    // The target website (use http:// or https://)
const String startTag = "<div class=\"divp\">";
const String endTag = "</div>";
// ---------------------

void setup() {
  Serial.begin(115200);
  delay(100);

  // 1. Connect to Wi-Fi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi!");
    return;
  }
}

void loop() {
  // We only want to run this once, so we put the logic in loop() and then stop.
  
  String htmlContent = "";
  String extractedText = "";

  // 2. Perform HTTP GET Request
  HTTPClient http;
  
  Serial.print("\n[HTTP] Starting GET request to: ");
  Serial.println(url);
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode > 0) { // Check for successful request
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      Serial.printf("[HTTP] GET success, code: %d\n", httpCode);
      htmlContent = http.getString();
      
      // 3. Simple HTML Parsing
      int startIndex = htmlContent.indexOf(startTag);
      
      if (startIndex != -1) {
        // Move past the starting tag
        startIndex += startTag.length();
        int endIndex = htmlContent.indexOf(endTag, startIndex);

        if (endIndex != -1) {
          // Extract the raw content between the tags
          String rawContent = htmlContent.substring(startIndex, endIndex);

          // Simple cleanup: remove all inner HTML tags (e.g., <p>, <span>)
          // This is a common but imperfect way to get the text on simple embedded parsers.
          bool inTag = false;
          for (int i = 0; i < rawContent.length(); i++) {
            char c = rawContent.charAt(i);
            if (c == '<') {
              inTag = true;
            } else if (c == '>') {
              inTag = false;
            } else if (!inTag) {
              extractedText += c;
            }
          }
          extractedText.trim(); // Remove leading/trailing whitespace
        }
      }
      
      // 4. Display the Result
      Serial.println("\n=============================================");
      if (extractedText.length() > 0) {
        Serial.println("Extracted Text from .divp node:");
        Serial.println(extractedText);
      } else {
        Serial.println("Could not find or extract content from the .divp node.");
      }
      Serial.println("=============================================");
      
    } else {
      Serial.printf("[HTTP] GET failed, error: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] Connection failed: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end(); // Free the resources

  // If there's stuff to update::
  DisplayStuff();

  // Else display "no buses" message
    
  // Loop after 1 minute
  delay(60000);
}

bool Displaystuff(){
    // Push stuff to E-ink display here
}