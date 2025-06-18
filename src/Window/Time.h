#pragma once

class Time {
private:
  static double s_Time;
  static double s_DeltaTime;

  static int s_CurrentFrame;
  static int s_AverageFPS[60];
  static int s_Fps;

public:
  /**
   * DO NOT CALL THIS METHOD
   * This method is for internal implimentation
   */
  static void UpdateDeltaTime();

  /**
   * Get the current time since the window initialized
   */
  static double GetTime();

  /**
   * Get the delta time for this loop
   */
  static double GetDeltaTime();

  /**
   * Get the average FPS over the last 60 frames
   */
  static int GetAverageFPS();
};