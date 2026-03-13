#include "CANMessages.h"

// ---------------------------------------------------------------------------
// CANMessages.cpp
// Project-specific CAN message definitions, callbacks, and registration.
// ---------------------------------------------------------------------------

// ===========================================================================
// RX  –  0x18EFF903, extended frame, command byte 0xAD
// ===========================================================================

static void onRx_18EFF903_AD(uint32_t id, const uint8_t* data, uint8_t len)
{
    // -----------------------------------------------------------------------
    // TODO: replace this with your application logic.
    // data[0] is command byte 0xAD; data[1..len-1] is the message payload.
    // -----------------------------------------------------------------------
    Serial.printf("[RX] 0x%08X  len=%u  bytes:", id, len);
    for (uint8_t i = 0; i < len; i++) {
        Serial.printf(" %02X", data[i]);
    }
    Serial.println();
}

CANRxHandler rxHandler_18EFF903_AD = {
    .id                = 0x18EFF903,
    .filterCommandByte = true,
    .commandByte       = 0xAD,
    .callback          = onRx_18EFF903_AD
};

// ===========================================================================
// TX  –  0x18EF03F9, extended frame, command byte 0xAC
//
// Populate data[1..7] before or after calling registerCANMessages().
// intervalMs = 0 means on-demand only; set >0 for periodic transmissions.
//
// Example periodic send every 100 ms:
//   txDesc_18EF03F9.intervalMs = 100;
// ===========================================================================

CANTxDescriptor txDesc_18EF03F9 = {
    .id          = 0x18EF03F9,
    .extendedId  = true,
    .data        = { 0xAC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    .len         = 8,
    .intervalMs  = 0,      // change to e.g. 100 for 100 ms periodic
    ._lastSentMs = 0
};

// ===========================================================================
// Register all messages with the CANManager
// ===========================================================================

void registerCANMessages(CANManager& canMgr)
{
    // RX handlers
    canMgr.addRxHandler(rxHandler_18EFF903_AD);

    // TX descriptors
    canMgr.addTxDescriptor(&txDesc_18EF03F9);
}
