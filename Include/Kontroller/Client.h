#pragma once

#include "Kontroller/Packet.h"
#include "Kontroller/State.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

namespace Kontroller
{
   class Client
   {
   public:
      Client(const char* endpoint = "127.0.0.1", bool printErrorMessages = false);
      ~Client();

      State getState() const;

      bool isConnected() const
      {
         return connected.load();
      }

      using ButtonCallback = std::function<void(Button, bool)>;
      using DialCallback = std::function<void(Dial, float)>;
      using SliderCallback = std::function<void(Slider, float)>;

      void setButtonCallback(ButtonCallback callback);
      void clearButtonCallback();

      void setDialCallback(DialCallback callback);
      void clearDialCallback();

      void setSliderCallback(SliderCallback callback);
      void clearSliderCallback();

   private:
      void run(const char* endpoint);
      void updateState(const EventPacket& packet);

      const bool printErrors = false;

      State state;
      mutable std::mutex stateMutex;

      std::thread thread;
      std::condition_variable cv;
      std::mutex shutDownMutex;
      std::atomic_bool shuttingDown = { false };
      std::atomic_bool connected = { false };

      std::recursive_mutex callbackMutex;
      ButtonCallback buttonCallback;
      DialCallback dialCallback;
      SliderCallback sliderCallback;
   };
}
