#include "Sock.h"

#include <cstdio>

namespace Kontroller
{
   namespace Sock
   {
      namespace Helpers
      {
         Result poll(Socket socket, short events, int timeoutMS, const char* caller /*= nullptr*/, bool printErrors /*= false*/)
         {
            pollfd pollData = {};
            pollData.fd = socket;
            pollData.events = events;

            int numSet = Sock::poll(&pollData, 1, timeoutMS);

            if (numSet < 0)
            {
               if (printErrors)
               {
                  fprintf(stderr, "%s - poll failed with error: %d\n", caller, Sock::System::getLastError());
               }
               return Result::Error;
            }

            if (numSet == 0)
            {
               return Result::Timeout;
            }

            if (!(pollData.revents & pollData.events) || (pollData.revents & (POLLERR | POLLHUP | POLLNVAL)))
            {
               if (printErrors)
               {
                  fprintf(stderr, "%s - poll failed with events: 0x%X\n", caller, pollData.revents);
               }
               return Result::Error;
            }

            return Result::Success;
         }
      }
   }
}
