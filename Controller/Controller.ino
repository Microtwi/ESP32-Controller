#include <Arduino.h>
#include <BleGamepad.h>

// At the end, this will output a string in this form, to allow us to use it inside Unity
// MOVE_0_0 CAM_0_0 BTN_0
// OR
// MOVE_3_10 CAM_3_-1 BTN_A

// Each hardware component has a pin associated with it
// Move Joystick Pins
const int MovementVRxPin = 35;
const int MovementVRyPin = 33;
const int CamVRxPin = 26;
const int CamVRyPin = 27;

// Move Joystick Pressed
const int MovementPressedSWPin = 32;
const int CamPressedSWPin = 25;

const int BtnsPin = 34;
const int vibrationPin = 12;
const int ledPin = 14;

// Joystick calibration constants
const int JOYSTICK_MIN = 0;
const int JOYSTICK_MAX = 4095;
const int JOYSTICK_CENTER = 2047;
const int JOYSTICK_DEADZONE = 200;

// Can we use the LEDs in the game, or is it on stand by
bool useLED = false;

// Handle Fade
int brightness = 0;
int fadeAmount = 5;

// BLE Gamepad Config
BleGamepad bleGamepad("Microtwi Controller");
BleGamepadConfiguration bleGamepadConfig;

// This runs on startup / initialization, like Unity's Start function
void setup() {
  Serial.begin(115200);

  // Configure BLE Gamepad
  bleGamepadConfig.setAxesMin(-32768); // Set the minimum for all axes
  bleGamepadConfig.setAxesMax(32767); // Set the maximum for all axes
  bleGamepadConfig.setAutoReport(false); // Disable auto-report for manual control

  // Initialize BLE Gamepad
  bleGamepad.begin(&bleGamepadConfig);

  // Configure pins
  pinMode(vibrationPin, OUTPUT);
  pinMode(MovementVRxPin, INPUT);
  pinMode(MovementVRyPin, INPUT);
  pinMode(MovementPressedSWPin, INPUT_PULLUP);
  pinMode(CamVRxPin, INPUT);
  pinMode(CamVRyPin, INPUT);
  pinMode(CamPressedSWPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  Serial.println("Gamepad ready!");
}

// Simmilar to Unity's Update and Coroutine functions (runs once 150 ms)
void loop() {
  SearchForFunctions();
  Serial.print("MOVE_");
  CalculateJoystickInputs(MovementVRxPin, MovementVRyPin, MovementPressedSWPin, true);

  Serial.print(" CAM_");
  CalculateJoystickInputs(CamVRxPin, CamVRyPin, CamPressedSWPin, false);

  HandleButtons();
  Serial.println("");

  if (bleGamepad.isConnected())
  {
    bleGamepad.sendReport();
  }

  // Trigger fade animation if the led is not used
  if (!useLED)
  {
    FadeLED();
  }

  delay(150);
}

// Handle The Joystick Values Manually
void CalculateJoystickInputs(int VRxPin, int VRyPin, int SWPin, bool isMovement)
{
  // Read joystick
  int XValue = analogRead(VRxPin);
  int YValue = analogRead(VRyPin);
  
  // Read button
  int PressedValue = digitalRead(SWPin);

  if (PressedValue == LOW)
  {
    Serial.print("1");
  }
  else
  {
    Serial.print("0");
  }

  if (bleGamepad.isConnected())
  {
    // Map the joystick values from 0-4095 to -32768 to 32767
    int mappedX = mapJoystickValue(4095 - XValue);
    int mappedY = mapJoystickValue(YValue);

    // Apply dead zone
    // if (abs(mappedX) < JOYSTICK_DEADZONE) {
    //   mappedX = 0;
    // }
    // if (abs(mappedY) < JOYSTICK_DEADZONE) {
    //   mappedY = 0;
    // }

    if (isMovement)
    {
      bleGamepad.setLeftThumb(mappedX, mappedY);
      bleGamepad.sendReport();

      if (PressedValue == LOW) 
      {
        bleGamepad.press(BUTTON_15);
      } 
      else 
      {
        bleGamepad.release(BUTTON_15);
      }
    }
    else
    {
      bleGamepad.setRightThumb(mappedX, mappedY);
      bleGamepad.sendReport();

      if (PressedValue == LOW) 
      {
        bleGamepad.press(BUTTON_16);
      } 
      else 
      {
        bleGamepad.release(BUTTON_16);
      }
    }
  }

  // Directions
  Serial.print("_");
  Serial.print(mapToRange(XValue));
  Serial.print("_");
  Serial.print(mapToRange(YValue));
}

// Normalize joystick values to -1 to 1 range
int mapToRange(int value) {
    return (int)((float)(value - JOYSTICK_CENTER) / -(JOYSTICK_CENTER) * 10);
}

int mapJoystickValue(int value) {
  // Map the joystick's 0-4095 range to -32768 to 32767
  return map(value, JOYSTICK_MIN, JOYSTICK_MAX, -32768, 32767);
}

// Handle button inputs and send to BLE Gamepad
void HandleButtons() {
  int rawValue = analogRead(BtnsPin);
  float voltage = (rawValue / 4095.0) * 3.3;

  Serial.print(" BTN_");

  // Map button presses to gamepad buttons
  if (voltage >= .3 && voltage <= .5)
  {
    Serial.print("Y");
    if (bleGamepad.isConnected())
    {
      bleGamepad.press(BUTTON_5);
    }
  }
  else if (voltage >= .9 && voltage <= 1)
  {
    Serial.print("B");
    if (bleGamepad.isConnected())
    {
      bleGamepad.press(BUTTON_2);
    }
  }
  else if (voltage >= 1.4 && voltage <= 1.6)
  {
    Serial.print("P");
    if (bleGamepad.isConnected())
    {
      bleGamepad.press(BUTTON_12);
    }
  }
  else if (voltage >= 2 && voltage <= 2.1)
  {
    Serial.print("S");
    if (bleGamepad.isConnected())
    {
      bleGamepad.press(BUTTON_11);
    }
  }
  else if (voltage >= 2.6 && voltage <= 2.7)
  {
    Serial.print("A");
    if (bleGamepad.isConnected())
    {
      bleGamepad.press(BUTTON_1);
    }
  }
  else if (voltage >= 3)
  {
    Serial.print("X");
    if (bleGamepad.isConnected())
    {
      bleGamepad.press(BUTTON_4);
    }
  }
  else
  {
    // Release all buttons if no valid input is detected
    Serial.print("NO");
    if (bleGamepad.isConnected())
    {
      bleGamepad.release(BUTTON_1);
      bleGamepad.release(BUTTON_2);
      bleGamepad.release(BUTTON_4);
      bleGamepad.release(BUTTON_5);
      bleGamepad.release(BUTTON_11);
      bleGamepad.release(BUTTON_12);
    }
  }
}

// This function allows us to read data from Unity / The Game
// The string, "data" is the commands that we give on Unity using the Serial Port 
void SearchForFunctions()
{
  if (Serial.available())
  { 
    String data = Serial.readStringUntil('\n'); // Read data until newline character
    data.trim();

    if (data == "LED_ON" && useLED)
    {
      HandleLED(true);
    }

    if (data == "LED_OFF" && useLED)
    {
      HandleLED(false);
    }

    if (data == "VIBRATE_ON")
    {
      HandleVibration(true);
    }

    if (data == "VIBRATE_OFF")
    {
      HandleVibration(false);
    }

    if (data == "DECREASE_FADE")
    {
      if (fadeAmount > 1)
      {
        fadeAmount--;
      }
    }

    if (data == "INCREASE_FADE")
    {
      if (fadeAmount < 25)
      {
        fadeAmount++;
      }
    }

    if (data == "RESET_FADE")
    {
      fadeAmount = 5;
    }

    if (data == "USE_LED")
    {
      useLED = true;
      HandleLED(false);
    }

    if (data == "DONT_USE_LED")
    {
      useLED = false;
    }
  }
}

// Functions To Handle LEDS AND VIBRATIONS
void HandleLED(bool isLEDOn)
{
  if (isLEDOn)
  {
    analogWrite(ledPin, 255); // Full brightness
  }
  else
  {
    analogWrite(ledPin, 0); // LED off
  }
}

void HandleVibration(bool isVribrating)
{
  if (isVribrating)
  {
    digitalWrite(vibrationPin, HIGH);
  }
  else
  {
    digitalWrite(vibrationPin, LOW);
  }
}

bool isFadingUp = true;

// Fade LEDs on stand by
void FadeLED()
{
  if (isFadingUp)
  {
    if (brightness + fadeAmount >= 250)
    {
      isFadingUp = false;
    }
    else
    {
      brightness += fadeAmount;
    }
  }
  else
  {
    if (brightness - fadeAmount <= 5)
    {
      isFadingUp = true;
    }
    else
    {
      brightness -= fadeAmount;
    }
  }

  analogWrite(ledPin, brightness);
}


