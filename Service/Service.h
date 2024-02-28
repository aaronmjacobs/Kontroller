#pragma once

#include "Kontroller/Server.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <memory>

namespace Kontroller
{
   class Service
   {
   public:
      Service();

      void onStart(SERVICE_STATUS_HANDLE handle);
      void onStop();
      void onShutdown();

   private:
      void updateServiceState(DWORD dwCurrentState);

      SERVICE_STATUS_HANDLE serviceStatusHandle = nullptr;
      SERVICE_STATUS serviceStatus = {};
      DWORD checkPoint = 0;

      std::unique_ptr<Server> server;
   };
}
