# privacy-light
An Arduino do-not-disturb indicator script. Receives IR input, and prints one of a number of preset displays to an RGB LCD display.

## Hardware & Materials

## Customization
This project is designed to be easily replicated and customized. You can use this code to generate a custom Arduino script that will set the Privacy Light hardware to custom text presets and colors.

Your customization options include:
- Custom text for LCD display
- Custom colors for LCD display
- Custom color settings for indicator LED on the *console unit*
- IR input keys

## Usage
3 files are required:
- `config.json` is where you define your presets and colors to be used.
- `script-template.ino` is a template Arduino script that will be used as the basis for generating your custom script.
- `buildscript.py` uses the configuration you defined to fill the template script and generate a final script which you will upload to your Arduino.

### Configuration
The `config.json` file has four main sections: *files*, *pins*, *colors*, and *presets*.

The *files* object contains only 2 fields: *template* and *output*. They are the filename of the script template file and the desired filename of the generated output file, respectively.

The *pins* object defines what pins on your Arduino board are being used. *ir* is the output pin for the IR receiver, and the *red*, *green*, and *blue* fields are the pin numbers for the R, G, and B connections of the indicator LED.

#### Colors
The *colors* object defines your color settings. You can control both the backlight color on your RGB LCD display and the color of the indicator RGB LED.
Each color key contains an *lcd* object and an *led* object.

*lcd* has the following properties:
- "fade": boolean, if true the LCD will fade between colors instead of using a specified color. **When true, other fields need not be defined.**
- "red": int value for red color, 0 or 1.
- "green": int value for green color, 0 or 1.
- "blue": int value for blue color, 0 or 1.

*led* has the following properties:
- "analog": boolean, if true uses analogWrite - giving more control to brightness of individual colors. If false uses digitalWrite, which is a basic on/off toggle.
- "blink": should the led blink? boolean. (TODO - NOT YET IMPLEMENTED)
- "red": if analog, can specify an int value from 0-255. Else, boolean.
- "green": if analog, can specify an int value from 0-255. Else, boolean.
- "blue": if analog, can specify an int value from 0-255. Else, boolean.

#### Presets
The *presets* object defines your custom presets.

Each preset at the very least has a *default* setting, an *ir_key* setting, a *text* setting and a *color* setting.

*default* is just a boolean representing whether or not the preset should be the default preset on startup.

*ir_key* is the input you receive from the IR remote that you want to correspond to this preset. Basically, this is how you define *which button on your IR remote you want to trigger this preset.*

*text* is an array of 16 character strings. Text is paired in twos - one string is the top line and the other is the bottom line of the 16x2 LCD display we're using. 
You can have any even number of strings in your config. 2, 4, 6, etc. Each pair of strings is one *page*. If a preset has multiple pages of text, it will render the next page after a period of time. This duration is also configurable, using the *scroll-timer* field. Scroll time is defined in milliseconds.

You can set the *color* field to correspond with one of your custom color settings defined in the *colors* object. This defines the LCD color and LED color for your preset.

Other settings include:
- *scroll-timer* (see *text* explanation above)
- *sleep-timeout* will put the display to sleep after a specified time. When set, the display will turn off after the given number of milliseconds.

### Generating the code
The included files give you the necessary tools to generate the script you will upload to your arduino. If your configuration file is correct, and all 3 files (`script-template.ino`, `config.json`, `buildscript.py`) are in the same directory, you should be able to run the following command to generate your `.ino` code:

```python3 buildscript.py```

You should now be able to upload the generated script directly to your arduino.
