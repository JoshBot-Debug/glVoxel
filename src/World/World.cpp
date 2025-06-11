#include "World.h"
#include "Debug.h"

World::World() : vbo(BufferTarget::ARRAY_BUFFER, VertexDraw::DYNAMIC)
{
  vao.generate();
  vbo.generate();
  initialize();
}

void World::initialize()
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

  heightMapBuilder.SetSourceModule(scaleBias);
  heightMapBuilder.SetDestNoiseMap(heightMap);
  heightMapBuilder.SetDestSize(terrain.destWidth, terrain.destHeight);
}

void World::setBuffer()
{
  vao.bind();
  vbo.set(vertices);
  vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, x)));
  vao.set(1, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, nx)));
  vao.set(2, 1, VertexType::INT, false, sizeof(Vertex), (void *)(offsetof(Vertex, color)));
  vao.set(3, 1, VertexType::INT, false, sizeof(Vertex), (void *)(offsetof(Vertex, material)));
}

void World::draw()
{
  vao.bind();
  glDrawArrays(static_cast<GLenum>(drawMode), 0, vertices.size());
}

void World::generateChunk(int worldX, int worldZ)
{
  tree.clear();

  heightMapBuilder.SetBounds(worldX + 1.0f, worldX + 2.0f, worldZ + 1.0f, worldZ + 2.0f);
  heightMapBuilder.Build();

  auto tt = START_TIMER;
  auto t2 = START_TIMER;

  const int size = tree.getSize();
  int stoneLimit = static_cast<int>(size * terrain.stoneThreshold);
  int dirtLimit = static_cast<int>(size * terrain.dirtThreshold);
  int grassLimit = static_cast<int>(size * terrain.grassThreshold);

#pragma omp parallel for collapse(2)
  for (int z = 0; z < size; ++z)
    for (int x = 0; x < size; ++x)
    {
      float n = heightMap.GetValue(x, z);
      unsigned int height = static_cast<unsigned int>(std::round((std::clamp(n, -1.0f, 1.0f) + 1) * (size / 2)));

      for (size_t y = 0; y < height; y++)
      {
        Voxel *voxel = new Voxel();

        if (y < stoneLimit)
        {
          voxel->setColor(45, 45, 45, 255);
        }
        else if (y < dirtLimit)
        {
          voxel->setColor(101, 67, 33, 255);
        }
        else if (y < grassLimit)
        {
          voxel->setColor(34, 139, 34, 255);
        }
        else
        {
          voxel->setColor(255, 255, 255, 255);
        }
        tree.set(x, y, z, voxel);
      }
    }

  END_TIMER(t2, "tree.set()");

  auto t3 = START_TIMER;

  std::vector<Voxel> filters = tree.getUniqueVoxels();

  std::vector<std::vector<Vertex>> tVertices;
  tVertices.resize(filters.size());

#pragma omp parallel for
  for (int i = 0; i < filters.size(); i++)
    tree.greedyMesh(tVertices[i], &filters[i]);

  for (int i = 0; i < filters.size(); i++)
  {
    Voxel *filter = &filters[i];
    std::vector<Vertex> &iv = tVertices[i];

    for (int j = 0; j < iv.size(); j++)
    {
      iv[j].x += static_cast<float>(worldX * tree.getSize());
      iv[j].z += static_cast<float>(worldZ * tree.getSize());
      iv[j].color = filter->color;
      iv[j].material = filter->material;
    }

    vertices.insert(vertices.end(),
                    std::make_move_iterator(iv.begin()),
                    std::make_move_iterator(iv.end()));
  }

  END_TIMER(t3, "tree.greedyMesh()");
  END_TIMER(tt, "Total Time");
  std::cout << "Voxels (Million): " << (double)(size * size * size) / 1000000.0 << std::endl;
  std::cout << "Memory (MB): " << (double)tree.getTotalMemoryUsage() / 1000000.0 << std::endl;
}

void World::fill()
{
  tree.clear();
  vertices.clear();

  const int size = tree.getSize();

  auto t1 = START_TIMER;
  for (size_t x = 0; x < size; x++)
    for (size_t y = 0; y < size; y++)
      for (size_t z = 0; z < size; z++)
        tree.set(x, y, z, new Voxel());

  END_TIMER(t1, "tree.set()");

  auto t2 = START_TIMER;
  tree.greedyMesh(vertices);
  END_TIMER(t1, "tree.greedyMesh()");

  setBuffer();
}

void World::fillSphere()
{
  tree.clear();
  vertices.clear();

  const unsigned int size = tree.getSize();
  const glm::ivec3 center(size / 2);
  int radius = std::min({center.x, center.y, center.z}) - 1.0f;

  auto t1 = START_TIMER;

  for (int x = 0; x < size; x++)
    for (int y = 0; y < size; y++)
      for (int z = 0; z < size; z++)
      {
        int dx = x - center.x;
        int dy = y - center.y;
        int dz = z - center.z;

        int distance = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (distance <= radius)
          tree.set(x, y, z, new Voxel());
      }

  END_TIMER(t1, "tree.set()");

  auto t2 = START_TIMER;
  tree.greedyMesh(vertices);
  END_TIMER(t1, "tree.greedyMesh()");

  setBuffer();
}

void World::setCamera(PerspectiveCamera *camera)
{
  this->camera = camera;
}
