#include "CANManager.h"

// ---------------------------------------------------------------------------
// CANManager.cpp
// ---------------------------------------------------------------------------

CANManager::CANManager(gpio_num_t txPin, gpio_num_t rxPin,
                       twai_timing_config_t timingCfg)
    : _txPin(txPin),
      _rxPin(rxPin),
      _timingCfg(timingCfg),
      _rxHandlerCount(0),
      _txDescriptorCount(0)
{
    memset(_rxHandlers,   0, sizeof(_rxHandlers));
    memset(_txDescriptors, 0, sizeof(_txDescriptors));
}

// ---------------------------------------------------------------------------
bool CANManager::begin()
{
    // Accept all frames; software filtering is done in _dispatchMessage().
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    twai_general_config_t g_config =
        TWAI_GENERAL_CONFIG_DEFAULT(_txPin, _rxPin, TWAI_MODE_NORMAL);

    // Increase RX queue depth for bursty traffic
    g_config.rx_queue_len = 32;
    g_config.tx_queue_len = 16;

    esp_err_t err;

    err = twai_driver_install(&g_config, &_timingCfg, &f_config);
    if (err != ESP_OK) {
        Serial.printf("[CAN] driver_install failed: %s\n", esp_err_to_name(err));
        return false;
    }

    err = twai_start();
    if (err != ESP_OK) {
        Serial.printf("[CAN] twai_start failed: %s\n", esp_err_to_name(err));
        return false;
    }

    Serial.println("[CAN] TWAI driver started.");
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
    twai_message_t msg;
    // Drain all available messages without blocking
    while (twai_receive(&msg, 0) == ESP_OK) {
        bool ext = (msg.extd == 1);
        _dispatchMessage(msg.identifier, ext, msg.data, msg.data_length_code);
    }
}

// ---------------------------------------------------------------------------
void CANManager::_processPeriodicTx()
{
    uint32_t now = millis();
    for (uint8_t i = 0; i < _txDescriptorCount; i++) {
        CANTxDescriptor* d = _txDescriptors[i];
        if (d->intervalMs == 0) continue;  // on-demand only

        if ((now - d->_lastSentMs) >= d->intervalMs) {
            if (_transmit(d->id, d->extendedId, d->data, d->len)) {
                d->_lastSentMs = now;
            }
        }
    }
}

// ---------------------------------------------------------------------------
void CANManager::_dispatchMessage(uint32_t id, bool extendedId,
                                  const uint8_t* data, uint8_t len)
{
    for (uint8_t i = 0; i < _rxHandlerCount; i++) {
        const CANRxHandler& h = _rxHandlers[i];

        // Match identifier
        if (h.id != id) continue;

        // Optionally match frame type
        if (h.extendedId != extendedId) continue;

        // Optionally match command byte (data[0])
        if (h.filterCommandByte) {
            if (len == 0 || data[0] != h.commandByte) continue;
        }

        if (h.callback) {
            h.callback(id, data, len);
        }
    }
}

// ---------------------------------------------------------------------------
bool CANManager::_transmit(uint32_t id, bool extendedId,
                            const uint8_t* data, uint8_t len)
{
    twai_message_t msg;
    memset(&msg, 0, sizeof(msg));

    msg.identifier       = id;
    msg.extd             = extendedId ? 1 : 0;
    msg.data_length_code = len;
    memcpy(msg.data, data, len);

    esp_err_t err = twai_transmit(&msg, pdMS_TO_TICKS(10));
    if (err != ESP_OK) {
        Serial.printf("[CAN] TX failed id=0x%08X err=%s\n",
                      id, esp_err_to_name(err));
        return false;
    }
    return true;
}
