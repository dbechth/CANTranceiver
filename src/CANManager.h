#pragma once
#include <Arduino.h>
#include <mcp2515.h>
#include "CANMessage.h"

// CANManager: thin wrapper around the autowp/mcp2515 library.
// - Dispatches received frames to registered RX handlers.
// - Fires periodic TX descriptors from update().

#ifndef CAN_MAX_RX_HANDLERS
#define CAN_MAX_RX_HANDLERS  16
#endif
#ifndef CAN_MAX_TX_DESCRIPTORS
#define CAN_MAX_TX_DESCRIPTORS 16
#endif

class CANManager {
public:
    // csPin    – SPI chip-select GPIO
    // speed    – e.g. CAN_250KBPS
    // clock    – oscillator on the MCP2515 board, e.g. MCP_8MHZ or MCP_16MHZ
    CANManager(uint8_t csPin, CAN_SPEED speed, CAN_CLOCK clock = MCP_8MHZ);

    bool begin();   // call once in setup(); returns false on failure
    void update();  // call every loop()

    bool addRxHandler(const CANRxHandler& h);
    bool addTxDescriptor(CANTxDescriptor* d);

    // Send a one-shot frame right now
    bool sendNow(uint32_t id, bool extendedId, const uint8_t* data, uint8_t len);

    // Force-send a registered TX descriptor immediately
    bool sendTxDescriptor(uint8_t index);

private:
    MCP2515 _mcp;
    CAN_SPEED _speed;
    CAN_CLOCK _clock;

    CANRxHandler     _rxHandlers[CAN_MAX_RX_HANDLERS];
    uint8_t          _rxHandlerCount = 0;
    CANTxDescriptor* _txDescriptors[CAN_MAX_TX_DESCRIPTORS];
    uint8_t          _txDescriptorCount = 0;

    void _processRx();
    void _processPeriodicTx();
    void _dispatch(const can_frame& f);
    bool _transmit(uint32_t id, bool ext, const uint8_t* data, uint8_t len);
};
