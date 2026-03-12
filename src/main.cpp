#include <Arduino.h>
#include "CANManager.h"
#include "CANMessages.h"

// ---------------------------------------------------------------------------
// CAN bus hardware pins  (override via build_flags in platformio.ini)
// ---------------------------------------------------------------------------
#ifndef CAN_TX_PIN
#define CAN_TX_PIN GPIO_NUM_5
#endif
#ifndef CAN_RX_PIN
#define CAN_RX_PIN GPIO_NUM_4
#endif

// ---------------------------------------------------------------------------
// Instantiate the manager at 250 kbps (J1939 default).
// Change TWAI_TIMING_CONFIG_250KBITS() to match your bus speed:
//   TWAI_TIMING_CONFIG_125KBITS()
//   TWAI_TIMING_CONFIG_250KBITS()
//   TWAI_TIMING_CONFIG_500KBITS()
//   TWAI_TIMING_CONFIG_1MBITS()
// ---------------------------------------------------------------------------
CANManager canMgr(
    (gpio_num_t)CAN_TX_PIN,
    (gpio_num_t)CAN_RX_PIN,
    TWAI_TIMING_CONFIG_250KBITS()
);

void setup()
{
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    // Register project messages (RX handlers + TX descriptors)
    registerCANMessages(canMgr);

    // Start the TWAI peripheral
    if (!canMgr.begin()) {
        Serial.println("[MAIN] CAN init failed – halting.");
        while (true) { delay(1000); }
    }

    // -----------------------------------------------------------------------
    // Optional: send the TX message once at startup.
    // To send: canMgr.sendTxDescriptor(0);
    //
    // Optional: enable periodic TX (e.g. every 100 ms).
    // txDesc_18EF03F9.intervalMs = 100;
    // -----------------------------------------------------------------------
}

void loop()
{
    // Process incoming frames and fire periodic TX descriptors
    canMgr.update();
}
