#include "Kontroller/Client.h"

int main(int argc, char* argv[])
{
   const char* endpoint = argc > 1 ? argv[1] : "127.0.0.1";
   Kontroller::Client client(endpoint);

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

   return 0;
}
