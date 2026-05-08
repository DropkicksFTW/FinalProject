//  Button of Doom
//  Author: Michael Santay
//  Date Updated: May 8, 2026
//  Description: A game of chance... pick your button wisely...

// assigns a name to each GPIO pin
#define WINNER 12
#define LOSER 13
#define LED_1 10
#define LED_2 9
#define LED_3 8
#define LED_4 7
#define BUZZER 11
#define SERVO 6

#define B1 5
#define B2 4
#define B3 3
#define B4 2

// creates an array of inputs/outputs for buttons and LEDs
int buttonPins[4] = {B1, B2, B3, B4};
int ledPins[4] = {LED_1, LED_2, LED_3, LED_4};

int activeButtons = 4;
int badButton = 0;
bool goodPressed[4] = {false, false, false, false};

// save states for round changes and output effects
bool effectActive = false;
unsigned long effectStart = 0;
unsigned long effectLength = 1000;
bool effectWasBad = false;

// checks if game should move to round 2
bool pendingStageChange = false; 
bool pendingReset = false;

bool lastStableState[4] = {HIGH, HIGH, HIGH, HIGH};
bool lastReading[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long lastDebounceTime[4] = {0, 0, 0, 0}; //helps keep button tollerence from creating extra unwanted inputs
const unsigned long debounceDelay = 40;

// sets up inputs/outputs and the serial monitor and prepares them for the game
void setup() {
  pinMode(B1, INPUT_PULLUP);
  pinMode(B2, INPUT_PULLUP);
  pinMode(B3, INPUT_PULLUP);
  pinMode(B4, INPUT_PULLUP);

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);

  pinMode(WINNER, OUTPUT);
  pinMode(LOSER, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.begin(9600);
  randomSeed(analogRead(A0));

  digitalWrite(WINNER, LOW);
  digitalWrite(LOSER, LOW);
  noTone(BUZZER);

  resetGame();
}

void loop() {
  // loop that checks the current button values to detect button presses
  updateEffects();
  updateButtonLEDs();

  if (effectActive) {
    return;
  }

  for (int i = 0; i < activeButtons; i++) {
    bool reading = digitalRead(buttonPins[i]);

    if (reading != lastReading[i]) {
      lastDebounceTime[i] = millis();
      lastReading[i] = reading;
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (reading != lastStableState[i]) {
        lastStableState[i] = reading;

        if (lastStableState[i] == LOW) {
          handleButtonPress(i);
          return;
        }
      }
    }
  }
}

// checks whether input pressed was good or bad
void handleButtonPress(int i) {
  if (goodPressed[i]) {
    return;
  }

  Serial.print("Button ");
  Serial.print(i + 1);
  Serial.println(" pressed");

  if (i == badButton) {
    Serial.println("That was the BAD button");
    startBadEffect();

    // sends game to round 2 if bad button is pressed during round 1
    if (activeButtons == 4) {
      pendingStageChange = true;
      pendingReset = false;
    } else {
      pendingStageChange = false;
      pendingReset = true;
    }

  } else {
    Serial.println("That was a GOOD button");
    goodPressed[i] = true;
    updateButtonLEDs();
    startGoodEffect();

    if (allGoodButtonsPressed()) {
      pendingStageChange = false; // prevents round change if all good buttons are pressed
      pendingReset = false;
      effectWasBad = false;
    }
  }
}

// sets game back to the first round with all 4 buttons still active
void resetGame() {
  activeButtons = 4;
  clearGoodPressed();
  badButton = random(activeButtons);
  clearEffects();

  Serial.println("Game reset to round 1");
  Serial.print("Bad button is now: ");
  Serial.println(badButton + 1);

  resetDebounceStates();
  updateButtonLEDs();
}

// starts a new round with the same number of active buttons
void startNewRoundSameStage() {
  clearGoodPressed();
  badButton = random(activeButtons);
  clearEffects();

  Serial.print("New round. Active buttons: ");
  Serial.println(activeButtons);
  Serial.print("Bad button is now: ");
  Serial.println(badButton + 1);

  resetDebounceStates();
  updateButtonLEDs();
}

// starts round 2 with button 4 disabled
void goToStage2() { 
  activeButtons = 3;
  clearGoodPressed();
  badButton = random(activeButtons);
  clearEffects();

  Serial.println("Round 2 started. Button 4 disabled.");
  Serial.print("Bad button is now: ");
  Serial.println(badButton + 1);

  resetDebounceStates();
  updateButtonLEDs();
}

void clearGoodPressed() {
  for (int i = 0; i < 4; i++) {
    goodPressed[i] = false;
  }
}

// Clears all active effects and reset flags
void clearEffects() {
  digitalWrite(WINNER, LOW);
  digitalWrite(LOSER, LOW);
  noTone(BUZZER);

  effectActive = false;
  effectStart = 0;
  effectWasBad = false;
  pendingStageChange = false;
  pendingReset = false;
}

// resets teh debounce for buttons 
void resetDebounceStates() {
  for (int i = 0; i < 4; i++) {
    lastReading[i] = digitalRead(buttonPins[i]);
    lastStableState[i] = lastReading[i];
    lastDebounceTime[i] = 0;
  }
}


// checks button inputs and updates what LEDs are active based on which buttons are pressed
void updateButtonLEDs() {
  for (int i = 0; i < 4; i++) {
    if (i < activeButtons) {
      if (goodPressed[i]) {
        digitalWrite(ledPins[i], LOW);
      } else {
        digitalWrite(ledPins[i], HIGH);
      }
    } else {
      digitalWrite(ledPins[i], LOW);
    }
  }
}

// check if only safe buttons where pressed
bool allGoodButtonsPressed() {
  int count = 0;
  int needed = activeButtons - 1;

  for (int i = 0; i < activeButtons; i++) {
    if (i != badButton && goodPressed[i]) {
      count++;
    }
  }

  return (count == needed);
}

// runs the good effect playing the buzzer and lighting up the winner LED
void startGoodEffect() {
  effectActive = true;
  effectStart = millis();
  effectWasBad = false;

  digitalWrite(WINNER, HIGH);
  digitalWrite(LOSER, LOW);

  tone(BUZZER, 1200);
  delay(100);
  tone(BUZZER, 2000);
  delay(100);
  noTone(BUZZER);

}

// runs the bad effect playing the buzzer and lighting up the loser LED
void startBadEffect() {
  effectActive = true;
  effectStart = millis();
  effectWasBad = true;

  
  digitalWrite(WINNER, LOW);
  digitalWrite(LOSER, HIGH);

  tone(BUZZER, 445); 
  delay(200);
  tone(BUZZER, 415);
  delay(200);
  tone(BUZZER, 392);
  delay(200);
  tone(BUZZER, 310); 
  delay(600);
  noTone(BUZZER);

}

// turns of the buzzer and WINNER/LOSER LEDs after effect ends
void updateEffects() {
  if (!effectActive) {
    return;
  }

  if (millis() - effectStart >= effectLength) {
    digitalWrite(WINNER, LOW);
    digitalWrite(LOSER, LOW);
    noTone(BUZZER);
    effectActive = false;

    if (effectWasBad) {
      if (pendingStageChange) {
        goToStage2();
      } else if (pendingReset) {
        resetGame();
      }

    } else {
      if (allGoodButtonsPressed()) {
        startNewRoundSameStage();
      }
    }
  }
}