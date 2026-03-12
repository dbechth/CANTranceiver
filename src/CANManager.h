#pragma once
#include <Arduino.h>
#include "CANMessage.h"
#include "driver/twai.h"

// ---------------------------------------------------------------------------
// CANManager.h
// Wraps the ESP32 native TWAI (CAN) peripheral.
//
// Usage:
//   1. Instantiate with TX/RX GPIO pins and a timing config.
//   2. Register RX handlers and TX descriptors.
//   3. Call begin() once in setup().
//   4. Call update() every loop iteration.
//
// Adding new messages:
//   RX: build a CANRxHandler and call addRxHandler().
//   TX: build a CANTxDescriptor and call addTxDescriptor(), or use sendNow()
//       for one-shot messages that need no descriptor.
// ---------------------------------------------------------------------------

#ifndef CAN_MAX_RX_HANDLERS
#define CAN_MAX_RX_HANDLERS  16
#endif

#ifndef CAN_MAX_TX_DESCRIPTORS
#define CAN_MAX_TX_DESCRIPTORS 16
#endif

class CANManager {
public:
    // timingCfg: use one of the TWAI_TIMING_CONFIG_xxx macros, e.g.
    //            TWAI_TIMING_CONFIG_250KBITS()
    CANManager(gpio_num_t txPin, gpio_num_t rxPin,
               twai_timing_config_t timingCfg);

    // Install and start the TWAI driver.  Returns true on success.
    bool begin();

    // Must be called repeatedly from loop().
    // Drains the RX queue, dispatches handlers, and fires periodic TX.
    void update();

    // Register a handler for an incoming message.
    // Returns false if the handler table is full.
    bool addRxHandler(const CANRxHandler& handler);

    // Register a TX descriptor (periodic or on-demand reference).
    // Pass a pointer to a descriptor whose lifetime outlasts the manager.
    // Returns false if the table is full.
    bool addTxDescriptor(CANTxDescriptor* descriptor);

    // Send a one-shot message immediately (no descriptor required).
    bool sendNow(uint32_t id, bool extendedId,
                 const uint8_t* data, uint8_t len);

    // Transmit the descriptor at the given index right now (ignores interval).
    bool sendTxDescriptor(uint8_t index);

    // Number of registered TX descriptors.
    uint8_t txDescriptorCount() const { return _txDescriptorCount; }

private:
    gpio_num_t           _txPin;
    gpio_num_t           _rxPin;
    twai_timing_config_t _timingCfg;

    CANRxHandler   _rxHandlers[CAN_MAX_RX_HANDLERS];
    uint8_t        _rxHandlerCount;

    CANTxDescriptor* _txDescriptors[CAN_MAX_TX_DESCRIPTORS];
    uint8_t          _txDescriptorCount;

    void _processRx();
    void _processPeriodicTx();
    void _dispatchMessage(uint32_t id, bool extendedId,
                          const uint8_t* data, uint8_t len);
    bool _transmit(uint32_t id, bool extendedId,
                   const uint8_t* data, uint8_t len);
};
