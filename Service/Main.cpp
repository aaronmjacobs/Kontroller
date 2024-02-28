#include "Service.h"

#include <PlatformUtils/IOUtils.h>
#include <PlatformUtils/OSUtils.h>

#include <chrono>
#include <string>
#include <thread>

#include <shellapi.h>

namespace
{
   const wchar_t* kServiceName = L"KontrollerServer";
   const wchar_t* kServiceDisplayName = L"Kontroller Server";
   const wchar_t* kServiceDescription = L"Communicates with the KORG nanoKONTROL2 and hosts a socket server, allowing multiple outside processes to obtain MIDI data.";
   const DWORD kServiceStartType = SERVICE_AUTO_START;
   const wchar_t* kServiceDependencies = L"";
   const wchar_t* kServiceAccount = L"NT AUTHORITY\\LocalService";
   const wchar_t* kServicePassword = nullptr;

   void printError(const char* name)
   {
      fprintf(stderr, "%s failed with error: 0x%08lx\n", name, GetLastError());
   }

   bool isElevated()
   {
      bool elevated = false;

      HANDLE hToken = nullptr;
      if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
      {
         TOKEN_ELEVATION elevation;
         DWORD cbSize = sizeof(TOKEN_ELEVATION);
         if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &cbSize))
         {
            elevated = elevation.TokenIsElevated;
         }
      }

      if (hToken)
      {
         CloseHandle(hToken);
      }

      return elevated;
   }

   void runElevated(const WCHAR* params)
   {
      if (std::optional<std::filesystem::path> exePath = OSUtils::getExecutablePath())
      {
         std::wstring paramsString = L"/k " + exePath->wstring() + L" " + params;

         SHELLEXECUTEINFOW shExInfo{};
         shExInfo.cbSize = sizeof(shExInfo);
         shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
         shExInfo.lpVerb = L"runas";
         shExInfo.lpFile = L"cmd";
         shExInfo.lpParameters = paramsString.c_str();
         shExInfo.nShow = SW_SHOW;

         if (ShellExecuteExW(&shExInfo) && shExInfo.hProcess != 0)
         {
            CloseHandle(shExInfo.hProcess);
         }
      }
   }

   std::optional<std::filesystem::path> getInstalledExePath()
   {
      return IOUtils::getAbsoluteKnownPath(OSUtils::KnownDirectory::CommonApplications, std::filesystem::path(KONTROLLER_PROJECT_NAME) / SERVICE_PROJECT_NAME ".exe");
   }

   bool installService(LPCWSTR lpServiceName, LPCWSTR lpDisplayName, LPCWSTR lpDescription, DWORD dwStartType, LPCWSTR lpDependencies, LPCWSTR lpAccount, LPCWSTR lpPassword)
   {
      std::optional<std::filesystem::path> exePath = OSUtils::getExecutablePath();
      if (!exePath)
      {
         fprintf(stderr, "Failed to determine executable path\n");
         return false;
      }

      std::filesystem::path exeName = exePath->filename();
      if (exeName.empty())
      {
         fprintf(stderr, "Failed to determine executable name\n");
         return false;
      }

      std::optional<std::filesystem::path> installedExePath = getInstalledExePath();
      if (!installedExePath)
      {
         fprintf(stderr, "Failed to determine installed executable path\n");
         return false;
      }

      if (*exePath != *installedExePath)
      {
         std::error_code errorCode;

         std::filesystem::create_directories(installedExePath->parent_path(), errorCode);
         if (errorCode)
         {
            fprintf(stderr, "Failed to create Program Files subdirectory: %s\n", errorCode.message().c_str());
            return false;
         }

         std::filesystem::copy_file(*exePath, *installedExePath, errorCode);
         if (errorCode)
         {
            fprintf(stderr, "Failed to copy executable to Program Files: %s\n", errorCode.message().c_str());
            return false;
         }
      }

      // Open the local default service control manager database
      SC_HANDLE schSCManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
      if (schSCManager == nullptr)
      {
         printError("OpenSCManager");
         return false;
      }

      // Install the service into SCM by calling CreateService
      DWORD access = SERVICE_QUERY_STATUS | SERVICE_CHANGE_CONFIG | SERVICE_START;
      SC_HANDLE schService = CreateServiceW(
         schSCManager,                   // SCManager database
         lpServiceName,                  // Name of service
         lpDisplayName,                  // Name to display
         access,                         // Desired access
         SERVICE_WIN32_OWN_PROCESS,      // Service type
         dwStartType,                    // Service start type
         SERVICE_ERROR_NORMAL,           // Error control type
         installedExePath->c_str(),      // Service's binary
         nullptr,                        // No load ordering group
         nullptr,                        // No tag identifier
         lpDependencies,                 // Dependencies
         lpAccount,                      // Service running account
         lpPassword                      // Password of the account
      );
      if (schService == nullptr)
      {
         printError("CreateService");

         CloseServiceHandle(schSCManager);
         return false;
      }

      // Set the service description
      SERVICE_DESCRIPTIONW serviceDescription = {};
      std::wstring description = lpDescription;
      serviceDescription.lpDescription = description.data();
      if (!ChangeServiceConfig2W(schService, SERVICE_CONFIG_DESCRIPTION, &serviceDescription))
      {
         printError("ChangeServiceConfig2");
      }

      printf("%ls has been installed to %ls\n", lpServiceName, installedExePath->c_str());

      if (StartServiceW(schService, 0, nullptr))
      {
         printf("%ls has been started\n", lpServiceName);
      }
      else
      {
         printError("StartService");
      }

      CloseServiceHandle(schService);
      CloseServiceHandle(schSCManager);

      return true;
   }

   bool uninstallService(LPCWSTR lpServiceName)
   {
      // Open the local default service control manager database
      SC_HANDLE schSCManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
      if (schSCManager == nullptr)
      {
         printError("OpenSCManager");
         return false;
      }

      // Open the service with delete, stop, and query status permissions
      SC_HANDLE schService = OpenServiceW(schSCManager, lpServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
      if (schService == nullptr)
      {
         printError("OpenService");

         CloseServiceHandle(schSCManager);
         return false;
      }

      // Try to stop the service
      SERVICE_STATUS ssSvcStatus = {};
      if (ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus))
      {
         printf("Stopping %ls", lpServiceName);
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
            printf("\n%ls has been stopped\n", lpServiceName);
         }
         else
         {
            printf("\n%ls failed to stop\n", lpServiceName);

            CloseServiceHandle(schService);
            CloseServiceHandle(schSCManager);
            return false;
         }
      }
      else
      {
         printf("%ls failed to stop\n", lpServiceName);

         CloseServiceHandle(schService);
         CloseServiceHandle(schSCManager);
         return false;
      }

      // Now remove the service by calling DeleteService.
      if (!DeleteService(schService))
      {
         printError("DeleteService");

         CloseServiceHandle(schService);
         CloseServiceHandle(schSCManager);
         return false;
      }

      printf("%ls has been uninstalled\n", lpServiceName);

      CloseServiceHandle(schService);
      CloseServiceHandle(schSCManager);

      return true;
   }

   bool deleteServiceExecutable()
   {
      std::optional<std::filesystem::path> installedExePath = getInstalledExePath();
      if (!installedExePath)
      {
         fprintf(stderr, "Failed to determine installed executable path\n");
         return false;
      }

      if (std::filesystem::exists(*installedExePath))
      {
         std::error_code errorCode;
         std::filesystem::remove(*installedExePath, errorCode);
         if (errorCode)
         {
            fprintf(stderr, "Failed to delete installed executable: %s\n", errorCode.message().c_str());
            return false;
         }

         printf("%ls has been deleted\n", installedExePath->c_str());

         std::filesystem::path installedExeDirectory = installedExePath->parent_path();
         if (!installedExeDirectory.empty() && std::filesystem::is_directory(installedExeDirectory))
         {
            std::filesystem::remove(installedExeDirectory, errorCode);
            if (errorCode)
            {
               fprintf(stderr, "Failed to delete installed executable directory: %s\n", errorCode.message().c_str());
               return false;
            }

            printf("%ls has been deleted\n", installedExeDirectory.c_str());
         }
      }

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

   VOID WINAPI serviceMain(DWORD dwArgc, LPWSTR* lpszArgv)
   {
      SERVICE_STATUS_HANDLE handle = RegisterServiceCtrlHandlerW(kServiceName, serviceControlHandler);
      if (!handle)
      {
         printError("RegisterServiceCtrlHandler");
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
         if (isElevated())
         {
            success = installService(kServiceName, kServiceDisplayName, kServiceDescription, kServiceStartType, kServiceDependencies, kServiceAccount, kServicePassword);
         }
         else
         {
            runElevated(L"install");
         }
      }
      else if (command == "uninstall")
      {
         bool deleteExecutable = argc > 2 && std::strcmp("/d", argv[2]) == 0;
         if (isElevated())
         {
            success = uninstallService(kServiceName);
            if (deleteExecutable)
            {
               success &= deleteServiceExecutable();
            }
         }
         else
         {
            runElevated(deleteExecutable ? L"uninstall /d" : L"uninstall");
         }
      }
      else
      {
         printf("Usage: %s [install | uninstall [/d]]\n", argv[0]);
      }

      return success ? 0 : 1;
   }

   std::wstring serviceName = kServiceName;
   SERVICE_TABLE_ENTRYW serviceTable[] =
   {
      { serviceName.data(), serviceMain },
      { nullptr, nullptr }
   };

   bool started = StartServiceCtrlDispatcherW(serviceTable) != 0;
   return started ? 0 : 1;
}
