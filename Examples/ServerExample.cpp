#include "Kontroller/Server.h"

int main(int argc, char* argv[])
{
   Kontroller::Server::Settings settings;
   settings.printErrorMessages = true;

   Kontroller::Server server(settings);

   while (!server.getState().stop)
   {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }

   return 0;
}
