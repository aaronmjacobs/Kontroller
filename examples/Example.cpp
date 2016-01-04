#include "Kontroller/Kontroller.h"

#include <cmath>
#include <map>
#include <thread>

namespace {

const int kSleepTime = 10;

const std::array<Kontroller::LED, 24> smrLEDs {{
   Kontroller::LED::kCol1S, Kontroller::LED::kCol1M, Kontroller::LED::kCol1R,
   Kontroller::LED::kCol2S, Kontroller::LED::kCol2M, Kontroller::LED::kCol2R,
   Kontroller::LED::kCol3S, Kontroller::LED::kCol3M, Kontroller::LED::kCol3R,
   Kontroller::LED::kCol4S, Kontroller::LED::kCol4M, Kontroller::LED::kCol4R,
   Kontroller::LED::kCol5S, Kontroller::LED::kCol5M, Kontroller::LED::kCol5R,
   Kontroller::LED::kCol6S, Kontroller::LED::kCol6M, Kontroller::LED::kCol6R,
   Kontroller::LED::kCol7S, Kontroller::LED::kCol7M, Kontroller::LED::kCol7R,
   Kontroller::LED::kCol8S, Kontroller::LED::kCol8M, Kontroller::LED::kCol8R,
}};

struct Loc {
   float x, y;
};

std::map<Kontroller::LED, Loc> locMap;

float dist(float x, float y, float x2, float y2) {
   float yDiff = y2 - y;
   float xDiff = x2 - x;
   return sqrtf(yDiff * yDiff + xDiff * xDiff);
}

void doTheWave(Kontroller* kontroller) {
   static const float dt = (kSleepTime / 1000.0f) * 5.0f;
   static const float dist = 0.2f;

   static float t = 0.0f;

   for (int i = 0; i < smrLEDs.size(); i += 3) {
      float y = sinf(t + (i / 3) * dist);

      bool s = y > 0.8f;
      bool m = y > 0.0f;
      bool r = y > -0.8f;

      kontroller->setLEDOn(smrLEDs[i + 0], s);
      kontroller->setLEDOn(smrLEDs[i + 1], m);
      kontroller->setLEDOn(smrLEDs[i + 2], r);
   }

   t += dt;
}

void explode(Kontroller* kontroller) {
   if (locMap.empty()) {
      for (int i = 0; i < smrLEDs.size(); i += 3) {
         float x = i - 10.0f;
         locMap[smrLEDs[i + 0]] = {x, 3.0f};
         locMap[smrLEDs[i + 1]] = {x, 0.0f};
         locMap[smrLEDs[i + 2]] = {x, -3.0f};
      }
   }

   Kontroller::State state = kontroller->getState();

   static float radius = 0.0f;
   static float speed = (kSleepTime / 1000.0f) * 30.0f;
   static Loc origin { 0.0f, 0.0f };
   if (radius == 0.0f) {
      for (int i = 0; i < smrLEDs.size(); ++i) {
         int col = i / 3;
         bool pressed;
         if (i % 3 == 0) {
            pressed = state.columns[col].s;
         } else if (i % 3 == 1) {
            pressed = state.columns[col].m;
         } else {
            pressed = state.columns[col].r;
         }

         if (pressed) {
            origin = locMap[smrLEDs[i]];
            radius = 0.00001f;
            break;
         }
      }
   }
   if (radius != 0.0f) {
      radius += speed;
   }
   if (radius > 25.0f) {
      radius = 0.0f;
   }

   for (auto pair : locMap) {
      float distance = dist(pair.second.x, pair.second.y, origin.x, origin.y);
      bool on = radius > 0.0f && fabsf(distance - radius) < 2.0f;

      kontroller->setLEDOn(pair.first, on);
   }
}

typedef void (*DisplayFunc)(Kontroller*);

std::array<DisplayFunc, 2> displayFunctions {
   doTheWave,
   explode
};

} // namespace

int main(int argc, char *argv[]) {
   Kontroller kontroller;
   Kontroller::State state = kontroller.getState();
   int displayFuncIndex = 0;

   static bool controlEnabled = false;
   kontroller.enableLEDControl(controlEnabled);

   while (!state.stop) {
      if (state.cycle) {
         controlEnabled = !controlEnabled;
         kontroller.enableLEDControl(controlEnabled);
      }

      if (state.trackRight) {
         displayFuncIndex = (displayFuncIndex + 1) % displayFunctions.size();
      }
      if (state.trackLeft) {
         displayFuncIndex = displayFuncIndex == 0 ? displayFunctions.size() - 1 : displayFuncIndex - 1;
      }

      displayFunctions[displayFuncIndex](&kontroller);

      std::this_thread::sleep_for(std::chrono::milliseconds(kSleepTime));
      state = kontroller.getState();
   }

   for (Kontroller::LED led : smrLEDs) {
      kontroller.setLEDOn(led, false);
   }
   kontroller.enableLEDControl(false);

   return 0;
}
