/*******************************************************************
    this is a basic example how to program a Telegram Bot
    using TelegramBOT library on ESP8266
 *                                                                 *
    Open a conversation with the bot, you can command via Telegram
    a led from ESP8266 GPIO
    https://web.telegram.org/#/im?p=@FlashledBot_bot
 *                                                                 *
    written by Giancarlo Bacchio
    modified by Leon van der Krogt (group commands and security)
 *******************************************************************/
/* Pin (on the board)  Function  ESP8266 correspondence
  TX    TXD       TXD
  RX    RXD       RXD
  A0    Analog input, max 3.3V input  A0
  D0  IO  GPIO16
  D1  IO, SCL GPIO5
  D2  IO, SDA GPIO4
  D3  IO, 10k Pull-up GPIO0
  D4  IO, 10k Pull-up, BUILTIN_LED  GPIO2  // do not use this as output, esp won't start at power on!
  D5  IO, SCK GPIO14
  D6  IO, MISO  GPIO12
  D7  IO, MOSI  GPIO13
  D8  IO, 10k Pull-down, SS GPIO15
  G GND GND
  5V  5V  –
  3V3 3.3V  3.3V
  RST Reset RST
*/

String the_path = __FILE__;
int slash_loc = the_path.lastIndexOf('/');
String the_cpp_name = the_path.substring(slash_loc + 1);
int dot_loc = the_cpp_name.lastIndexOf('.');
String the_sketchname = the_cpp_name.substring(0, dot_loc);
String Date = __DATE__;
String Time = __TIME__;

char ssid[] = "***********";    //  your network SSID (name)
char password[] = "***********";   // your network password

void pinup();
void pindown();
// #include <Bounce2.h> // https://github.com/thomasfredericks/Bounce2
#define buttonPinUp 12
#define buttonPinDown 13
#define ledPin 16

// Variables will change:
volatile int SetTemp = 16;   // counter for the number of button presses
int SetTempOld;
float MTempOld;
String Brander;
String HeaterStatus;
float pinVoltage = 0;
boolean wait = false;

#include <ESP8266WiFi.h>
// ************************** Begin OTA **************************
// OTA Includes. Use IDE 1.6.7 or higher (https://github.com/esp8266/Arduino/blob/master/doc/ota_updates/readme.md)
#include <ESP8266mDNS.h>
#include <WiFiUdp.h> //nodig?
#include <ArduinoOTA.h>
// ArduinoOTA.setPassword((const char *)"123");
// ************************** End OTA **************************

// Initialize Telegram BOT
#include <WiFiClientSecure.h>
#include <ESP8266TelegramBOT.h>
#include <ThingSpeak.h>

// mac dc350f thermostaat
#define BOTtoken "*********:************************************"  //token of BOT
#define BOTname "*********"
#define BOTusername "*********_bot"

#define VOLTAGE_MAX 5.0
#define VOLTAGE_MAXCOUNTS 1023.0

TelegramBOT bot(BOTtoken, BOTname, BOTusername);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done
bool Start = false;
bool ReStart = false;

void thingspeak(float MTemp, int SetTemp, float pinVoltage);
// thingspeak
unsigned long myChannelNumber = ******;
const char * myWriteAPIKey = "******************";
const char * myReadAPIKey = "******************";
// timer
#define interval_ts 15000 // ThingSpeak will only accept updates every 15 seconds.
unsigned long waitUntil = 0;
const char* server = "api.thingspeak.com";

/* Telegram menu to cut&pate in the Telegram bot (beware: do not use CAPITALS!)
status - status
plus1 - graad omhoog
min1 - graad omlaag
versie - programma info
*/
// end Initialize Telegram BOT

#include <OneWire.h>
#include <DallasTemperature.h>
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

// Temperature
float MTemp ;
#define ONE_WIRE_BUS 14  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, 4, 5);   //display(I2C address, SDA, SCL)

WiFiClient  client;

void setup() {
  // ************************** Begin OTA **************************
  ArduinoOTA.begin();
  // ************************** End OTA **************************

  // Setup the first button with an internal pull-up :
  pinMode(buttonPinUp, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPinUp), pinup, RISING);

  // Setup the second button with an internal pull-up :
  pinMode(buttonPinDown, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPinDown), pindown, RISING);

  // initialize the display
  display.init();  // Initialising the UI will init the display too.
  display.flipScreenVertically();   // up side down or straight
  display.setFont(ArialMT_Plain_10);  //default font. Create more fonts at http://oleddisplay.squix.ch/



  pinMode(ledPin, OUTPUT); // initialize the LED as an output
  Serial.begin(115200); // initialize serial communication
  Serial.println();
  Serial.print("Welcome SmartTerm");

  WiFi.mode(WIFI_STA); // client only, not visible as access point
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
} // end setup

void loop() {
  // ************************** Begin OTA **************************
  ArduinoOTA.handle();
  // ************************** End OTA **************************
  float pinVoltage = analogRead(A0) * (VOLTAGE_MAX / VOLTAGE_MAXCOUNTS);
  Serial.print("waitUntil :");
  Serial.println(waitUntil);
  Serial.print("SetTempOld :");
  Serial.println(SetTempOld);
  Serial.print("SetTemp :");
  Serial.println(SetTemp);

  if (millis() > Bot_lasttime + Bot_mtbs)  {
    delay(50);
    bot.getUpdates(bot.message[0][1]);   // launch API GetUpdates up to xxx message
    delay(50);
    Bot_ExecMessages();   // reply to message with Echo
    Bot_lasttime = millis();
  }
  MTemp = getTemp(MTemp);

  if (SetTemp % 1 == 0) {    // checking the modulo of the button push counter.
    if (SetTemp > MTemp) {
      delay(10000); //delay to start heating, normally 10 sec(10000)
      Brander = "aan";
      digitalWrite(ledPin, HIGH);
    } else {
      Brander = "uit";
      digitalWrite(ledPin, LOW);
    }
  } // end thermostaat

  String ShowTemp = "";       // float to string
  ShowTemp.concat(MTemp);     // float to string
  ShowTemp.remove(4);         // remove last character 0  //buggy, if temp <10 remove(3)
  ShowTemp +=  "°C";
  String ShowSetTemp = "";
  ShowSetTemp.concat(SetTemp);
  ShowSetTemp +=  "°C";
  //     ShowTemp = ShowTemp + "°C (" + ShowSetTemp + "°C)";  //all on 1 line
  display.clear();    // clear the display
  display.setFont(ArialMT_Plain_24);    // change font
  display.drawString(0, 0, ShowTemp);   // draw string (column(0-128), row(0-64, "string")
  display.setFont(ArialMT_Plain_16);    // change font
  display.drawString(0, 48, ShowSetTemp);   // draw string (column(0-128), row(0-64, "string")
  display.drawString(100, 48, Brander);   // draw string (column(0-128), row(0-64, "string")
  display.display();  //display all you have in display memory

  checkClient();

} //end loop

float getTemp(float MTemp) { // Read temperature
  do {
    DS18B20.requestTemperatures();
    MTemp = DS18B20.getTempCByIndex(0);
    MTemp = (int) (MTemp * 10 + 0.5);
    MTemp =  MTemp / 10;
    //    Serial.print("MTemp=");
    //    Serial.println(MTemp);
  }
  while (MTemp == 85.0 || MTemp == (-127.0));
  return MTemp;
}


void Bot_ExecMessages() {
  for (int i = 1; i < bot.message[0][0].toInt() + 1; i++)      {
    bot.message[i][5] = bot.message[i][5].substring(1, bot.message[i][5].length());

    if (bot.message[i][1] == bot.message[i][4])  { // Security: Group members only!
      String wellcome = "You are not authorised!";
      String wellcome1 = "If you think you should....";
      String wellcome2 = "@leonvanderkrogt";
      bot.sendMessage(bot.message[i][4], wellcome, "");
      bot.sendMessage(bot.message[i][4], wellcome1, "");
      bot.sendMessage(bot.message[i][4], wellcome2, "");
      Start = true;

    }
    else {
      int findString = bot.message[i][5].indexOf('@');                //find @ in text
      String BotName = bot.message[i][5].substring(findString + 1); // use text before @ only (reply from group command is start@accountname_bot
      String FindString = String(findString); // int to String

      if (BotName == BOTusername) {

        bot.message[i][5] = bot.message[i][5].substring(0, findString); // use text before @ only (reply from group command is start@accountname_bot

        if (bot.message[i][5] == "plus1") {
          SetTemp++;
          String ShowSetTemp = "";
          ShowSetTemp.concat(SetTemp);     // float to string
          bot.sendMessage(bot.message[i][4], ShowSetTemp, "");
        }
        if (bot.message[i][5] == "min1") {
          SetTemp--;
          String ShowSetTemp = "";
          ShowSetTemp.concat(SetTemp);     // float to string
          bot.sendMessage(bot.message[i][4], ShowSetTemp, "");
        }
        if (bot.message[i][5] == "status" || ReStart == false) {
          String ShowSetTemp = "";
          ShowSetTemp.concat(SetTemp);     // float to string
          String ShowMTemp = "";
          ShowMTemp.concat(MTemp);     // float to string

          String wellcome = "welkom";
          String wellcome1 = "overzicht thermostaat";
          String wellcome2 = "ingesteld: " + ShowSetTemp;
          String wellcome3 = "huidige temperatuur: " + ShowMTemp;
          String wellcome4 = "https://thingspeak.com/apps/plugins/132192";
          bot.sendMessage(bot.message[i][4], wellcome, "");
          bot.sendMessage(bot.message[i][4], wellcome1, "");
          bot.sendMessage(bot.message[i][4], wellcome2, "");
          bot.sendMessage(bot.message[i][4], wellcome3, "");
          bot.sendMessage(bot.message[i][4], wellcome4, "");
          ReStart = true;
        }
        if (bot.message[i][5] == "versie") {
          String Showssid = "";
          Showssid.concat(ssid);     // float to string
          String wellcome = the_sketchname;
          String wellcome1 = "Wifi: " + Showssid;
          String wellcome2 = "datum: " + Date;
          String wellcome3 = "tijd: " + Time;
          String wellcome4 = "https://thingspeak.com/apps/plugins/132192";
          bot.sendMessage(bot.message[i][4], wellcome, "");
          bot.sendMessage(bot.message[i][4], wellcome1, "");
          bot.sendMessage(bot.message[i][4], wellcome2, "");
          bot.sendMessage(bot.message[i][4], wellcome3, "");
          bot.sendMessage(bot.message[i][4], wellcome4, "");
        }
        if (bot.message[i][5] == "\start") {
          String wellcome = "Welkom, dit zijn de opdrachten die ik ken;";
          String wellcome1 = "/min1 : een graad omhoog";
          String wellcome2 = "/plus1 : een graad omlaag";
          String wellcome3 = "/status : geeft de temperatuur en ingestelde temperatuur";
          String wellcome4 = "klik op de link voor de grafiek";
          bot.sendMessage(bot.message[i][4], wellcome, "");
          bot.sendMessage(bot.message[i][4], wellcome1, "");
          bot.sendMessage(bot.message[i][4], wellcome2, "");
          bot.sendMessage(bot.message[i][4], wellcome3, "");
          bot.sendMessage(bot.message[i][4], wellcome4, "");
          Start = true;
        }
      }
    }
    bot.message[0][0] = "";   // All messages have been replied - reset new messages
  }
}

void thingspeak(float MTemp, int SetTemp, float pinVoltage) {
  Serial.println("thingspeak");
  ThingSpeak.setField(1, MTemp);
  ThingSpeak.setField(2, SetTemp);
  ThingSpeak.setField(3, pinVoltage);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); // Write the fields that you've set all at once.
}

void checkClient() {
  if ((unsigned long)(millis() - waitUntil) >= interval_ts) {  // timer
    //    Serial.println(" thingspeak ");
    thingspeak(MTemp, SetTemp, pinVoltage);   //send arguments to thingspeak
  }
  waitUntil = waitUntil + interval_ts;  // wait another interval cycle
  wait = !wait;
}

void pinup() {
  SetTemp++;
  Serial.print("plus");
}
void pindown() {
  SetTemp--;
  Serial.print("min");
}

