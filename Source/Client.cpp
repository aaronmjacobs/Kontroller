#include "Kontroller/Client.h"

#include "Sock.h"

#include <cstdio>
#include <cstring>
#include <string>

namespace Kontroller
{
   namespace
   {
      enum class ReceiveResult
      {
         Success,
         Error,
         Timeout
      };

      Sock::Socket connect(const char* endpoint)
      {
         Sock::Socket socket = Sock::kInvalidSocket;
         addrinfo* addrInfo = nullptr;
         bool success = false;

         do 
         {
            addrinfo hints = {};
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;

            int addrInfoResult = Sock::getaddrinfo(endpoint, kPort, &hints, &addrInfo);
            if (addrInfoResult != 0 || !addrInfo)
            {
               fprintf(stderr, "getaddrinfo failed with error: %d\n", addrInfoResult);
               break;
            }

            socket = Sock::socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
            if (socket == Sock::kInvalidSocket)
            {
               fprintf(stderr, "socket failed with error: %d\n", Sock::System::getLastError());
               break;
            }

            unsigned long nonBlocking = 1;
            int ioctrlResult = Sock::ioctl(socket, FIONBIO, &nonBlocking);
            if (ioctrlResult == Sock::kSocketError)
            {
               fprintf(stderr, "ioctl failed with error: %d\n", Sock::System::getLastError());
               break;
            }

            int connectResult = Sock::connect(socket, addrInfo->ai_addr, static_cast<socklen_t>(addrInfo->ai_addrlen));
            if (connectResult == Sock::kSocketError)
            {
               int error = Sock::System::getLastError();
               if (error != Sock::NoError && error != Sock::WouldBlock && error != Sock::InProgress)
               {
                  fprintf(stderr, "connect failed with error: %d\n", Sock::System::getLastError());
                  break;
               }
            }

            success = true;
         } while (false);

         if (addrInfo)
         {
            Sock::freeaddrinfo(addrInfo);
            addrInfo = nullptr;
         }

         if (socket != Sock::kInvalidSocket && !success)
         {
            Sock::shutdown(socket, Sock::ShutdownMethod::ReadWrite);
            Sock::close(socket);
            socket = Sock::kInvalidSocket;
         }

         return socket;
      }

      bool receiveData(Sock::Socket socket, uint8_t* data, size_t size)
      {
         size_t bytesRead = 0;

         while (bytesRead < size)
         {
            Sock::SignedResult result = Sock::recv(socket, data + bytesRead, static_cast<Sock::Length>(size - bytesRead), 0);
            if (result <= 0)
            {
               // Connection lost
               fprintf(stderr, "recv failed with error: %d\n", Sock::System::getLastError());
               return false;
            }

            bytesRead += result;
         }

         return true;
      }

      ReceiveResult receivePacket(Sock::Socket socket, EventPacket& packet)
      {
         // Wait (with timeout) until there is data available
         fd_set fds = {};
         FD_ZERO(&fds);
         FD_SET(socket, &fds);
         timeval timeout = { 0, 100'000 }; // 100ms
         int selectResult = Sock::select(socket + 1, &fds, nullptr, nullptr, &timeout);
         if (selectResult < 0)
         {
            return ReceiveResult::Error;
         }
         else if (selectResult == 0)
         {
            return ReceiveResult::Timeout;
         }

         // Make sure there is at least one full packet's worth of data available
         EventPacket dummyPacket;
         Sock::SignedResult bytesReady = Sock::recv(socket, &dummyPacket, sizeof(dummyPacket), MSG_PEEK);
         if (bytesReady < 0)
         {
            int error = Sock::System::getLastError();
            if (error != Sock::WouldBlock)
            {
               return ReceiveResult::Error;
            }
         }
         else if (bytesReady == 0)
         {
            // Socket shut down by server
            return ReceiveResult::Error;
         }
         else if (bytesReady != sizeof(EventPacket))
         {
            return ReceiveResult::Timeout;
         }

         // Read the data
         EventPacket networkPacket;
         if (!receiveData(socket, reinterpret_cast<uint8_t*>(&networkPacket), sizeof(networkPacket)))
         {
            return ReceiveResult::Error;
         }

         // Translate from network byte order to host byte order
         packet.type = Sock::Endian::networkToHostShort(networkPacket.type);
         packet.id = Sock::Endian::networkToHostShort(networkPacket.id);
         packet.value = Sock::Endian::networkToHostLong(networkPacket.value);
         return ReceiveResult::Success;
      }
   }

   Client::Client(const char* endpoint /*= "127.0.0.1"*/)
   {
      thread = std::thread([this, endpointString = std::string(endpoint)]() { run(endpointString.c_str()); });
   }

   Client::~Client()
   {
      {
         std::lock_guard<std::mutex> lock(shutDownMutex);
         shuttingDown.store(true);
      }
      cv.notify_all();
      thread.join();
   }

   Kontroller::State Client::getState() const
   {
      std::lock_guard<std::mutex> lock(stateMutex);
      return state;
   }

   void Client::setButtonCallback(ButtonCallback callback)
   {
      std::lock_guard<std::recursive_mutex> lock(callbackMutex);
      buttonCallback = std::move(callback);
   }

   void Client::clearButtonCallback()
   {
      setButtonCallback({});
   }

   void Client::setDialCallback(DialCallback callback)
   {
      std::lock_guard<std::recursive_mutex> lock(callbackMutex);
      dialCallback = std::move(callback);
   }

   void Client::clearDialCallback()
   {
      setDialCallback({});
   }

   void Client::setSliderCallback(SliderCallback callback)
   {
      std::lock_guard<std::recursive_mutex> lock(callbackMutex);
      sliderCallback = std::move(callback);
   }

   void Client::clearSliderCallback()
   {
      setSliderCallback({});
   }

   void Client::run(const char* endpoint)
   {
      int initializeResult = Sock::System::initialize();
      if (initializeResult != 0)
      {
         fprintf(stderr, "Socket system startup failed with error: %d\n", initializeResult);
         return;
      }

      while (!shuttingDown.load())
      {
         Sock::Socket socket = connect(endpoint);
         if (socket == Sock::kInvalidSocket)
         {
            std::unique_lock<std::mutex> lock(shutDownMutex);
            cv.wait_for(lock, std::chrono::milliseconds(1000), [this]
            {
               return shuttingDown.load();
            });

            continue;
         }

         connected.store(true);

         while (!shuttingDown.load())
         {
            EventPacket packet;
            ReceiveResult result = receivePacket(socket, packet);

            if (result == ReceiveResult::Success)
            {
               updateState(packet);
            }
            else if (result == ReceiveResult::Error)
            {
               break;
            }
         }

         Sock::shutdown(socket, Sock::ShutdownMethod::ReadWrite);
         Sock::close(socket);

         connected.store(false);
      }

      Sock::System::terminate();
   }

   void Client::updateState(const EventPacket& packet)
   {
      static_assert(sizeof(packet.value) == sizeof(float), "Packet data size does not match event data size");

      bool boolValue = packet.value != 0;
      float floatValue = 0.0f;
      std::memcpy(&floatValue, &packet.value, sizeof(floatValue));

      {
         std::lock_guard<std::mutex> lock(stateMutex);

         switch (packet.type)
         {
         case EventPacket::Button:
            if (bool* buttonPointer = state.getButtonPointer(static_cast<Button>(packet.id)))
            {
               *buttonPointer = boolValue;
            }
            break;
         case EventPacket::Dial:
            if (float* dialPointer = state.getDialPointer(static_cast<Dial>(packet.id)))
            {
               *dialPointer = floatValue;
            }
            break;
         case EventPacket::Slider:
            if (float* sliderPointer = state.getSliderPointer(static_cast<Slider>(packet.id)))
            {
               *sliderPointer = floatValue;
            }
            break;
         default:
            break;
         }
      }

      {
         std::lock_guard<std::recursive_mutex> lock(callbackMutex);

         switch (packet.type)
         {
         case EventPacket::Button:
            if (buttonCallback)
            {
               buttonCallback(static_cast<Button>(packet.id), boolValue);
            }
            break;
         case EventPacket::Dial:
            if (dialCallback)
            {
               dialCallback(static_cast<Dial>(packet.id), floatValue);
            }
            break;
         case EventPacket::Slider:
            if (sliderCallback)
            {
               sliderCallback(static_cast<Slider>(packet.id), floatValue);
            }
            break;
         default:
            break;
         }
      }
   }
}
