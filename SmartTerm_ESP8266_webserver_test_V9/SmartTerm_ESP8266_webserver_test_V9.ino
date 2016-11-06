// source: https://github.com/ltvanderkrogt/Thermostat



#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ThingSpeak.h>


void thingspeak(float MTemp, float SetTemp, float pinVoltage);  //declaration of this function

// Replace with your network credentials
const char* ssid = "nnnnnnnnnn";
const char* password = "********************";

// https://thingspeak.com/  //setup your own account an channels
unsigned long myChannelNumber = myChannelNumber;
const char * myWriteAPIKey = "myWriteAPIKey";
const char * myReadAPIKey = "myReadAPIKey";

//Feedback_LED
#ifndef LED_BUILTIN
#define LED_BUILTIN 13  //on board LED Blue 
#define LED_RED 15  //on board LED Red 
#define LED_GREEN 12  //on board LED Green 
#endif

// battery level
#define VOLTAGE_MAX 5.0
#define VOLTAGE_MAXCOUNTS 1023.0

// set pin numbers:
const int buttonUpPin = 2;     // the number of the pushbutton pin
const int buttonDownPin = 3;   // the number of the pushbutton pin

// timer
#define interval 15000 // ThingSpeak will only accept updates every 15 seconds.
unsigned long waitUntil = 0;

// variables will change:
int buttonUpState = 0;         // variable for reading the pushbutton status
int buttonDownState = 0;         // variable for reading the pushbutton status
int lastbuttonUpState = 0;


// Temperature
float oldMTemp;
float MTemp = 20;
float SetTemp = 16;
#define ONE_WIRE_BUS 14  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
String Heater = "";

// HTTP server will listen at port 80
ESP8266WebServer server(80);

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

  server.send(200, "text/html", String(webpage));
}

// Connect to WiFi network
WiFiClient  client;

void setup(void) {
  Serial.begin(115200);             //for debugging with terminal
  Serial.println("hello");          //for debugging with terminal

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(LED_RED, OUTPUT);         // Initialize the LED_BUILTIN pin as an output
  pinMode(LED_GREEN, OUTPUT);       // Initialize the LED_GREEN pin as an output
  pinMode(buttonUpPin, INPUT);      // initialize the pushbutton pin as an input:
  pinMode(buttonDownPin, INPUT);    // initialize the pushbutton pin as an input:

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
  Serial.println("SetTemp?");
  Serial.println(SetTemp);
  Serial.print("MTemp1=");
  Serial.println(MTemp);
  Serial.print("Heater=");
  Serial.println(Heater);
}

void loop(void) {


  float pinVoltage = analogRead(A0) * (VOLTAGE_MAX / VOLTAGE_MAXCOUNTS);

  // temperature
  do {
    DS18B20.requestTemperatures();
    MTemp = DS18B20.getTempCByIndex(0);
    MTemp = (int) (MTemp * 10 + 0.5);
    MTemp =  MTemp / 10;
  }
  while (MTemp == 85.0 || MTemp == (-127.0));

  //Thermostat
  if (MTemp < SetTemp)
  {
    Serial.println("Heater on in 10 sec");
    delay(10000);
    Serial.println("Heater off");
    digitalWrite(LED_GREEN, HIGH );  // Turn the LED on
    Heater = "Heater off";
  } else {
    Serial.println("Heater on");
    Heater = "Heater on";
    digitalWrite(LED_GREEN, LOW );  // Turn the LED on
  }

  server.handleClient();  // check for incomming client connections frequently in the main loop:

  if ((unsigned long)(millis() - waitUntil) >= interval) {  // timer
    Serial.println(" thingspeak ");
    thingspeak(MTemp, SetTemp, pinVoltage);   //send arguments to thingspeak
  }
  waitUntil = waitUntil + interval;  // wait another interval cycle
}

void thingspeak(float MTemp, float SetTemp, float pinVoltage) {
  ThingSpeak.setField(1, MTemp);
  ThingSpeak.setField(2, SetTemp);
  ThingSpeak.setField(3, pinVoltage);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); // Write the fields that you've set all at once.
}
