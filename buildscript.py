import json
# TODO:
## led blinking
## test scroll/sleep timers (try different values as "default")
## replacement bugs
## cant assign arrs to diff size vars

# config = [json.load(line) for line in open('config.json','r')]
with open('config.json', 'r') as rawConfig:
  config = json.load(rawConfig)

files = config["files"]
pins = config["pins"]
colors = config["colors"]
presets = config["presets"]

defaultColor = ""
defaultText = ""
defaultMaxPage = ""
defaultSleeper = "false"
defaultScroller = "false"
defaultFade = "false"
## TODO: test different values as the defaults here to make sure this works right (according to config)
scrollTime = "5000"
timeToSleep = "3600000"

# Blank line of lcd text
blankLine = "                "

## gets string for setting led of a color
def setLed(color):
  ledSettings = colors[color]["led"]
  if ledSettings["analog"]:
    return "setLEDAnalog(%d, %d, %d);" %(ledSettings["red"], ledSettings["green"], ledSettings["blue"])
  else:
    return "setLED(%d, %d, %d);" %(int(ledSettings["red"]), int(ledSettings["green"]), int(ledSettings["blue"]))

## set pins
irPin = str(pins["ir"])
redPin = str(pins["red"])
greenPin = str(pins["green"])
bluePin = str(pins["blue"])

## gen colors enum
enumColors = ",\n  ".join(colors.keys())

maxTextSize = 0
textDefs = []
irKeys = []
irDecode = []
## gen loop blocks
for key in presets:
  if key == "power":
    continue
  
  preset = presets[key]

  # handle default
  if preset["default"]:
    defaultColor = preset["color"]

    defaultText = preset["text"]
    # If odd number of strings, the last will be omitted. this is because each page must contain exactly 
    ## 2 lines (rows) - any blank lines should be represented as a string of 16 spaces
    # converted to string
    defaultMaxPage = str(int((len(preset["text"]) / 2) - 1))

    # if scroller
    if "scroll-timer" in preset:
      defaultScroller = "true"
      scrollTime = preset["scroll-timer"]

    # if sleeper
    if "sleep-timeout" in preset:
      defaultSleeper = "true"
      timeToSleep = preset["sleep-timeout"]

    # if fade
    if colors[preset["color"]]["lcd"]["fade"]:
      defaultFade = "true"
  
  if "ir_key" in preset and preset["ir_key"] != None:
    maxPage = defaultMaxPage
    currentText = ""
    scroller = "false"
    sleeper = "false"
    fade = "false"

    # define ir key
    irKeys.append("const uint32_t %sKey = %s;" %(key, preset["ir_key"]))
    
    # create case for preset
    irDecodeCase = []
    irDecodeCase.append("      case %sKey:" %key)
    # irDecodeCase.append("currentColor = %s;" %preset["color"])
    currentColor = preset["color"] # todo

    if "text" in preset:
      if len(preset["text"]) > maxTextSize:
        maxTextSize = len(preset["text"])

      # irDecodeCase.append("maxPage = %d;" %(int((len(preset["text"]) / 2) - 1)))
      maxPage = (int((len(preset["text"]) / 2) - 1)) #todo
      irDecodeCase.append("currentText = %sText;" %key)
      # currentText = "%sText" %key #todo

      # define a text constant for this preset
      textDefs.append("const char %sText[%d][17] = {\"%s\"};" %(key, len(preset["text"]), "\", \"".join(preset["text"])))

    # if scroller
    if "scroll-timer" in preset:
      # irDecodeCase.append("scroller = true;")
      scroller = "true" # todo
      irDecodeCase.append("scrollTime = %d;" %preset["scroll-timer"])
    else:
      # irDecodeCase.append("scroller = false;")
      scroller = "false"

    # if sleeper
    if "sleep-timeout" in preset:
      # irDecodeCase.append("sleeper = true;")
      sleeper = "true"
      irDecodeCase.append("timeToSleep = %d;" %preset["sleep-timeout"])
    else:
      # irDecodeCase.append("sleeper = false;")
      sleeper = "false"

    # if fade
    if "fade" in colors[preset["color"]]:
      # irDecodeCase.append("fade = true;")
      fade = "true"
    else:
      # irDecodeCase.append("fade = false;")
      fade = "false"
    
    irDecodeCase.append("setVals(%s);" %(', '.join([currentColor, str(maxPage), scroller, sleeper, fade])))

    # end switch case
    irDecodeCase.append("break;")

    # join lines into a string with proper indentation
    irDecodeCase = '\n        '.join(irDecodeCase)
    # add case to block
    irDecode.append(irDecodeCase)

# Set Default Display
setDefaultColor = ""
if defaultColor == "":
  setDefaultColor = "setLED(HIGH, HIGH, HIGH);"
else:
  setDefaultColor = setLed(defaultColor)

setDefaultText = ""
if len(defaultText) >= 2:
  setDefaultText = "setText(\"" + defaultText[0] + "\", \"" + defaultText[1] + "\");"
else:
  setDefaultText = "setText(\"Hello\", \"World\");"

initDefaultDisplay = "\n  ".join([setDefaultColor, setDefaultText])

powerKey = str(presets["power"]["ir_key"])

# type conversions
irKeys = '\n'.join(irKeys)
irDecode = '\n'.join(irDecode)
textDefs = '\n'.join(textDefs)
maxTextSize = str(maxTextSize)
defaultText = "{\"%s\"}" % ("\", \"".join(defaultText))

#__display_color_cases
## gen setDisplay blocks
displayColorCases = []
for key in colors:
  case = []
  setColor, setLED = "", ""
  color = colors[key]

  if color["lcd"]["fade"]:
    setColor = "renderFade();"
  else:
    setColor = "setColors(%s, %s, %s);" %(color["lcd"]["red"], color["lcd"]["green"], color["lcd"]["blue"])
    setColor = "fade = false;\n      " + setColor

  setLED = setLed(key)

  case = '\n      '.join(["    case %s:" %key, setColor, setLED, "break;"])
  displayColorCases.append(case)
# Type conversion
displayColorCases = '\n'.join(displayColorCases)

## Fill Template
template = "script-template.cpp"
if "template" in files:
  template = files["template"]

output = "privacylight_generated.ino"
if "output" in files:
  output = files["output"]

with open(template, "r") as file:
  filedata = file.read()

  # Replace the target string
  filedata = filedata.replace("__enum_colors", enumColors)#SUCCESS
  filedata = filedata.replace("__default_color", defaultColor)#SUCCESS
  filedata = filedata.replace("__max_text_size", maxTextSize)#SUCCESS
  filedata = filedata.replace("__default_text", defaultText)#SUCCESS
  filedata = filedata.replace("__ir_pin", irPin)#SUCCESS
  filedata = filedata.replace("__ir_keys", irKeys)#SUCCESS
  filedata = filedata.replace("__red_led", redPin)#SUCCESS
  filedata = filedata.replace("__green_led", greenPin)#SUCCESS
  filedata = filedata.replace("__blue_led", bluePin)#SUCCESS
  filedata = filedata.replace("__time_to_sleep", timeToSleep)
  filedata = filedata.replace("__scroll_time", scrollTime)
  filedata = filedata.replace("__text_defs", textDefs)#SUCCESS
  filedata = filedata.replace("__default_max_page", defaultMaxPage)#Seemingly works, test with "Red" as default
  filedata = filedata.replace("__default_scroller", defaultScroller)#SUCCESS
  filedata = filedata.replace("__default_sleeper", defaultSleeper)#SUCCESS
  filedata = filedata.replace("__default_fade", defaultFade)#SUCCESS
  filedata = filedata.replace("__init_default_display", initDefaultDisplay)#SUCCESS
  filedata = filedata.replace("__ir_decode", irDecode)#SUCCESS
  filedata = filedata.replace("__power_key", powerKey)#SUCCESS
  filedata = filedata.replace("__display_color_cases", displayColorCases)#SUCCESS

  # Write the file out again
  with open(output, "w") as file:
      file.write(filedata)


"""
glossary of template placeholders -

__enum_colors: color enum contents
__default_color: BacklightColor enum val of default preset color
__max_text_size: max num of text lines across all presets
__default_text: default preset text
__ir_pin: pin for ir receiver
__ir_keys: uint32_t mappings for preset ir keys
__red_led: pin num for red led
__green_led: pin num for green led
__blue_led: pin num for blue led
__time_to_sleep: (default) value for time until sleep
__scroll_time: (default) time to wait before scrolling to next page
__text_defs: text for each preset
__default_max_page: default max pages
__default_scroller: if default preset has scroll
__default_sleeper: if default preset sleeps
__default_fade: if default preset has fade
__init_default_display: setup() code block for init display based on default preset
__ir_decode: loop() switch cases for ir keys of presets
__power_key: ir key for "power" preset
__display_color_cases: setDisplay() switch statement color cases

"""
