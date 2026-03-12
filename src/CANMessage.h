#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// CANMessage.h
// Core types shared between CANManager and project-specific message modules.
// ---------------------------------------------------------------------------

// Callback invoked when a matching RX message is received.
// id       - 11-bit or 29-bit CAN identifier
// data     - pointer to payload bytes
// len      - number of payload bytes (0-8)
typedef void (*CANRxCallback)(uint32_t id, const uint8_t* data, uint8_t len);

// ---------------------------------------------------------------------------
// RX Handler
// Register one of these for every CAN message (or command byte) you want to
// receive.  If filterCommandByte is true, the handler fires only when
// data[0] == commandByte.
// ---------------------------------------------------------------------------
struct CANRxHandler {
    uint32_t     id;                 // CAN identifier to match
    bool         extendedId;         // true = 29-bit extended frame
    bool         filterCommandByte;  // enable data[0] command-byte filter
    uint8_t      commandByte;        // data[0] value to match (if above = true)
    CANRxCallback callback;          // function called on match
};

// ---------------------------------------------------------------------------
// TX Descriptor
// Describes a CAN message that can be sent periodically or on-demand.
// For periodic messages set intervalMs > 0; they are transmitted by
// CANManager::update().  For on-demand messages use CANManager::sendNow() or
// CANManager::sendTxDescriptor().
// ---------------------------------------------------------------------------
struct CANTxDescriptor {
    uint32_t  id;                // CAN identifier
    bool      extendedId;        // true = 29-bit extended frame
    uint8_t   data[8];           // payload (caller fills before sending)
    uint8_t   len;               // payload length (1-8)
    uint32_t  intervalMs;        // 0 = on-demand only; >0 = periodic interval

    // Internal – managed by CANManager, do not set manually
    uint32_t  _lastSentMs;
};
