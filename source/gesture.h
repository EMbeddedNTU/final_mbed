#include "pch.h"
#include "scanner.h"
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_gyro.h"
#include <cstdio>
#include <http_service.h>
#include <queue>
#include <string>

namespace GSH {

class Gesture {
  static constexpr int MAXPOINTS = 10;
  enum GestureType {
    left,
    right,
    backward,
    forward,
    down,
    up,
  };
  enum { NEGATIVE, ZERO, POSITIVE };

public:
  Gesture(BLEScanner &scanner, GSH::HttpService *http_service)
      : _scanner(scanner), _http_service(http_service) {}

  ~Gesture() {}

  void init() {
    BSP_GYRO_Init();
    BSP_ACCELERO_Init();
  };

  void startDetect() {
    while (1) {
      BSP_ACCELERO_AccGetXYZ(currentAcc);
      BSP_GYRO_GetXYZ(currentGyro);
      updateData();
      if (acc[2] > acc[1] && acc[2] > acc[0]) {
        totalAcc[2] -= 10000;
        detection();
        totalAcc[2] += 10000;
      }
      ThisThread::sleep_for(20ms);
    }
  }

private:
  void updateData() {
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
  }

  void detection() {
    device = NULL;
    gesture = -1;
    bool multiple = false;
    for (int i = 0; i < 3; i++) {
      if (totalAcc[i] > accthreshold[i] * MAXPOINTS) {
        if (previous[i] == NEGATIVE && counter[i] <= 7) {
          if (gesture == -1) {
            gesture = i * 2;
          } else {
            multiple = true;
          }
          device = _scanner.getNearestDevice();
          while (device == NULL) {
            device = _scanner.getNearestDevice();
          }
        }
        previous[i] = POSITIVE;
        counter[i] = 0;
      } else if (totalAcc[i] < -accthreshold[i] * MAXPOINTS) {
        if (previous[i] == POSITIVE && counter[i] <= 7) {
          if (gesture == -1) {
            gesture = i * 2 + 1;
          } else {
            multiple = true;
          }
          device = _scanner.getNearestDevice();
          while (device == NULL) {
            device = _scanner.getNearestDevice();
          }
        }
        previous[i] = NEGATIVE;
        counter[i] = 0;
      } else {
        counter[i]++;
      }
    }
    if (!multiple && gesture != -1) {
      send_gesture(gesture, device);
    }
  }

  void send_gesture(int gesture, char device) {
    GSH_ERROR("gesture %d device %c", gesture, device);
    char msg[1024];
    sprintf(msg, "{\"agentId\": %c,\"gestureType\": %d}", device, gesture);

    SharedPtr<GSH::HttpService::HttpResponse> response =
        _http_service->http_post("http://192.168.0.101:3000/agent/gesture",
                                 NULL, msg);
  }

private:
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
  char device;
  int gesture;
  BLEScanner &_scanner;
  GSH::HttpService *_http_service;
};

} // namespace GSH