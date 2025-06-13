#include "Time.h"

#include <GLFW/glfw3.h>
#include <iostream>

double Time::time = 0.0f;
double Time::deltaTime = 0.0f;

int Time::currentFrame = 0;
int Time::averageFPS[60];
int Time::fps = 0;

void Time::UpdateDeltaTime() {
  double time = glfwGetTime();
  Time::deltaTime = time - Time::time;
  Time::time = time;

  if (Time::deltaTime > 0) {
    int fps = (int)(1.0 / Time::deltaTime);

    Time::averageFPS[Time::currentFrame] = fps;

    int totalFPS = 0;
    for (int i = 0; i <= Time::currentFrame; i++)
      totalFPS += Time::averageFPS[i];

    Time::fps = totalFPS / (Time::currentFrame + 1);

    Time::currentFrame = (Time::currentFrame + 1) % 60;
  }
}

const double Time::GetTime() { return glfwGetTime(); }

const double Time::GetDeltaTime() { return Time::deltaTime; }

const int Time::GetAverageFPS() { return Time::fps; }
