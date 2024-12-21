#pragma once

class Time
{
private:
  static double time;
  static double deltaTime;

  static int currentFrame;
  static int averageFPS[60];
  static int fps;

public:
  /**
   * DO NOT CALL THIS METHOD
   * This method is for internal implimentation
   */
  static void UpdateDeltaTime();

  /**
   * Get the current time since the window initialized
   */
  static const double GetTime();

  /**
   * Get the delta time for this loop
   */
  static const double GetDeltaTime();

  /**
   * Get the average FPS over the last 60 frames
   */
  static const int GetAverageFPS();
};