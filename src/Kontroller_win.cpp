#include "Kontroller/Kontroller.h"

#include <cassert>
#include <mutex>
#include <string>

#include <Windows.h>

#if !defined(KONTROLLER_ASSERT)
#  define KONTROLLER_ASSERT assert
#endif

#define KONTROLLER_ASSERT_RESULT(x) KONTROLLER_ASSERT((x) == MMSYSERR_NOERROR)

#include "KontrollerIDs.inc"

namespace {

std::array<uint8_t, 3> decode(DWORD_PTR value) {
   return {
      static_cast<uint8_t>((value & 0x0000FF) >> 0),
      static_cast<uint8_t>((value & 0x00FF00) >> 8),
      static_cast<uint8_t>((value & 0xFF0000) >> 16)
   };
}

void CALLBACK midiInputCallback(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance,
                                DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
   if (wMsg != MIM_DATA && wMsg != MIM_MOREDATA) {
      return;
   }

   Kontroller *kontroller = reinterpret_cast<Kontroller*>(dwInstance);
   if (!kontroller) {
      return;
   }

   std::array<uint8_t, 3> values(decode(dwParam1));
   kontroller->update(values[1], values[2]);
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

template<size_t numBytes>
void send(HMIDIOUT outHandle, std::array<uint8_t, numBytes> data) {
   MIDIHDR header { 0 };
   header.lpData = reinterpret_cast<LPSTR>(data.data());
   header.dwBufferLength = data.size();

   MMRESULT prepareResult = midiOutPrepareHeader(outHandle, &header, sizeof(header));
   KONTROLLER_ASSERT_RESULT(prepareResult);

   MMRESULT messageResult = midiOutLongMsg(outHandle, &header, sizeof(header));
   KONTROLLER_ASSERT_RESULT(messageResult);

   MMRESULT unprepareResult = MIDIERR_STILLPLAYING;
   while (unprepareResult == MIDIERR_STILLPLAYING) {
      unprepareResult = midiOutUnprepareHeader(outHandle, &header, sizeof(header));
   }
   KONTROLLER_ASSERT_RESULT(unprepareResult);
}

void enableLEDControl(HMIDIOUT outHandle, bool enable) {
   send(outHandle, kStartSysex);
   send(outHandle, kSecondSysex);
   send(outHandle, kStartSysex);

   if (enable) {
      constexpr size_t size = kMainSysex.size();
      std::array<uint8_t, size> enableSysex(kMainSysex);
      enableSysex[kLEDModeOffset] = 0x01;
      send(outHandle, enableSysex);
   } else {
      send(outHandle, kMainSysex);
   }

   send(outHandle, kStartSysex);
   send(outHandle, kEndSysex);
}

void setLEDOn(HMIDIOUT outHandle, ControlID id, bool enable) {
   std::array<uint8_t, 3> data;
   data[0] = 0xB0;
   data[1] = id;
   data[2] = enable ? 0x7F : 0x00;

   send(outHandle, data);
}

} // namespace

struct Kontroller::ImplData {
   std::mutex mutex;

   HMIDIIN inHandle { nullptr };
   HMIDIOUT outHandle { nullptr };
};

Kontroller::Kontroller() {
   data = std::unique_ptr<ImplData>(new ImplData);

   DeviceIDs deviceIDs = findIDs();
   KONTROLLER_ASSERT(deviceIDs.inID != DeviceIDs::kInvalidID && deviceIDs.outID != DeviceIDs::kInvalidID);

   MMRESULT inOpenResult = midiInOpen(&data->inHandle, deviceIDs.inID, reinterpret_cast<DWORD_PTR>(midiInputCallback),
                                      reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
   KONTROLLER_ASSERT_RESULT(inOpenResult);

   MMRESULT outOpenResult = midiOutOpen(&data->outHandle, deviceIDs.outID, 0,
                                        reinterpret_cast<DWORD_PTR>(this), CALLBACK_NULL);
   KONTROLLER_ASSERT_RESULT(outOpenResult);

   MMRESULT inStartResult = midiInStart(data->inHandle);
   KONTROLLER_ASSERT_RESULT(inStartResult);
}

Kontroller::~Kontroller() {
   if (data->inHandle) {
      MMRESULT inStopResult = midiInStop(data->inHandle);
      KONTROLLER_ASSERT_RESULT(inStopResult);

      MMRESULT inCloseResult = midiInClose(data->inHandle);
      KONTROLLER_ASSERT_RESULT(inCloseResult);
   }

   if (data->outHandle) {
      MMRESULT outCloseResult = midiOutClose(data->outHandle);
      KONTROLLER_ASSERT_RESULT(outCloseResult);
   }
}

Kontroller::State Kontroller::getState() const {
   std::lock_guard<std::mutex> lock(data->mutex);
   return state;
}

void Kontroller::enableLEDControl(bool enable) {
   ::enableLEDControl(data->outHandle, enable);
}

void Kontroller::setLEDOn(Kontroller::LED led, bool on) {
   ::setLEDOn(data->outHandle, idForLED(led), on);
}

void Kontroller::update(uint8_t id, uint8_t value) {
   std::lock_guard<std::mutex> lock(data->mutex);

   float *floatVal = getFloatVal(state, id);
   if (floatVal) {
      *floatVal = value / 127.0f;
   } else {
      bool *boolVal = getBoolVal(state, id);
      KONTROLLER_ASSERT(boolVal); // If it isn't a dial or slider, it should be a button

      *boolVal = value != 0;
   }
}
