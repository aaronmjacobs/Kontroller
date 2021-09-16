#include "Kontroller/Device.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <chrono>
#include <random>
#include <thread>

namespace
{
   constexpr size_t kScreenWidth = 8;
   constexpr size_t kScreenHeight = 3;

   constexpr size_t kLeft = 1;
   constexpr size_t kRight = 7;
   constexpr size_t kBottom = 0;
   constexpr size_t kTop = 2;

   struct Vec2
   {
      float x = 0.0f;
      float y = 0.0f;

      Vec2(float _x = 0.0f, float _y = 0.0f)
         : x(_x)
         , y(_y)
      {
      }

      void operator+=(const Vec2& other)
      {
         x += other.x;
         y += other.y;
      }

      void operator*=(const Vec2& other)
      {
         x *= other.x;
         y *= other.y;
      }

      void operator*=(float val)
      {
         x *= val;
         y *= val;
      }

      void operator/=(const Vec2& other)
      {
         x /= other.x;
         y /= other.y;
      }

      void operator/=(float val)
      {
         x /= val;
         y /= val;
      }
   };

   Vec2 operator*(const Vec2& vec, float val)
   {
      return Vec2(vec.x * val, vec.y * val);
   }

   class PongGame
   {
   public:
      PongGame(float initialSpeed);

      void reset();

      void tick(float dt, float playerOnePaddle, float playerTwoPaddle);
      void draw(std::array<std::array<bool, 3>, 8>& pixels);

   private:
      const float initialSpeed;
      Vec2 ballPos;
      Vec2 ballVel;
      int playerOneScore = 0;
      int playerTwoScore = 0;
      std::random_device randomDevice;
      std::uniform_real_distribution<float> randomDistribution;
   };

   PongGame::PongGame(float _initialSpeed)
      : initialSpeed(_initialSpeed)
      , randomDistribution(1.0f, 2.0f)
   {
      reset();
   }

   void PongGame::reset()
   {
      std::printf("Player 1: %d, Player 2: %d\n", playerOneScore, playerTwoScore);

      ballPos.x = (static_cast<float>(kLeft) + static_cast<float>(kRight)) / 2.0f;
      ballPos.y = (static_cast<float>(kBottom) + static_cast<float>(kTop)) / 2.0f;

      ballVel.x = randomDistribution(randomDevice);
      ballVel.y = randomDistribution(randomDevice);
      float magnitude = std::sqrt(ballVel.x * ballVel.x + ballVel.y * ballVel.y);
      ballVel /= magnitude;

      ballVel.x *= initialSpeed * (randomDistribution(randomDevice) > 1.5f ? 1.0f : -1.0f);
      ballVel.y *= initialSpeed * (randomDistribution(randomDevice) > 1.5f ? 1.0f : -1.0f);
   }

   void PongGame::tick(float dt, float playerOnePaddle, float playerTwoPaddle)
   {
      ballPos += ballVel * dt;

      static const float kHalfPaddleSize = 0.75f;

      float xCorrection = 0.0f;
      if (ballPos.x < kLeft)
      {
         float paddlePos = (1.0f - playerOnePaddle) * kBottom + playerOnePaddle * kTop;
         if (ballPos.y >= paddlePos - kHalfPaddleSize && ballPos.y <= paddlePos + kHalfPaddleSize)
         {
            xCorrection = 2.0f * (kLeft - ballPos.x);
         }
         else
         {
            ++playerTwoScore;
            reset();
         }
      }
      else if (ballPos.x > kRight)
      {
         float paddlePos = (1.0f - playerTwoPaddle) * kBottom + playerTwoPaddle * kTop;
         if (ballPos.y >= paddlePos - kHalfPaddleSize && ballPos.y <= paddlePos + kHalfPaddleSize)
         {
            xCorrection = 2.0f * (kRight - ballPos.x);
         }
         else
         {
            ++playerOneScore;
            reset();
         }
      }

      float yCorrection = 0.0f;
      if (ballPos.y < kBottom)
      {
         yCorrection = 2.0f * (kBottom - ballPos.y);
      }
      else if (ballPos.y > kTop)
      {
         yCorrection = 2.0f * (kTop - ballPos.y);
      }

      if (xCorrection != 0.0f)
      {
         ballVel.x *= -1.0f;
         ballPos.x += xCorrection;
      }

      if (yCorrection != 0.0f)
      {
         ballVel.y *= -1.0f;
         ballPos.y += yCorrection;
      }

      static const float kXSpeedup = 0.5f;
      static const float kYSpeedup = 0.01f;

      ballVel.x += kXSpeedup * dt * (ballVel.x >= 0.0f ? 1.0f : -1.0f);
      ballVel.y += kYSpeedup * dt * (ballVel.y >= 0.0f ? 1.0f : -1.0f);
   }

   void PongGame::draw(std::array<std::array<bool, kScreenHeight>, kScreenWidth>& pixels)
   {
      pixels = {};

      int ballX = (int)(ballPos.x + 0.5f);
      int ballY = (int)(ballPos.y + 0.5f);
      if ((ballX >= kLeft && ballX <= kRight) && (ballY >= kBottom && ballY <= kTop))
      {
         pixels[ballX][ballY] = true;
      }
   }

   bool waitForConnection(Kontroller::Device& device)
   {
      int numAttemptsLeft = 10;
      while (!device.isConnected() && --numAttemptsLeft > 0)
      {
         std::printf("Unable to connect to the KORG nanoKontrol2, trying %d more time%s\n", numAttemptsLeft, numAttemptsLeft > 1 ? "s" : "");
         std::this_thread::sleep_for(std::chrono::seconds(1));
      }

      return numAttemptsLeft > 0;
   }

   void draw(Kontroller::Device& device, const std::array<std::array<bool, kScreenHeight>, kScreenWidth>& pixels)
   {
      static const std::array<std::array<Kontroller::LED, 3>, 8> kLEDs
      { {
         Kontroller::LED::Group1Record, Kontroller::LED::Group1Mute, Kontroller::LED::Group1Solo,
         Kontroller::LED::Group2Record, Kontroller::LED::Group2Mute, Kontroller::LED::Group2Solo,
         Kontroller::LED::Group3Record, Kontroller::LED::Group3Mute, Kontroller::LED::Group3Solo,
         Kontroller::LED::Group4Record, Kontroller::LED::Group4Mute, Kontroller::LED::Group4Solo,
         Kontroller::LED::Group5Record, Kontroller::LED::Group5Mute, Kontroller::LED::Group5Solo,
         Kontroller::LED::Group6Record, Kontroller::LED::Group6Mute, Kontroller::LED::Group6Solo,
         Kontroller::LED::Group7Record, Kontroller::LED::Group7Mute, Kontroller::LED::Group7Solo,
         Kontroller::LED::Group8Record, Kontroller::LED::Group8Mute, Kontroller::LED::Group8Solo,
      } };

      for (size_t x = 0; x < pixels.size(); ++x)
      {
         for (size_t y = 0; y < pixels[x].size(); ++y)
         {
            device.setLEDOn(kLEDs[x][y], pixels[x][y]);
         }
      }
   }
}

int main(int argc, char* argv[])
{
   Kontroller::Device device;

   if (!waitForConnection(device))
   {
      return 0;
   }
   device.enableLEDControl(true);

   PongGame game(3.0f);
   std::array<std::array<bool, kScreenHeight>, kScreenWidth> pixels = {};

   std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();
   bool quit = false;
   while (!quit && device.isConnected())
   {
      std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
      double dt = std::chrono::duration_cast<std::chrono::duration<double>>(now - lastTime).count();
      lastTime = now;

      Kontroller::State state = device.getState();
      quit = state.stop;

      float leftPaddlePos = state.groups[0].slider;
      float rightPaddlePos = state.groups[7].slider;

      game.tick(static_cast<float>(dt), leftPaddlePos, rightPaddlePos);
      game.draw(pixels);

      draw(device, pixels);

      std::this_thread::sleep_for(std::chrono::milliseconds(16));
   }

   // Clear the LEDs
   if (device.isConnected())
   {
      pixels = {};
      draw(device, pixels);
      device.enableLEDControl(false);
   }

   return 0;
}
