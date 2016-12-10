#include "Kontroller/Kontroller.h"

#include <cmath>
#include <cstdio>
#include <map>
#include <thread>

namespace {

const int kSleepTime = 10;

const std::array<Kontroller::LED, 24> smrLEDs {{
   Kontroller::LED::kGroup1Solo, Kontroller::LED::kGroup1Mute, Kontroller::LED::kGroup1Record,
   Kontroller::LED::kGroup2Solo, Kontroller::LED::kGroup2Mute, Kontroller::LED::kGroup2Record,
   Kontroller::LED::kGroup3Solo, Kontroller::LED::kGroup3Mute, Kontroller::LED::kGroup3Record,
   Kontroller::LED::kGroup4Solo, Kontroller::LED::kGroup4Mute, Kontroller::LED::kGroup4Record,
   Kontroller::LED::kGroup5Solo, Kontroller::LED::kGroup5Mute, Kontroller::LED::kGroup5Record,
   Kontroller::LED::kGroup6Solo, Kontroller::LED::kGroup6Mute, Kontroller::LED::kGroup6Record,
   Kontroller::LED::kGroup7Solo, Kontroller::LED::kGroup7Mute, Kontroller::LED::kGroup7Record,
   Kontroller::LED::kGroup8Solo, Kontroller::LED::kGroup8Mute, Kontroller::LED::kGroup8Record,
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

   for (size_t i = 0; i < smrLEDs.size(); i += 3) {
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
      for (size_t i = 0; i < smrLEDs.size(); i += 3) {
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
      for (size_t i = 0; i < smrLEDs.size(); ++i) {
         size_t group = i / 3;
         bool pressed;
         if (i % 3 == 0) {
            pressed = state.groups[group].solo;
         } else if (i % 3 == 1) {
            pressed = state.groups[group].mute;
         } else {
            pressed = state.groups[group].record;
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

void followSliders(Kontroller* kontroller) {
   Kontroller::State state = kontroller->getState();

   for (size_t i = 0; i < smrLEDs.size(); i += 3) {
      size_t group = i / 3;
      float val = state.groups[group].slider;

      bool s = val > 0.75f;
      bool m = val > 0.5f;
      bool r = val > 0.25f;

      kontroller->setLEDOn(smrLEDs[i + 0], s);
      kontroller->setLEDOn(smrLEDs[i + 1], m);
      kontroller->setLEDOn(smrLEDs[i + 2], r);
   }
}

typedef void (*DisplayFunc)(Kontroller*);

std::array<DisplayFunc, 3> displayFunctions {{
   doTheWave,
   explode,
   followSliders
}};

void clearLEDs(Kontroller *kontroller) {
   for (Kontroller::LED led : smrLEDs) {
      kontroller->setLEDOn(led, false);
   }
}

} // namespace

int main(int argc, char *argv[]) {
   Kontroller kontroller;

   kontroller.setButtonCallback([](Kontroller::Button button, bool pressed) {
      printf("%d pressed: %d\n", static_cast<int>(button), pressed);
   });

   kontroller.setDialCallback([](Kontroller::Dial dial, float value) {
      printf("%d dial val: %f\n", static_cast<int>(dial), value);
   });

   kontroller.setSliderCallback([](Kontroller::Slider slider, float value) {
      printf("%d slider val: %f\n", static_cast<int>(slider), value);
   });

   Kontroller::State state = kontroller.getState();
   Kontroller::State current = state;
   Kontroller::State previous = state;
   int displayFuncIndex = 0;

   bool controlEnabled = false;
   kontroller.enableLEDControl(controlEnabled);

   while (!state.stop) {
      if (kontroller.isConnected()) {
         if (state.cycle) {
            controlEnabled = !controlEnabled;

            if (!controlEnabled) {
               clearLEDs(&kontroller);
            }

            kontroller.enableLEDControl(controlEnabled);
         }

         if (state.trackNext) {
            displayFuncIndex = (displayFuncIndex + 1) % displayFunctions.size();
         }
         if (state.trackPrevious) {
            displayFuncIndex = displayFuncIndex == 0 ? displayFunctions.size() - 1 : displayFuncIndex - 1;
         }

         if (controlEnabled) {
            displayFunctions[displayFuncIndex](&kontroller);
         }
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(kSleepTime));

      previous = current;
      current = kontroller.getState();
      state = Kontroller::onlyNewButtons(previous, current);
   }

   clearLEDs(&kontroller);
   kontroller.enableLEDControl(false);

   return 0;
}
