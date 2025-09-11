#include <LiquidCrystal.h>
#include <Keypad.h>

// ---------------- LCD setup ----------------
// RS=3, E=4, D4=2, D5=A0, D6=A1, D7=A2
LiquidCrystal lcd(3, 4, 2, A0, A1, A2);

// ---------------- Keypad setup ----------------
const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','+'},
  {'4','5','6','-'},
  {'7','8','9','*'},
  {'C','0','=','/'}
};

byte rowPins[ROWS] = {13, 12, 11, 10}; 
byte colPins[COLS] = {9, 7, 6, 5};     

Keypad keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------------- Calculator state ----------------
String currentNum = "";
long firstNum = 0;
char op = 0;
bool waitingForSecond = false;

void setup() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Calc Ready");
  delay(1000);
  lcd.clear();
}

void loop() {
  char k = keypad.getKey();
  if (!k) return;

  if (k >= '0' && k <= '9') {
    // digit
    currentNum += k;
    updateLCD();
  } else if (k == '+' || k == '-' || k == '*' || k == '/') {
    // operator
    if (currentNum.length() > 0) {
      firstNum = currentNum.toInt();
      currentNum = "";
      op = k;
      waitingForSecond = true;
      updateLCD();
    }
  } else if (k == '=') {
    // calculate
    if (waitingForSecond && currentNum.length() > 0) {
      long secondNum = currentNum.toInt();
      long result = 0;

      switch (op) {
        case '+': result = firstNum + secondNum; break;
        case '-': result = firstNum - secondNum; break;
        case '*': result = firstNum * secondNum; break;
        case '/': 
          if (secondNum != 0) result = firstNum / secondNum;
          else {
            lcd.clear();
            lcd.print("Error: Div 0");
            delay(1500);
            clearAll();
            return;
          }
          break;
      }

      lcd.clear();
      lcd.print(firstNum);
      lcd.print(op);
      lcd.print(secondNum);
      lcd.setCursor(0,1);
      lcd.print("= ");
      lcd.print(result);

      // reset
      firstNum = result;
      currentNum = "";
      op = 0;
      waitingForSecond = false;
    }
  } else if (k == 'C') {
    // clear
    clearAll();
  }
}

// ---------------- Helpers ----------------
void updateLCD() {
  lcd.clear();
  if (!waitingForSecond) {
    lcd.print(currentNum);
  } else {
    lcd.print(firstNum);
    lcd.print(op);
    lcd.print(currentNum);
  }
}

void clearAll() {
  currentNum = "";
  firstNum = 0;
  op = 0;
  waitingForSecond = false;
  lcd.clear();
}
