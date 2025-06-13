#include <iostream>
#include "VoxelManager.h"
#include "Debug.h"
#include <future>
#include <mutex>
#include <noise/noiseutils.h>

#include <unordered_set>

VoxelManager::VoxelManager(int chunkSize, int chunkRadius, float worldStep) : chunkSize(chunkSize), chunkRadius(chunkRadius), worldStep(worldStep) {}

VoxelManager::~VoxelManager()
{
  for (Voxel *voxel : voxelPalette)
    delete voxel;
}

void VoxelManager::initialize(const glm::vec3 &position, HeightMap *heightMap)
{
  this->heightMap = heightMap;

  const std::vector<glm::ivec2> coords = getChunkPositionsInRadius(getChunkPosition(position));

  for (const auto &coord : coords)
    futures.push_back(std::async(std::launch::async, &VoxelManager::generateChunk, this, coord));

  std::thread([this, coords = coords, futures = std::move(futures)]() mutable
              {
                auto t1 = START_TIMER;
                for (auto &f : futures) f.get();

                for (size_t i = 0; i < coords.size(); i++)
                {
                  SparseVoxelOctree &tree = chunks[coords[i]];
                  tree.setNeighbours(coords[i], chunks);
                }

                futures.clear();

                for (const auto &coord : coords)
                  futures.push_back(std::async(std::launch::async, &VoxelManager::meshChunk, this, coord));
                
                for (auto &f : futures) f.get();

                std::cout << "Chunks: " << coords.size() << std::endl;
                std::cout << "Size: " << chunkSize << std::endl;
                END_TIMER(t1, "Chunks"); })

      .detach();
}

void VoxelManager::update(const glm::vec3 &position)
{
  const glm::ivec2 chunkPosition = getChunkPosition(position);
  const std::vector<glm::ivec2> coords = getChunkPositionsInRadius(chunkPosition);

  // std::cout << chunkPosition.x << " " << chunkPosition.y << std::endl;
  // for(const auto &coord : coords)
  // {
  //   std::cout << coord.x << " " << coord.y << std::endl;
  // }
  // const std::unordered_set<glm::ivec2, IVec2Hash, IVec2Equal> coordSet(coords.begin(), coords.end());

  // for (auto it = chunks.begin(); it != chunks.end();)
  // {
  //   if (coordSet.find(it->first) == coordSet.end())
  //   {
  //     std::cout << "Not found: " << it->first.x << " " << it->first.y << std::endl;
  //     // it = chunks.erase(it); // Not in radius, remove
  //     break;
  //   }
  //   else
  //     ++it;
  // }

  // for (const auto &coord : coords)
  //   futures.push_back(std::async(std::launch::async, &VoxelManager::generateChunk, this, coord));

  // std::thread([this, coords = coords, futures = std::move(futures)]() mutable
  //             {
  //               for (auto &f : futures) f.get();

  //               for (size_t i = 0; i < coords.size(); i++)
  //               {
  //                 SparseVoxelOctree &tree = chunks[coords[i]];
  //                 tree.setNeighbours(coords[i], chunks);
  //               }

  //               futures.clear();

  //               for (const auto &coord : coords)
  //                 futures.push_back(std::async(std::launch::async, &VoxelManager::meshChunk, this, coord)); })
  //     .detach();
}

const std::vector<glm::ivec2> VoxelManager::getChunkPositionsInRadius(const glm::ivec2 &center) const
{
  std::vector<glm::ivec2> result;

  for (int dz = -chunkRadius; dz < chunkRadius; ++dz)
    for (int dx = -chunkRadius; dx < chunkRadius; ++dx)
      result.emplace_back(center.x + dx, center.y + dz);

  return result;
}

const glm::ivec2 VoxelManager::getChunkPosition(const glm::vec3 &position) const
{
  return {static_cast<int>(std::floor(position.x / chunkSize)), static_cast<int>(std::floor(position.z / chunkSize))};
}

void VoxelManager::generateChunk(const glm::ivec2 &coord)
{
  auto [it, inserted] = chunks.try_emplace(coord, chunkSize);
  SparseVoxelOctree &tree = it->second;

  tree.clear();

  utils::NoiseMap map = heightMap->build(coord.x + 1.0f, coord.x + worldStep + 1.0f, coord.y + 1.0f, coord.y + worldStep + 1.0f);

  const int size = tree.getSize();
  int stoneLimit = static_cast<int>(size * heightMap->terrain.stoneThreshold);
  int dirtLimit = static_cast<int>(size * heightMap->terrain.dirtThreshold);
  int grassLimit = static_cast<int>(size * heightMap->terrain.grassThreshold);

  for (int z = 0; z < size; ++z)
    for (int x = 0; x < size; ++x)
    {
      float n = map.GetValue(x, z);
      unsigned int height = static_cast<unsigned int>(std::round((std::clamp(n, -1.0f, 1.0f) + 1) * (size / 2)));

      for (int y = 0; y < height; ++y)
        if (y < stoneLimit)
          tree.set(x, y, z, voxelPalette[VoxelPalette::STONE]);
        else if (y < dirtLimit)
          tree.set(x, y, z, voxelPalette[VoxelPalette::DIRT]);
        else if (y < grassLimit)
          tree.set(x, y, z, voxelPalette[VoxelPalette::GRASS]);
        else
          tree.set(x, y, z, voxelPalette[VoxelPalette::SNOW]);
    }
}

void VoxelManager::meshChunk(const glm::ivec2 &coord)
{
  SparseVoxelOctree &tree = chunks[coord];

  const std::vector<Voxel *> &filters = tree.getUniqueVoxels();

  std::vector<std::vector<Vertex>> tVertices;
  tVertices.resize(filters.size());

#pragma omp parallel for
  for (int i = 0; i < filters.size(); i++)
    tree.greedyMesh(tVertices[i], filters[i]);

  for (int i = 0; i < filters.size(); i++)
  {
    Voxel *filter = filters[i];
    std::vector<Vertex> &iv = tVertices[i];

    for (int j = 0; j < iv.size(); j++)
    {
      iv[j].x += static_cast<float>(coord.x * tree.getSize());
      iv[j].z += static_cast<float>(coord.y * tree.getSize());
      iv[j].color = filter->color;
      iv[j].material = filter->material;
    }

    {
      std::lock_guard<std::mutex> lock(verticesMutex);
      vertices.insert(vertices.end(),
                      std::make_move_iterator(iv.begin()),
                      std::make_move_iterator(iv.end()));
    }
  }
}