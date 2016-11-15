// source: https://github.com/ltvanderkrogt/Thermostat


const int  buttonPinUp = 2;    // the pin that the pushbutton is attached to
const int  buttonPinDown = 3;    // the pin that the pushbutton is attached to

// Variables will change:
int buttonStateUp = 0;         // current state of the button
int lastButtonStateUp = 0;     // previous state of the button
int buttonStateDown = 0;         // current state of the button
int lastButtonStateDown = 0;     // previous state of the button
float pinVoltage = 0;

boolean wait = false;

int heat = 0;

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ThingSpeak.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 4, 5);   //display(I2C address, SDA, SCL)

void thingspeak(float MTemp, float SetTemp, float pinVoltage);  //declaration of this function
void handle_range();                                            //declaration of this function
void chkButtons(int SetTemp);                                   //declaration of this function
void getTemp(float MTemp);                                      //declaration of this function
void checkClient();                                             //declaration of this function
void Thermostat(String Heater);                                 //declaration of this function

char ssid[] = " ";    //  your network SSID (name)
char password[] = " ";   // your network password
//
// thingspeak
unsigned long myChannelNumber =  ;
const char * myWriteAPIKey = " ";
const char * myReadAPIKey = " ";

//// Replace with your network credentials
//const char* ssid = "nnnnnnnnnn";
//const char* password = "********************";
//
//// https://thingspeak.com/  //setup your own account an channels
//unsigned long myChannelNumber = myChannelNumber;
//const char * myWriteAPIKey = "myWriteAPIKey";
//const char * myReadAPIKey = "myReadAPIKey";

//Feedback_LED
#ifndef LED_BUILTIN
#define LED_BUILTIN 13  //on board LED Blue 
#define LED_RED 15  //on board LED Red 
#define LED_GREEN 12  //on board LED Green 
#endif

// battery level
#define VOLTAGE_MAX 5.0
#define VOLTAGE_MAXCOUNTS 1023.0

// timer
#define interval 15000 // ThingSpeak will only accept updates every 15 seconds.
unsigned long waitUntil = 0;

// Temperature
float MTemp = 20;
int SetTemp = 16;
#define ONE_WIRE_BUS 14  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
String Heater = "";

// HTTP server will listen at port 80
ESP8266WebServer server(80);

// Connect to WiFi network
WiFiClient  client;

void setup(void) {
  // initialize serial communication:
  Serial.begin(115200);             //for debugging with terminal
  Serial.println("hello");          //for debugging with terminal
  Serial.println();
  Serial.print("Welcome SmartTerm");

  // initialize the display
  display.init();  // Initialising the UI will init the display too.
  display.flipScreenVertically();   // up side down or straight
  display.setFont(ArialMT_Plain_10);  //default font. Create more fonts at http://oleddisplay.squix.ch/

  // initialize the button pin as a input:
  pinMode(buttonPinUp, INPUT);      // Initialize the pushbutton pin as an input:
  pinMode(buttonPinDown, INPUT);    // initialize the pushbutton pin as an input:

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(LED_RED, OUTPUT);         // Initialize the LED_BUILTIN pin as an output
  pinMode(LED_GREEN, OUTPUT);       // Initialize the LED_GREEN pin as an output

  WiFi.begin(ssid, password);
  ThingSpeak.begin(client);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  server.begin();
  Serial.println("HTTP server started");

  server.on("/", handle_range);       //if no arguments on url, goto "handle_range"
  server.on("/range", handle_range);  //if argument "range" is found on url, goto handle_range
  //  Serial.println("SetTemp?");
  //  Serial.println(SetTemp);
  //  Serial.print("MTemp1=");
  //  Serial.println(MTemp);
  // Serial.print("Heater handle_range=");
  // Serial.println(Heater);
}

void loop(void) {
  float pinVoltage = analogRead(A0) * (VOLTAGE_MAX / VOLTAGE_MAXCOUNTS);
  //  Serial.println(pinVoltage);
  Thermostat(Heater);
  //  Serial.print("Heater Loop=");
  //  Serial.println(Heater);
  //   Serial.print("Heat=");
  //  Serial.println(heat);

  getTemp(MTemp);
  //  Serial.println(MTemp);
  chkButtons(SetTemp);
  //  Serial.println(SetTemp);
  lastButtonStateUp = buttonStateUp;       // save the current state as the last state, for next time through the loop
  lastButtonStateDown = buttonStateDown;   // save the current state as the last state, for next time through the loop

  checkClient();

  display.clear();    // clear the display
  display.setFont(ArialMT_Plain_24);    // change font

  String ShowTemp = "";       // float to string
  ShowTemp.concat(MTemp);     // float to string
  ShowTemp.remove(4);         // remove last character 0  //buggy, if temp <10 remove(3)
  ShowTemp +=  "°C";
  String ShowSetTemp = "";
  ShowSetTemp.concat(SetTemp);
  ShowSetTemp +=  "°C";
  //     ShowTemp = ShowTemp + "°C (" + ShowSetTemp + "°C)";  //all on 1 line

  display.drawString(0, 0, ShowTemp);   // draw string (column(0-128), row(0-64, "string")
  display.setFont(ArialMT_Plain_16);    // change font
  display.drawString(0, 48, ShowSetTemp);   // draw string (column(0-128), row(0-64, "string")
  if (Heater = "aan") {
    //    Serial.print("Heater Loop2=");
    //    Serial.println(Heater);

    //   display.drawString(50, 48, Heater);   // draw string (column(0-128), row(0-64, "string")
  }
  else {
    display.clear();    // clear the display
    Heater = "uit";
  }
  display.display();  //display all you have in display memory
}

void Thermostat(String Heater) {
  int heat = 0;
  if (SetTemp % 1 == 0) {                 // the modulo function gives you the remainder of the division of two numbers.
    if (SetTemp > MTemp) {
      Heater = "aan";
      Serial.println("SetTemp=");
      Serial.println(SetTemp);
      Serial.println("MTemp1=");
      Serial.println(MTemp);
      Serial.println("Heater on in 10 sec");
      delay(10000);                       // delay 10 seconds before switching on prevent toggling the heater.
      Serial.println("Heater Thermostat1=");
      Serial.println(Heater);
      digitalWrite(LED_GREEN, HIGH );  // Turn the LED on
    } else {
      Heater = "uit";
      Serial.println("Heater Thermostat2=");
      Serial.println(Heater);
      digitalWrite(LED_GREEN, LOW );  // Turn the LED on
    }
    display.drawString(50, 48, Heater);   // draw string (column(0-128), row(0-64, "string")
    display.display();  //display all you have in display memory
  }
  return;
}

void checkClient() {
  server.handleClient();  // check for incomming client connections frequently in the main loop:
  if ((unsigned long)(millis() - waitUntil) >= interval) {  // timer
    //    Serial.println(" thingspeak ");
    thingspeak(MTemp, SetTemp, pinVoltage);   //send arguments to thingspeak
  }
  waitUntil = waitUntil + interval;  // wait another interval cycle
  wait = !wait;
  //  Serial.println(waitUntil);
  //  Serial.println(wait);
}


void handle_range() {
  SetTemp = server.arg("SetTemp").toInt();  // get the value of request argument "SetTemp" and convert it to an int
  if (SetTemp < 5) {
    SetTemp = 16;
  }
  String HTML_start = "<!DOCTYPE html><html><meta http-equiv='Refresh' content='15'>"; //refresh page every 15 seconds
  String StyleSheet = "<head><style>input[type=number] {width: 450;padding: 22px 20px;margin: 8px 0;box-sizing: border-box;border: 2px solid red;border-radius: 4px;background-color: lightblue;font-size: 40px;}</style></head>";
  String Data = "<p><center><span style='font-size:20px;'><span style='font-family:arial,helvetica,sans-serif;'>Thermostaat Berkenheuvel</span><p><span style='font-size:20px;'><span style='font-family:arial,helvetica,sans-serif;'>" + String(MTemp) + "°C</span></p>";
  String form = "<form action='range'><center><input type='number' name='SetTemp' step='0.5' value='" + String(SetTemp) + "'></center><br><input type='submit'></form>";
  String HeaterStatus = "<p><span style='font-size:20px;'><span style='font-family:arial,helvetica,sans-serif;'>" + String(Heater) + "</span><br>";
  String Thingspeak_iframe = "<br><embed width='450' height='260' style='border: 1px solid #cccccc;' src='https://thingspeak.com/apps/plugins/45285'></embed><br>";
  String HTML_end = "</center></html>";
  String webpage = (String(HTML_start) + String(StyleSheet) + String(Data) + String(form) + String(HeaterStatus) + String(Thingspeak_iframe) + String(HTML_end));
  //  Serial.println("Heater handle_range=");


  server.send(200, "text/html", String(webpage));
  return;
}

void chkButtons(int SetTemp) { // read the pushbutton input pin:
  buttonStateUp = digitalRead(buttonPinUp);
  buttonStateDown = digitalRead(buttonPinDown);

  // compare the buttonState to its previous state
  if (buttonStateUp != lastButtonStateUp || buttonStateDown != lastButtonStateDown) {
    // if the state has changed, increment the counter
    if (buttonStateUp == HIGH) {          // if the current state is HIGH then the button wend from off to on:
      SetTemp++;                          //increment the counter
    }
    if (buttonStateDown == HIGH) {        // if the current state is HIGH then the button wend from off to on:
      SetTemp--;                          //increment the counter
    }
    delay(50); // Delay a little bit to avoid bouncing
  }
  return;
}

void getTemp(float MTemp) { // Read temperature
  do {
    DS18B20.requestTemperatures();
    MTemp = DS18B20.getTempCByIndex(0);
    MTemp = (int) (MTemp * 10 + 0.5);
    MTemp =  MTemp / 10;
  }
  while (MTemp == 85.0 || MTemp == (-127.0));
  return;
}

void thingspeak(float MTemp, int SetTemp, float pinVoltage) {
  ThingSpeak.setField(1, MTemp);
  ThingSpeak.setField(2, SetTemp);
  ThingSpeak.setField(3, pinVoltage);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); // Write the fields that you've set all at once.
}

