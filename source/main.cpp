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

Thread sensor;

enum { NEGATIVE, ZERO, POSITIVE };

string gesture[3][2] = {{"left", "right"}, {"back", "front"}, {"down", "up"}};

void gestureDetecttion() {
  int previous[3] = {ZERO, ZERO, ZERO};
  int counter[3] = {0};
  int16_t accthreshold[3] = {400, 400, 500};
  float gyrothreshold[3] = {50000};
  int16_t currentAcc[3] = {0};
  float currentGyro[3] = {0};
  queue<int16_t> acc[3];
  queue<float> gyro[3];
  int16_t totalAcc[3] = {0};
  float totalGyro[3] = {0};
  BSP_GYRO_Init();
  BSP_ACCELERO_Init();

  while (1) {
    BSP_ACCELERO_AccGetXYZ(currentAcc);
    BSP_GYRO_GetXYZ(currentGyro);
    for (int i = 0; i < 3; i++) {
      acc[i].push(currentAcc[i]);
      gyro[i].push(currentGyro[i]);
      totalAcc[i] += currentAcc[i];
      totalGyro[i] += currentGyro[i];
      if (acc[i].size() > MAXPOINTS) {
        totalAcc[i] -= acc[i].front();
        totalGyro[i] -= gyro[i].front();
        acc[i].pop();
        gyro[i].pop();
      }
    }
    if (acc[2] > acc[1] && acc[2] > acc[0]) {
      totalAcc[2] -= 10000;
      for (int i = 0; i < 3; i++) {
        if (totalAcc[i] > accthreshold[i] * MAXPOINTS) {
          if (previous[i] == NEGATIVE && counter[i] <= 7) {
            GSH_INFO("%s\n", gesture[i][0].c_str());
          }
          previous[i] = POSITIVE;
          counter[i] = 0;
        } else if (totalAcc[i] < -accthreshold[i] * MAXPOINTS) {
          if (previous[i] == POSITIVE && counter[i] <= 7) {
            GSH_INFO("%s\n", gesture[i][1].c_str());
          }
          previous[i] = NEGATIVE;
          counter[i] = 0;
        } else {
          counter[i]++;
        }
      }
      totalAcc[2] += 10000;
    }
    fflush(stdout);
    ThisThread::sleep_for(20ms);
  }
}

int main() 
{     
#ifdef MBED_CONF_MBED_TRACE_ENABLE
    mbed_trace_init();
#endif

    GSH::Socket *socket = new GSH::Socket();
    MBED_ASSERT(socket);

    if (!socket->init()) 
    {
        GSH_INFO("Socket init failed");
        return 1;
    }
    
    socket->wifi_scan();
    // socket->wifi_connect("509-2", "max30201");
    socket->wifi_connect("Ethan\'s iPhone", "111111111");
    // socket->connect("192.168.0.106", 3000);
    socket->connect("192.168.0.106", 3000);

    // while (1) 
    // {
        socket->send_http_request("abc");
        char buffer[512] = {};
        socket->recv_chunk(buffer, 512);
        // ThisThread::sleep_for(2000ms);
    // }
    GSH_INFO("PROGRAM END");
    // sensor.start(gestureDetecttion); 
}