#include <Wire.h>
#include "Waveshare_LCD1602_RGB.h"
#include <IRremote.h>

// Privacy Light - A Do-Not-Disturb Indicator

// Define a few colors we can set up as presets
enum BacklightColor { 
  BL_UNDEF,
  BL_RED,
  BL_ORANGE,
  BL_YELLOW,
  BL_GREEN,
  BL_BLUE,
  BL_PURPLE,
  BL_PINK,
  BL_WHITE
};

BacklightColor currentColor = BL_UNDEF;

// Colors mapped to IR codes
//numeric keys are hardcoded expected input from the IR remote I'm using to test (could differ with different remotes possibly)
const uint32_t ylwKey = 16718055;
const uint32_t redKey = 16718565;
const uint32_t grnKey = 16751205;
const uint32_t bluKey = 16753245;
const uint32_t whtKey = 16720605;
const uint32_t pnkKey = 16734375;

// init for IR receiver
int IrReceiverPin = 12;
IRrecv irrecv(IrReceiverPin);
decode_results results;

// LCD
Waveshare_LCD1602_RGB lcd(16,2);  //16 characters and 2 lines of show
int r,g,b, t = 0;

// LED
const int red = 11;
const int green = 10;
const int blue = 9;

// Delays
unsigned long startSleepTimer = 0; // when the timer started
bool awake = false; // if true, display is on and timer until sleep is running

unsigned long fadeDelay = 0; // the time we last updated the backlight
unsigned long scrollDelay = 0; // the time we last updated text

// Text to use for each preset
const char noneText[2][17] = {"     Hello      ", "      World!    "};
const char yellowText[2][17] = {"   On a call.   ", "Or on the phone."};
const char greenText[2][17] = {"  Come on in!   ", "  But... knock  "};
const char blueText[2][17] = {"  Bedtime Boii  ", "   Goodnight!   "};
const char redText[4][17] = {"   Stay Out!!   ", "                ", " Or risk waking ", "    the bear.   "};
const char pinkText[2][17] = {"  Do Not Come   ", "    Inside!!!   "};
const char whiteText[2][17] = {"   Come Party   ", " I'm having fun "};

// Indicates the current page for presets with changing text
int page = 0;

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
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

  // Starting delay
  startSleepTimer = millis(); // start countdown to sleep
  awake = true; // display is on
}

void loop() {
  // put your main code here, to run repeatedly:
  if (irrecv.decode(&results)){
    Serial.println(results.value);
    switch (results.value) {
      case grnKey:
        currentColor = BL_GREEN;
        break;
      case ylwKey:
        currentColor = BL_YELLOW;
        break;
      case redKey:
        currentColor = BL_RED;
        break;
      case pnkKey:
        currentColor = BL_PINK;
        break;
      case bluKey:
        currentColor = BL_BLUE;
        break;
      case whtKey:
        currentColor = BL_WHITE;
        break;
      default:
        irrecv.resume();
        return;
    }

    wake();

    irrecv.resume();
  }
  
  if (awake) {
    if (millis() - startSleepTimer < 20000) {
      // stay awake, re-render
      renderScroll();
      renderFade();
    }
    else {
      sleep();
    }
  }
  
  // just until we hook up sensor
  if (!awake && millis() - startSleepTimer >= 20000) {
    wake();
  }
  
//  // Delay a little bit to avoid bouncing
//  delay(150);
}

void sleep () {
  //  display Off
  lcd.command(LCD_DISPLAYCONTROL | LCD_DISPLAYOFF);
  lcd.setRGB(0, 0, 0);
  
  awake = false;
  
  // just until we hook up sensor
  startSleepTimer = millis();
}

void wake () {
  awake = true;
  startSleepTimer = millis();

  t = 0;
  page = 0;
  setDisplay();
  lcd.display();
}


void setLED (uint8_t r, uint8_t g, uint8_t b) {
  digitalWrite(red, r);
  digitalWrite(green, g);
  digitalWrite(blue, b);
}

void setColors (float pr, float pg, float pb) {
  r = (pr)*255;
  g = (pg)*255;
  b = (pb)*255;

  lcd.setRGB(r,g,b);
}

void setText (char* top, char* bottom) {
  lcd.setCursor(0,0);
  lcd.send_string(top);
  lcd.setCursor(0,1);
  lcd.send_string(bottom);
}

void setDisplay () {
  int index = page * 2;
  
  switch (currentColor) {
    case BL_UNDEF:
    case BL_WHITE:
      // renderFade() sets the RGB backlight, so we don't have to call setColors()
      renderFade();

      setText(whiteText[0], whiteText[1]);
      setLED(HIGH, HIGH, HIGH);
      break;
    case BL_GREEN:
      setColors(0, 1, 0);
      setText(greenText[0], greenText[1]);
      setLED(LOW, HIGH, LOW);
      break;
    case BL_YELLOW:
      setColors(1, 0.3, 0);
      setText(yellowText[0], yellowText[1]);
      setLED(HIGH, HIGH, LOW);
      break;
    case BL_RED:
      setColors(1, 0, 0);
      setText(redText[0], redText[1]);
      setLED(HIGH, LOW, LOW);

      // set the scrollDelay when starting a preset with multiple pages of text
      scrollDelay = millis();
      break;
    case BL_PINK:
      setColors(0.85, 0, 0.5);
      setText(pinkText[0], pinkText[1]);
      setLED(HIGH, LOW, HIGH);
      break;
    case BL_BLUE:
      setColors(0.3, 0, 1);
      setText(blueText[0], blueText[1]);
      setLED(LOW, LOW, HIGH);
      break;
    default:
      return;
  }
}

// unique cases must be made for each color you want to set a scroll for
void renderScroll () {
  //TODO: dynamically tell if a color should scroll without being hardcoded
    // if colorText.size > 2, scroll; if size % 2 == 1, on last page setText(colorText[size], <16 space chars>);
  if (currentColor == BL_RED) {
    int dur = millis() - scrollDelay;

    if (dur <= 6500) {
      page = 0;
    }
    else if (dur > 6500 && dur < 10000) {
      page = 1;
    }
    else {
      page = 0;
      scrollDelay = millis();
    }

    setText(redText[(page * 2)], redText[(page * 2) + 1]);
  }
}

void renderFade () {
  if (currentColor != BL_UNDEF && currentColor != BL_WHITE) {
    return;
  }

  // unsigned subtraction is always positive, so even when millis() reaches its max (4,294,967,295)
  // and resets to 0, (millis() - delayStart) >= 10000) will still always work
  int dur = millis() - fadeDelay;
  
  if (dur >= 100) {
    fadeDelay = millis();
    
    setColors((abs(sin(3.14*t/180))), (abs(sin(3.14*(t + 60)/180))), (abs(sin(3.14*(t + 120)/180))));
    t = t + 3;
  }
}
