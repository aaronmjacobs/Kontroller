#include "Kontroller/Device.h"

#include "Kontroller/Client.h"
#include "Kontroller/Server.h"

#include <cmath>
#include <cstdio>
#include <thread>
#include <unordered_map>

namespace
{
   const int kSleepMillis = 10;
   const float kDeltaTime = (kSleepMillis / 1000.0f);

   const std::array<Kontroller::LED, 24> kGroupLEDs
   {{
      Kontroller::LED::Group1Solo, Kontroller::LED::Group1Mute, Kontroller::LED::Group1Record,
      Kontroller::LED::Group2Solo, Kontroller::LED::Group2Mute, Kontroller::LED::Group2Record,
      Kontroller::LED::Group3Solo, Kontroller::LED::Group3Mute, Kontroller::LED::Group3Record,
      Kontroller::LED::Group4Solo, Kontroller::LED::Group4Mute, Kontroller::LED::Group4Record,
      Kontroller::LED::Group5Solo, Kontroller::LED::Group5Mute, Kontroller::LED::Group5Record,
      Kontroller::LED::Group6Solo, Kontroller::LED::Group6Mute, Kontroller::LED::Group6Record,
      Kontroller::LED::Group7Solo, Kontroller::LED::Group7Mute, Kontroller::LED::Group7Record,
      Kontroller::LED::Group8Solo, Kontroller::LED::Group8Mute, Kontroller::LED::Group8Record,
   }};

   struct Loc
   {
      float x = 0.0f;
      float y = 0.0f;
   };

   float dist(float x, float y, float x2, float y2)
   {
      float yDiff = y2 - y;
      float xDiff = x2 - x;
      return sqrtf(yDiff * yDiff + xDiff * xDiff);
   }

   void doTheWave(Kontroller::Device& device)
   {
      static const float kSpeed = 5.0f;
      static const float kDistance = 0.2f;

      static float time = 0.0f;

      for (size_t i = 0; i < kGroupLEDs.size(); i += 3)
      {
         float y = sinf(time + (i / 3) * kDistance);

         bool s = y > 0.8f;
         bool m = y > 0.0f;
         bool r = y > -0.8f;

         device.setLEDOn(kGroupLEDs[i + 0], s);
         device.setLEDOn(kGroupLEDs[i + 1], m);
         device.setLEDOn(kGroupLEDs[i + 2], r);
      }

      time += kDeltaTime * kSpeed;
   }

   void explode(Kontroller::Device& device)
   {
      static const float kSpeed = 30.0f;

      static std::unordered_map<Kontroller::LED, Loc> locMap;
      if (locMap.empty())
      {
         for (size_t i = 0; i < kGroupLEDs.size(); i += 3)
         {
            float x = i - 10.0f;
            locMap[kGroupLEDs[i + 0]] = { x, 3.0f };
            locMap[kGroupLEDs[i + 1]] = { x, 0.0f };
            locMap[kGroupLEDs[i + 2]] = { x, -3.0f };
         }
      }

      Kontroller::State state = device.getState();

      static float radius = 0.0f;
      static Loc origin;

      if (radius == 0.0f)
      {
         for (size_t i = 0; i < kGroupLEDs.size(); ++i)
         {
            size_t group = i / 3;
            bool pressed = false;
            switch (i % 3)
            {
            case 0:
               pressed = state.groups[group].solo;
               break;
            case 1:
               pressed = state.groups[group].mute;
               break;
            case 2:
               pressed = state.groups[group].record;
               break;
            default:
               break;
            }

            if (pressed)
            {
               origin = locMap[kGroupLEDs[i]];
               radius = 0.00001f;
               break;
            }
         }
      }

      if (radius != 0.0f)
      {
         radius += kDeltaTime * kSpeed;
      }
      if (radius > 25.0f)
      {
         radius = 0.0f;
      }

      for (const auto& pair : locMap)
      {
         float distance = dist(pair.second.x, pair.second.y, origin.x, origin.y);
         bool on = radius > 0.0f && fabsf(distance - radius) < 2.0f;

         device.setLEDOn(pair.first, on);
      }
   }

   void followSliders(Kontroller::Device& device)
   {
      Kontroller::State state = device.getState();

      for (size_t i = 0; i < kGroupLEDs.size(); i += 3)
      {
         size_t group = i / 3;
         float val = state.groups[group].slider;

         bool s = val > 0.75f;
         bool m = val > 0.5f;
         bool r = val > 0.25f;

         device.setLEDOn(kGroupLEDs[i + 0], s);
         device.setLEDOn(kGroupLEDs[i + 1], m);
         device.setLEDOn(kGroupLEDs[i + 2], r);
      }
   }

   using DisplayFunc = void(*)(Kontroller::Device&);

   const std::array<DisplayFunc, 3> kDisplayFunctions
   {{
      doTheWave,
      explode,
      followSliders
   }};

   void clearLEDs(Kontroller::Device& device)
   {
      for (Kontroller::LED led : kGroupLEDs)
      {
         device.setLEDOn(led, false);
      }
   }
}

int main(int argc, char* argv[])
{
   Kontroller::Device device;

   device.setButtonCallback([](Kontroller::Button button, bool pressed)
   {
      printf("%s %s\n", Kontroller::getName(button), pressed ? "pressed" : "released");
   });

   device.setDialCallback([](Kontroller::Dial dial, float value)
   {
      printf("%s dial value: %f\n", Kontroller::getName(dial), value);
   });

   device.setSliderCallback([](Kontroller::Slider slider, float value)
   {
      printf("%s slider value: %f\n", Kontroller::getName(slider), value);
   });

   Kontroller::State state = device.getState();
   Kontroller::State current = state;
   Kontroller::State previous = state;
   size_t displayFuncIndex = 0;

   bool controlEnabled = false;
   device.enableLEDControl(controlEnabled);

   while (!state.stop)
   {
      if (device.isConnected())
      {
         if (state.cycle)
         {
            controlEnabled = !controlEnabled;

            if (!controlEnabled)
            {
               clearLEDs(device);
            }

            device.enableLEDControl(controlEnabled);
         }

         if (state.trackNext)
         {
            displayFuncIndex = (displayFuncIndex + 1) % kDisplayFunctions.size();
         }
         if (state.trackPrevious)
         {
            displayFuncIndex = displayFuncIndex == 0 ? kDisplayFunctions.size() - 1 : displayFuncIndex - 1;
         }

         if (controlEnabled)
         {
            kDisplayFunctions[displayFuncIndex](device);
         }
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(kSleepMillis));

      previous = current;
      current = device.getState();
      state = Kontroller::State::getOnlyNewButtons(previous, current);
   }

   clearLEDs(device);
   device.enableLEDControl(false);

   return 0;
}
