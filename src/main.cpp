#include <Arduino.h>
#include "CANManager.h"
#include "CANMessages.h"

// MCP2515 chip-select pin. Change to match your wiring.
#ifndef CAN_CS_PIN
#define CAN_CS_PIN 5
#endif

// CAN_250KBPS = J1939 default. Change if your bus runs at a different speed.
// MCP_8MHZ / MCP_16MHZ – match the crystal on your MCP2515 board.
CANManager canMgr(CAN_CS_PIN, CAN_250KBPS, MCP_8MHZ);

void setup()
{
    Serial.begin(115200);
    SPI.begin();

    registerCANMessages(canMgr);

    if (!canMgr.begin()) {
        Serial.println("[MAIN] CAN init failed – halting.");
        while (true) { delay(1000); }
    }

    // Send the TX message once at startup:
    //   canMgr.sendTxDescriptor(0);
    //
    // Enable periodic TX (e.g. every 100 ms):
    //   txDesc_18EF03F9.intervalMs = 100;
}

void loop()
{
    canMgr.update();
}
