#include <Adafruit_ThinkInk.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include <String>

// Eink config
#define EPD_DC 9
#define EPD_CS 6
#define EPD_BUSY 7 // can set to -1 to not use a pin (will wait a fixed delay)
#define SRAM_CS -1
#define EPD_RESET 8  // can set to -1 and share with microcontroller Reset!
#define EPD_SPI &SPI // primary SPI

ThinkInk_213_Mono_GDEY0213B74 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY, EPD_SPI);

// Network config 
const char* ssid = "ATTYus6YvR";          // Replace with your Wi-Fi network name
const char* password = "t8hwu+hxd%n7";  // Replace with your Wi-Fi password
const char* url = "https://www.transsee.ca/predict?s=cota.001.NHI1STN";    // The target website (use http:// or https://)
const String startTag = "<div class=divp id=\"001_NHI1STN_1\">"; // for immediately next bus
const String startTag2 = "<div class=divp id=\"001_NHI1STN_2\">"; // for following bus
const String endTag = "</div>";

// Status flag
bool isError = false;

// Other globals
String errorText = "";
String nextBusText_1 = "";
String nextBusText_2 = "";
String oxtBusText_1 = "";
String oxtBusText_2 = "";

// Preamble text
String preamble1 = "Next bus in ";
String preamble2 = "Oxt bus in ";


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  display.begin(THINKINK_MONO);

  // Connect to Wi-Fi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  displayError("Connecting to WiFi: \n" + String(ssid));
  delay(1000);
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
    isError = true;
    errorText = "Failed to connect to WiFi!\nPlease power cycle and try \nagain.";
    displayError(errorText);
    while(true){delay(1000);}
  }
}

void loop() {
  String htmlContent = "";

  // Perform HTTP GET Request
  HTTPClient http;
  
  Serial.print("\n[HTTP] Starting GET request to: ");
  Serial.println(url);
  
  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode > 0) { // Check for successful request
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      Serial.printf("[HTTP] GET success, code: %d\n", httpCode);
      htmlContent = http.getString();
      
      // FOR DEBUG, print HTML to serial
      //Serial.println(htmlContent);

      // Extract/parse text
      String nextBusText = partitionText(FetchText(htmlContent, startTag, endTag));
      String oxtBusText = partitionText(FetchText(htmlContent, startTag2, endTag));

      // Break into before and after bits
      nextBusText_1 = atSplitBefore(nextBusText);
      nextBusText_2 = atSplitAfter(nextBusText);
      oxtBusText_1 = atSplitBefore(oxtBusText);
      oxtBusText_2 = atSplitAfter(oxtBusText);

      

      // Serial print result for debugging
      Serial.println("\n=============================================");
      if (nextBusText.length() > 0) {
        Serial.println("Next bus in " + nextBusText_1 + nextBusText_2);
        Serial.println("Oxt bus in " + oxtBusText_1 + oxtBusText_2);
        isError = false;
      } else {
        Serial.println("Could not find or extract content from html.");
        // If there's no next bus:
        isError= true;
        displayError("No bus information to display!");
      }
      Serial.println("=============================================");
      
    } else {
      Serial.printf("[HTTP] GET failed, error: %d\n", httpCode);
      isError = true;
      displayError("[HTTP] GET failed: " + String(http.errorToString(httpCode)));
    }
  } else {
    Serial.printf("[HTTP] Connection failed: %s\n", http.errorToString(httpCode).c_str());
    isError = true;
    displayError("[HTTP] Connection failed: " + String(http.errorToString(httpCode)));
  }

  http.end(); // Free the resources

  

  if (!isError){
    DisplayStuff(preamble1+nextBusText_1, nextBusText_2, preamble2+oxtBusText_1, oxtBusText_2);
  }
    
  // Loop after 1 minute
  delay(60000);
}

void DisplayStuff(String string1, String string2, String string3, String string4){
  display.clearBuffer();

  display.setFont(&FreeSansBold9pt7b);
  display.setTextSize(1);
  display.setTextWrap(true);
  display.setTextColor(EPD_BLACK);
  display.setCursor(30, 30); // Start location of text
  display.println(string1);

  display.setFont(&FreeSans9pt7b);
  display.setTextColor(EPD_DARK);
  display.setCursor(60, 50);
  display.println(string2);

  display.setFont(&FreeSansBold9pt7b);
  display.setTextColor(EPD_BLACK);
  display.setCursor(30, 80);
  display.println(string3);

  display.setFont(&FreeSans9pt7b);
  display.setTextColor(EPD_DARK);
  display.setCursor(60, 100);
  display.println(string4);
  display.display();
}

String FetchText(String content, String tag, String end){
  String output = "";
  int startIndex = content.indexOf(tag);
      
  if (startIndex != -1) {
    // Move past the starting tag
    startIndex += startTag.length();
    int endIndex = content.indexOf(end, startIndex);

    if (endIndex != -1) {
      // Extract the raw content between the tags
      String rawContent = content.substring(startIndex, endIndex);

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
          output += c;
        }
      }
      output.trim(); // Remove leading/trailing whitespace
      return output;
    }
  }
  return "";
}

String partitionText(String input){
  // Substring time. Grab between first ":" and "."
  int colon = input.indexOf(":")+2;
  int period = input.indexOf(".");
  return input.substring(colon, period); // Start index, stop index
  // Example output:  "0:35 Mins at 4:33:02PM"
}

String atSplitAfter(String input){
  return input.substring(input.indexOf("a"));
}

String atSplitBefore(String input){
  return input.substring(0,input.indexOf("a"));
}

void displayError(String errorMessage){
  display.clearBuffer();

  display.setFont(&FreeSansBold9pt7b);
  display.setTextSize(1);
  display.setTextWrap(true);
  display.setTextColor(EPD_BLACK);
  display.setCursor(10, 30); // Start location of text
  display.println(errorMessage);
  display.display();
}