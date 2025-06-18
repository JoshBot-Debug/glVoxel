#include "Time.h"

#include <GLFW/glfw3.h>
#include <iostream>

double Time::s_Time = 0.0f;
double Time::s_DeltaTime = 0.0f;

int Time::s_CurrentFrame = 0;
int Time::s_AverageFPS[60];
int Time::s_Fps = 0;

void Time::UpdateDeltaTime() {
  double time = glfwGetTime();
  Time::s_DeltaTime = time - Time::s_Time;
  Time::s_Time = time;

  if (Time::s_DeltaTime > 0) {
    int fps = (int)(1.0 / Time::s_DeltaTime);

    Time::s_AverageFPS[Time::s_CurrentFrame] = fps;

    int totalFPS = 0;
    for (int i = 0; i <= Time::s_CurrentFrame; i++)
      totalFPS += Time::s_AverageFPS[i];

    Time::s_Fps = totalFPS / (Time::s_CurrentFrame + 1);

    Time::s_CurrentFrame = (Time::s_CurrentFrame + 1) % 60;
  }
}

double Time::GetTime() { return glfwGetTime(); }

double Time::GetDeltaTime() { return Time::s_DeltaTime; }

int Time::GetAverageFPS() { return Time::s_Fps; }
