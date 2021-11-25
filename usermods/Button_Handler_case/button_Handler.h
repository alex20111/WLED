#pragma once

#include "wled.h"
// #include "/nodemcuv2/JC_BUTTON/src/JC_BUTTON.h"

/*

 */

//class name. Use something descriptive and leave the ": public Usermod" part :)

class ButtonHandlerCase : public Usermod
{
private:
  const byte BTN1_PIN = 14;  //D5
  const byte BTN2_PIN = 13;  //D7
  const byte LED_SLEEP = 15; //D8
  const byte LED_WIFI = 16;  //D0

  static const char _name[];
  static const char _enbWifiShutdown[];
  static const char _wifi_sleep[];
  static const char _time_to_sleep[];
  static const char _last_preset[];
  static const char _preset1[];
  static const char _preset2[];
  static const char _preset3[];
  static const char _preset4[];
  static const char _preset5[];

  //button variables
  bool button1Pressed = false;
  unsigned long button1Delay = 0;
  int btn1State = 1; // since it's supposed to be high when starting
  int lastButton1State = 1;
  unsigned long btn1LastDebounceTime = 0; // the last time the output pin was toggled

  bool button2Pressed = false;
  unsigned long button2Delay = 0;
  int btn2State = 1;
  int lastButton2State = 1;
  unsigned long btn2LastDebounceTime = 0; // the last time the output pin was toggled

  unsigned long debounceDelay = 50; // the debounce time; increase if the output flickers

  //sleep variables
  int timeToSleep = 10; //time to sleep In minutes
  int lstTimeToSleep = 10;
  bool sleepEnabled = false;
  bool sleeping = false;
  unsigned long sleepStartCountdown = 0;

  //Wifi sleep variables
  int wifiShutdown = 1;        //Time to wifi to shutdown in minutes.
  int lstWifiShutdownTime = 1; // same as wifiShutdown.. to verify if variable has changed.
  bool wifiShutdownEnabled = true;
  bool wifiAsleep = false;
  unsigned long wifiSleepStartCountdown = 0;

  // //Private class members. You can declare variables and functions only accessible to your usermod here
  unsigned long lastTime = 0;

  // Preset selection
  static const int8_t nbrOfPreset = 5;
  int8_t presetIdx = 0;
  int8_t presets[nbrOfPreset];
  int8_t lastSelectedPreset = -1;

public:
  //Functions called by WLED

  /*
     * setup() is called once at boot. WiFi is not yet connected at this point.
     * You can use it to initialize variables, sensors or similar.
     */
  void setup()
  {
    pinManager.allocatePin(BTN1_PIN, false, PinOwner::UM_BTN_H);  //no
    pinManager.allocatePin(BTN2_PIN, false, PinOwner::UM_BTN_H);  //yes
    pinManager.allocatePin(LED_SLEEP, false, PinOwner::UM_BTN_H); //no
    pinManager.allocatePin(LED_WIFI, false, PinOwner::UM_BTN_H);  //yes

    pinMode(LED_SLEEP, OUTPUT);
    pinMode(LED_WIFI, OUTPUT);
    pinMode(BTN1_PIN, INPUT);
    pinMode(BTN2_PIN, INPUT);

    if (lastSelectedPreset > -1)
    {
      presetIdx = lastSelectedPreset;
      // System.out.println("Starting with preset: " + presets[presetIdx] );
      presetIdx++;
    }

    wifiSleepStartCountdown = millis();
    lstWifiShutdownTime = wifiShutdown; //in case user change it in the settings
    lstTimeToSleep = timeToSleep;
  }

  /*
     * connected() is called every time the WiFi is (re)connected
     * Use it to initialize network interfaces
     */
  void connected()
  {
    Serial.println(F("Connected to WiFi!"));
    digitalWrite(LED_WIFI, HIGH);
  }

  /*
     * loop() is called continuously. Here you can check for events, read sensors, etc.
     * 
     * Tips:
     * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
     *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
     * 
     * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
     *    Instead, use a timer check as shown here.
     */
  void loop()
  {
    handleButtons();

    handleWifi();


    if (lstTimeToSleep != timeToSleep){
      Serial.println(F("Reset TIME TO SLEEP "));
      sleepStartCountdown = millis();
      lstTimeToSleep = timeToSleep;
    }
    //timer to turn off the leds
    if (sleepEnabled && !sleeping && (millis() - sleepStartCountdown > (timeToSleep * 1000 * 60)))
    {
      Serial.println(F("!!Sleeping"));
      sleeping = true;
      toggleOnOffLeds(false); //toggle leds off
      digitalWrite(LED_SLEEP, LOW);
    }

    debug();
  }

  /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
  /*
    void addToJsonInfo(JsonObject& root)
    {
      int reading = 20;
      //this code adds "u":{"Light":[20," lux"]} to the info object
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");

      JsonArray lightArr = user.createNestedArray("Light"); //name
      lightArr.add(reading); //value
      lightArr.add(" lux"); //unit
    }
    */

  /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
  // void addToJsonState(JsonObject &root)
  // {
  //   //root["user0"] = userVar0;
  // }

  // /*
  //    * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
  //    * Values in the state object may be modified by connected clients
  //    */
  // void readFromJsonState(JsonObject &root)
  // {
  //   // userVar0 = root["user0"] | userVar0; //if "user0" key exists in JSON, update, else keep old value
  //   //if (root["bri"] == 255) Serial.println(F("Don't burn down your garage!"));
  // }

  /*
     * addToConfig() can be used to add custom persistent settings to the cfg.json file in the "um" (usermod) object.
     * It will be called by WLED when settings are actually saved (for example, LED settings are saved)
     * If you want to force saving the current state, use serializeConfig() in your loop().
     * 
     * CAUTION: serializeConfig() will initiate a filesystem write operation.
     * It might cause the LEDs to stutter and will cause flash wear if called too often.
     * Use it sparingly and always in the loop, never in network callbacks!
     * 
     * addToConfig() will make your settings editable through the Usermod Settings page automatically.
     *
     * Usermod Settings Overview:
     * - Numeric values are treated as floats in the browser.
     *   - If the numeric value entered into the browser contains a decimal point, it will be parsed as a C float
     *     before being returned to the Usermod.  The float data type has only 6-7 decimal digits of precision, and
     *     doubles are not supported, numbers will be rounded to the nearest float value when being parsed.
     *     The range accepted by the input field is +/- 1.175494351e-38 to +/- 3.402823466e+38.
     *   - If the numeric value entered into the browser doesn't contain a decimal point, it will be parsed as a
     *     C int32_t (range: -2147483648 to 2147483647) before being returned to the usermod.
     *     Overflows or underflows are truncated to the max/min value for an int32_t, and again truncated to the type
     *     used in the Usermod when reading the value from ArduinoJson.
     * - Pin values can be treated differently from an integer value by using the key name "pin"
     *   - "pin" can contain a single or array of integer values
     *   - On the Usermod Settings page there is simple checking for pin conflicts and warnings for special pins
     *     - Red color indicates a conflict.  Yellow color indicates a pin with a warning (e.g. an input-only pin)
     *   - Tip: use int8_t to store the pin value in the Usermod, so a -1 value (pin not set) can be used
     *
     * See usermod_v2_auto_save.h for an example that saves Flash space by reusing ArduinoJson key name strings
     * 
     * If you need a dedicated settings page with custom layout for your Usermod, that takes a lot more work.  
     * You will have to add the setting to the HTML, xml.cpp and set.cpp manually.
     * See the WLED Soundreactive fork (code and wiki) for reference.  https://github.com/atuline/WLED
     * 
     * I highly recommend checking out the basics of ArduinoJson serialization and deserialization in order to use custom settings!
     */
  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top[FPSTR(_enbWifiShutdown)] = wifiShutdownEnabled;
    top[FPSTR(_wifi_sleep)] = wifiShutdown;   //save these vars persistently whenever settings are saved
    top[FPSTR(_time_to_sleep)] = timeToSleep; //save these vars persistently whenever settings are saved
    top[FPSTR(_last_preset)] = lastSelectedPreset;
    top[FPSTR(_preset1)] = presets[0];
    top[FPSTR(_preset2)] = presets[1];
    top[FPSTR(_preset3)] = presets[2];
    top[FPSTR(_preset4)] = presets[3];
    top[FPSTR(_preset5)] = presets[4];
  }

  /*
     * readFromConfig() can be used to read back the custom settings you added with addToConfig().
     * This is called by WLED when settings are loaded (currently this only happens immediately after boot, or after saving on the Usermod Settings page)
     * 
     * readFromConfig() is called BEFORE setup(). This means you can use your persistent values in setup() (e.g. pin assignments, buffer sizes),
     * but also that if you want to write persistent values to a dynamic buffer, you'd need to allocate it here instead of in setup.
     * If you don't know what that is, don't fret. It most likely doesn't affect your use case :)
     * 
     * Return true in case the config values returned from Usermod Settings were complete, or false if you'd like WLED to save your defaults to disk (so any missing values are editable in Usermod Settings)
     * 
     * getJsonValue() returns false if the value is missing, or copies the value into the variable provided and returns true if the value is present
     * The configComplete variable is true only if the "exampleUsermod" object and all values are present.  If any values are missing, WLED will know to call addToConfig() to save them
     * 
     * This function is guaranteed to be called on boot, but could also be called every time settings are updated
     */
  bool readFromConfig(JsonObject &root)
  {
    // default settings values could be set here (or below using the 3-argument getJsonValue()) instead of in the class definition or constructor
    // setting them inside readFromConfig() is slightly more robust, handling the rare but plausible use case of single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)

    JsonObject top = root[FPSTR(_name)];

    bool configComplete = !top.isNull();


    configComplete &= getJsonValue(top[FPSTR(_enbWifiShutdown)], wifiShutdownEnabled, true);
    configComplete &= getJsonValue(top[FPSTR(_wifi_sleep)], wifiShutdown, 10);
    configComplete &= getJsonValue(top[FPSTR(_time_to_sleep)], timeToSleep); //top["preset1"] = presets[0];
    configComplete &= getJsonValue(top[FPSTR(_last_preset)], lastSelectedPreset, -1);
    configComplete &= getJsonValue(top[FPSTR(_preset1)], presets[0], 0);
    configComplete &= getJsonValue(top[FPSTR(_preset2)], presets[1], 0);
    configComplete &= getJsonValue(top[FPSTR(_preset3)], presets[2], 0);
    configComplete &= getJsonValue(top[FPSTR(_preset4)], presets[3], 0);
    configComplete &= getJsonValue(top[FPSTR(_preset5)], presets[4], 0);

    return configComplete;
  }

  /*
     * getId() allows you to optionally give your V2 usermod an unique ID (please define it in const.h!).
     * This could be used in the future for the system to determine whether your usermod is installed.
     */
  uint16_t getId()
  {
    return 912;
  }

  /* handle the button functions
  */
  void handleButtons()
  {
    readButtons();

    if (btn1State == LOW && !button1Pressed)
    {
      // Serial.println("Button 1 pressed ");
      // digitalWrite(LED_SLEEP, HIGH);
      button1Pressed = true;
      presetChooser();
    }
    else if (btn1State == HIGH && button1Pressed)
    {
      button1Pressed = false;
    }

    if (btn2State == LOW && !button2Pressed)
    {
      Serial.println(F("Button 2 pressed - sleep"));
      button2Pressed = true;
      if (sleepEnabled) //if sleep is enabled
      {
        sleepEnabled = false; //turn off sleep and reset variables.

        digitalWrite(LED_SLEEP, LOW);
        if (sleeping)
        {
          toggleOnOffLeds(true); //turn back on the lights if it was sleeping
          sleeping = false;
        }
      }
      else
      {
        sleepEnabled = true;
        sleepStartCountdown = millis();
        digitalWrite(LED_SLEEP, HIGH);
      }
    }
    else if (btn2State == HIGH && button2Pressed)
    {
      button2Pressed = false;
    }
  }

  /*
    Function to read the button and add debounce on it.
   */
  void readButtons()
  {

    //sample the state of the button - is it pressed or not?
    int btn1Value = digitalRead(BTN1_PIN);
    int btn2Value = digitalRead(BTN2_PIN);

    if (btn1Value != lastButton1State)
    {
      // reset the debouncing timer
      btn1LastDebounceTime = millis();
    }

    if (btn2Value != lastButton2State)
    {
      // reset the debouncing timer
      btn2LastDebounceTime = millis();
    }

    //filter out any noise by setting a time buffer
    if ((millis() - btn1LastDebounceTime) > debounceDelay)
    {
      btn1State = btn1Value;
    }

    if ((millis() - btn2LastDebounceTime) > debounceDelay)
    {
      btn2State = btn2Value;
    }

    lastButton1State = btn1Value;
    lastButton2State = btn2Value;
  }

  /*
  Select and increment presets
 */
  void presetChooser()
  {
    //basically if preset is 0
    Serial.print(F("Preset Idx: "));
    Serial.println(presetIdx);

    for (int i = 0; i < nbrOfPreset; i++)
    {

      if (presets[presetIdx] > 0)
      {
        Serial.print(F("--- Preset: "));
        Serial.println(presets[presetIdx]);
        applyPreset(presets[presetIdx]);
        // System.out.println("-- Preset: " + presets[i]);
        lastSelectedPreset = presetIdx;
        presetIdx++;
        break;
      }
      presetIdx++;

      if (presetIdx == nbrOfPreset)
      {
        presetIdx = 0;
      }
    }
  }
  /*
  */
  void handleWifi()
  {

    if (lstWifiShutdownTime != wifiShutdown)
    {
      //reset wifiSleepStartCountdown since time has changed.
      Serial.println(F("!!!!reset wifi time shutdown"));
      wifiSleepStartCountdown = millis();
      lstWifiShutdownTime = wifiShutdown;
    }

    //timer to shutdown the wifi
    if (wifiShutdownEnabled && !wifiAsleep &&
        (millis() - wifiSleepStartCountdown > (wifiShutdown * 1000 * 60)))
    {
      Serial.println(F("sleep wifi"));
      wifiAsleep = true;
      WiFi.mode(WIFI_OFF);
      WiFi.forceSleepBegin();
      digitalWrite(LED_WIFI, LOW);
    }

    //verify if WIFI is active
    if (WiFi.getMode() > 0 && digitalRead(LED_WIFI) == LOW) // greater than 0 means an active wifi
    {
      digitalWrite(LED_WIFI, HIGH);
    }
    else
    {
      digitalWrite(LED_WIFI, LOW);
    }
  }

  void toggleOnOffLeds(bool on)
  {
    if (on)
    {
      bri = briLast;
      colorUpdated(2);
    }
    else
    {
      bri = 0;
      colorUpdated(2);
    }
  }

  void debug()
  {
    if (millis() - lastTime > 5000)
    {
      Serial.println(F("---Debug---"));
      Serial.print(F("presetIdx: "));
      Serial.println(presetIdx);
      Serial.print(F("Sleep Enabled: "));
      Serial.println(sleepEnabled);
      Serial.print(F("timeToSleep: "));
      Serial.println(timeToSleep);
      Serial.print(F("wifiShutdown: "));
      Serial.println(wifiShutdown);
      Serial.print(F("Wifi Mode: "));
      Serial.println(WiFi.getMode());
      Serial.print(F("Wifi sleep Mode: "));
      Serial.println(WiFi.getSleepMode());
      Serial.print(F("Wifi Status: "));
      Serial.println(WiFi.status());
      Serial.print(F("Wifi softAPIP: "));
      Serial.println(WiFi.softAPIP());
      Serial.println(F("---END---"));
      lastTime = millis();
    }
  }
};

const char ButtonHandlerCase::_name[] PROGMEM = "ButtonHandlerCase";
const char ButtonHandlerCase::_enbWifiShutdown[] PROGMEM = "Enable_wifi_shutdown";
const char ButtonHandlerCase::_wifi_sleep[] PROGMEM = "wifiSleep";
const char ButtonHandlerCase::_time_to_sleep[] PROGMEM = "timeToSleep";
const char ButtonHandlerCase::_last_preset[] PROGMEM = "lastSelectedPreset";
const char ButtonHandlerCase::_preset1[] PROGMEM = "preset1";
const char ButtonHandlerCase::_preset2[] PROGMEM = "preset2";
const char ButtonHandlerCase::_preset3[] PROGMEM = "preset3";
const char ButtonHandlerCase::_preset4[] PROGMEM = "preset4";
const char ButtonHandlerCase::_preset5[] PROGMEM = "preset5";