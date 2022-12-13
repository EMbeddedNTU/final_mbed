#include "core/socket.h"
#include "gesture.h"

Thread sensor;
Thread bleScanner;

GSH::HttpService &http_service = GSH::HttpService::GetInstance();
static EventQueue event_queue(16 * EVENTS_EVENT_SIZE);

const static char WIFI_SSID[] = "509-2";
const static char WIFI_KEY[] = "max30201";

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
  event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

void gestureDetection(GSH::Gesture *gesture) {
  gesture->init();
  gesture->startDetect();
}

void bleScan(BLEScanner *scanner) { scanner->scan(); }

int main() {
  http_service.init(WIFI_SSID, WIFI_KEY);

  BLE &ble = BLE::Instance();
  ble.onEventsToProcess(schedule_ble_events);
  BLEScanner scanner(ble, event_queue);

  GSH::Gesture *gesture = new GSH::Gesture(scanner, http_service);

  sensor.start(callback(gestureDetection, gesture));
  bleScanner.start(Callback<void()>(bleScan, &scanner));
}