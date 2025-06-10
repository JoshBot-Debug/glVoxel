#include "World.h"
#include "Debug.h"

#include <noise/noise.h>
#include <noise/noiseutils.h>

World::World() : vbo(BufferTarget::ARRAY_BUFFER, VertexDraw::DYNAMIC)
{
  vao.generate();
  vbo.generate();

  unsigned char colors[256 * 4]; // 256 colors * RGBA (4 bytes per pixel)

  for (int i = 0; i < 256; ++i)
  {
    colors[i * 4 + 0] = i;
    colors[i * 4 + 1] = 0;
    colors[i * 4 + 2] = 255 - i;
    colors[i * 4 + 3] = 255;
  }

  colorPalette.generateTexture();
  colorPalette.bind();
  colorPalette.setFilter(TextureFilter::NEAREST, TextureFilter::NEAREST);
  colorPalette.setWrap(TextureWrap::CLAMP_TO_EDGE, TextureWrap::CLAMP_TO_EDGE, TextureWrap::CLAMP_TO_EDGE);
  colorPalette.setData(colors);
  colorPalette.setWidth(256);
  colorPalette.setHeight(1);
  colorPalette.setTexture(0, GL_RGBA8, GL_RGBA);
  colorPalette.unbind();
}

void World::draw(Shader &shader)
{
  colorPalette.bind(0);
  shader.setUniform1i("colorPalette", 0);

  vao.bind();
  glDrawArrays(static_cast<GLenum>(drawMode), 0, vertices.size());
}

void World::setBuffer()
{
  vao.bind();
  vbo.set(vertices);
  vao.set(0, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, x)));
  vao.set(1, 3, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, nx)));
  vao.set(2, 1, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, color)));
  vao.set(3, 1, VertexType::FLOAT, false, sizeof(Vertex), (void *)(offsetof(Vertex, material)));
}

const Voxel::SparseVoxelOctree &World::getTree() const
{
  return tree;
}

void World::generateTerrain()
{
  tree.clear();
  vertices.clear();

  auto t1 = START_TIMER;

  noise::module::Perlin perlin;
  terrain.maxHeight = tree.getSize();
  if (terrain.seed == 0)
    perlin.SetSeed(static_cast<int>(std::time(0)));
  else
    perlin.SetSeed(terrain.seed);
  perlin.SetFrequency(terrain.frequency);
  perlin.SetPersistence(terrain.persistence);
  perlin.SetOctaveCount(terrain.octaveCount);

  noise::module::ScaleBias scaleBias;
  scaleBias.SetSourceModule(0, perlin);
  scaleBias.SetScale(terrain.scale);
  scaleBias.SetBias(terrain.bias);

  utils::NoiseMap heightMap;
  utils::NoiseMapBuilderPlane heightMapBuilder;

  heightMapBuilder.SetSourceModule(scaleBias);
  heightMapBuilder.SetDestNoiseMap(heightMap);
  heightMapBuilder.SetDestSize(terrain.destWidth, terrain.destHeight);
  heightMapBuilder.SetBounds(terrain.lowerXBound, terrain.upperXBound, terrain.lowerZBound, terrain.upperZBound);
  heightMapBuilder.Build();

  END_TIMER(t1, "Heightmap generation");

  auto tt = START_TIMER;
  auto t2 = START_TIMER;

  const int size = tree.getSize();
  int stoneLimit = static_cast<int>(size * terrain.stoneThreshold);
  int dirtLimit = static_cast<int>(size * terrain.dirtThreshold);
  int grassLimit = static_cast<int>(size * terrain.grassThreshold);

  // #pragma omp parallel for collapse(2)
  for (int z = 0; z < size; ++z)
    for (int x = 0; x < size; ++x)
    {
      float n = heightMap.GetValue(x, z);
      unsigned int height = static_cast<unsigned int>(std::round((std::clamp(n, -1.0f, 1.0f) + 1) * (size / 2)));

      for (size_t y = 0; y < height; y++)
      {
        int color;

        if (y < stoneLimit)
        {
          color = 3;
        }
        else if (y < dirtLimit)
        {
          color = 2;
        }
        else if (y < grassLimit)
        {
          color = 1;
        }
        else
        {
          color = 4;
        }
        tree.set(x, y, z, color);
      }
    }

  END_TIMER(t2, "tree.set()");

  auto t3 = START_TIMER;

  const int colors = 4;
  std::vector<Voxel::Voxel *> filters;
  filters.reserve(colors);

  for (int i = 1; i <= colors; ++i)
    filters.push_back(new Voxel::Voxel{i, 0});

  std::vector<std::vector<Vertex>> tVertices;
  tVertices.resize(filters.size());

#pragma omp parallel for
  for (int i = 0; i < filters.size(); i++)
    tree.greedyMesh(tVertices[i], filters[i]);

  for (int i = 0; i < filters.size(); i++)
  {
    Voxel::Voxel *filter = filters[i];
    std::vector<Vertex> &iv = tVertices[i];

    for (int j = 0; j < iv.size(); j++)
    {
      iv[j].color = filter->color;
      iv[j].material = filter->material;
    }

    vertices.insert(vertices.end(),
                    std::make_move_iterator(iv.begin()),
                    std::make_move_iterator(iv.end()));

    delete filters[i];
    filters[i] = nullptr;
  }

  END_TIMER(t3, "tree.greedyMesh()");

  END_TIMER(tt, "Total Time");

  std::cout << "Voxels (Million): " << (double)(size * size * size) / 1000000.0 << std::endl;
  std::cout << "Memory (MB): " << (double)tree.getTotalMemoryUsage() / 1000000.0 << std::endl;

  setBuffer();
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
        tree.set(x, y, z, 1);

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
          tree.set(x, y, z, 1);
      }

  END_TIMER(t1, "tree.set()");

  auto t2 = START_TIMER;
  tree.greedyMesh(vertices);
  END_TIMER(t1, "tree.greedyMesh()");

  setBuffer();
}