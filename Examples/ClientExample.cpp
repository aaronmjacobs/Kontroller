#include "Kontroller/Client.h"

int main(int argc, char* argv[])
{
   Kontroller::Client client;

   client.setButtonCallback([](Kontroller::Button button, bool pressed)
   {
      printf("%s %s\n", Kontroller::getName(button), pressed ? "pressed" : "released");
   });

   client.setDialCallback([](Kontroller::Dial dial, float value)
   {
      printf("%s dial value: %f\n", Kontroller::getName(dial), value);
   });

   client.setSliderCallback([](Kontroller::Slider slider, float value)
   {
      printf("%s slider value: %f\n", Kontroller::getName(slider), value);
   });

   while (!client.getState().stop)
   {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }
}
