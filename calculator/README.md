# Keypad + LCD Calculator 🔢

This Arduino project turns a **4x4 keypad** and a **16x2 LCD** into a simple calculator.  
It supports addition, subtraction, multiplication, and division.  

---

## 📸 Demo
https://x.com/LukeOlympus/status/1965937779663319122
---

## 🛠 Hardware
- Arduino Uno R3  
- 16x2 LCD (HD44780 compatible)  
- 4x4 Keypad  
- 10k potentiometer (for LCD contrast)  
- 220Ω resistor (for LCD backlight)  
- Breadboard + jumper wires  

---

## 🔌 Wiring
**LCD Connections**  
- RS → D3  
- E  → D4  
- D4 → D2  
- D5 → A0  
- D6 → A1  
- D7 → A2  
- RW → GND  
- VSS → GND, VDD → 5V  
- LED+ (Pin 15) → 5V through 220Ω  
- LED– (Pin 16) → GND  
- VO (Pin 3) → Potentiometer middle pin (pot sides → 5V & GND)  

**Keypad Connections**  
- Keypad pins left→right: D13, D12, D11, D10, D9, D7, D6, D5  

---

## 💻 Code
See [calculator.ino](calculator.ino) for the full Arduino sketch.  

---


