#include "Kontroller/Kontroller.h"
#include "Communicator.h"

#include <Windows.h>

#include <string>

#define KONTROLLER_ASSERT_RESULT(x) KONTROLLER_ASSERT((x) == MMSYSERR_NOERROR)

namespace {

std::array<uint8_t, 3> decode(DWORD_PTR value) {
   return {
      static_cast<uint8_t>((value & 0x0000FF) >> 0),
      static_cast<uint8_t>((value & 0x00FF00) >> 8),
      static_cast<uint8_t>((value & 0xFF0000) >> 16)
   };
}

// Called on a separate thread
void CALLBACK midiInputCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance,
                                DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
   if (wMsg != MIM_DATA && wMsg != MIM_MOREDATA) {
      return;
   }

   std::array<uint8_t, 3> values(decode(dwParam1));
   reinterpret_cast<Kontroller::Communicator*>(dwInstance)->onMessageReceived(values[1], values[2]);
}

struct DeviceIDs {
   static const unsigned int kInvalidID = static_cast<unsigned int>(-1);

   unsigned int inID { kInvalidID };
   unsigned int outID { kInvalidID };
};

DeviceIDs findIDs() {
   DeviceIDs ids;

   MIDIINCAPS inCapabilities;
   unsigned int numInDevices = midiInGetNumDevs();
   for (unsigned int i = 0; i < numInDevices; ++i) {
      if (midiInGetDevCaps(i, &inCapabilities, sizeof(MIDIINCAPS)) != MMSYSERR_NOERROR) {
         continue;
      }

      if (std::string(inCapabilities.szPname) != "nanoKONTROL2") {
         continue;
      }

      ids.inID = i;
      break;
   }

   MIDIOUTCAPS outCapabilities;
   unsigned int numOutDevices = midiOutGetNumDevs();
   for (unsigned int i = 0; i < numOutDevices; ++i) {
      if (midiOutGetDevCaps(i, &outCapabilities, sizeof(MIDIOUTCAPS)) != MMSYSERR_NOERROR) {
         continue;
      }

      if (std::string(outCapabilities.szPname) != "nanoKONTROL2") {
         continue;
      }

      ids.outID = i;
      break;
   }

   return ids;
}

} // namespace

struct Kontroller::Communicator::ImplData {
   HMIDIIN inHandle { nullptr };
   HMIDIOUT outHandle { nullptr };
};

Kontroller::Communicator::Communicator(Kontroller* kontroller)
   : implData(new ImplData), kontroller(kontroller) {
   DeviceIDs deviceIDs = findIDs();
   KONTROLLER_ASSERT(deviceIDs.inID != DeviceIDs::kInvalidID && deviceIDs.outID != DeviceIDs::kInvalidID);

   MMRESULT inOpenResult = midiInOpen(&implData->inHandle, deviceIDs.inID,
                                      reinterpret_cast<DWORD_PTR>(midiInputCallback),
                                      reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
   KONTROLLER_ASSERT_RESULT(inOpenResult);

   MMRESULT outOpenResult = midiOutOpen(&implData->outHandle, deviceIDs.outID, 0,
                                        reinterpret_cast<DWORD_PTR>(this), CALLBACK_NULL);
   KONTROLLER_ASSERT_RESULT(outOpenResult);

   MMRESULT inStartResult = midiInStart(implData->inHandle);
   KONTROLLER_ASSERT_RESULT(inStartResult);
}

Kontroller::Communicator::~Communicator() {
   if (implData->inHandle) {
      MMRESULT inStopResult = midiInStop(implData->inHandle);
      KONTROLLER_ASSERT_RESULT(inStopResult);

      MMRESULT inCloseResult = midiInClose(implData->inHandle);
      KONTROLLER_ASSERT_RESULT(inCloseResult);
   }

   if (implData->outHandle) {
      MMRESULT outCloseResult = midiOutClose(implData->outHandle);
      KONTROLLER_ASSERT_RESULT(outCloseResult);
   }
}

void Kontroller::Communicator::initializeMessage() {
}

void Kontroller::Communicator::appendToMessage(uint8_t* data, size_t numBytes) {
   MIDIHDR header { 0 };
   header.lpData = reinterpret_cast<LPSTR>(data);
   header.dwBufferLength = numBytes;

   MMRESULT prepareResult = midiOutPrepareHeader(implData->outHandle, &header, sizeof(header));
   KONTROLLER_ASSERT_RESULT(prepareResult);

   MMRESULT messageResult = midiOutLongMsg(implData->outHandle, &header, sizeof(header));
   KONTROLLER_ASSERT_RESULT(messageResult);

   MMRESULT unprepareResult = MIDIERR_STILLPLAYING;
   while (unprepareResult == MIDIERR_STILLPLAYING) {
      unprepareResult = midiOutUnprepareHeader(implData->outHandle, &header, sizeof(header));
   }
   KONTROLLER_ASSERT_RESULT(unprepareResult);
}

void Kontroller::Communicator::finalizeMessage() {
}
