//test 
//test 3
const int  buttonPinUp = 2;    // the pin that the pushbutton is attached to
const int  buttonPinDown = 3;    // the pin that the pushbutton is attached to
const int ledPin = 13;       // the pin that the LED is attached to

// Variables will change:
int SetTemp = 16;   // counter for the number of button presses
int buttonStateUp = 0;         // current state of the button
int lastButtonStateUp = 0;     // previous state of the button
int buttonStateDown = 0;         // current state of the button
int lastButtonStateDown = 0;     // previous state of the button
int Temp = 20;
String Brander;

void setup() {
  // initialize the button pin as a input:
  pinMode(buttonPinUp, INPUT);
  pinMode(buttonPinDown, INPUT);
  // initialize the LED as an output:
  pinMode(ledPin, OUTPUT);
  // initialize serial communication:
  Serial.begin(115200);
  Serial.println();
  Serial.print("Welcome SmartTerm");
}


void loop() {
  // read the pushbutton input pin:
  buttonStateUp = digitalRead(buttonPinUp);
  buttonStateDown = digitalRead(buttonPinDown);
  //  Serial.println("ingesteld: ");
  //  Serial.println(SetTemp);

  // compare the buttonState to its previous state
  if (buttonStateUp != lastButtonStateUp || buttonStateDown != lastButtonStateDown) {
    // if the state has changed, increment the counter
    if (buttonStateUp == HIGH) {
      // if the current state is HIGH then the button
      // wend from off to on:
      SetTemp++;
      //      Serial.println("on");
      //      Serial.print("number of button pushes:  ");
      //      Serial.println(SetTemp);
    }
    if (buttonStateDown == HIGH) {
      // if the current state is HIGH then the button
      // wend from off to on:
      SetTemp--;
 
    } else {

//    }
    // Delay a little bit to avoid bouncing
    delay(50);
    Serial.print("ingesteld: ");
    Serial.println(SetTemp);
    Serial.print("Temp: ");
    Serial.println(Temp);
    Serial.print("Brander: ");
    Serial.println(Brander);
  }
  // save the current state as the last state,
  //for next time through the loop
  lastButtonStateUp = buttonStateUp;
  lastButtonStateDown = buttonStateDown;

  // turns on the LED every 1 button pushes by
  // checking the modulo of the button push counter.
  // the modulo function gives you the remainder of
  // the division of two numbers:
  if (SetTemp % 1 == 0) {
    if (SetTemp > Temp) {
      delay(500);
      Brander = "aan";
      digitalWrite(ledPin, HIGH);
    } else {

      Brander = "uit";
      digitalWrite(ledPin, LOW);
      //
    }
  }

