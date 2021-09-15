#include "Communicator.h"

#include <Windows.h>

#include <string>

namespace Kontroller
{
   namespace
   {
      std::array<uint8_t, 3> decode(DWORD_PTR value)
      {
         return
         {
            static_cast<uint8_t>((value & 0x0000FF) >> 0),
            static_cast<uint8_t>((value & 0x00FF00) >> 8),
            static_cast<uint8_t>((value & 0xFF0000) >> 16)
         };
      }

      // Potentially called on a separate thread
      void CALLBACK midiInputCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
      {
         Device::Communicator* communicator = reinterpret_cast<Device::Communicator*>(dwInstance);

         if (wMsg == MIM_DATA || wMsg == MIM_MOREDATA)
         {
            std::array<uint8_t, 3> values = decode(dwParam1);
            communicator->onMessageReceived(values[1], values[2]);
         }
         else if (wMsg == MIM_CLOSE)
         {
            communicator->onConnectionLost();
         }
      }

      struct DeviceIDs
      {
         static const UINT kInvalidID = static_cast<UINT>(-1);

         UINT inID = kInvalidID;
         UINT outID = kInvalidID;
      };

      DeviceIDs findIDs(const char* deviceName)
      {
         DeviceIDs ids;

         MIDIINCAPS inCapabilities;
         UINT numInDevices = midiInGetNumDevs();
         for (UINT i = 0; i < numInDevices; ++i)
         {
            if (midiInGetDevCaps(i, &inCapabilities, sizeof(MIDIINCAPS)) != MMSYSERR_NOERROR)
            {
               continue;
            }

            if (std::string(inCapabilities.szPname) != deviceName)
            {
               continue;
            }

            ids.inID = i;
            break;
         }

         MIDIOUTCAPS outCapabilities;
         UINT numOutDevices = midiOutGetNumDevs();
         for (UINT i = 0; i < numOutDevices; ++i)
         {
            if (midiOutGetDevCaps(i, &outCapabilities, sizeof(MIDIOUTCAPS)) != MMSYSERR_NOERROR)
            {
               continue;
            }

            if (std::string(outCapabilities.szPname) != deviceName)
            {
               continue;
            }

            ids.outID = i;
            break;
         }

         return ids;
      }
   }

   struct Device::Communicator::ImplData
   {
      HMIDIIN inHandle = nullptr;
      HMIDIOUT outHandle = nullptr;
   };

   Device::Communicator::Communicator(Device& owningDevice)
      : device(owningDevice)
      , implData(std::make_unique<ImplData>())
   {
   }

   Device::Communicator::~Communicator()
   {
      disconnect();
   }

   bool Device::Communicator::isConnected() const
   {
      return implData->inHandle && implData->outHandle;
   }

   bool Device::Communicator::connect()
   {
      if (isConnected())
      {
         return true;
      }

      bool success = false;
      do
      {
         DeviceIDs deviceIDs = findIDs(Device::kDeviceName);
         if (deviceIDs.inID == DeviceIDs::kInvalidID || deviceIDs.outID == DeviceIDs::kInvalidID)
         {
            break;
         }

         MMRESULT inOpenResult = midiInOpen(&implData->inHandle, deviceIDs.inID, reinterpret_cast<DWORD_PTR>(midiInputCallback), reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
         if (inOpenResult != MMSYSERR_NOERROR)
         {
            break;
         }

         MMRESULT outOpenResult = midiOutOpen(&implData->outHandle, deviceIDs.outID, 0, reinterpret_cast<DWORD_PTR>(this), CALLBACK_NULL);
         if (outOpenResult != MMSYSERR_NOERROR)
         {
            break;
         }

         MMRESULT inStartResult = midiInStart(implData->inHandle);
         if (inStartResult != MMSYSERR_NOERROR)
         {
            break;
         }

         success = true;
      } while (false);

      if (!success)
      {
         disconnect();
      }

      return success;
   }

   void Device::Communicator::disconnect()
   {
      if (implData->inHandle)
      {
         midiInStop(implData->inHandle);
         midiInClose(implData->inHandle);
         implData->inHandle = nullptr;
      }

      if (implData->outHandle)
      {
         midiOutClose(implData->outHandle);
         implData->outHandle = nullptr;
      }
   }

   void Device::Communicator::poll()
   {
      // Attempt to use the device, so that Windows will notify us if it is gone
      MIDIHDR header{};
      midiOutPrepareHeader(implData->outHandle, &header, sizeof(header));
      midiOutUnprepareHeader(implData->outHandle, &header, sizeof(header));

      // We can be notified about a lost connection any time a midi API call is made, so we'll check here to keep things up to date
      checkForLostConnection();
   }

   bool Device::Communicator::initializeMessage()
   {
      return true;
   }

   bool Device::Communicator::appendToMessage(uint8_t* data, size_t numBytes)
   {
      bool success = false;
      do
      {
         MIDIHDR header{};
         header.lpData = reinterpret_cast<LPSTR>(data);
         header.dwBufferLength = static_cast<DWORD>(numBytes);

         MMRESULT prepareResult = midiOutPrepareHeader(implData->outHandle, &header, sizeof(header));
         if (prepareResult != MMSYSERR_NOERROR)
         {
            break;
         }

         MMRESULT messageResult = midiOutLongMsg(implData->outHandle, &header, sizeof(header));
         if (messageResult != MMSYSERR_NOERROR)
         {
            break;
         }

         MMRESULT unprepareResult = MIDIERR_STILLPLAYING;
         while (unprepareResult == MIDIERR_STILLPLAYING)
         {
            unprepareResult = midiOutUnprepareHeader(implData->outHandle, &header, sizeof(header));
         }
         if (unprepareResult != MMSYSERR_NOERROR)
         {
            break;
         }

         success = true;
      } while (false);

      // We can be notified about a lost connection any time a midi API call is made, so we'll check here to keep things up to date
      checkForLostConnection();

      return success;
   }

   bool Device::Communicator::finalizeMessage()
   {
      return true;
   }
}
