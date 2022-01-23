#include "Kontroller/Server.h"

#include "Kontroller/Device.h"
#include "Kontroller/Packet.h"

#include "Sock.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

namespace Kontroller
{
   namespace
   {
      Sock::Socket createListenSocket(bool printErrors)
      {
         Sock::Socket listenSocket = Sock::kInvalidSocket;
         addrinfo* addrInfo = nullptr;
         bool success = false;

         do 
         {
            addrinfo hints = {};
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            hints.ai_flags = AI_PASSIVE;

            int addrInfoResult = Sock::getaddrinfo(nullptr, kPort, &hints, &addrInfo);
            if (addrInfoResult != 0 || !addrInfo)
            {
               if (printErrors)
               {
                  fprintf(stderr, "Kontroller::Server - getaddrinfo failed with error: %d\n", addrInfoResult);
               }
               break;
            }

            listenSocket = Sock::socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
            if (listenSocket == Sock::kInvalidSocket)
            {
               if (printErrors)
               {
                  fprintf(stderr, "Kontroller::Server - socket failed with error: %d\n", Sock::System::getLastError());
               }
               break;
            }

            unsigned long nonBlocking = 1;
            int ioctrlResult = Sock::ioctl(listenSocket, FIONBIO, &nonBlocking);
            if (ioctrlResult == Sock::kSocketError)
            {
               if (printErrors)
               {
                  fprintf(stderr, "Kontroller::Server - ioctl failed with error: %d\n", Sock::System::getLastError());
               }
               break;
            }

            int bindResult = Sock::bind(listenSocket, addrInfo->ai_addr, static_cast<socklen_t>(addrInfo->ai_addrlen));
            if (bindResult == Sock::kSocketError)
            {
               if (printErrors)
               {
                  fprintf(stderr, "Kontroller::Server - bind failed with error: %d\n", Sock::System::getLastError());
               }
               break;
            }

            int listenResult = Sock::listen(listenSocket, SOMAXCONN);
            if (listenResult == Sock::kSocketError)
            {
               if (printErrors)
               {
                  fprintf(stderr, "Kontroller::Server - listen failed with error: %d\n", Sock::System::getLastError());
               }
               break;
            }

            success = true;
         } while (0);

         if (addrInfo)
         {
            Sock::freeaddrinfo(addrInfo);
            addrInfo = nullptr;
         }

         if (listenSocket != Sock::kInvalidSocket && !success)
         {
            Sock::shutdown(listenSocket, Sock::ShutdownMethod::ReadWrite);
            Sock::close(listenSocket);
            listenSocket = Sock::kInvalidSocket;
         }

         return listenSocket;
      }

      uint64_t encodeSocket(Sock::Socket socket)
      {
         static_assert(sizeof(Sock::Socket) <= sizeof(uint64_t), "Socket can not fit in a uint64_t");

         uint64_t encodedSocket = 0;
         memcpy(&encodedSocket, &socket, sizeof(socket));

         return encodedSocket;
      }

      Sock::Socket decodeSocket(uint64_t encodedSocket)
      {
         Sock::Socket socket = Sock::kInvalidSocket;
         memcpy(&socket, &encodedSocket, sizeof(socket));

         return socket;
      }

      bool sendData(Sock::Socket socket, const uint8_t* data, size_t size)
      {
         size_t bytesWritten = 0;

         while (bytesWritten < size)
         {
            Sock::SignedResult result = Sock::send(socket, data + bytesWritten, static_cast<Sock::Length>(size - bytesWritten), 0);
            if (result == Sock::kSocketError)
            {
               // Connection lost
               return false;
            }

            assert(result > 0);
            bytesWritten += result;
         }

         return true;
      }

      bool sendPacket(Sock::Socket socket, EventPacket packet)
      {
         EventPacket networkPacket;
         networkPacket.type = Sock::Endian::hostToNetworkShort(packet.type);
         networkPacket.id = Sock::Endian::hostToNetworkShort(packet.id);
         networkPacket.value = Sock::Endian::hostToNetworkLong(packet.value);

         return sendData(socket, reinterpret_cast<const uint8_t*>(&networkPacket), sizeof(networkPacket));
      }

      bool sendButtonEvent(Sock::Socket socket, const Server::ButtonEvent& buttonEvent)
      {
         EventPacket packet;
         packet.type = EventPacket::Button;
         packet.id = static_cast<uint16_t>(buttonEvent.button);
         packet.value = static_cast<uint32_t>(buttonEvent.pressed);

         return sendPacket(socket, packet);
      }

      bool sendDialEvent(Sock::Socket socket, const Server::DialEvent& dialEvent)
      {
         EventPacket packet;
         packet.type = EventPacket::Dial;
         packet.id = static_cast<uint16_t>(dialEvent.dial);
         static_assert(sizeof(packet.value) == sizeof(dialEvent.value), "Packet data size does not match event data size");
         memcpy(&packet.value, &dialEvent.value, sizeof(packet.value));

         return sendPacket(socket, packet);
      }

      bool sendSliderEvent(Sock::Socket socket, const Server::SliderEvent& sliderEvent)
      {
         EventPacket packet;
         packet.type = EventPacket::Slider;
         packet.id = static_cast<uint16_t>(sliderEvent.slider);
         static_assert(sizeof(packet.value) == sizeof(sliderEvent.value), "Packet data size does not match event data size");
         memcpy(&packet.value, &sliderEvent.value, sizeof(packet.value));

         return sendPacket(socket, packet);
      }

      bool sendEvents(Sock::Socket socket, moodycamel::ReaderWriterQueue<Server::ButtonEvent>& buttonQueue, moodycamel::ReaderWriterQueue<Server::DialEvent>& dialQueue, moodycamel::ReaderWriterQueue<Server::SliderEvent>& sliderQueue)
      {
         bool success = true;

         Server::ButtonEvent buttonEvent;
         while (buttonQueue.try_dequeue(buttonEvent))
         {
            success = success && sendButtonEvent(socket, buttonEvent);
         }

         Server::DialEvent dialEvent;
         while (dialQueue.try_dequeue(dialEvent))
         {
            success = success && sendDialEvent(socket, dialEvent);
         }

         Server::SliderEvent sliderEvent;
         while (sliderQueue.try_dequeue(sliderEvent))
         {
            success = success && sendSliderEvent(socket, sliderEvent);
         }

         return success;
      }

      bool sendInitialEvents(Sock::Socket socket, const State& state)
      {
         std::vector<Server::ButtonEvent> buttonEvents;
         buttonEvents.reserve(35);
         buttonEvents.push_back({ Button::TrackPrevious, state.trackPrevious });
         buttonEvents.push_back({ Button::TrackNext, state.trackNext });
         buttonEvents.push_back({ Button::Cycle, state.cycle });
         buttonEvents.push_back({ Button::MarkerSet, state.markerSet });
         buttonEvents.push_back({ Button::MarkerPrevious, state.markerPrevious });
         buttonEvents.push_back({ Button::MarkerNext, state.markerNext });
         buttonEvents.push_back({ Button::Rewind, state.rewind });
         buttonEvents.push_back({ Button::FastForward, state.fastForward });
         buttonEvents.push_back({ Button::Stop, state.stop });
         buttonEvents.push_back({ Button::Play, state.play });
         buttonEvents.push_back({ Button::Record, state.record });
         buttonEvents.push_back({ Button::Group1Solo, state.groups[0].solo });
         buttonEvents.push_back({ Button::Group1Mute, state.groups[0].mute });
         buttonEvents.push_back({ Button::Group1Record, state.groups[0].record });
         buttonEvents.push_back({ Button::Group2Solo, state.groups[1].solo });
         buttonEvents.push_back({ Button::Group2Mute, state.groups[1].mute });
         buttonEvents.push_back({ Button::Group2Record, state.groups[1].record });
         buttonEvents.push_back({ Button::Group3Solo, state.groups[2].solo });
         buttonEvents.push_back({ Button::Group3Mute, state.groups[2].mute });
         buttonEvents.push_back({ Button::Group3Record, state.groups[2].record });
         buttonEvents.push_back({ Button::Group4Solo, state.groups[3].solo });
         buttonEvents.push_back({ Button::Group4Mute, state.groups[3].mute });
         buttonEvents.push_back({ Button::Group4Record, state.groups[3].record });
         buttonEvents.push_back({ Button::Group5Solo, state.groups[4].solo });
         buttonEvents.push_back({ Button::Group5Mute, state.groups[4].mute });
         buttonEvents.push_back({ Button::Group5Record, state.groups[4].record });
         buttonEvents.push_back({ Button::Group6Solo, state.groups[5].solo });
         buttonEvents.push_back({ Button::Group6Mute, state.groups[5].mute });
         buttonEvents.push_back({ Button::Group6Record, state.groups[5].record });
         buttonEvents.push_back({ Button::Group7Solo, state.groups[6].solo });
         buttonEvents.push_back({ Button::Group7Mute, state.groups[6].mute });
         buttonEvents.push_back({ Button::Group7Record, state.groups[6].record });
         buttonEvents.push_back({ Button::Group8Solo, state.groups[7].solo });
         buttonEvents.push_back({ Button::Group8Mute, state.groups[7].mute });
         buttonEvents.push_back({ Button::Group8Record, state.groups[7].record });

         std::vector<Server::DialEvent> dialEvents;
         dialEvents.reserve(8);
         dialEvents.push_back({ Dial::Group1, state.groups[0].dial });
         dialEvents.push_back({ Dial::Group2, state.groups[1].dial });
         dialEvents.push_back({ Dial::Group3, state.groups[2].dial });
         dialEvents.push_back({ Dial::Group4, state.groups[3].dial });
         dialEvents.push_back({ Dial::Group5, state.groups[4].dial });
         dialEvents.push_back({ Dial::Group6, state.groups[5].dial });
         dialEvents.push_back({ Dial::Group7, state.groups[6].dial });
         dialEvents.push_back({ Dial::Group8, state.groups[7].dial });

         std::vector<Server::SliderEvent> sliderEvents;
         sliderEvents.reserve(8);
         sliderEvents.push_back({ Slider::Group1, state.groups[0].slider });
         sliderEvents.push_back({ Slider::Group2, state.groups[1].slider });
         sliderEvents.push_back({ Slider::Group3, state.groups[2].slider });
         sliderEvents.push_back({ Slider::Group4, state.groups[3].slider });
         sliderEvents.push_back({ Slider::Group5, state.groups[4].slider });
         sliderEvents.push_back({ Slider::Group6, state.groups[5].slider });
         sliderEvents.push_back({ Slider::Group7, state.groups[6].slider });
         sliderEvents.push_back({ Slider::Group8, state.groups[7].slider });

         bool success = true;

         for (const Server::ButtonEvent& buttonEvent : buttonEvents)
         {
            success = success && sendButtonEvent(socket, buttonEvent);
         }

         for (const Server::DialEvent& dialEvent : dialEvents)
         {
            success = success && sendDialEvent(socket, dialEvent);
         }

         for (const Server::SliderEvent& sliderEvent : sliderEvents)
         {
            success = success && sendSliderEvent(socket, sliderEvent);
         }

         return success;
      }
   }

   Server::Server(bool printErrorMessages /*= false*/)
      : printErrors(printErrorMessages)
   {
      listenThread = std::thread([this]() { listen(); });
   }

   Server::~Server()
   {
      {
         std::lock_guard<std::mutex> lock(shutDownMutex);
         shuttingDown.store(true);
      }

      listenThread.join();
   }

   Kontroller::State Server::getState() const
   {
      std::lock_guard<std::mutex> lock(stateMutex);
      return state;
   }

   void Server::listen()
   {
      int initializeResult = -1;
      while (initializeResult != 0 && !shuttingDown.load())
      {
         initializeResult = Sock::System::initialize();

         if (initializeResult != 0)
         {
            if (printErrors)
            {
               fprintf(stderr, "Kontroller::Server - socket system startup failed with error: %d\n", initializeResult);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
         }
      }

      Sock::Socket listenSocket = Sock::kInvalidSocket;
      while (listenSocket == Sock::kInvalidSocket && !shuttingDown.load())
      {
         listenSocket = createListenSocket(printErrors);

         if (listenSocket == Sock::kInvalidSocket)
         {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
         }
      }

      if (!shuttingDown.load())
      {
         Device device;
         setCallbacks(device);

         listening.store(true);
         while (!shuttingDown.load())
         {
            // Wait until there is a socket ready to be accepted
            pollfd pollData = {};
            pollData.fd = listenSocket;
            pollData.events = POLLRDNORM;
            int pollResult = Sock::poll(&pollData, 1, 100); // 100ms timeout

            if (pollResult > 0 && (pollData.revents & POLLRDNORM) != 0 && !shuttingDown.load())
            {
               Sock::Socket clientSocket = Sock::accept(listenSocket, nullptr, nullptr);
               if (clientSocket == Sock::kInvalidSocket)
               {
                  if (printErrors)
                  {
                     fprintf(stderr, "Kontroller::Server - accept failed with error: %d\n", Sock::System::getLastError());
                  }
               }
               else
               {
                  std::lock_guard<std::mutex> lock(threadDataMutex);

                  ThreadData* data = threadData.emplace_back(std::make_unique<ThreadData>()).get();
                  data->encodedSocket = encodeSocket(clientSocket);
                  data->thread = std::thread([this, data]() { manageConnection(data); });
               }
            }

            pruneThreads();
         }
         listening.store(false);
      }

      {
         std::unique_lock<std::mutex> lock(threadDataMutex);

         while (!threadData.empty())
         {
            for (std::unique_ptr<ThreadData>& data : threadData)
            {
               data->cv.notify_all();
            }

            {
               lock.unlock();
               pruneThreads();
               lock.lock();
            }
         }
      }

      if (listenSocket != Sock::kInvalidSocket)
      {
         Sock::shutdown(listenSocket, Sock::ShutdownMethod::ReadWrite);
         Sock::close(listenSocket);
      }

      if (initializeResult == 0)
      {
         Sock::System::terminate();
      }
   }

   void Server::manageConnection(ThreadData* data)
   {
      assert(data != nullptr);

      Sock::Socket socket = decodeSocket(data->encodedSocket);

      int tcpNoDelay = 1;
      int optResult = Sock::setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &tcpNoDelay, sizeof(tcpNoDelay));
      if (optResult == Sock::kSocketError && printErrors)
      {
         fprintf(stderr, "Kontroller::Server - unable to disable the Nagle algorithm, connection may be jittery!\n");
      }

      if (sendInitialEvents(socket, getState()))
      {
         while (!shuttingDown.load())
         {
            {
               std::unique_lock<std::mutex> lock(data->eventMutex);
               data->cv.wait(lock, [this, data]()
               {
                  return data->eventPending.load() || shuttingDown.load();
               });

               data->eventPending.store(false);
            }

            if (!shuttingDown.load())
            {
               bool success = sendEvents(socket, data->buttonQueue, data->dialQueue, data->sliderQueue);
               if (!success)
               {
                  break;
               }
            }
         }
      }

      Sock::shutdown(socket, Sock::ShutdownMethod::ReadWrite);
      Sock::close(socket);

      data->complete.store(true);
   }

   void Server::pruneThreads()
   {
      std::unique_lock<std::mutex> lock(threadDataMutex);

      for (const std::unique_ptr<ThreadData>& data : threadData)
      {
         if (data->complete.load() && data->thread.joinable())
         {
            data->thread.join();
         }
      }

      threadData.erase(std::remove_if(threadData.begin(), threadData.end(), [](const std::unique_ptr<ThreadData>& data) { return data->complete.load() && !data->thread.joinable(); }), threadData.end());
   }

   void Server::setCallbacks(Device& device)
   {
      device.setButtonCallback([this, &device](Button button, bool pressed)
      {
         updateState(device);

         ButtonEvent buttonEvent;
         buttonEvent.button = button;
         buttonEvent.pressed = pressed;

         {
            std::lock_guard<std::mutex> lock(threadDataMutex);
            for (std::unique_ptr<ThreadData>& data : threadData)
            {
               data->buttonQueue.enqueue(buttonEvent);

               {
                  std::lock_guard<std::mutex> eventLock(data->eventMutex);
                  data->eventPending.store(true);
               }

               data->cv.notify_all();
            }
         }
      });

      device.setDialCallback([this, &device](Dial dial, float value)
      {
         updateState(device);

         DialEvent dialEvent;
         dialEvent.dial = dial;
         dialEvent.value = value;

         {
            std::lock_guard<std::mutex> lock(threadDataMutex);
            for (std::unique_ptr<ThreadData>& data : threadData)
            {
               data->dialQueue.enqueue(dialEvent);

               {
                  std::lock_guard<std::mutex> eventLock(data->eventMutex);
                  data->eventPending.store(true);
               }

               data->cv.notify_all();
            }
         }
      });

      device.setSliderCallback([this, &device](Slider slider, float value)
      {
         updateState(device);

         SliderEvent sliderEvent;
         sliderEvent.slider = slider;
         sliderEvent.value = value;

         {
            std::lock_guard<std::mutex> lock(threadDataMutex);
            for (std::unique_ptr<ThreadData>& data : threadData)
            {
               data->sliderQueue.enqueue(sliderEvent);

               {
                  std::lock_guard<std::mutex> eventLock(data->eventMutex);
                  data->eventPending.store(true);
               }

               data->cv.notify_all();
            }
         }
      });
   }

   void Server::updateState(Device& device)
   {
      std::lock_guard<std::mutex> lock(stateMutex);
      state = device.getState();
   }
}
