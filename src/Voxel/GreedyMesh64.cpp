#include "GreedyMesh64.h"

void GreedyMesh64::SetWidthHeight(uint8_t a, uint8_t b, uint64_t bits,
                                  uint64_t (&widthMasks)[],
                                  uint64_t (&heightMasks)[],
                                  unsigned int chunkSize) {
  while (bits) {
    const uint8_t w = __builtin_ffsll(bits) - 1;

    const unsigned int wi = a + (chunkSize * (w + (chunkSize * b)));
    widthMasks[wi / chunkSize] |= (1ULL << (wi % chunkSize));

    const unsigned int hi = b + (chunkSize * (w + (chunkSize * a)));
    heightMasks[hi / chunkSize] |= (1ULL << (hi % chunkSize));

    bits = ClearLowestBits(bits, w + 1);
  }
}

void GreedyMesh64::PrepareWidthHeightMasks(
    const uint64_t (&bits)[], uint8_t paddingIndex, uint8_t (&padding)[],
    uint64_t (&widthStart)[], uint64_t (&heightStart)[], uint64_t (&widthEnd)[],
    uint64_t (&heightEnd)[], unsigned int chunkSize) {
  for (uint8_t a = 0; a < chunkSize; a++)
    for (uint8_t b = 0; b < chunkSize; b++) {
      unsigned int i = (chunkSize * (a + (chunkSize * b))) / chunkSize;

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
      // const uint32_t mask = (paddingMask >> 1) & 0xFFFFFFFF;
      // const uint32_t mask = (paddingMask >> 1) & 0xFFFFFFFFFFFFFFFFULL;
      // const uint64_t mask = paddingMask & 0xFFFFFFFFFFFFFFFFULL;
      const uint64_t mask = bits[i];

      /**
       * The first bit that is on on the left/top/front
       */
      const unsigned int msbIndex =
          (mask == 0) ? (CHUNK_SIZE - 1) : (CHUNK_SIZE - 1) - __builtin_clzll(mask);

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
      // if ((paddingMask >> paddingIndex) & 1)
      //   startMask &= ~(1ULL << lsbIndex);

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
      // if ((paddingMask >> (paddingIndex + 1)) & 1)
      //   endMask &= ~(1ULL << msbIndex);

      SetWidthHeight(a, b, startMask, widthStart, heightStart, chunkSize);
      SetWidthHeight(a, b, endMask, widthEnd, heightEnd, chunkSize);
    }
}

void GreedyMesh64::GreedyMesh64Face(const glm::ivec3 &offsetPosition, uint8_t a,
                                    uint8_t b, uint64_t bits,
                                    uint64_t (&widthMasks)[],
                                    uint64_t (&heightMasks)[],
                                    std::vector<Vertex> &vertices,
                                    FaceType type, unsigned int chunkSize) {
  while (bits) {
    const unsigned int w = __builtin_ffsll(bits) - 1;
    bits = ClearLowestBits(bits, w + 1);

    LOG_TO_FILE(
        "GreedyMesh64Face-widthMasks.txt", chunkSize, w, (int)a, (int)b,
        std::bitset<64>(
            widthMasks[(chunkSize * (w + (chunkSize * a))) / chunkSize]));

    const uint64_t &width =
        widthMasks[(chunkSize * (w + (chunkSize * a))) / chunkSize] &=
        ~((1ULL << b) - 1);

    LOG_TO_FILE("GreedyMesh64Face-width.txt", (int)a, (int)b,
                std::bitset<64>(width));

    if (!width)
      continue;

    const unsigned int widthOffset = __builtin_ffsll(width) - 1;
    const unsigned int widthSize = __builtin_ctzll(~(width >> widthOffset));

    const uint64_t &height =
        heightMasks[(chunkSize * (w + (chunkSize * (int)(widthOffset)))) /
                    chunkSize] &= ~((1ULL << a) - 1);

    const unsigned int heightOffset = __builtin_ffsll(height) - 1;
    unsigned int heightSize = __builtin_ctzll(~(height >> heightOffset));

    for (uint8_t i = heightOffset; i < heightOffset + heightSize; i++) {
      const unsigned int index =
          (chunkSize * (w + (chunkSize * i))) / chunkSize;
      const uint64_t SIZE =
          widthMasks[index] & (((1ULL << (int)widthSize) - 1) << widthOffset);

      if (__builtin_ctzll(~(SIZE >> (__builtin_ffsll(SIZE) - 1))) !=
          widthSize) {
        heightSize = i - heightOffset;
        break;
      }

      widthMasks[index] &= ~(((1ULL << (int)widthSize) - 1) << widthOffset);
    }

    switch (type) {
    case FaceType::TOP:
      Face::Top(vertices, widthOffset + (offsetPosition.x * chunkSize),
                w + (offsetPosition.y * chunkSize),
                a + (offsetPosition.z * chunkSize), widthSize, 1.0f,
                heightSize);
      break;
    case FaceType::BOTTOM:
      Face::Bottom(vertices, widthOffset + (offsetPosition.x * chunkSize),
                   w + (offsetPosition.y * chunkSize),
                   a + (offsetPosition.z * chunkSize), widthSize, 1.0f,
                   heightSize);
      break;

    case FaceType::LEFT:
      Face::Left(vertices, w + (offsetPosition.x * chunkSize),
                 widthOffset + (offsetPosition.y * chunkSize),
                 a + (offsetPosition.z * chunkSize), 1.0f, widthSize,
                 heightSize);
      break;
    case FaceType::RIGHT:
      Face::Right(vertices, w + (offsetPosition.x * chunkSize),
                  widthOffset + (offsetPosition.y * chunkSize),
                  a + (offsetPosition.z * chunkSize), 1.0f, widthSize,
                  heightSize);

      break;
    case FaceType::FRONT:
      Face::Front(vertices, a + (offsetPosition.x * chunkSize),
                  widthOffset + (offsetPosition.y * chunkSize),
                  w + (offsetPosition.z * chunkSize), heightSize, widthSize,
                  1.0f);
      break;
    case FaceType::BACK:
      Face::Back(vertices, a + (offsetPosition.x * chunkSize),
                 widthOffset + (offsetPosition.y * chunkSize),
                 w + (offsetPosition.z * chunkSize), heightSize, widthSize,
                 1.0f);
      break;
    }
  }
}

void GreedyMesh64::GreedyMesh64Axis(
    const glm::ivec3 &offsetPosition, const uint64_t (&bits)[],
    uint64_t (&widthStart)[], uint64_t (&heightStart)[], uint64_t (&widthEnd)[],
    uint64_t (&heightEnd)[], std::vector<Vertex> &vertices, FaceType startType,
    FaceType endType, unsigned int chunkSize) {

  for (uint8_t a = 0; a < chunkSize; a++)
    for (uint8_t b = 0; b < chunkSize; b++) {
      const uint64_t mask =
          bits[(chunkSize * (b + (chunkSize * a))) / chunkSize] &
          0xFFFFFFFFFFFFFFFF;

      LOG_TO_FILE(
          "GreedyMesh64Axis-widthStart.txt", chunkSize, (int)a, (int)b,
          std::bitset<64>(
              widthStart[(chunkSize * (b + (chunkSize * a))) / chunkSize]));

      GreedyMesh64Face(offsetPosition, a, b, mask & ~(mask << 1), widthStart,
                       heightStart, vertices, startType, chunkSize);
      // GreedyMesh64Face(offsetPosition, a, b, mask & ~(mask >> 1), widthEnd,
      //                  heightEnd, vertices, endType, chunkSize);
    }
}

void GreedyMesh64::CullMesh(const glm::ivec3 &offsetPosition,
                            std::vector<Vertex> &vertices,
                            uint64_t (&columns)[], uint64_t (&rows)[],
                            uint64_t (&layers)[], unsigned int chunkSize) {
  for (uint8_t a = 0; a < chunkSize; a++) {
    for (uint8_t b = 0; b < chunkSize; b++) {
      uint64_t &column =
          columns[(chunkSize * (b + (chunkSize * a))) / chunkSize];
      uint64_t &row = rows[(chunkSize * (b + (chunkSize * a))) / chunkSize];
      uint64_t &depth = layers[(chunkSize * (b + (chunkSize * a))) / chunkSize];

      while (column) {
        const unsigned int offset = __builtin_ffs(column) - 1;
        unsigned int size =
            __builtin_ctz(~(column >> (__builtin_ffs(column) - 1)));

        column &= ~((1ULL << (size + offset)) - 1);

        Face::Top(vertices, (offsetPosition.x * chunkSize) + b,
                  (offsetPosition.y * chunkSize) + offset,
                  (offsetPosition.z * chunkSize) + a, 1.0f, size, 1.0f);
        Face::Bottom(vertices, (offsetPosition.x * chunkSize) + b,
                     (offsetPosition.y * chunkSize) + offset,
                     (offsetPosition.z * chunkSize) + a, 1.0f, size, 1.0f);
      }

      while (row) {
        const unsigned int offset = __builtin_ffs(row) - 1;
        unsigned int size = __builtin_ctz(~(row >> (__builtin_ffs(row) - 1)));

        row &= ~((1ULL << (size + offset)) - 1);

        Face::Left(vertices, (offsetPosition.x * chunkSize) + offset,
                   (offsetPosition.y * chunkSize) + b,
                   (offsetPosition.z * chunkSize) + a, size, 1.0f, 1.0f);
        Face::Right(vertices, (offsetPosition.x * chunkSize) + offset,
                    (offsetPosition.y * chunkSize) + b,
                    (offsetPosition.z * chunkSize) + a, size, 1.0f, 1.0f);
      }

      while (depth) {
        const unsigned int offset = __builtin_ffs(depth) - 1;
        unsigned int size =
            __builtin_ctz(~(depth >> (__builtin_ffs(depth) - 1)));

        depth &= ~((1ULL << (size + offset)) - 1);

        Face::Front(vertices, (offsetPosition.x * chunkSize) + a,
                    (offsetPosition.y * chunkSize) + b,
                    (offsetPosition.z * chunkSize) + offset, 1.0f, 1.0f, size);
        Face::Back(vertices, (offsetPosition.x * chunkSize) + a,
                   (offsetPosition.y * chunkSize) + b,
                   (offsetPosition.z * chunkSize) + offset, 1.0f, 1.0f, size);
      }
    }
  }
}

void GreedyMesh64::Octree(SparseVoxelOctree *tree,
                          std::vector<Vertex> &vertices, int originX,
                          int originY, int originZ, unsigned int chunkSize,
                          unsigned int maskLength, Voxel *filter) {
  glm::vec3 coord = {originX / chunkSize, originY / chunkSize,
                     originZ / chunkSize};

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
  uint64_t rows[maskLength] = {};
  uint64_t columns[maskLength] = {};
  uint64_t layers[maskLength] = {};

  bool hasVoxels = false;

  for (int x = 0; x < chunkSize; x++)
    for (int y = 0; y < chunkSize; y++)
      for (int z = 0; z < chunkSize; z++)
        if (tree->get(x + originX, y + originY, z + originZ, -1, filter)) {
          hasVoxels = true;

          const unsigned int rowIndex = x + (chunkSize * (y + (chunkSize * z)));
          const unsigned int columnIndex =
              y + (chunkSize * (x + (chunkSize * z)));
          const unsigned int layerIndex =
              z + (chunkSize * (y + (chunkSize * x)));

          rows[rowIndex / chunkSize] |= (1ULL << (rowIndex % chunkSize));
          columns[columnIndex / chunkSize] |=
              (1ULL << (columnIndex % chunkSize));
          layers[layerIndex / chunkSize] |= (1ULL << (layerIndex % chunkSize));
        }

  if (!hasVoxels)
    return;

  /**
   * Cull meshing, ~0.13ms slower than greedy meshing
   *
   * CullMesh(coord, vertices, columns, rows, layers, chunkSize);
   * return;
   */

  /**
   * Here we capture the padding bit
   * Every chunk we need to get the neighbour chunks and check
   * if the bits at the 0 & 31st index are on. If so, we need to skip making
   * faces on that end.
   */
  uint8_t padding[maskLength] = {};

  for (int i = 0; i < maskLength; i++) {
    uint64_t &row = rows[i];
    uint64_t &column = columns[i];
    uint64_t &layer = layers[i];

    int rMSB = 63 - __builtin_clzll(row) + 1;
    int rLSB = __builtin_ctzll(row) - 1;

    int cMSB = 63 - __builtin_clzll(column) + 1;
    int cLSB = __builtin_ctzll(column) - 1;

    int lMSB = 63 - __builtin_clzll(layer) + 1;
    int lLSB = __builtin_ctzll(layer) - 1;

    int fast = i % chunkSize;
    int slow = (i / chunkSize) % chunkSize;

    if (row > 0 && tree->get(originX + rMSB, fast + originY, slow + originZ))
      padding[i] |= (1ULL << 1);

    if (row > 0 && tree->get(originX + rLSB, fast + originY, slow + originZ))
      padding[i] |= (1ULL << 0);

    if (column > 0 && tree->get(fast + originX, originY + cMSB, slow + originZ))
      padding[i] |= (1ULL << 3);

    if (column > 0 && tree->get(fast + originX, originY + cLSB, slow + originZ))
      padding[i] |= (1ULL << 2);

    if (layer > 0 && tree->get(slow + originX, fast + originY, originZ + lMSB))
      padding[i] |= (1ULL << 5);

    if (layer > 0 && tree->get(slow + originX, fast + originY, originZ + lLSB))
      padding[i] |= (1ULL << 4);
  }

  uint64_t widthStart[maskLength] = {};
  uint64_t heightStart[maskLength] = {};

  uint64_t widthEnd[maskLength] = {};
  uint64_t heightEnd[maskLength] = {};

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
  PrepareWidthHeightMasks(columns, 0, padding, widthStart, heightStart,
                          widthEnd, heightEnd, chunkSize);

  GreedyMesh64Axis(coord, columns, widthStart, heightStart, widthEnd, heightEnd,
                   vertices, FaceType::BOTTOM, FaceType::TOP, chunkSize);

  // std::memset(widthStart, 0, sizeof(widthStart));
  // std::memset(heightStart, 0, sizeof(heightStart));
  // std::memset(widthEnd, 0, sizeof(widthEnd));
  // std::memset(heightEnd, 0, sizeof(heightEnd));

  // PrepareWidthHeightMasks(rows, 2, padding, widthStart, heightStart,
  // widthEnd,
  //                         heightEnd, chunkSize);
  // GreedyMesh64Axis(coord, rows, widthStart, heightStart, widthEnd, heightEnd,
  //                  vertices, FaceType::LEFT, FaceType::RIGHT, chunkSize);

  // std::memset(widthStart, 0, sizeof(widthStart));
  // std::memset(heightStart, 0, sizeof(heightStart));
  // std::memset(widthEnd, 0, sizeof(widthEnd));
  // std::memset(heightEnd, 0, sizeof(heightEnd));

  // PrepareWidthHeightMasks(layers, 4, padding, widthStart, heightStart,
  // widthEnd,
  //                         heightEnd, chunkSize);
  // GreedyMesh64Axis(coord, layers, widthStart, heightStart, widthEnd,
  // heightEnd,
  //                  vertices, FaceType::FRONT, FaceType::BACK, chunkSize);
}