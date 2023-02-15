#include <Wire.h>
#include "Waveshare_LCD1602_RGB.h"
#include <IRremote.h>

// Define a few colors we can set up as presets
enum BacklightColor {
  __enum_colors
};

BacklightColor currentColor = __default_color;

// LCD
Waveshare_LCD1602_RGB lcd(16,2);  //16 characters and 2 lines
int r,g,b, t = 0;

// LED Pins
const int red = __red_led;
const int green = __green_led;
const int blue = __blue_led;

// IR
//// init for IR receiver
IRrecv irrecv(__ir_pin);
decode_results results;

//// Presets mapped to IR codes
const uint32_t powerKey = __power_key;
__ir_keys

// Behavior setting trackers
bool scroller = __default_scroller;
bool sleeper = __default_sleeper;
bool fade = __default_fade;

// Text Presets
// char currentText[__max_text_size][17] = __default_text;
const char helloText[2][17] = {"     Hello      ", "      World!    "};
__text_defs

char const (*currentText)[17] = helloText;

//// Indicates the current page for presets with changing text
int page = 0;
int maxPage = __default_max_page;

// Delays
//// Sleep
bool awake = false; // if true, display is on and timer until sleep is running
unsigned long startSleepTimer = 0; // when the timer started
long timeToSleep = __time_to_sleep; // timeout duration until sleep (1 hour)

//// Color Fade
unsigned long fadeDelay = 0; // the time we last updated the backlight

//// Text Scroll
unsigned long scrollDelay = 0; // the time we last updated text
long scrollTime = __scroll_time;

// SETUP
void setup() {
//  Serial.begin(9600);
  // LED init
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

  // IR init
  irrecv.enableIRIn();

  // Builtin LED
  digitalWrite(LED_BUILTIN, LOW);

  // LCD
  lcd.init();
  //// Init default display
  __init_default_display 

  awake = true; // display is on
}

// STATIC FUNCS

void sleep () {
  //  display Off
  lcd.command(LCD_DISPLAYCONTROL | LCD_DISPLAYOFF);
  lcd.setRGB(0, 0, 0);

  // LED off
  setLED(LOW, LOW, LOW);
  
  awake = false;
}

void wake () {
  awake = true;

  if (sleeper) {
    // start countdown to sleep
    startSleepTimer = millis();
  }

  // Reset counters
  t = 0;
  page = 0;

  // Restart display
  setDisplay();
  lcd.display();
}

void setLED (uint8_t r, uint8_t g, uint8_t b) {
  // Basic LED toggle
  digitalWrite(red, r);
  digitalWrite(green, g);
  digitalWrite(blue, b);
}

void setLEDAnalog (int r, int g, int b) {
  // Set LED values (Analog)
  analogWrite(red, r);
  analogWrite(green, g);
  analogWrite(blue, b);
}

void setColors (float pr, float pg, float pb) {
  // Set LCD color
  r = (pr)*255;
  g = (pg)*255;
  b = (pb)*255;

  lcd.setRGB(r,g,b);
}

void setText (char* top, char* bottom) {
  // Set LCD Text
  lcd.setCursor(0,0);
  lcd.send_string(top);
  lcd.setCursor(0,1);
  lcd.send_string(bottom);
}

void renderScroll () {
  // Scroll to next page of text
  // Duration since page last flipped
  int dur = millis() - scrollDelay;

  if (dur > (scrollTime * (page + 1))) {
    // Flip the page if duration > scrollTime
    page++;

    if (page > maxPage) {
      // Loop back to pg 1 after reaching the end; restart delay
      page = 0;
      scrollDelay = millis();
    }
    
    // Set Text to current page
    setText(currentText[(page * 2)], currentText[(page * 2) + 1]);
  }
}

void renderFade () {
  // unsigned subtraction is always positive, so even when millis() reaches its max (4,294,967,295)
  // and resets to 0, (millis() - delayStart) >= 1000) will still always work
  int dur = millis() - fadeDelay;
  
  if (dur >= 100) {
    fadeDelay = millis();
    
    // Get color values
    setColors((abs(sin(3.14*t/180))), (abs(sin(3.14*(t + 60)/180))), (abs(sin(3.14*(t + 120)/180))));

    // Increase t
    t = t + 3;
  }
}

void setVals (BacklightColor color, int pages, bool scroll, bool sleep, bool cfade) {
  currentColor = color;
  maxPage = pages;
  scroller = scroll;
  sleeper = sleep;
  fade = cfade;
}

// DYNAMICALLY GENERATED FUNCTIONS

void loop() {
  // Main code - Runs repeatedly

  // Decode IR input
  if (irrecv.decode(&results)){
//    Serial.println(results.value);
    switch (results.value) {
__ir_decode
      case powerKey:
        // break to wake & resume ir receiver
        if (!awake) {
          break;
        }
        // sleep, resume, return
        sleep();
        irrecv.resume();
        return;
      default:
        // do nothing, resume ir receiver, return
        irrecv.resume();
        return;
    }

    wake();
    irrecv.resume();
  }
  
  if (awake) {
    if (sleeper && millis() - startSleepTimer > timeToSleep) {
      // If night-mode has been on for [1 hour], sleep
      sleep();
    }
    else {
      // stay awake, re-render
      if (scroller) {
        renderScroll();
      }
      if (fade) {
        renderFade();
      }
    }
  }
}

void setDisplay () {
  switch (currentColor) {
__display_color_cases
    default:
      return;
  }
  
  // Set Text of current Preset
  setText(currentText[0], currentText[1]);
}
