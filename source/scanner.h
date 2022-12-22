#include "ble/BLE.h"
#include "core/pch.h"
#include <cstdint>
#include <cstdio>
#include <events/mbed_events.h>
#include <stdint.h>

class BLEScanner : ble::Gap::EventHandler {
  static constexpr bool ACTIVE_SCANNING = true;
  static constexpr int SCAN_INTERVAL = 1000;
  static constexpr int SCAN_WINDOW = 1000;

public:
  BLEScanner(BLE &ble, events::EventQueue &event_queue)
      : _ble(ble), _event_queue(event_queue) {}

  ~BLEScanner() {}

  void scan() {
    _ble.gap().setEventHandler(this);
    _ble.init(this, &BLEScanner::on_init_complete);
    _event_queue.call_every(5s, [this] { reset(); });
    _event_queue.dispatch_forever();
  }

  char getNearestDevice() { return nearestDevice; }

private:
  BLE &_ble;
  events::EventQueue &_event_queue;
  ble::rssi_t largestRssi = NULL;
  char nearestDevice = NULL;

  void reset() {
    largestRssi = NULL;
    nearestDevice = NULL;
  }

  void on_init_complete(BLE::InitializationCompleteCallbackContext *params) {
    if (params->error != BLE_ERROR_NONE) {
      return;
    }
    ble::ScanParameters scan_params;
    scan_params.set1mPhyConfiguration(ble::scan_interval_t(SCAN_INTERVAL),
                                      ble::scan_window_t(SCAN_WINDOW),
                                      ACTIVE_SCANNING);
    _ble.gap().setScanParameters(scan_params);
    _ble.gap().startScan();
  }

  void onAdvertisingReport(const ble::AdvertisingReportEvent &event) override {
    ble::AdvertisingDataParser adv_data(event.getPayload());
    ble::address_t address = event.getPeerAddress();
    ble::rssi_t rssi = event.getRssi();
    while (adv_data.hasNext()) {
      ble::AdvertisingDataParser::element_t field = adv_data.next();

      if (field.type == ble::adv_data_type_t::COMPLETE_LOCAL_NAME) {

        Span<const unsigned char, -1>::pointer device_name_ptr =
            field.value.data();
        if (device_name_ptr[0] == 'A' && device_name_ptr[1] == 'g' &&
            device_name_ptr[2] == 'e' && device_name_ptr[3] == 'n' &&
            device_name_ptr[4] == 't') {
          if (largestRssi == NULL || rssi > largestRssi) {
            largestRssi = rssi;
            nearestDevice = device_name_ptr[5];
          }
        }
      }
    }
  }
};