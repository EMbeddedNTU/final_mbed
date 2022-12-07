#include "gesture.h"
#include "pch.h"
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_gyro.h"
#include "mbed-trace/mbed_trace.h"
#include <cstdint>
#include <cstdio>
#include <queue>
#include <string>

#include "core/socket.h"
#include "http_service.h"

#define MAXPOINTS 10
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

#ifdef MBED_CONF_MBED_TRACE_ENABLE
    mbed_trace_init();
#endif

    GSH::HttpService& http_service = GSH::HttpService::GetInstance();
    http_service.init("509-2", "max30201");

    GSH::HttpService::HttpResponse* response = http_service.http_get("http://192.168.0.106:3000", NULL);

    GSH_INFO("body: %s", response->body);
    // GSH::Gesture *gesture = new GSH::Gesture();

    // BLE &ble = BLE::Instance();
    // ble.onEventsToProcess(schedule_ble_events);
    // BLEScanner scanner(ble, event_queue);

    // sensor.start(Callback<void()>(gestureDetection, gesture));
    // bleScanner.start(Callback<void()>(bleScan, &scanner));
    GSH_INFO("PROGRAM END");    
}