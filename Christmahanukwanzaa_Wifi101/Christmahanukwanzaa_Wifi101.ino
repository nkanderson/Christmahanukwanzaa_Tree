/***************************************************
  Neo Pixel Christmahanukwanzaa Tree - Wifi101 Version

  Light up a tree with all the colors of the holidays!
  Control the color, pattern, size, and speed of animation of a
  strip of neo pixels through a web page.

  See the Adafruit learning system guide for more details
  and usage information:

  Dependencies:
  - Neo Pixel Library
    https://github.com/adafruit/Adafruit_NeoPixel

  License:

  This example is copyright (c) 2016 Nicole Anderson
  and is released under an open source MIT license.  See details at:
    http://opensource.org/licenses/MIT

  This code was adapted from Adafruit CC3000 library example
  code which has the following license:

  Designed specifically to work with the Adafruit WiFi products:
  ----> https://www.adafruit.com/products/1469

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/
#include <SPI.h>
#include <WiFi101.h>
#include <WiFiMDNSResponder.h>
#include <Adafruit_NeoPixel.h>

// Neo pixel configuration
#define     PIXEL_PIN              6    // The pin which is connected to the neo pixel strip input.
#define     PIXEL_COUNT            100   // The number of neo pixels in the strip.

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

char ssid[] = "";      //  your network SSID (name)
char pass[] = "";   // your network password
char mdnsName[] = "lights"; // the MDNS name that the board will respond to: lights.local
int status = WL_IDLE_STATUS;
// Create a MDNS responder to listen and respond to MDNS name requests.
// removing mdns
//WiFiMDNSResponder mdnsResponder;
WiFiServer server(80);

// Color scheme definitions.
struct Color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;

  Color(uint8_t red, uint8_t green, uint8_t blue): red(red), green(green), blue(blue) {}
  Color(): red(0), green(0), blue(0) {}
};

struct ColorScheme {
  Color* colors;
  uint8_t count;

  ColorScheme(Color* colors, uint8_t count): colors(colors), count(count) {}
};

Color rgbColors[3] = { Color(255, 0, 0), Color(0, 255, 0), Color(0, 0, 255) };
ColorScheme rgb(rgbColors, 3);

Color christmasColors[2] = { Color(255, 0, 0), Color(0, 255, 0) };
ColorScheme christmas(christmasColors, 2);

Color hanukkahColors[2] = { Color(0, 0, 255), Color(255, 255, 255) };
ColorScheme hanukkah(hanukkahColors, 2);

Color kwanzaaColors[3] = { Color(255, 0, 0), Color(0, 0, 0), Color(0, 255, 0) };
ColorScheme kwanzaa(kwanzaaColors, 3);

Color rainbowColors[7] = { Color(255, 0, 0), Color(255, 128, 0), Color(255, 255, 0), Color(0, 255, 0), Color(0, 0, 255), Color(128, 0, 255), Color(255, 0, 255) };
ColorScheme rainbow(rainbowColors, 7);

Color incandescentColors[2] = { Color(255, 140, 20), Color(0, 0, 0) };
ColorScheme incandescent(incandescentColors, 2);

Color fireColors[3] = { Color(255, 0, 0), Color(255, 102, 0), Color(255, 192, 0) };
ColorScheme fire(fireColors, 3);

ColorScheme schemes[7] = { incandescent, rgb, christmas, hanukkah, kwanzaa, rainbow, fire };

// Enumeration of possible pattern types.
enum Pattern { BARS = 0, GRADIENT };

// Bar width values (in number of pixels/lights) for different size options.
int barWidthValues[3] = { 1,      // Small
                          3,      // Medium
                          6  };   // Large

// Gradient width values (in number of gradient repetitions, i.e. more repetitions equals a smaller gradient) for different size options.
int gradientWidthValues[3] = { 12,     // Small
                               6,      // Medium
                               2   };  // Large

// Speed values in amount of milliseconds to move one pixel/light.  Zero means no movement.
int speedValues[4] = { 0,       // None
                       500,     // Slow
                       250,     // Medium
                       50   };  // Fast

// Variables to hold current state.
int currentScheme = 0;
Pattern currentPattern = BARS;
int currentWidth = 0;
int currentSpeed = 0;
String buffer;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network.
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the status:
  printWifiStatus();

  server.begin();

  // Setup the MDNS responder to listen to the configured name.
  // NOTE: You _must_ call this _after_ connecting to the WiFi network and
  // being assigned an IP address.
  // removing mdns
//  if (!mdnsResponder.begin(mdnsName)) {
//    Serial.println("Failed to start MDNS responder!");
//    while(1);
//  }
//
//  Serial.print("Server listening at http://");
//  Serial.print(mdnsName);
//  Serial.println(".local/");
//  Serial.print("Firmware version: ");
//  Serial.println(WiFi.firmwareVersion());

  // Initialize the neo pixel strip.
  strip.begin();
  strip.show();

}

// Compute the color of a pixel at position i using a gradient of the color scheme.
// This function is used internally by the gradient function.
struct Color gradientColor(struct ColorScheme& scheme, int range, int gradRange, int i) {
  int curRange = i / range;
  int rangeIndex = i % range;
  int colorIndex = rangeIndex / gradRange;
  int start = colorIndex;
  int end = colorIndex+1;
  if (curRange % 2 != 0) {
    start = (scheme.count-1) - start;
    end = (scheme.count-1) - end;
  }
  return Color(map(rangeIndex % gradRange, 0, gradRange, scheme.colors[start].red,   scheme.colors[end].red),
               map(rangeIndex % gradRange, 0, gradRange, scheme.colors[start].green, scheme.colors[end].green),
               map(rangeIndex % gradRange, 0, gradRange, scheme.colors[start].blue,  scheme.colors[end].blue));
}

// Display a gradient of colors for the provided color scheme.
// Repeat is the number of repetitions of the gradient (pick a multiple of 2 for smooth looping of the gradient).
// SpeedMS is the number of milliseconds it takes for the gradient to move one pixel.  Set to zero for no animation.
void gradient(struct ColorScheme& scheme, int repeat = 1, int speedMS = 1000) {
  if (scheme.count < 2) return;

  int range = (int)ceil((float)PIXEL_COUNT / (float)repeat);
  int gradRange = (int)ceil((float)range / (float)(scheme.count - 1));

  unsigned long time = millis();
  int offset = speedMS > 0 ? time / speedMS : 0;

  Color oldColor = gradientColor(scheme, range, gradRange, PIXEL_COUNT-1+offset);
  for (int i = 0; i < PIXEL_COUNT; ++i) {
    Color currentColor = gradientColor(scheme, range, gradRange, i+offset);
    if (speedMS > 0) {
      // Blend old and current color based on time for smooth movement.
      strip.setPixelColor(i, map(time % speedMS, 0, speedMS, oldColor.red,   currentColor.red),
                             map(time % speedMS, 0, speedMS, oldColor.green, currentColor.green),
                             map(time % speedMS, 0, speedMS, oldColor.blue,  currentColor.blue));
    }
    else {
      // No animation, just use the current color.
      strip.setPixelColor(i, currentColor.red, currentColor.green, currentColor.blue);
    }
    oldColor = currentColor;
  }
  strip.show();
}

// Display solid bars of color for the provided color scheme.
// Width is the width of each bar in pixels/lights.
// SpeedMS is number of milliseconds it takes for the bars to move one pixel.  Set to zero for no animation.
void bars(struct ColorScheme& scheme, int width = 1, int speedMS = 1000) {
  int maxSize = PIXEL_COUNT / scheme.count;
  if (width > maxSize) return;

  int offset = speedMS > 0 ? millis() / speedMS : 0;

  for (int i = 0; i < PIXEL_COUNT; ++i) {
    int colorIndex = ((i + offset) % (scheme.count * width)) / width;
    strip.setPixelColor(i, scheme.colors[colorIndex].red, scheme.colors[colorIndex].green, scheme.colors[colorIndex].blue);
  }
  strip.show();
}
void loop() {
  // Call the update() function on the MDNS responder every loop iteration to
  // make sure it can detect and respond to name requests.
//  mdnsResponder.poll();
  char c;

  // Handle any HTTP connections.
  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        buffer = "";
        // Expect "GET /arduino/".
        while (client.available() > 0 && buffer.length() < 13) {
          c = char(client.read());
          buffer += c;
        }
        if (!buffer.equals("GET /arduino/") || !client.available() > 0) {
          Serial.println("not = or not cnctd");
          Serial.println(buffer);
          // Return error
          client.stop();
          return;
        }
        // Parse value until '/' character.
        buffer = "";
        char ch = client.read();
        while (client.available() > 0 && ch != '/') {
          buffer += ch;
          ch = client.read();
        }
        if (ch != '/' || !client.available() > 0) {
          client.stop();
          return;
        }
        // Parse a single digit value.
        int value = client.read() - '0';
        // Update state appropriately.
        if (buffer.equals("scheme")) {
          Serial.println(buffer);
          Serial.println(value);
          currentScheme = constrain(value, 0, 6);
        }
        else if (buffer.equals("pattern")) {
          Serial.println(buffer);
          Serial.println(value);
          currentPattern = (Pattern)constrain(value, 0, 1);
        }
        else if (buffer.equals("width")) {
          Serial.println(buffer);
          Serial.println(value);
          currentWidth = constrain(value, 0, 2);
        }
        else if (buffer.equals("speed")) {
          Serial.println(buffer);
          Serial.println(value);
          currentSpeed = speedValues[constrain(value, 0, 3)];
        }
        // Send an empty response and close the connection.
        // Note that sending a custom status code and content type as below is not
        // documented officially: http://forum.arduino.cc/index.php?PHPSESSID=es72i5nserl8lojnk3vl86d8r6&topic=191895.0
        // ^ Above is for the Yun; keeping documentation in place to explain intent of response
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type: application/javascript");
        client.println("Connection: close");
        client.println();
        Serial.println(client.available());
        client.flush();
        client.stop();
        Serial.println("client disconnected");
      }
    }
  }

  // Update pixels based on current state.
  if (currentPattern == BARS) {
    bars(schemes[currentScheme], barWidthValues[currentWidth], currentSpeed);
  }
  else if (currentPattern == GRADIENT) {
    gradient(schemes[currentScheme], gradientWidthValues[currentWidth], currentSpeed);
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
