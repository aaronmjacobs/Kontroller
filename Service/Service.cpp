#include "Service.h"

#include <cassert>

namespace Kontroller
{
   Service::Service()
   {
      serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
      serviceStatus.dwCurrentState = SERVICE_START_PENDING;
      serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
      serviceStatus.dwWin32ExitCode = NO_ERROR;
      serviceStatus.dwServiceSpecificExitCode = 0;
      serviceStatus.dwCheckPoint = checkPoint++;
      serviceStatus.dwWaitHint = 0;
   }

   void Service::onStart(SERVICE_STATUS_HANDLE handle)
   {
      assert(handle != nullptr && serviceStatusHandle == nullptr && !server);

      serviceStatusHandle = handle;
      updateServiceState(SERVICE_RUNNING);

      server = std::make_unique<Server>();
   }

   void Service::onStop()
   {
      updateServiceState(SERVICE_STOP_PENDING);

      server.reset();

      updateServiceState(SERVICE_STOPPED);
   }

   void Service::onShutdown()
   {
      if (server)
      {
         onStop();
      }

      serviceStatusHandle = nullptr;
   }

   void Service::updateServiceState(DWORD dwCurrentState)
   {
      serviceStatus.dwCurrentState = dwCurrentState;
      serviceStatus.dwCheckPoint = (dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED) ? 0 : checkPoint++;

      SetServiceStatus(serviceStatusHandle, &serviceStatus);
   }
}
