#include "pch.h"
#include "scanner.h"
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_gyro.h"
#include <queue>
#include <string>

namespace GSH {

class Gesture {
  static constexpr int MAXPOINTS = 10;
  inline static string gesture[6] = {"LEFT",  "RIGHT", "BACK",
                                     "FRONT", "DOWN",  "UP"};
  enum { NEGATIVE, ZERO, POSITIVE };

public:
  Gesture(BLEScanner &scanner) : _scanner(scanner) {}

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
      fflush(stdout);
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
    for (int i = 0; i < 3; i++) {
      if (totalAcc[i] > accthreshold[i] * MAXPOINTS) {
        if (previous[i] == NEGATIVE && counter[i] <= 7) {
          GSH_INFO("%s\n", gesture[i * 2].c_str());
          GSH_DEBUG("%c\n", _scanner.getNearestDevice());
        }
        previous[i] = POSITIVE;
        counter[i] = 0;
      } else if (totalAcc[i] < -accthreshold[i] * MAXPOINTS) {
        if (previous[i] == POSITIVE && counter[i] <= 7) {
          GSH_INFO("%s\n", gesture[i * 2 + 1].c_str());
          GSH_DEBUG("%c\n", _scanner.getNearestDevice());
        }
        previous[i] = NEGATIVE;
        counter[i] = 0;
      } else {
        counter[i]++;
      }
    }
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
  BLEScanner &_scanner;
};

} // namespace GSH