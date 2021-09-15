#include "Kontroller/Client.h"

int main(int argc, char* argv[])
{
   Kontroller::Client client;

   client.setButtonCallback([](Kontroller::Button button, bool pressed)
   {
      printf("%d pressed: %d\n", static_cast<int>(button), pressed);
   });

   client.setDialCallback([](Kontroller::Dial dial, float value)
   {
      printf("%d dial val: %f\n", static_cast<int>(dial), value);
   });

   client.setSliderCallback([](Kontroller::Slider slider, float value)
   {
      printf("%d slider val: %f\n", static_cast<int>(slider), value);
   });

   while (!client.getState().stop)
   {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }
}
