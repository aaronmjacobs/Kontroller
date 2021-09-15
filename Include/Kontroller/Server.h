#pragma once

#include "Kontroller/State.h"

#include <readerwriterqueue.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace Kontroller
{
   class Device;

   class Server
   {
   public:
      Server();
      ~Server();

      State getState() const;

      struct ButtonEvent
      {
         Button button = Button::None;
         bool pressed = false;
      };

      struct DialEvent
      {
         Dial dial = Dial::None;
         float value = 0.0f;
      };

      struct SliderEvent
      {
         Slider slider = Slider::None;
         float value = 0.0f;
      };

   private:
      struct ThreadData
      {
         uint64_t encodedSocket = 0;
         std::atomic_bool complete = { false };

         std::thread thread;
         std::condition_variable cv;
         std::mutex eventMutex;
         std::atomic_bool eventPending = { false };

         moodycamel::ReaderWriterQueue<ButtonEvent> buttonQueue;
         moodycamel::ReaderWriterQueue<DialEvent> dialQueue;
         moodycamel::ReaderWriterQueue<SliderEvent> sliderQueue;

         static inline void* operator new(std::size_t size)
         {
#if defined(_MSC_VER)
            return _aligned_malloc(size, MOODYCAMEL_CACHE_LINE_SIZE);
#elif defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
            void* result = nullptr;
            posix_memalign(&result, MOODYCAMEL_CACHE_LINE_SIZE, size);
            return result;
#else
            return std::aligned_alloc(MOODYCAMEL_CACHE_LINE_SIZE, size);
#endif
         }

         static inline void operator delete(void* ptr)
         {
#if defined(_MSC_VER)
            _aligned_free(ptr);
#else
            free(ptr);
#endif
         }
      };

      void listen();
      void manageConnection(ThreadData* data);
      void pruneThreads();

      void setCallbacks(Device& device);
      void clearCallbacks(Device& device);

      void updateState(Device& device);

      State state;
      mutable std::mutex stateMutex;

      std::mutex shutDownMutex;
      std::atomic_bool shuttingDown = { false };

      std::thread listenThread;

      std::vector<std::unique_ptr<ThreadData>> threadData;
      std::mutex threadDataMutex;
   };
}
