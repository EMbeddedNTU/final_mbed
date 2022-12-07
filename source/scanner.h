#include "core/pch.h"
#include "ble/BLE.h"
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
    _event_queue.dispatch_forever();
  }

private:
  BLE &_ble;
  events::EventQueue &_event_queue;
  ble::rssi_t largestRssi = NULL;

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
    GSH_INFO("RSSI %d", rssi);
    // GSH_INFO("");
    while (adv_data.hasNext()) {
      ble::AdvertisingDataParser::element_t field = adv_data.next();
    //   
        printf("\n");
        // printf("type %d\r\n", field.type);

        if (field.type == ble::adv_data_type_t::COMPLETE_LOCAL_NAME)
        {
            for (int i = 0; i < field.value.size(); i++)
            {
                char* ptr = (char*)field.value.data() + i;
                printf("%c", *ptr);
            }
            printf("found\r\n");
        }

      //       printf("%.*s", field.value.size(), field.value.data());
      //   }
    }

    // printf("Received advertising data from address "
    //        "%02x:%02x:%02x:%02x:%02x:%02x, RSSI: %ddBm\r\n",
    //        address[5], address[4], address[3], address[2], address[1],
    //        address[0], rssi);
  }
};
