#include "GreedyMesh64.h"

void GreedyMesh64::SetWidthHeight(uint8_t a, uint8_t b, uint64_t bits,
                                  uint64_t (&widthMasks)[],
                                  uint64_t (&heightMasks)[]) {
  // LOG_TO_FILE("bits-64", std::bitset<64>(bits));

  while (bits) {
    const uint8_t w = __builtin_ffsll(bits) - 1;

    const unsigned int wi = a + (CHUNK_SIZE * (w + (CHUNK_SIZE * b)));
    widthMasks[wi / CHUNK_SIZE] |= (1ULL << (wi % CHUNK_SIZE));

    const unsigned int hi = b + (CHUNK_SIZE * (w + (CHUNK_SIZE * a)));
    heightMasks[hi / CHUNK_SIZE] |= (1ULL << (hi % CHUNK_SIZE));

    bits = ClearLowestBits(bits, w + 1);
    // LOG_TO_FILE("64", (int)a, (int)w, (int)b, (int)wi, (int)w);
  }

  // for (size_t i = 0; i < MASK_LENGTH; i++) {
  //   if (widthMasks[i] != 0)
  //     LOG_TO_FILE("widthMasks-64", i, std::bitset<64>(widthMasks[i]));
  // }
}

void GreedyMesh64::PrepareWidthHeightMasks(
    const uint64_t (&bits)[], uint8_t paddingIndex, uint8_t (&padding)[],
    uint64_t (&widthStart)[], uint64_t (&heightStart)[], uint64_t (&widthEnd)[],
    uint64_t (&heightEnd)[]) {

  for (uint8_t a = 0; a < CHUNK_SIZE; a++)
    for (uint8_t b = 0; b < CHUNK_SIZE; b++) {

      unsigned int i = a + (CHUNK_SIZE * b);

      /**
       * Get the bitmask at index a,b
       * The padding mask has an extra bit as the LSB and MSB.
       * The MSB is the LSB of the pervious neighbour chunk
       * The LSB is the MSB of the next neighbour chunk
       * The first & last will always be a zero because there is no neighbour
       * next to them. 0...1 => 1...1 => 1...0
       */
      const uint8_t paddingMask = padding[i];

      /**
       * Shift right to remove the LSB padding bit and extract the following
       * 32bits into a new mask This is the actual mask we will use for the
       * height and width
       */
      const uint64_t mask = bits[i];

      /**
       * The first bit that is on on the left/top/front
       */
      const unsigned int msbIndex =
          (mask == 0) ? (CHUNK_SIZE - 1)
                      : (CHUNK_SIZE - 1) - __builtin_clzll(mask);

      /**
       * The first bit that is on on the right/bottom/back
       */
      const unsigned int lsbIndex = (mask == 0) ? 0 : __builtin_ctzll(mask);

      /**
       * Remove all the bits other than the start face
       * 11100111100011 => 00100000100001
       */
      uint64_t startMask = mask & ~(mask << 1);

      /**
       * Likewise remove all the bits other than the end face
       * 11100111100011 => 10000100000010
       */
      uint64_t endMask = mask & ~(mask >> 1);

      /**
       * Check the padding mask, if the bit at 0 index is on
       * turn off the MSB of the start mask
       *
       * If bit 0 of paddingMask is set, then clear bit 0 of startMask.
       * if ((paddingMask >> 0) & 1)
       *   startMask &= ~(1ULL << 0);
       */
      if ((paddingMask >> paddingIndex) & 1)
        startMask &= ~(1ULL << lsbIndex);

      /**
       * Check the padding mask, if the bit at 63 index is on
       * turn off the LSB of the end mask
       *
       * This is done in order to not set the height & width of the face at the
       * end of the chunk if the neighbour is the same To avoid creating faces
       * inbetween chunks
       *
       * If bit 63 of paddingMask is set, then clear bit 31 of endMask.
       * if ((paddingMask >> 63) & 1)
       *   endMask &= ~(1ULL << 31);
       */
      if ((paddingMask >> (paddingIndex + 1)) & 1)
        endMask &= ~(1ULL << msbIndex);

      SetWidthHeight(a, b, startMask, widthStart, heightStart);
      SetWidthHeight(a, b, endMask, widthEnd, heightEnd);
    }
}

void GreedyMesh64::GreedyMesh64Face(const glm::ivec3 &offsetPosition, uint8_t a,
                                    uint8_t b, uint64_t bits,
                                    uint64_t (&widthMasks)[],
                                    uint64_t (&heightMasks)[],
                                    std::vector<Vertex> &vertices,
                                    FaceType type) {

  // LOG_TO_FILE("64", std::bitset<64>(bits));

  // for (size_t i = 0; i < MASK_LENGTH; i++) {
  //   LOG_TO_FILE("64", std::bitset<64>(heightMasks[i]));
  // }

  while (bits) {
    const uint8_t w = __builtin_ffsll(bits) - 1;
    bits = ClearLowestBits(bits, w + 1);

    const uint64_t &width =
        ClearLowestBits(widthMasks[(w + (CHUNK_SIZE * a))], b);

    // LOG_TO_FILE("widthMasks-64",
    //             std::bitset<64>(widthMasks[(w + (CHUNK_SIZE * a))]));
    // LOG_TO_FILE("width-64", std::bitset<64>(width));

    if (!width)
      continue;

    const uint8_t widthOffset = __builtin_ffsll(width) - 1;

    uint8_t widthSize =
        ~width == 0 ? CHUNK_SIZE : __builtin_ctzll(~(width >> widthOffset));

    // LOG_TO_FILE("widthOffset-widthSize-64", (int)widthOffset,
    // (int)widthSize);

    const uint64_t &height =
        ClearLowestBits(heightMasks[w + (CHUNK_SIZE * (int)(widthOffset))], a);

    // LOG_TO_FILE("heightMasks-64", std::bitset<64>(heightMasks[(w +
    // (CHUNK_SIZE * (int)(widthOffset)))])); LOG_TO_FILE("height-64",
    // std::bitset<64>(height));

    const uint8_t heightOffset = __builtin_ffsll(height) - 1;

    uint8_t heightSize =
        ~height == 0 ? CHUNK_SIZE : __builtin_ctzll(~(height >> heightOffset));

    // LOG_TO_FILE("heightOffset-heightSize-64", (int)heightOffset,
    // (int)heightSize);

    for (uint8_t i = heightOffset; i < heightOffset + heightSize; i++) {
      const unsigned int index = w + (CHUNK_SIZE * i);

      const uint64_t widthSizeMask =
          (((widthSize >= CHUNK_SIZE ? 0ULL : (1ULL << widthSize)) - 1)
           << widthOffset);

      // LOG("i-widthSizeMask-64", std::bitset<64>(widthSizeMask));
      // LOG("i-widthMask-64", std::bitset<64>(widthMasks[index]));

      const uint64_t size = widthMasks[index] & widthSizeMask;

      // // LOG((int)widthSize, (int)widthOffset);

      if (size == 0 ||
          (size != ~0ULL && __builtin_ctzll(~(size >> (__builtin_ffsll(size) -
                                                       1))) != widthSize)) {
        heightSize = i - heightOffset;
        break;
      }

      // LOG_TO_FILE("i-widthSizeMask-64", std::bitset<64>(widthSizeMask));
      // LOG_TO_FILE("i-widthMask-64", std::bitset<64>(widthMasks[index]));
      // LOG(std::bitset<64>(size));

      widthMasks[index] &= ~widthSizeMask;

      // LOG(std::bitset<64>(~widthSizeMask));
    }

    switch (type) {
    case FaceType::TOP:
      Face::Top(vertices, widthOffset + (offsetPosition.x * CHUNK_SIZE),
                w + (offsetPosition.y * CHUNK_SIZE),
                a + (offsetPosition.z * CHUNK_SIZE), widthSize, 1.0f,
                heightSize);
      break;
    case FaceType::BOTTOM:
      Face::Bottom(vertices, widthOffset + (offsetPosition.x * CHUNK_SIZE),
                   w + (offsetPosition.y * CHUNK_SIZE),
                   a + (offsetPosition.z * CHUNK_SIZE), widthSize, 1.0f,
                   heightSize);
      break;

    case FaceType::LEFT:
      Face::Left(vertices, w + (offsetPosition.x * CHUNK_SIZE),
                 widthOffset + (offsetPosition.y * CHUNK_SIZE),
                 a + (offsetPosition.z * CHUNK_SIZE), 1.0f, widthSize,
                 heightSize);
      break;
    case FaceType::RIGHT:
      Face::Right(vertices, w + (offsetPosition.x * CHUNK_SIZE),
                  widthOffset + (offsetPosition.y * CHUNK_SIZE),
                  a + (offsetPosition.z * CHUNK_SIZE), 1.0f, widthSize,
                  heightSize);

      break;
    case FaceType::FRONT:
      Face::Front(vertices, a + (offsetPosition.x * CHUNK_SIZE),
                  widthOffset + (offsetPosition.y * CHUNK_SIZE),
                  w + (offsetPosition.z * CHUNK_SIZE), heightSize, widthSize,
                  1.0f);
      break;
    case FaceType::BACK:
      Face::Back(vertices, a + (offsetPosition.x * CHUNK_SIZE),
                 widthOffset + (offsetPosition.y * CHUNK_SIZE),
                 w + (offsetPosition.z * CHUNK_SIZE), heightSize, widthSize,
                 1.0f);
      break;
    }
  }
}

void GreedyMesh64::GreedyMesh64Axis(
    const glm::ivec3 &offsetPosition, const uint64_t (&bits)[],
    uint64_t (&widthStart)[], uint64_t (&heightStart)[], uint64_t (&widthEnd)[],
    uint64_t (&heightEnd)[], std::vector<Vertex> &vertices, FaceType startType,
    FaceType endType) {
  for (uint8_t a = 0; a < CHUNK_SIZE; a++)
    for (uint8_t b = 0; b < CHUNK_SIZE; b++) {
      const uint64_t mask = bits[b + (CHUNK_SIZE * a)];

      //  for (size_t i = 0; i < MASK_LENGTH; i++) {
      //     LOG_TO_FILE("64", std::bitset<64>(widthStart[i]));
      //   }

      GreedyMesh64Face(offsetPosition, a, b, mask & ~(mask << 1), widthStart,
                       heightStart, vertices, startType);
      GreedyMesh64Face(offsetPosition, a, b, mask & ~(mask >> 1), widthEnd,
                       heightEnd, vertices, endType);
    }
}

void GreedyMesh64::CullMesh(const glm::ivec3 &offsetPosition,
                            std::vector<Vertex> &vertices,
                            uint64_t (&columns)[], uint64_t (&rows)[],
                            uint64_t (&layers)[]) {
  for (uint8_t a = 0; a < CHUNK_SIZE; a++) {
    for (uint8_t b = 0; b < CHUNK_SIZE; b++) {
      uint64_t &column = columns[b + (CHUNK_SIZE * a)];
      uint64_t &row = rows[b + (CHUNK_SIZE * a)];
      uint64_t &depth = layers[b + (CHUNK_SIZE * a)];

      while (column) {
        const unsigned int offset = __builtin_ffsll(column) - 1;
        unsigned int size =
            __builtin_ctzll(~(column >> (__builtin_ffsll(column) - 1)));

        column &= ~((1ULL << (size + offset)) - 1);

        Face::Top(vertices, (offsetPosition.x * CHUNK_SIZE) + b,
                  (offsetPosition.y * CHUNK_SIZE) + offset,
                  (offsetPosition.z * CHUNK_SIZE) + a, 1.0f, size, 1.0f);
        Face::Bottom(vertices, (offsetPosition.x * CHUNK_SIZE) + b,
                     (offsetPosition.y * CHUNK_SIZE) + offset,
                     (offsetPosition.z * CHUNK_SIZE) + a, 1.0f, size, 1.0f);
      }

      while (row) {
        const unsigned int offset = __builtin_ffsll(row) - 1;
        unsigned int size =
            __builtin_ctzll(~(row >> (__builtin_ffsll(row) - 1)));

        row &= ~((1ULL << (size + offset)) - 1);

        Face::Left(vertices, (offsetPosition.x * CHUNK_SIZE) + offset,
                   (offsetPosition.y * CHUNK_SIZE) + b,
                   (offsetPosition.z * CHUNK_SIZE) + a, size, 1.0f, 1.0f);
        Face::Right(vertices, (offsetPosition.x * CHUNK_SIZE) + offset,
                    (offsetPosition.y * CHUNK_SIZE) + b,
                    (offsetPosition.z * CHUNK_SIZE) + a, size, 1.0f, 1.0f);
      }

      while (depth) {
        const unsigned int offset = __builtin_ffsll(depth) - 1;
        unsigned int size =
            __builtin_ctzll(~(depth >> (__builtin_ffsll(depth) - 1)));

        depth &= ~((1ULL << (size + offset)) - 1);

        Face::Front(vertices, (offsetPosition.x * CHUNK_SIZE) + a,
                    (offsetPosition.y * CHUNK_SIZE) + b,
                    (offsetPosition.z * CHUNK_SIZE) + offset, 1.0f, 1.0f, size);
        Face::Back(vertices, (offsetPosition.x * CHUNK_SIZE) + a,
                   (offsetPosition.y * CHUNK_SIZE) + b,
                   (offsetPosition.z * CHUNK_SIZE) + offset, 1.0f, 1.0f, size);
      }
    }
  }
}

void GreedyMesh64::Octree(SparseVoxelOctree *tree,
                          std::vector<Vertex> &vertices, int originX,
                          int originY, int originZ, int depth, Voxel *filter) {
  glm::vec3 coord = {originX / CHUNK_SIZE, originY / CHUNK_SIZE,
                     originZ / CHUNK_SIZE};

  /**
   * Generate the bit mask for rows, columns and layers.
   *
   * For columns:
   *
   *       At (x, z) you can get the height of the column in one bitwise
   * operation and you can get the location of all the top & bottom faces in one
   * bitwise operation y0 y1 y2 y3 z0 x0 0  0  0  0 z0 x1 0  0  0  0 z0 x2 0  0
   * 0  0 z0 x3 0  0  0  0 z1 x0 0  0  0  0 z1 x1 0  0  0  0 z1 x2 0  0  0  0 z1
   * x3 0  0  0  0
   */
  uint64_t rows[MASK_LENGTH] = {};
  uint64_t columns[MASK_LENGTH] = {};
  uint64_t layers[MASK_LENGTH] = {};
  uint8_t padding[MASK_LENGTH] = {};

  bool hasVoxels = false;

  for (int x = 0; x < CHUNK_SIZE; x++)
    for (int y = 0; y < CHUNK_SIZE; y++)
      for (int z = 0; z < CHUNK_SIZE; z++)
        if (tree->get(x + originX, y + originY, z + originZ, depth, filter)) {
          hasVoxels = true;

          const unsigned int rowIndex =
              x + (CHUNK_SIZE * (y + (CHUNK_SIZE * z)));
          const unsigned int columnIndex =
              y + (CHUNK_SIZE * (x + (CHUNK_SIZE * z)));
          const unsigned int layerIndex =
              z + (CHUNK_SIZE * (y + (CHUNK_SIZE * x)));

          rows[rowIndex / CHUNK_SIZE] |= (1ULL << (rowIndex % CHUNK_SIZE));
          columns[columnIndex / CHUNK_SIZE] |= (1ULL << (columnIndex % CHUNK_SIZE));
          layers[layerIndex / CHUNK_SIZE] |= (1ULL << (layerIndex % CHUNK_SIZE));
        }

  if (!hasVoxels)
    return;

      for (size_t i = 0; i < MASK_LENGTH; i++) {
    LOG_TO_FILE("columns-64", std::bitset<64>(columns[i]));
  }

  for (size_t i = 0; i < MASK_LENGTH; i++) {
    LOG_TO_FILE("rows-64", std::bitset<64>(rows[i]));
  }

  for (size_t i = 0; i < MASK_LENGTH; i++) {
    LOG_TO_FILE("layers-64", std::bitset<64>(layers[i]));
  }


  /**
   * Cull meshing, ~0.13ms slower than greedy meshing
   *
   * CullMesh(coord, vertices, columns, rows, layers, CHUNK_SIZE);
   * return;
   */

  /**
   * Here we capture the padding bit
   * Every chunk we need to get the neighbour chunks and check
   * if the bits at the 0 & 31st index are on. If so, we need to skip making
   * faces on that end.
   */
  for (int i = 0; i < MASK_LENGTH; i++) {
    uint64_t &row = rows[i];
    uint64_t &column = columns[i];
    uint64_t &layer = layers[i];

    int fast = i % CHUNK_SIZE;
    int slow = (i / CHUNK_SIZE) % CHUNK_SIZE;

    if (row > 0) {
      int rMSB = (CHUNK_SIZE - 1) - __builtin_clzll(row) + 1;
      int rLSB = __builtin_ctzll(row) - 1;

      if (tree->get(originX + rMSB, fast + originY, slow + originZ, depth))
        padding[i] |= (1ULL << 1);

      if (tree->get(originX + rLSB, fast + originY, slow + originZ, depth))
        padding[i] |= (1ULL << 0);
    }

    if (column > 0) {
      int cMSB = (CHUNK_SIZE - 1) - __builtin_clzll(column) + 1;
      int cLSB = __builtin_ctzll(column) - 1;

      if (tree->get(fast + originX, originY + cMSB, slow + originZ, depth))
        padding[i] |= (1ULL << 3);

      if (tree->get(fast + originX, originY + cLSB, slow + originZ, depth))
        padding[i] |= (1ULL << 2);
    }

    if (layer > 0) {
      int lMSB = (CHUNK_SIZE - 1) - __builtin_clzll(layer) + 1;
      int lLSB = __builtin_ctzll(layer) - 1;

      if (tree->get(slow + originX, fast + originY, originZ + lMSB, depth))
        padding[i] |= (1ULL << 5);

      if (tree->get(slow + originX, fast + originY, originZ + lLSB, depth))
        padding[i] |= (1ULL << 4);
    }
  }

  uint64_t widthStart[MASK_LENGTH] = {};
  uint64_t heightStart[MASK_LENGTH] = {};

  uint64_t widthEnd[MASK_LENGTH] = {};
  uint64_t heightEnd[MASK_LENGTH] = {};

  /**
   * Culls the column/row/layer
   * From: 001110110011111
   * To: 001010110010001
   *
   * Loop over the correct axis for each column/row/layer
   * for columns x,z
   * for rows    y,z
   * for layers  y,x
   *
   * Set the width and height of each face for both sides (start & end)
   * for columns bottom & top
   * for rows    left & right
   * for layers  front & back
   */

  //    for (size_t i = 0; i < MASK_LENGTH; i++)
  // {
  //   LOG_TO_FILE("columns-64", std::bitset<64>(columns[i]));
  // }

  PrepareWidthHeightMasks(rows, 0, padding, widthStart, heightStart, widthEnd,
                          heightEnd);

  // for (size_t i = 0; i < MASK_LENGTH; i++) {
  //   if (heightStart[i])
  //     LOG(std::bitset<64>(heightStart[i]));
  // }

  GreedyMesh64Axis(coord, rows, widthStart, heightStart, widthEnd, heightEnd,
                   vertices, FaceType::LEFT, FaceType::RIGHT);

  std::memset(widthStart, 0, sizeof(widthStart));
  std::memset(heightStart, 0, sizeof(heightStart));
  std::memset(widthEnd, 0, sizeof(widthEnd));
  std::memset(heightEnd, 0, sizeof(heightEnd));

  PrepareWidthHeightMasks(columns, 2, padding, widthStart, heightStart,
                          widthEnd, heightEnd);

  GreedyMesh64Axis(coord, columns, widthStart, heightStart, widthEnd, heightEnd,
                   vertices, FaceType::BOTTOM, FaceType::TOP);

  std::memset(widthStart, 0, sizeof(widthStart));
  std::memset(heightStart, 0, sizeof(heightStart));
  std::memset(widthEnd, 0, sizeof(widthEnd));
  std::memset(heightEnd, 0, sizeof(heightEnd));

  PrepareWidthHeightMasks(layers, 4, padding, widthStart, heightStart, widthEnd,
                          heightEnd);
  GreedyMesh64Axis(coord, layers, widthStart, heightStart, widthEnd, heightEnd,
                   vertices, FaceType::FRONT, FaceType::BACK);
}