#include "gesture.h"
#include "pch.h"
#include "scanner.h"

Thread sensor;
Thread bleScanner;

static EventQueue event_queue(16 * EVENTS_EVENT_SIZE);

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
  event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

void gestureDetection(GSH::Gesture *gesture) {
  gesture->init();
  gesture->startDetect();
}

void bleScan(BLEScanner *scanner) { scanner->scan(); }

int main() {
  GSH::Gesture *gesture = new GSH::Gesture();

  BLE &ble = BLE::Instance();
  ble.onEventsToProcess(schedule_ble_events);
  BLEScanner scanner(ble, event_queue);

  sensor.start(Callback<void()>(gestureDetection, gesture));
  bleScanner.start(Callback<void()>(bleScan, &scanner));
}