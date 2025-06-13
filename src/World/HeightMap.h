#pragma once

#include <noise/noise.h>
#include <noise/noiseutils.h>

struct TerrainProperties
{
  int seed = 50;
  int octaveCount = 3.0f;
  float frequency = 1.0f;
  float persistence = 0.4f;

  float scale = 0.9f;
  float bias = 0.0f;

  float stoneThreshold = 0.20f;
  float dirtThreshold = 0.30f;
  float grassThreshold = 0.5f;
};

class HeightMap
{
private:
  int width;
  int height;

  noise::module::Perlin perlin;
  noise::module::ScaleBias scaleBias;

public:
  TerrainProperties terrain;

public:
  HeightMap(int width, int height);

  void initialize();

  utils::NoiseMap build(double lowerXBound, double upperXBound, double lowerZBound, double upperZBound);
};
