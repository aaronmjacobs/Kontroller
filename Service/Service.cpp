#include "Service.h"

#include <PlatformUtils/IOUtils.h>

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

      // Don't serialize state if the do_not_serialize_state.txt file exists
      Server::Settings settings;
      if (std::optional<std::filesystem::path> doNotSerializeStatePath = IOUtils::getAbsoluteCommonAppDataPath("Kontroller", "do_not_serialize_state.txt"))
      {
         if (std::optional<std::string> doNotSerializeStateContent = IOUtils::readTextFile(doNotSerializeStatePath.value()))
         {
            settings.serializeStateToFile = false;
         }
      }

      server = std::make_unique<Server>(settings);
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
