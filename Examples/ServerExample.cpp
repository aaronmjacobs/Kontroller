#include "Kontroller/Server.h"

int main(int argc, char* argv[])
{
   Kontroller::Server server(100, true);

   while (!server.getState().stop)
   {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }

   return 0;
}
