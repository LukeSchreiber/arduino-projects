#include <LiquidCrystal.h>

// LCD pins: RS=12, E=11, D4=5, D5=4, D6=3, D7=6
LiquidCrystal lcd(12, 11, 5, 4, 3, 6);

const int LED_PIN = 8;
const int BTN_PIN = 2;   // button to GND, INPUT_PULLUP

const unsigned long DEBOUNCE_MS     = 15;
const unsigned long START_WINDOW_MS = 2000;   // gap allowed between triple-start presses
const unsigned long MIN_WAIT_MS     = 1500;   // random pre-signal delay
const unsigned long EXTRA_WAIT_MS   = 2500;   // total wait = 1.5..4.0 s
const unsigned long COOLDOWN_MS     = 800;    // LED off between trials

enum State { IDLE, ARMING, PRE_WAIT, SIGNAL_ON, INTER_COOLDOWN, ROUND_DONE };
State state = IDLE;

// debounce + edge detect
bool stable = true;  // HIGH initially (pull-up)
bool lastStable = true;
unsigned long lastChangeMs = 0;
bool pressedEvent = false; // HIGH->LOW one-shot

// triple-press start
int startPressCount = 0;
unsigned long startWindowBegin = 0;

// trials (per round)
int trial = 0;  // 0..2
unsigned long t_signal = 0;
unsigned long times_ms[3] = {0,0,0};
unsigned long bestRound = 999999;  // resets each round

unsigned long waitStart = 0;
unsigned long waitFor  = 0;

void lcd2(const char* l1, const char* l2 = "") {
  lcd.clear(); lcd.setCursor(0,0); lcd.print(l1);
  lcd.setCursor(0,1); lcd.print(l2);
}

void resetRoundStats() {
  trial = 0;
  times_ms[0] = times_ms[1] = times_ms[2] = 0;
  bestRound = 999999;
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW);

  lcd.begin(16, 2);
  lcd2("Reaction Timer", "Triple-press to go");

  Serial.begin(9600);
  randomSeed(analogRead(A0));
}

void loop() {
  unsigned long now = millis();

  // debounce + edge
  bool raw = digitalRead(BTN_PIN); // HIGH=not pressed, LOW=pressed
  if (raw != stable && (now - lastChangeMs) > DEBOUNCE_MS) {
    stable = raw;
    lastChangeMs = now;
  }
  pressedEvent = (lastStable == HIGH && stable == LOW);
  lastStable = stable;

  switch (state) {
    case IDLE:
      if (pressedEvent) {
        startPressCount = 1; startWindowBegin = now;
        lcd2("Start: 1/3", "Press quickly");
        state = ARMING;
      }
      break;

    case ARMING:
      if (now - startWindowBegin > START_WINDOW_MS) {
        lcd2("Start timed out", "Triple-press again");
        state = IDLE; break;
      }
      if (pressedEvent) {
        startPressCount++; startWindowBegin = now;
        char l1[17]; snprintf(l1, sizeof(l1), "Start: %d/3", startPressCount);
        lcd2(l1, "Press quickly");
        if (startPressCount >= 3) {
          // NEW: reset per-round stats here
          resetRoundStats();
          lcd2("Round started!", "Get ready...");
          beginPreWait();
          state = PRE_WAIT;
        }
      }
      break;

    case PRE_WAIT:
      // false start: press before LED -> restart random wait (same trial)
      if (pressedEvent) {
        lcd2("False start!", "Wait for LED");
        beginPreWait();
      }
      if (now - waitStart >= waitFor) {
        digitalWrite(LED_PIN, HIGH);
        t_signal = now;
        char l1[17]; snprintf(l1, sizeof(l1), "Trial %d: GO!", trial + 1);
        lcd2(l1, "Press ASAP");
        state = SIGNAL_ON;
      }
      break;

    case SIGNAL_ON:
      if (pressedEvent) {
        unsigned long rt = now - t_signal;
        times_ms[trial] = rt;
        if (rt < bestRound) bestRound = rt;

        digitalWrite(LED_PIN, LOW);

        char l1[17], l2[17];
        snprintf(l1, sizeof(l1), "Trial %d result", trial + 1);
        snprintf(l2, sizeof(l2), "%lu ms", rt);
        lcd2(l1, l2);

        beginCooldown();
        state = INTER_COOLDOWN;
      }
      break;

    case INTER_COOLDOWN:
      if (now - waitStart >= waitFor) {
        trial++;
        if (trial < 3) {
          lcd2("Get ready...", "Wait for LED");
          beginPreWait();
          state = PRE_WAIT;
        } else {
          showRoundSummary();  // uses bestRound + per-round avg
          state = ROUND_DONE;
        }
      }
      break;

    case ROUND_DONE:
      if (pressedEvent) {
        startPressCount = 1; startWindowBegin = now;
        lcd2("Start: 1/3", "Press quickly");
        state = ARMING;
      }
      break;
  }
}

// helpers
void beginPreWait() {
  waitStart = millis();
  waitFor  = MIN_WAIT_MS + (unsigned long)random(0, EXTRA_WAIT_MS + 1);
}

void beginCooldown() {
  waitStart = millis();
  waitFor = COOLDOWN_MS;
}

void showRoundSummary() {
  unsigned long sum = times_ms[0] + times_ms[1] + times_ms[2];
  unsigned long avg = sum / 3;

  char l1[17], l2[17];
  snprintf(l1, sizeof(l1), "Avg: %lu ms", avg);
  snprintf(l2, sizeof(l2), "Best: %lu ms", bestRound);
  lcd2(l1, l2);

  Serial.println("\n=== Round Summary ===");
  Serial.print("T1: "); Serial.print(times_ms[0]); Serial.println(" ms");
  Serial.print("T2: "); Serial.print(times_ms[1]); Serial.println(" ms");
  Serial.print("T3: "); Serial.print(times_ms[2]); Serial.println(" ms");
  Serial.print("Avg (round): "); Serial.print(avg); Serial.println(" ms");
  Serial.print("Best (round): "); Serial.print(bestRound); Serial.println(" ms");
  Serial.println("=====================");
  Serial.println("\nTriple-press to start a new round.");
}