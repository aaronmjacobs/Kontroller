#include "Service.h"

#include <chrono>
#include <string>
#include <thread>

namespace
{
   const char* kServiceName = "KontrollerServer";
   const char* kServiceDisplayName = "Kontroller Server";
   const char* kServiceDescription = "Communicates with the KORG nanoKONTROL2 and hosts a socket server, allowing multiple outside processes to obtain MIDI data.";
   const DWORD kServiceStartType = SERVICE_AUTO_START;
   const char* kServiceDependencies = "";
   const char* kServiceAccount = "NT AUTHORITY\\LocalService";
   const char* kServicePassword = nullptr;

   bool installService(LPCSTR lpServiceName, LPCSTR lpDisplayName, LPCSTR lpDescription, DWORD dwStartType, LPCSTR lpDependencies, LPCSTR lpAccount, LPCSTR lpPassword)
   {
      char path[MAX_PATH];
      if (GetModuleFileName(nullptr, path, ARRAYSIZE(path)) == 0)
      {
         fprintf(stderr, "GetModuleFileName failed with error: 0x%08lx\n", GetLastError());
         return false;
      }

      // Open the local default service control manager database
      SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
      if (schSCManager == nullptr)
      {
         fprintf(stderr, "OpenSCManager failed with error: 0x%08lx\n", GetLastError());
         return false;
      }

      // Install the service into SCM by calling CreateService
      DWORD access = SERVICE_QUERY_STATUS | SERVICE_CHANGE_CONFIG | SERVICE_START;
      SC_HANDLE schService = CreateService(
         schSCManager,                   // SCManager database
         lpServiceName,                  // Name of service
         lpDisplayName,                  // Name to display
         access,                         // Desired access
         SERVICE_WIN32_OWN_PROCESS,      // Service type
         dwStartType,                    // Service start type
         SERVICE_ERROR_NORMAL,           // Error control type
         path,                           // Service's binary
         nullptr,                        // No load ordering group
         nullptr,                        // No tag identifier
         lpDependencies,                 // Dependencies
         lpAccount,                      // Service running account
         lpPassword                      // Password of the account
      );
      if (schService == nullptr)
      {
         fprintf(stderr, "CreateService failed with error 0x%08lx\n", GetLastError());

         CloseServiceHandle(schSCManager);
         return false;
      }

      // Set the service description
      SERVICE_DESCRIPTION serviceDescription = {};
      std::string description = lpDescription;
      serviceDescription.lpDescription = description.data();
      if (!ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &serviceDescription))
      {
         fprintf(stderr, "ChangeServiceConfig2 failed with error: 0x%08lx\n", GetLastError());
      }

      printf("%s has been installed\n", lpServiceName);

      if (StartService(schService, 0, nullptr))
      {
         printf("%s has been started\n", lpServiceName);
      }
      else
      {
         fprintf(stderr, "StartService failed with error 0x%08lx\n", GetLastError());
      }

      CloseServiceHandle(schService);
      CloseServiceHandle(schSCManager);

      return true;
   }

   bool uninstallService(LPCSTR lpServiceName)
   {
      // Open the local default service control manager database
      SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
      if (schSCManager == nullptr)
      {
         fprintf(stderr, "OpenSCManager failed with error: 0x%08lx\n", GetLastError());
         return false;
      }

      // Open the service with delete, stop, and query status permissions
      SC_HANDLE schService = OpenService(schSCManager, lpServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
      if (schService == nullptr)
      {
         fprintf(stderr, "OpenService failed with error: 0x%08lx\n", GetLastError());

         CloseServiceHandle(schSCManager);
         return false;
      }

      // Try to stop the service
      SERVICE_STATUS ssSvcStatus = {};
      if (ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus))
      {
         printf("Stopping %s", lpServiceName);
         std::this_thread::sleep_for(std::chrono::milliseconds(250));

         while (QueryServiceStatus(schService, &ssSvcStatus))
         {
            if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING)
            {
               printf(".");
               std::this_thread::sleep_for(std::chrono::milliseconds(250));
            }
            else
            {
               break;
            }
         }

         if (ssSvcStatus.dwCurrentState == SERVICE_STOPPED)
         {
            printf("\n%s has been stopped\n", lpServiceName);
         }
         else
         {
            printf("\n%s failed to stop\n", lpServiceName);
         }
      }

      // Now remove the service by calling DeleteService.
      if (!DeleteService(schService))
      {
         fprintf(stderr, "DeleteService failed with error: 0x%08lx\n", GetLastError());

         CloseServiceHandle(schService);
         CloseServiceHandle(schSCManager);
         return false;
      }

      printf("%s has been uninstalled\n", lpServiceName);

      CloseServiceHandle(schService);
      CloseServiceHandle(schSCManager);

      return true;
   }

   Kontroller::Service service;

   void WINAPI serviceControlHandler(DWORD dwCtrl)
   {
      switch (dwCtrl)
      {
      case SERVICE_CONTROL_STOP:
         service.onStop();
         break;
      case SERVICE_CONTROL_PAUSE:
         break;
      case SERVICE_CONTROL_CONTINUE:
         break;
      case SERVICE_CONTROL_SHUTDOWN:
         service.onShutdown();
         break;
      case SERVICE_CONTROL_INTERROGATE:
         break;
      default:
         break;
      }
   }

   VOID WINAPI serviceMain(DWORD dwArgc, LPSTR* lpszArgv)
   {
      SERVICE_STATUS_HANDLE handle = RegisterServiceCtrlHandler(kServiceName, serviceControlHandler);
      if (!handle)
      {
         fprintf(stderr, "RegisterServiceCtrlHandler failed\n");
         return;
      }

      service.onStart(handle);
   }
}

int main(int argc, char* argv[])
{
   if (argc > 1)
   {
      bool success = true;
      std::string command(argv[1]);
      if (command == "install")
      {
         success = installService(kServiceName, kServiceDisplayName, kServiceDescription, kServiceStartType, kServiceDependencies, kServiceAccount, kServicePassword);
      }
      else if (command == "uninstall")
      {
         success = uninstallService(kServiceName);
      }
      else
      {
         printf("Usage: %s [install | uninstall]\n", argv[0]);
      }

      return success ? 0 : 1;
   }

   std::string serviceName = kServiceName;
   SERVICE_TABLE_ENTRY serviceTable[] =
   {
      { serviceName.data(), serviceMain },
      { nullptr, nullptr }
   };

   bool started = StartServiceCtrlDispatcher(serviceTable) != 0;
   return started ? 0 : 1;
}
