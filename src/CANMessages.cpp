#include "CANMessages.h"

// ---------------------------------------------------------------------------
// CANMessages.cpp
// Project-specific CAN message definitions, callbacks, and registration.
// ---------------------------------------------------------------------------

static float s_percentSpeed = 0.0f;
static bool  s_percentSpeedUpdated = false;

// ===========================================================================
// RX  –  0x18EFFA03, extended frame, command byte 0xAD
// ===========================================================================

static void onRx_18EFFA03_AD(uint32_t id, const uint8_t* data, uint8_t len)
{
    // -----------------------------------------------------------------------
    // TODO: replace this with your application logic.
    // data[0] is command byte 0xAD; data[1..len-1] is the message payload.
    // -----------------------------------------------------------------------
    //Serial.printf("[RX] 0x%08X  len=%u  bytes:", id, len);
    //for (uint8_t i = 0; i < len; i++) {
    //    Serial.printf(" %02X", data[i]);
    //}
    //Serial.println();
}

CANRxHandler rxHandler_18EFFA03_AD = {
    .id                = 0x18EFFA03,
    .filterCommandByte = true,
    .commandByte       = 0xAD,
    .callback          = onRx_18EFFA03_AD
};

// ===========================================================================
// RX  –  0x0CFB7496, extended frame, no command-byte filter
//       Percent speed: bytes 4-5 (data[3], data[4]), scale 0.00390625, offset -125
// ===========================================================================

static void onRx_0CFB7496(uint32_t id, const uint8_t* data, uint8_t len)
{
    if (len < 5) {
        Serial.printf("[RX] 0x%08X len=%u (too short for speed decode)\n", id, len);
        return;
    }

    uint16_t rawSpeed = static_cast<uint16_t>(data[3]) |
                        (static_cast<uint16_t>(data[4]) << 8);
    float speedPercent = (static_cast<float>(rawSpeed) * 0.00390625f) - 125.0f;

    s_percentSpeed = speedPercent;
    s_percentSpeedUpdated = true;

    //Serial.printf("[RX] 0x%08X speed_raw=%u speed_pct=%.3f%%\n", id, rawSpeed, speedPercent);
}

CANRxHandler rxHandler_0CFB7496 = {
    .id                = 0x0CFB7496,
    .filterCommandByte = false,
    .commandByte       = 0x00,
    .callback          = onRx_0CFB7496
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
    //canMgr.addRxHandler(rxHandler_18EFFA03_AD);
    canMgr.addRxHandler(rxHandler_0CFB7496);

    // TX descriptors
    //canMgr.addTxDescriptor(&txDesc_18EF03F9);
}

bool consumePercentSpeed(float& speedPercent)
{
    if (!s_percentSpeedUpdated) return false;
    speedPercent = s_percentSpeed;
    s_percentSpeedUpdated = false;
    return true;
}
