#include "World.h"
#include "Debug.h"

World::World() { voxels.setHeightMap(&heightMap); }

void World::initialize() {
  vao.generate();
  vbo.generate();
  heightMap.initialize();

  const std::vector<glm::ivec2> coords =
      voxels.getChunkPositionsInRadius(voxels.getChunkPosition({0, 0, 0}));

  for (const auto &coord : coords)
    futures.push_back(std::async(std::launch::async,
                                 &VoxelManager::generateChunk, &voxels, coord));

  std::thread([this, coords = coords, futures = std::move(futures)]() mutable {
    auto t1 = START_TIMER;
    for (auto &f : futures)
      f.get();

    for (size_t i = 0; i < coords.size(); i++) {
      SparseVoxelOctree &tree = voxels.getChunk(coords[i]);
      tree.setNeighbours(coords[i], voxels.getChunks());
    }

    futures.clear();

    for (const auto &coord : coords)
      futures.push_back(std::async(std::launch::async, &VoxelManager::meshChunk,
                                   &voxels, coord));

    for (auto &f : futures)
      f.get();

    std::cout << "Chunks: " << coords.size() << std::endl;
    std::cout << "Size: " << voxels.getChunkSize() << std::endl;
    END_TIMER(t1, "Chunks");
  })

      .detach();
}

void World::draw() {
  vao.bind();
  glDrawArrays(static_cast<GLenum>(drawMode), 0, voxels.vertices.size());
}

void World::update() {
  const std::vector<glm::ivec2> coords = voxels.getChunkPositionsInRadius(
      voxels.getChunkPosition(camera->position));
}

void World::setBuffer() {
  vao.bind();
  vbo.set(voxels.vertices);
  vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex),
          (void *)(offsetof(Vertex, x)));
  vao.set(1, 3, VertexType::FLOAT, false, sizeof(Vertex),
          (void *)(offsetof(Vertex, nx)));
  vao.set(2, 1, VertexType::INT, false, sizeof(Vertex),
          (void *)(offsetof(Vertex, color)));
  vao.set(3, 1, VertexType::INT, false, sizeof(Vertex),
          (void *)(offsetof(Vertex, material)));
}

void World::setRegistry(Registry *registry) { this->registry = registry; }

void World::setCamera(PerspectiveCamera *camera) { this->camera = camera; }