#pragma once
#include <Arduino.h>

// Fired when a matching frame arrives. data[0] is the command byte if used.
typedef void (*CANRxCallback)(uint32_t id, const uint8_t* data, uint8_t len);

// Match an incoming message. Set filterCommandByte=true to also match data[0].
struct CANRxHandler {
    uint32_t      id;
    bool          filterCommandByte;
    uint8_t       commandByte;
    CANRxCallback callback;
};

// A message to send. intervalMs>0 = periodic; 0 = on-demand via sendNow().
struct CANTxDescriptor {
    uint32_t  id;
    bool      extendedId;        // true = 29-bit J1939/extended frame
    uint8_t   data[8];
    uint8_t   len;
    uint32_t  intervalMs;        // 0 = on-demand only
    uint32_t  _lastSentMs;       // managed internally
};
