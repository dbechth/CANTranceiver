#pragma once
#include "CANManager.h"

// ---------------------------------------------------------------------------
// CANMessages.h
// Project-specific CAN message declarations.
//
// HOW TO ADD NEW MESSAGES
// -----------------------
// RX:
//   1. Add an extern CANRxHandler declaration below.
//   2. In CANMessages.cpp define the handler and its callback function.
//   3. In registerCANMessages() call canMgr.addRxHandler(yourHandler).
//
// TX:
//   1. Add an extern CANTxDescriptor declaration below.
//   2. In CANMessages.cpp define the descriptor (set id, extendedId, data,
//      len, intervalMs).
//   3. In registerCANMessages() call canMgr.addTxDescriptor(&yourDescriptor).
//   4. To update payload before sending (periodic), modify the descriptor's
//      data[] array anywhere before update() is called.
// ---------------------------------------------------------------------------

// ---- RX handlers -----------------------------------------------------------
// 0x18EFA03 | extended | command byte 0xAD
extern CANRxHandler rxHandler_18EFFA03_AD;
// 0x0CFB7496 | extended | no command-byte filter
extern CANRxHandler rxHandler_0CFB7496;

// ---- TX descriptors --------------------------------------------------------
// 0x18EF03F9 | extended | command byte 0xAC  (periodic, see CANMessages.cpp)
extern CANTxDescriptor txDesc_18EF03F9;

// ---------------------------------------------------------------------------
// Call this once in setup() after constructing the CANManager.
// ---------------------------------------------------------------------------
void registerCANMessages(CANManager& canMgr);

// ---------------------------------------------------------------------------
// Latest decoded speed (from 0x0CFB7496 bytes 4-5).
// Returns true only when a new value has arrived since the last call.
// ---------------------------------------------------------------------------
bool consumePercentSpeed(float& speedPercent);
