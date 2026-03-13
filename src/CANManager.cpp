#include "CANManager.h"

CANManager::CANManager(uint8_t csPin, CAN_SPEED speed, CAN_CLOCK clock)
    : _mcp(csPin), _speed(speed), _clock(clock)
{}

bool CANManager::begin()
{
    _mcp.reset();
    if (_mcp.setBitrate(_speed, _clock) != MCP2515::ERROR_OK) {
        Serial.println("[CAN] setBitrate failed");
        return false;
    }
    _mcp.setNormalMode();
    Serial.println("[CAN] MCP2515 ready");
    return true;
}

// ---------------------------------------------------------------------------
void CANManager::update()
{
    _processRx();
    _processPeriodicTx();
}

// ---------------------------------------------------------------------------
bool CANManager::addRxHandler(const CANRxHandler& handler)
{
    if (_rxHandlerCount >= CAN_MAX_RX_HANDLERS) {
        Serial.println("[CAN] addRxHandler: table full");
        return false;
    }
    _rxHandlers[_rxHandlerCount++] = handler;
    return true;
}

// ---------------------------------------------------------------------------
bool CANManager::addTxDescriptor(CANTxDescriptor* descriptor)
{
    if (_txDescriptorCount >= CAN_MAX_TX_DESCRIPTORS) {
        Serial.println("[CAN] addTxDescriptor: table full");
        return false;
    }
    descriptor->_lastSentMs = 0;
    _txDescriptors[_txDescriptorCount++] = descriptor;
    return true;
}

// ---------------------------------------------------------------------------
bool CANManager::sendNow(uint32_t id, bool extendedId,
                         const uint8_t* data, uint8_t len)
{
    return _transmit(id, extendedId, data, len);
}

// ---------------------------------------------------------------------------
bool CANManager::sendTxDescriptor(uint8_t index)
{
    if (index >= _txDescriptorCount) return false;
    CANTxDescriptor* d = _txDescriptors[index];
    bool ok = _transmit(d->id, d->extendedId, d->data, d->len);
    if (ok) d->_lastSentMs = millis();
    return ok;
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void CANManager::_processRx()
{
    can_frame f;
    while (_mcp.readMessage(&f) == MCP2515::ERROR_OK) {
        _dispatch(f);
    }
}

void CANManager::_processPeriodicTx()
{
    uint32_t now = millis();
    for (uint8_t i = 0; i < _txDescriptorCount; i++) {
        CANTxDescriptor* d = _txDescriptors[i];
        if (d->intervalMs == 0) continue;

        if ((now - d->_lastSentMs) >= d->intervalMs) {
            if (_transmit(d->id, d->extendedId, d->data, d->len)) {
                d->_lastSentMs = now;
            }
        }
    }
}

void CANManager::_dispatch(const can_frame& f)
{
    // Strip EFF/RTR flags to get the plain numeric ID
    uint32_t id  = f.can_id & (f.can_id & CAN_EFF_FLAG ? CAN_EFF_MASK : CAN_SFF_MASK);
    uint8_t  len = f.can_dlc;

    for (uint8_t i = 0; i < _rxHandlerCount; i++) {
        const CANRxHandler& h = _rxHandlers[i];
        if (h.id != id) continue;
        if (h.filterCommandByte && (len == 0 || f.data[0] != h.commandByte)) continue;
        if (h.callback) h.callback(id, f.data, len);
    }
}

bool CANManager::_transmit(uint32_t id, bool ext, const uint8_t* data, uint8_t len)
{
    can_frame f;
    f.can_id  = ext ? (id | CAN_EFF_FLAG) : id;
    f.can_dlc = len;
    memcpy(f.data, data, len);

    if (_mcp.sendMessage(&f) != MCP2515::ERROR_OK) {
        Serial.printf("[CAN] TX failed id=0x%08X\n", id);
        return false;
    }
    return true;
}
