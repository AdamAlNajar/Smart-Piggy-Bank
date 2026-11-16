# Smart Piggy Bank

## 📝 Project Overview
A smart, multi-currency storage system designed to accept and count multiple fiat currencies (AED, BYN, JOD). The system solves the problem of managing mixed currencies across different countries. It features custom hardware, a custom 3D-printed enclosure, and remote access via a mobile app.

**Current Status:** Phase 1 (Core Coin System) — In Progress.

## 🚀 Features
- **Multi-Currency Coin Acceptance:** Programmable coin acceptor learns up to 6 different coin types (perfect for AED, BYN, JOD).
- **Smart Tracking:** Real-time balance updates via OLED display.
- **Secure Access:** Unlocks a compartment using a physical keypad (PIN) or an IoT mobile app (Blynk).
- **Custom PCB:** Designed from scratch in KiCad for a compact, reliable system.
- **Custom Enclosure:** 3D-printed housing designed to hold the electronics and cash securely.
- **Upgrade Path:** Bill acceptor integration planned for Phase 2.

## 🧠 Hardware Architecture (Phase 1)
- **Microcontroller:** ESP32-S3 (N16R8) — handles Wi-Fi, UART, and GPIO logic.
- **Coin Input:** CH-926 Programmable Multi-Coin Acceptor (12V).
- **User Interface:** 4x4 Matrix Keypad + 0.96" I2C OLED Display.
- **Security:** 12V Solenoid Lock controlled via a 5V Relay Module.
- **Power:** 12V DC input via USB-C PD Trigger Board, stepped down to 5V using an LM2596 Buck Converter.

## 💻 Software Architecture
- **Language:** C++ (Arduino Framework)
- **Libraries:** `WiFi.h`, `BlynkSimpleEsp32.h`, `Wire.h` (I2C), `Keypad.h`, hardware serial for the coin acceptor.
- **Logic:** State machine for handling user input (IDLE -> AUTHENTICATING -> DISPENSING).

## 📂 Repository Structure
- `/hardware` : KiCad schematic files and Gerber files for JLCPCB manufacturing.
- `/firmware` : Arduino C++ code for the ESP32.
- `/3d_models` : STL files for the 3D-printed enclosure.
- `/docs` : Project images, wiring diagrams, and this README.

## 🛒 Bill of Materials & Budget (Phase 1)
All prices are approximate AliExpress/Amazon.ae prices in AED. **Total = ~95 AED.**

| Component | Search Term / Link | Qty | Est. Price (AED) |
| :--- | :--- | :--- | :--- |
| **ESP32-S3 (N16R8)** | [AliExpress Link](https://www.aliexpress.com/item/1005005955555555.html) | 1 | 16.00 |
| **CH-926 Coin Acceptor** | [AliExpress Link](https://www.aliexpress.com/item/1005006000000000.html) | 1 | 25.00 |
| **USB-C PD Trigger (12V)** | [AliExpress Link](https://www.aliexpress.com/item/1005007000000000.html) | 1 | 8.00 |
| **LM2596 Buck Converter** | [AliExpress Link](https://www.aliexpress.com/item/1005008000000000.html) | 1 | 7.00 |
| **5V Relay Module (1-Ch)** | [AliExpress Link](https://www.aliexpress.com/item/1005009000000000.html) | 1 | 8.00 |
| **12V Solenoid Lock** | [AliExpress Link](https://www.aliexpress.com/item/1005010000000000.html) | 1 | 15.00 |
| **0.96" I2C OLED** | [AliExpress Link](https://www.aliexpress.com/item/1005011000000000.html) | 1 | 10.00 |
| **4x4 Matrix Keypad** | [AliExpress Link](https://www.aliexpress.com/item/1005012000000000.html) | 1 | 4.00 |
| **Jumper Wires & Breadboard** | [AliExpress Link](https://www.aliexpress.com/item/1005013000000000.html) | 1 | 2.00 |
| **TOTAL** | | | **~95.00** |

> **Note:** The **Bill Acceptor (Banknote Validator)** is intentionally excluded from the 100 AED budget. Reliable units (like TB74 or NV10) cost 50–80 USD minimum. The system is designed to be fully functional for coins first; the bill acceptor will be added in Phase 2 once the core system is validated and budget allows.

## 🗺️ Project Roadmap & Milestones
- [ ] **Phase 1: Breadboard Prototype**
  - Test ESP32 with OLED, Keypad, and Relay.
  - Test CH-926 coin acceptor and calibrate for AED/BYN/JOD coins.
- [ ] **Phase 2: PCB Design & Manufacture**
  - Draw schematic in KiCad.
  - Route PCB, export Gerbers, order from JLCPCB.
- [ ] **Phase 3: Mechanical Design**
  - Design enclosure in CAD.
  - 3D print and test fit components.
- [ ] **Phase 4: System Integration & App**
  - Solder components to PCB.
  - Finalize Blynk IoT app integration.
  - Full system test.
- [ ] **Phase 5: Bill Acceptor Integration (Stretch Goal)**
