#include "HeightMap.h"
#include <chrono>

HeightMap::HeightMap(int width, int height) : width(width), height(height) {}

void HeightMap::initialize()
{
  if (terrain.seed == 0)
    perlin.SetSeed(static_cast<int>(std::time(0)));
  else
    perlin.SetSeed(terrain.seed);
  perlin.SetFrequency(terrain.frequency);
  perlin.SetPersistence(terrain.persistence);
  perlin.SetOctaveCount(terrain.octaveCount);

  scaleBias.SetSourceModule(0, perlin);
  scaleBias.SetScale(terrain.scale);
  scaleBias.SetBias(terrain.bias);
}

utils::NoiseMap HeightMap::build(double lowerXBound, double upperXBound, double lowerZBound, double upperZBound)
{
  utils::NoiseMap heightMap;
  utils::NoiseMapBuilderPlane heightMapBuilder;

  heightMapBuilder.SetSourceModule(scaleBias);
  heightMapBuilder.SetDestNoiseMap(heightMap);
  heightMapBuilder.SetDestSize(width, height);
  heightMapBuilder.SetBounds(lowerXBound, upperXBound, lowerZBound, upperZBound);
  heightMapBuilder.Build();

  return heightMap;
}