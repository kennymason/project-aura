#include <Wire.h>
#include "Waveshare_LCD1602_RGB.h"
#include <IRremote.h>

// Privacy Light - A Do-Not-Disturb Indicator

// colors mapped to keys
//numeric keys are hardcoded expected input from the IR remote I'm using to test (could differ with different remotes possibly)
uint32_t ylwKey = 16718055;
uint32_t redKey = 16718565;
uint32_t grnKey = 16751205;
uint32_t bluKey = 16753245;

// init for IR
int IrReceiverPin = 12;
IRrecv irrecv(IrReceiverPin);
decode_results results;

// LCD
Waveshare_LCD1602_RGB lcd(16,2);  //16 characters and 2 lines of show
int r,g,b, t = 0;

// LED
const int button = 3;
const int red = 11;
const int green = 10;
const int blue = 9;
int counter = 0;
int buttonState = 0;
int lastButtonState = 0;

String currentColor = "none";

void setup() {
  // IR
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  // colors
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

  Serial.println("Starting IR-receiver...");

  irrecv.enableIRIn();

  Serial.println("IR-receiver active");

  digitalWrite(LED_BUILTIN, LOW);

  // LCD
  lcd.init();
  
  lcd.setCursor(0,0);
  lcd.send_string("Hello");
  lcd.setCursor(0,1);
  lcd.send_string("World");

  // LED
  pinMode(button, INPUT);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (irrecv.decode(&results)){
    Serial.println(results.value);

    if (results.value == grnKey) {
      onGreen();
    }

    irrecv.resume();
  }
  
//  // Delay a little bit to avoid bouncing
//  delay(150);
}

void onGreen() {
  currentColor = "green";
  
  // setting LCD
  lcd.setCursor(0,0);
  lcd.send_string("  Come on in!   ");
  lcd.setCursor(0,1);
  lcd.send_string("  But... knock  ");

  // setting LED
  digitalWrite(green, HIGH);
  digitalWrite(red, LOW);
  digitalWrite(blue, LOW);
  
  r = (0)*255;
  g = (1)*255;
  b = (0)*255;
  lcd.setRGB(r,g,b);
}
