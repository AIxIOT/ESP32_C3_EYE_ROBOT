# ESP32-C3 EYE ROBOT

![ESP32-C3 Eye Robot](public/Screenshot%202026-05-06%20211001.png)

A compact, interactive robot based on the ESP32-C3 Super Mini and a 1.3" OLED display. It features expressive eye animations, a smart clock, and a playable mini-game.

---

## 🛠️ Components Used (BOM)

Based on the project assets, the following hardware components are used:

1.  **Microcontroller**: ESP32-C3 Super Mini — **~71 THB**
2.  **Display**: 1.3" OLED Module (SH1106) — **~89 THB**
3.  **Input**: TTP223 Touch Key Switch — **~6 THB**
4.  **Power Management**: 5V 2A Charge/Discharge Module — **~11 THB**
5.  **Battery**: 3.7V 250mAh LiPo Battery (502030) — **~15 THB**
6.  **Power Switch**: SS12F23G4 Slide Switch — **~3 THB**

**Total Estimated Cost: ~195 THB** (Excluding shipping and case)

---

## 🔌 Hardware Connection (Circuit)

The project uses the **ESP32-C3 Super Mini** board connected to an **SH1106 1.3" OLED Display** via I2C and a single physical button for control.

| Component | Pin | ESP32-C3 Pin | Notes |
|-----------|-----|--------------|-------|
| **OLED (I2C)** | VCC | 3.3V | |
| | GND | GND | |
| | SCL | **GPIO 21** | Hardware I2C |
| | SDA | **GPIO 20** | Hardware I2C |
| **Button** | PIN 1 | **GPIO 10** | Configured as `INPUT_PULLUP` |
| | PIN 2 | GND | Connect to Ground when pressed |

---

## 🕹️ Operation Modes

The robot operates in four main modes, cycled via a **Long Press (1 second)**.

### 1. EYE Mode (Default)
The "Living Robot" mode. It features expressive eyes that react dynamically.
- **Scheduled Emotions**: The robot's base emotion changes automatically based on the time of day:
    - **Sleepy 😴**: (00:00 - 07:00) Eyes are closed, accompanied by "z z z" animations.
    - **Hungry 🍔**: (Meal times) Displays a hungry expression at 07:00, 12:00, and 18:00.
    - **Normal 👀**: (Active hours) Eyes look around and blink naturally.
- **Random Mood Swings**: During "Normal" periods, the robot randomly switches to other moods like **Happy**, **Angry**, **Crazy**, or **Laughing** for 10-15 seconds.
- **Manual Override**: A **Short Press** cycles through emotions manually. This override lasts for 10 seconds before returning to automatic mode.

### 2. CLOCK Mode
A functional desk clock with WiFi synchronization.
- **NTP Sync**: Automatically fetches precise time via WiFi (GMT+7).
- **Visuals**: Displays large digital time and WiFi signal status.
- **Extra Info**: A **Short Press** toggles additional information, such as the local IP address.

### 3. STOPWATCH Mode
A precise stopwatch with large digits for easy reading.
- **Visuals**: Displays time in a large `MM:SS` format filling the screen.
- **Control**: **Short Press** to Start / Stop / Reset (Cycle: Idle ➡️ Running ➡️ Stopped ➡️ Idle).

### 4. DINO Mode
A "Chrome Dino" inspired mini-game.
- **Gameplay**: The dinosaur runs continuously; your goal is to jump over obstacles.
- **Control**: **Short Press** to make the Dino jump.
- **High Score**: Keeps track of your progress during the session.

---

## ⌨️ Control Summary

| Action | Result |
|--------|--------|
| **Short Press** | **EYE**: Manual emotion change<br>**CLOCK**: Toggle IP info<br>**STOPWATCH**: Start / Stop / Reset<br>**DINO**: Jump |
| **Long Press (1s)** | Cycle Mode: **EYE** ➡️ **CLOCK** ➡️ **STOPWATCH** ➡️ **DINO** ➡️ (Loop) |

---

## 🚀 Getting Started

1. **PlatformIO**: Open the project in VS Code with PlatformIO installed.
2. **WiFi Setup**: The robot will attempt to connect to WiFi on boot. If it fails (after 10s), it enters **EYE mode** and uses internal timing.
3. **Libraries**: Uses `U8g2` for high-performance graphics on the SH1106 display.

---

## 🛠️ Tech Stack
- **MCU**: ESP32-C3 Super Mini (RISC-V)
- **Display**: 1.3" OLED (SH1106) 128x64
- **Framework**: Arduino / PlatformIO
- **Graphics**: U8g2 (Full Buffer Mode)
