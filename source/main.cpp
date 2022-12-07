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

#define MAXPOINTS 10
#include "scanner.h"

Thread sensor;
Thread bleScanner;

static EventQueue event_queue(16 * EVENTS_EVENT_SIZE);

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
  event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

// int main() 
// {     
// #ifdef MBED_CONF_MBED_TRACE_ENABLE
//     mbed_trace_init();
// #endif

//     GSH::Socket *socket = new GSH::Socket();
//     MBED_ASSERT(socket);

//     if (!socket->init()) 
//     {
//         GSH_INFO("Socket init failed");
//         return 1;
//     }
    
//     socket->wifi_scan();
//     // socket->wifi_connect("509-2", "max30201");
//     socket->wifi_connect("Ethan\'s iPhone", "111111111");
//     // socket->connect("192.168.0.106", 3000);
//     socket->connect("192.168.0.106", 3000);

//     // while (1) 
//     // {
//         socket->send_http_request("abc");
//         char buffer[512] = {};
//         socket->recv_chunk(buffer, 512);
//         // ThisThread::sleep_for(2000ms);
//     // }
//     GSH_INFO("PROGRAM END");
// }

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