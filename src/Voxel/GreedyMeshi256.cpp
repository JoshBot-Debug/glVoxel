#include "GreedyMeshi256.h"

void GreedyMeshi256::SetWidthHeight(uint8_t a, uint8_t b, uint64_t bits,
                                    uint64_t (&widthMasks)[],
                                    uint64_t (&heightMasks)[]) {
  while (bits) {
    const uint8_t w = __builtin_ffsll(bits) - 1;

    const unsigned int wi = a + (s_CHUNK_SIZE * (w + (s_CHUNK_SIZE * b)));
    widthMasks[wi / s_CHUNK_SIZE] |= (1ULL << (wi % s_CHUNK_SIZE));

    const unsigned int hi = b + (s_CHUNK_SIZE * (w + (s_CHUNK_SIZE * a)));
    heightMasks[hi / s_CHUNK_SIZE] |= (1ULL << (hi % s_CHUNK_SIZE));

    bits = ClearLowestBits(bits, w + 1);
  }
}

void GreedyMeshi256::PrepareWidthHeightMasks(
    const uint64_t (&bits)[], uint8_t paddingIndex, uint8_t (&padding)[],
    uint64_t (&widthStart)[], uint64_t (&heightStart)[], uint64_t (&widthEnd)[],
    uint64_t (&heightEnd)[]) {
  for (uint8_t a = 0; a < s_CHUNK_SIZE; a++)
    for (uint8_t b = 0; b < s_CHUNK_SIZE; b++) {

      // unsigned int i = (s_CHUNK_SIZE * (a + (s_CHUNK_SIZE * b))) / s_CHUNK_SIZE;

      // /**
      //  * Get the bitmask at index a,b
      //  * The padding mask has an extra bit as the LSB and MSB.
      //  * The MSB is the LSB of the pervious neighbour chunk
      //  * The LSB is the MSB of the next neighbour chunk
      //  * The first & last will always be a zero because there is no neighbour
      //  * next to them. 0...1 => 1...1 => 1...0
      //  */
      // const uint8_t paddingMask = padding[i];

      // /**
      //  * Shift right to remove the LSB padding bit and extract the following
      //  * 32bits into a new mask This is the actual mask we will use for the
      //  * height and width
      //  */
      // const uint64_t mask = bits[i];

      // /**
      //  * The first bit that is on on the left/top/front
      //  */
      // const unsigned int msbIndex =
      //     (mask == 0) ? (s_CHUNK_SIZE - 1)
      //                 : (s_CHUNK_SIZE - 1) - __builtin_clzll(mask);

      // /**
      //  * The first bit that is on on the right/bottom/back
      //  */
      // const unsigned int lsbIndex = (mask == 0) ? 0 : __builtin_ctzll(mask);

      // /**
      //  * Remove all the bits other than the start face
      //  * 11100111100011 => 00100000100001
      //  */
      // uint64_t startMask = mask & ~(mask << 1);

      // /**
      //  * Likewise remove all the bits other than the end face
      //  * 11100111100011 => 10000100000010
      //  */
      // uint64_t endMask = mask & ~(mask >> 1);

      // /**
      //  * Check the padding mask, if the bit at 0 index is on
      //  * turn off the MSB of the start mask
      //  *
      //  * If bit 0 of paddingMask is set, then clear bit 0 of startMask.
      //  * if ((paddingMask >> 0) & 1)
      //  *   startMask &= ~(1ULL << 0);
      //  */
      // if ((paddingMask >> paddingIndex) & 1)
      //   startMask &= ~(1ULL << lsbIndex);

      // /**
      //  * Check the padding mask, if the bit at 63 index is on
      //  * turn off the LSB of the end mask
      //  *
      //  * This is done in order to not set the height & width of the face at the
      //  * end of the chunk if the neighbour is the same To avoid creating faces
      //  * inbetween chunks
      //  *
      //  * If bit 63 of paddingMask is set, then clear bit 31 of endMask.
      //  * if ((paddingMask >> 63) & 1)
      //  *   endMask &= ~(1ULL << 31);
      //  */
      // if ((paddingMask >> (paddingIndex + 1)) & 1)
      //   endMask &= ~(1ULL << msbIndex);

      // SetWidthHeight(a, b, startMask, widthStart, heightStart);
      // SetWidthHeight(a, b, endMask, widthEnd, heightEnd);
    }
}

void GreedyMeshi256::GreedyMeshi256Face(const glm::ivec3 &offsetPosition,
                                        uint8_t a, uint8_t b, uint64_t bits,
                                        uint64_t (&widthMasks)[],
                                        uint64_t (&heightMasks)[],
                                        std::vector<Vertex> &vertices,
                                        FaceType type) {
  while (bits) {
    const uint8_t w = __builtin_ffsll(bits) - 1;
    bits = ClearLowestBits(bits, w + 1);

    const uint64_t &width =
        widthMasks[(s_CHUNK_SIZE * (w + (s_CHUNK_SIZE * a))) / s_CHUNK_SIZE] &
        ~((1ULL << b) - 1);

    if (!width)
      continue;

    const uint8_t widthOffset = __builtin_ffsll(width) - 1;

    uint8_t widthSize =
        ~width == 0 ? s_CHUNK_SIZE : __builtin_ctzll(~(width >> widthOffset));

    const uint64_t &height =
        heightMasks[(s_CHUNK_SIZE * (w + (s_CHUNK_SIZE * (int)(widthOffset)))) /
                    s_CHUNK_SIZE] &
        ~((1ULL << a) - 1);

    const uint8_t heightOffset = __builtin_ffsll(height) - 1;

    uint8_t heightSize = ~height == 0
                             ? s_CHUNK_SIZE
                             : __builtin_ctzll(~(height >> heightOffset));

    for (uint8_t i = heightOffset; i < heightOffset + heightSize; i++) {
      const unsigned int index =
          (s_CHUNK_SIZE * (w + (s_CHUNK_SIZE * i))) / s_CHUNK_SIZE;

      const uint64_t SIZE =
          widthMasks[index] &
          (((widthSize >= s_CHUNK_SIZE ? 0ULL : (1ULL << widthSize)) - 1)
           << widthOffset);

      if (SIZE == 0 ||
          (~SIZE == 0
               ? 0
               : __builtin_ctzll(~(SIZE >> (__builtin_ffsll(SIZE) - 1))) !=
                     widthSize)) {
        heightSize = i - heightOffset;
        break;
      }

      widthMasks[index] &=
          ~(((widthSize >= s_CHUNK_SIZE ? 0ULL : (1ULL << widthSize)) - 1)
            << widthOffset);
    }

    switch (type) {
    case FaceType::TOP:
      Face::Top(vertices, widthOffset + (offsetPosition.x * s_CHUNK_SIZE),
                w + (offsetPosition.y * s_CHUNK_SIZE),
                a + (offsetPosition.z * s_CHUNK_SIZE), widthSize, 1.0f,
                heightSize);
      break;
    case FaceType::BOTTOM:
      Face::Bottom(vertices, widthOffset + (offsetPosition.x * s_CHUNK_SIZE),
                   w + (offsetPosition.y * s_CHUNK_SIZE),
                   a + (offsetPosition.z * s_CHUNK_SIZE), widthSize, 1.0f,
                   heightSize);
      break;

    case FaceType::LEFT:
      Face::Left(vertices, w + (offsetPosition.x * s_CHUNK_SIZE),
                 widthOffset + (offsetPosition.y * s_CHUNK_SIZE),
                 a + (offsetPosition.z * s_CHUNK_SIZE), 1.0f, widthSize,
                 heightSize);
      break;
    case FaceType::RIGHT:
      Face::Right(vertices, w + (offsetPosition.x * s_CHUNK_SIZE),
                  widthOffset + (offsetPosition.y * s_CHUNK_SIZE),
                  a + (offsetPosition.z * s_CHUNK_SIZE), 1.0f, widthSize,
                  heightSize);

      break;
    case FaceType::FRONT:
      Face::Front(vertices, a + (offsetPosition.x * s_CHUNK_SIZE),
                  widthOffset + (offsetPosition.y * s_CHUNK_SIZE),
                  w + (offsetPosition.z * s_CHUNK_SIZE), heightSize, widthSize,
                  1.0f);
      break;
    case FaceType::BACK:
      Face::Back(vertices, a + (offsetPosition.x * s_CHUNK_SIZE),
                 widthOffset + (offsetPosition.y * s_CHUNK_SIZE),
                 w + (offsetPosition.z * s_CHUNK_SIZE), heightSize, widthSize,
                 1.0f);
      break;
    }
  }
}

void GreedyMeshi256::GreedyMeshi256Axis(
    const glm::ivec3 &offsetPosition, const uint64_t (&bits)[],
    uint64_t (&widthStart)[], uint64_t (&heightStart)[], uint64_t (&widthEnd)[],
    uint64_t (&heightEnd)[], std::vector<Vertex> &vertices, FaceType startType,
    FaceType endType) {

  for (uint8_t a = 0; a < s_CHUNK_SIZE; a++)
    for (uint8_t b = 0; b < s_CHUNK_SIZE; b++) {
      const uint64_t mask =
          bits[(s_CHUNK_SIZE * (b + (s_CHUNK_SIZE * a))) / s_CHUNK_SIZE] &
          0xFFFFFFFFFFFFFFFFULL;

      GreedyMeshi256Face(offsetPosition, a, b, mask & ~(mask << 1), widthStart,
                         heightStart, vertices, startType);
      GreedyMeshi256Face(offsetPosition, a, b, mask & ~(mask >> 1), widthEnd,
                         heightEnd, vertices, endType);
    }
}

void GreedyMeshi256::CullMesh(const glm::ivec3 &offsetPosition,
                              std::vector<Vertex> &vertices,
                              uint64_t (&columns)[], uint64_t (&rows)[],
                              uint64_t (&layers)[]) {
  for (uint8_t a = 0; a < s_CHUNK_SIZE; a++) {
    for (uint8_t b = 0; b < s_CHUNK_SIZE; b++) {
      uint64_t &column =
          columns[(s_CHUNK_SIZE * (b + (s_CHUNK_SIZE * a))) / s_CHUNK_SIZE];
      uint64_t &row =
          rows[(s_CHUNK_SIZE * (b + (s_CHUNK_SIZE * a))) / s_CHUNK_SIZE];
      uint64_t &depth =
          layers[(s_CHUNK_SIZE * (b + (s_CHUNK_SIZE * a))) / s_CHUNK_SIZE];

      while (column) {
        const unsigned int offset = __builtin_ffsll(column) - 1;
        unsigned int size =
            __builtin_ctzll(~(column >> (__builtin_ffsll(column) - 1)));

        column &= ~((1ULL << (size + offset)) - 1);

        Face::Top(vertices, (offsetPosition.x * s_CHUNK_SIZE) + b,
                  (offsetPosition.y * s_CHUNK_SIZE) + offset,
                  (offsetPosition.z * s_CHUNK_SIZE) + a, 1.0f, size, 1.0f);
        Face::Bottom(vertices, (offsetPosition.x * s_CHUNK_SIZE) + b,
                     (offsetPosition.y * s_CHUNK_SIZE) + offset,
                     (offsetPosition.z * s_CHUNK_SIZE) + a, 1.0f, size, 1.0f);
      }

      while (row) {
        const unsigned int offset = __builtin_ffsll(row) - 1;
        unsigned int size =
            __builtin_ctzll(~(row >> (__builtin_ffsll(row) - 1)));

        row &= ~((1ULL << (size + offset)) - 1);

        Face::Left(vertices, (offsetPosition.x * s_CHUNK_SIZE) + offset,
                   (offsetPosition.y * s_CHUNK_SIZE) + b,
                   (offsetPosition.z * s_CHUNK_SIZE) + a, size, 1.0f, 1.0f);
        Face::Right(vertices, (offsetPosition.x * s_CHUNK_SIZE) + offset,
                    (offsetPosition.y * s_CHUNK_SIZE) + b,
                    (offsetPosition.z * s_CHUNK_SIZE) + a, size, 1.0f, 1.0f);
      }

      while (depth) {
        const unsigned int offset = __builtin_ffsll(depth) - 1;
        unsigned int size =
            __builtin_ctzll(~(depth >> (__builtin_ffsll(depth) - 1)));

        depth &= ~((1ULL << (size + offset)) - 1);

        Face::Front(vertices, (offsetPosition.x * s_CHUNK_SIZE) + a,
                    (offsetPosition.y * s_CHUNK_SIZE) + b,
                    (offsetPosition.z * s_CHUNK_SIZE) + offset, 1.0f, 1.0f,
                    size);
        Face::Back(vertices, (offsetPosition.x * s_CHUNK_SIZE) + a,
                   (offsetPosition.y * s_CHUNK_SIZE) + b,
                   (offsetPosition.z * s_CHUNK_SIZE) + offset, 1.0f, 1.0f,
                   size);
      }
    }
  }
}

inline int clzll(const uint64_t (&bits)[4]) {
  for (int i = 3; i >= 0; --i)
    if (bits[i] != 0)
      return __builtin_clzll(bits[i]) + (3 - i) * 64;
  return 256;
}

inline int ctzll(const uint64_t (&bits)[4]) {
  for (int i = 0; i < 4; ++i)
    if (bits[i] != 0)
      return __builtin_ctzll(bits[i]) + (i * 64);
  return 256;
}

void GreedyMeshi256::Octree(SparseVoxelOctree *tree,
                            std::vector<Vertex> &vertices, int originX,
                            int originY, int originZ, int depth,
                            Voxel *filter) {
  glm::vec3 coord = {originX / s_CHUNK_SIZE, originY / s_CHUNK_SIZE,
                     originZ / s_CHUNK_SIZE};

  LOG_IVEC3("coord", coord);

  /**
   * I align by 32 bytes here because I want to _mm256_load_si256 not
   * _mm256_loadu_si256 Since my offset is 4, it should work out. Otherwise I'll
   * just use _mm256_loadu_si256
   */
  alignas(32) uint64_t rows[s_MASK_LENGTH] = {};
  alignas(32) uint64_t columns[s_MASK_LENGTH] = {};
  alignas(32) uint64_t layers[s_MASK_LENGTH] = {};
  uint8_t padding[s_MASK_LENGTH] = {};

  bool hasVoxels = false;

  for (int x = 0; x < s_CHUNK_SIZE; x++)
    for (int y = 0; y < s_CHUNK_SIZE; y++)
      for (int z = 0; z < s_CHUNK_SIZE; z++)
        if (tree->get(x + originX, y + originY, z + originZ, depth, filter)) {
          hasVoxels = true;

          const unsigned int rowIndex =
              x + (s_CHUNK_SIZE * (y + (s_CHUNK_SIZE * z)));
          const unsigned int columnIndex =
              y + (s_CHUNK_SIZE * (x + (s_CHUNK_SIZE * z)));
          const unsigned int layerIndex =
              z + (s_CHUNK_SIZE * (y + (s_CHUNK_SIZE * x)));

          rows[rowIndex / s_BITS] |= (1ULL << (rowIndex % s_BITS));
          columns[columnIndex / s_BITS] |= (1ULL << (columnIndex % s_BITS));
          layers[layerIndex / s_BITS] |= (1ULL << (layerIndex % s_BITS));
        }

  if (!hasVoxels)
    return;

  for (size_t i = 0; i < s_MASK_LENGTH; i++) {
    if (rows[i] ==
        0b1111111111111111111111111111111111111111111111111111111111111111)
      LOG("INDEX", i);
  }

  LOG("Row Bits", std::bitset<64>(rows[0]), std::bitset<64>(rows[1]),
      std::bitset<64>(rows[2]), std::bitset<64>(rows[3]));

  /**
   * Here we capture the padding bit
   * Every chunk we need to get the neighbour chunks and check
   * if the bits at the 0 & 31st index are on. If so, we need to skip making
   * faces on that end.
   *
   * TODO: Needs to be tested with multiple chunks.
   */
  for (int i = 0; i < s_MASK_LENGTH; i += s_STEPS) {

    union {
      __m256i row;
      alignas(32) uint64_t rowBits[4];
    };
    row = _mm256_load_si256(reinterpret_cast<const __m256i *>(&rows[i]));

    union {
      __m256i column;
      alignas(32) uint64_t columnBits[4];
    };
    column = _mm256_load_si256(reinterpret_cast<const __m256i *>(&column[i]));

    union {
      __m256i layer;
      alignas(32) uint64_t layerBits[4];
    };
    layer = _mm256_load_si256(reinterpret_cast<const __m256i *>(&layer[i]));

    int fast = i % s_CHUNK_SIZE;
    int slow = (i / s_CHUNK_SIZE) % s_CHUNK_SIZE;

    if (!_mm256_testz_si256(row, row)) {
      int rMSB = (s_CHUNK_SIZE - 1) - clzll(rowBits) + 1;
      int rLSB = ctzll(rowBits) - 1;

      if (tree->get(originX + rMSB, fast + originY, slow + originZ, depth))
        padding[i] |= (1ULL << 1);

      if (tree->get(originX + rLSB, fast + originY, slow + originZ, depth))
        padding[i] |= (1ULL << 0);
    }

    if (!_mm256_testz_si256(column, column)) {
      int cMSB = (s_CHUNK_SIZE - 1) - clzll(columnBits) + 1;
      int cLSB = ctzll(columnBits) - 1;

      if (tree->get(fast + originX, originY + cMSB, slow + originZ, depth))
        padding[i] |= (1ULL << 3);

      if (tree->get(fast + originX, originY + cLSB, slow + originZ, depth))
        padding[i] |= (1ULL << 2);
    }

    if (!_mm256_testz_si256(layer, layer)) {
      int lMSB = (s_CHUNK_SIZE - 1) - clzll(layerBits) + 1;
      int lLSB = ctzll(layerBits) - 1;

      if (tree->get(slow + originX, fast + originY, originZ + lMSB, depth))
        padding[i] |= (1ULL << 5);

      if (tree->get(slow + originX, fast + originY, originZ + lLSB, depth))
        padding[i] |= (1ULL << 4);
    }
  }

  LOG("Padding at Index 0", std::bitset<8>(padding[0]));

  uint64_t widthStart[s_MASK_LENGTH] = {};
  uint64_t heightStart[s_MASK_LENGTH] = {};

  uint64_t widthEnd[s_MASK_LENGTH] = {};
  uint64_t heightEnd[s_MASK_LENGTH] = {};

  // /**
  //  * Culls the column/row/layer
  //  * From: 001110110011111
  //  * To: 001010110010001
  //  *
  //  * Loop over the correct axis for each column/row/layer
  //  * for columns x,z
  //  * for rows    y,z
  //  * for layers  y,x
  //  *
  //  * Set the width and height of each face for both sides (start & end)
  //  * for columns bottom & top
  //  * for rows    left & right
  //  * for layers  front & back
  //  */

  PrepareWidthHeightMasks(rows, 0, padding, widthStart, heightStart, widthEnd,
                          heightEnd);
  // GreedyMeshi256Axis(coord, rows, widthStart, heightStart, widthEnd,
  // heightEnd,
  //                  vertices, FaceType::LEFT, FaceType::RIGHT);

  // std::memset(widthStart, 0, sizeof(widthStart));
  // std::memset(heightStart, 0, sizeof(heightStart));
  // std::memset(widthEnd, 0, sizeof(widthEnd));
  // std::memset(heightEnd, 0, sizeof(heightEnd));

  // PrepareWidthHeightMasks(columns, 2, padding, widthStart, heightStart,
  //                         widthEnd, heightEnd);

  // GreedyMeshi256Axis(coord, columns, widthStart, heightStart, widthEnd,
  // heightEnd,
  //                  vertices, FaceType::BOTTOM, FaceType::TOP);

  // std::memset(widthStart, 0, sizeof(widthStart));
  // std::memset(heightStart, 0, sizeof(heightStart));
  // std::memset(widthEnd, 0, sizeof(widthEnd));
  // std::memset(heightEnd, 0, sizeof(heightEnd));

  // PrepareWidthHeightMasks(layers, 4, padding, widthStart, heightStart,
  // widthEnd,
  //                         heightEnd);
  // GreedyMeshi256Axis(coord, layers, widthStart, heightStart, widthEnd,
  // heightEnd,
  //                  vertices, FaceType::FRONT, FaceType::BACK);
}