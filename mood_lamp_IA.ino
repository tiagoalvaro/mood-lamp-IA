#include "SevSeg.h"
#include <FastLED.h>
#include <Tone.h>

SevSeg sevseg;

unsigned long startTime;
unsigned long stopDisplayTime = 0;
unsigned long blinkStartTime = 0;
bool isBlinkOn = false;
bool isCountdownRunning = true;
int mode = 1;
int buttonPin = 13;
int buttonState = HIGH;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

#define LED_PIN 1
#define NUM_LEDS 26
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
Tone buzzer;

void setup() {
  byte numDigits = 4;
  byte digitPins[] = {2, 3, 4, 5};
  byte segmentPins[] = {6, 7, 8, 9, 10, 11, 12, 13};
  bool resistorsOnSegments = 0;

  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins, resistorsOnSegments);
  sevseg.setBrightness(60);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);

  pinMode(buttonPin, INPUT_PULLUP);

  startTime = millis();

  buzzer.begin(A5);
}

int seconds = 10;
const int countdownDuration = 10 * 1000;
const int stopPhaseDuration = 5 * 1000;
const int buzzerPin = A5;

void loop() {
  long elapsed = elapsedSec();

  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if (millis() - lastDebounceTime > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        // Button is pressed, switch to the next mode
        mode = (mode % 5) + 1;
        restartCycle();
      }
    }
  }

  lastButtonState = reading;

  if (isCountdownRunning) {
    long remainingSec = seconds - elapsed;

    if (remainingSec <= 0) {
      sevseg.setChars("STOP");
      setAllLEDs(CRGB::Blue);
      isBlinkOn = false;
      isCountdownRunning = false;
      stopDisplayTime = millis();
      buzzer.play(1000, 500); // Play a sound with frequency 1000Hz for 500ms
    } else {
      sevseg.setNumber(remainingSec);

      // Start of the 10-second phase, initialize LED colors
      if (remainingSec == seconds) {
        initializeLEDColors();
      }

      // Update the progress bar
      int progress = map(remainingSec, seconds, 0, 0, NUM_LEDS);
      setProgressBar(progress);
    }
  } else {
    if (millis() - stopDisplayTime >= stopPhaseDuration) {
      restartCycle();
      buzzer.play(1000, 500); // Play a sound with frequency 2000Hz for 500ms
    } else {
      if (!isBlinkOn && millis() - blinkStartTime >= 1000) {
        isBlinkOn = true;
        blinkStartTime = millis();
        sevseg.setChars("");
        setAllLEDs(CRGB::Black);
      } else if (isBlinkOn && millis() - blinkStartTime >= 500) {
        isBlinkOn = false;
        blinkStartTime = millis();
        sevseg.setChars("STOP");
        setAllLEDs(CRGB::Blue);
      }
    }
  }

  sevseg.refreshDisplay();
  FastLED.show();
}

unsigned long elapsedSec() {
  return (millis() - startTime) / 1000;
}

void setAllLEDs(const CRGB& color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
}

void setProgressBar(int progress) {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < progress) {
      leds[i] = CRGB::Green; // Lit segments represent progress
    } else {
      leds[i] = CRGB::Black; // Unlit segments represent remaining time
    }
  }
}

void initializeLEDColors() {
  int numColors = 5; // Number of colors in the 10-second phase
  int colorIndex = mode - 1; // Starting color index based on the mode

  // Set the LED colors in a loop
  for (int i = 0; i < NUM_LEDS; i++) {
    // Get the color based on the current color index
    CRGB color;

    switch (colorIndex) {
      case 0:
        color = CRGB::Orange;
        break;
      case 1:
        color = CRGB::Lime;
        break;
      case 2:
        color = CRGB::Red;
        break;
      case 3:
        color = CRGB::Navy;
        break;
      case 4:
        color = CRGB::Indigo;
        break;
      default:
        break;
    }

    setAllLEDs(color);

    // Update the color index for the next LED
    colorIndex = (colorIndex + 1) % numColors;
  }
}

void restartCycle() {
  seconds = 10; // Reset the countdown duration
  isCountdownRunning = true; // Start the countdown
  startTime = millis(); // Reset the start time
}

void stopChargingEffect() {
  setAllLEDs(CRGB::Black);
  FastLED.show();
}
