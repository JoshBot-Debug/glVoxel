#include "GreedyMeshi256.h"

int GreedyMeshi256::FlipScalarIndex(int i) {
  return (static_cast<int>((i / s_STEPS)) * s_STEPS) + ((s_STEPS - 1) - (i % s_STEPS));
}

void GreedyMeshi256::SetWidthHeight(uint8_t a, uint8_t b, __m256i &bits,
                                    uint64_t *widthMasks,
                                    uint64_t *heightMasks) {

  // LOG_TO_FILE("bits-256", std::bitset<64>(((uint64_t *)&bits)[0]),
  //             std::bitset<64>(((uint64_t *)&bits)[1]),
  //             std::bitset<64>(((uint64_t *)&bits)[2]),
  //             std::bitset<64>(((uint64_t *)&bits)[3]));

  while (!_mm256_testz_si256(bits, bits)) {
    const uint16_t w = ffs256(bits) - 1;

    const unsigned int wi = a + (s_CHUNK_SIZE * (w + (s_CHUNK_SIZE * b)));
    const unsigned int hi = b + (s_CHUNK_SIZE * (w + (s_CHUNK_SIZE * a)));

    widthMasks[FlipScalarIndex(static_cast<int>(wi / s_BITS))] |=
        (1ULL << (wi % s_BITS));
    heightMasks[FlipScalarIndex(static_cast<int>(hi / s_BITS))] |=
        (1ULL << (hi % s_BITS));

    bits = clb256(bits, w + 1);
  }

  // for (size_t i = 0; i < s_MASK_LENGTH; i += s_BITS) {
  //   if (widthMasks[i + 3] != 0)
  //     LOG_TO_FILE("widthMasks-256", std::bitset<64>(widthMasks[i]),
  //                 std::bitset<64>(widthMasks[i + 1]),
  //                 std::bitset<64>(widthMasks[i + 2]),
  //                 std::bitset<64>(widthMasks[i + 3]));
  // }
}

void GreedyMeshi256::PrepareWidthHeightMasks(
    uint64_t *bits, uint8_t paddingIndex, uint8_t *padding,
    uint64_t *widthStart, uint64_t *heightStart, uint64_t *widthEnd,
    uint64_t *heightEnd) {

  for (uint16_t a = 0; a < s_CHUNK_SIZE; a++)
    for (uint16_t b = 0; b < s_CHUNK_SIZE; b++) {

      unsigned int ri = a + (s_CHUNK_SIZE * b);
      unsigned int i = ri * s_STEPS;

      /**
       * Get the bitmask at index a,b
       * The padding mask has an extra bit as the LSB and MSB.
       * The MSB is the LSB of the pervious neighbour chunk
       * The LSB is the MSB of the next neighbour chunk
       * The first & last will always be a zero because there is no neighbour
       * next to them. 0...1 => 1...1 => 1...0
       */
      const uint8_t paddingMask = padding[ri];

      /**
       * Shift right to remove the LSB padding bit and extract the following
       * 32bits into a new mask This is the actual mask we will use for the
       * height and width
       */
      // const uint64_t mask = bits[i];
      __m256i mask =
          _mm256_load_si256(reinterpret_cast<const __m256i *>(&bits[i]));

      // if (!_mm256_testz_si256(mask, mask)) {
      //   // LOG(i);
      //   // LOG_256(mask);
      //   LOG_TO_FILE("mask-256", std::bitset<64>(((uint64_t *)&mask)[0]),
      //               std::bitset<64>(((uint64_t *)&mask)[1]),
      //               std::bitset<64>(((uint64_t *)&mask)[2]),
      //               std::bitset<64>(((uint64_t *)&mask)[3]));
      // }

      int isEmpty = _mm256_testz_si256(mask, mask);

      /**
       * The first bit that is on on the left/top/front
       */
      // const unsigned int msbIndex =
      //     (mask == 0) ? (s_CHUNK_SIZE - 1)
      //                 : (s_CHUNK_SIZE - 1) - __builtin_clzll(mask);
      const unsigned int msbIndex =
          isEmpty ? (s_CHUNK_SIZE - 1) : (s_CHUNK_SIZE - 1) - clz256(mask);

      /**
       * The first bit that is on on the right/bottom/back
       */
      // const unsigned int lsbIndex = (mask == 0) ? 0 : __builtin_ctzll(mask);
      const unsigned int lsbIndex = isEmpty ? 0 : ctz256(mask);

      /**
       * Remove all the bits other than the start face
       * 11100111100011 => 00100000100001
       */
      // uint64_t startMask = mask & ~(mask << 1);
      union {
        __m256i startMask;
        alignas(32) uint64_t startMaskBits[4];
      };
      startMask = sl256(mask, 1);
      startMask = _mm256_xor_si256(startMask, _mm256_set1_epi64x(-1));
      startMask = _mm256_and_si256(mask, startMask);

      // /**
      //  * Likewise remove all the bits other than the end face
      //  * 11100111100011 => 10000100000010
      //  */
      // uint64_t endMask = mask & ~(mask >> 1);
      union {
        __m256i endMask;
        alignas(32) uint64_t endMaskBits[4];
      };
      endMask = sr256(mask, 1);
      endMask = _mm256_xor_si256(endMask, _mm256_set1_epi64x(-1));
      endMask = _mm256_and_si256(mask, endMask);

      /**
       * Check the padding mask, if the bit at 0 index is on
       * turn off the MSB of the start mask
       *
       * If bit 0 of paddingMask is set, then clear bit 0 of startMask.
       * if ((paddingMask >> 0) & 1)
       *   startMask &= ~(1ULL << 0);
       */
      // if ((paddingMask >> paddingIndex) & 1)
      //   startMaskBits[lsbIndex / s_BITS] &= ~(1ULL << (lsbIndex % s_BITS));

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
      //   endMaskBits[msbIndex / s_BITS] &= ~(1ULL << (msbIndex % s_BITS));

      SetWidthHeight(a, b, startMask, widthStart, heightStart);
      SetWidthHeight(a, b, endMask, widthEnd, heightEnd);
    }
}

void GreedyMeshi256::GreedyMeshi256Face(const glm::ivec3 &offsetPosition,
                                        uint8_t a, uint8_t b, __m256i &bits,
                                        uint64_t *widthMasks,
                                        uint64_t *heightMasks,
                                        std::vector<Vertex> &vertices,
                                        FaceType type) {

  // uint64_t *p = (uint64_t *)&bits;
  // LOG_TO_FILE("128", std::bitset<64>(p[0]), std::bitset<64>(p[1]),
  //             std::bitset<64>(p[2]), std::bitset<64>(p[3]));

  // for (size_t i = 0; i < s_MASK_LENGTH; i += s_STEPS) {
  //   LOG_TO_FILE("128", std::bitset<64>(heightMasks[i]),
  //               std::bitset<64>(heightMasks[i + 1]),
  //               std::bitset<64>(heightMasks[i + 2]),
  //               std::bitset<64>(heightMasks[i + 3]));
  // }

  while (!_mm256_testz_si256(bits, bits)) {
    const uint16_t w = ffs256(bits) - 1;
    bits = clb256(bits, w + 1);

    unsigned int cwi = w + (s_CHUNK_SIZE * a);
    unsigned int wi = cwi * s_STEPS;

    __m256i widthMask =
        _mm256_load_si256(reinterpret_cast<const __m256i *>(&widthMasks[wi]));

    __m256i width = clb256(widthMask, b);

    // LOG_TO_FILE("widthMasks-256", std::bitset<64>(((uint64_t
    // *)&widthMask)[0]),
    //             std::bitset<64>(((uint64_t *)&widthMask)[1]),
    //             std::bitset<64>(((uint64_t *)&widthMask)[2]),
    //             std::bitset<64>(((uint64_t *)&widthMask)[3]));
    // LOG_TO_FILE("width-256", std::bitset<64>(((uint64_t *)&width)[0]),
    //             std::bitset<64>(((uint64_t *)&width)[1]),
    //             std::bitset<64>(((uint64_t *)&width)[2]),
    //             std::bitset<64>(((uint64_t *)&width)[3]));

    if (_mm256_testz_si256(width, width))
      continue;

    const uint16_t widthOffset = ffs256(width) - 1;

    __m256i wsMask =
        _mm256_xor_si256(sr256(width, widthOffset), _mm256_set1_epi64x(-1));

    uint16_t widthSize = _mm256_testc_si256(width, _mm256_set1_epi64x(-1))
                             ? s_CHUNK_SIZE
                             : ctz256(wsMask);

    // LOG_TO_FILE("widthOffset-widthSize-256", (int)widthOffset,
    // (int)widthSize);

    unsigned int chi = w + (s_CHUNK_SIZE * (int)(widthOffset));
    unsigned int hi = chi * s_STEPS;

    __m256i heightMask =
        _mm256_load_si256(reinterpret_cast<const __m256i *>(&heightMasks[hi]));

    __m256i height = clb256(heightMask, a);

    // LOG_TO_FILE("heightMask-256", std::bitset<64>(((uint64_t
    // *)&heightMask)[0]),
    //             std::bitset<64>(((uint64_t *)&heightMask)[1]),
    //             std::bitset<64>(((uint64_t *)&heightMask)[2]),
    //             std::bitset<64>(((uint64_t *)&heightMask)[3]));
    // LOG_TO_FILE("height-256", std::bitset<64>(((uint64_t *)&height)[0]),
    //             std::bitset<64>(((uint64_t *)&height)[1]),
    //             std::bitset<64>(((uint64_t *)&height)[2]),
    //             std::bitset<64>(((uint64_t *)&height)[3]));

    const uint16_t heightOffset = ffs256(height) - 1;

    __m256i hsMask =
        _mm256_xor_si256(sr256(height, heightOffset), _mm256_set1_epi64x(-1));
    uint16_t heightSize = _mm256_testc_si256(height, _mm256_set1_epi64x(-1))
                              ? s_CHUNK_SIZE
                              : ctz256(hsMask);

    // LOG_TO_FILE("heightOffset-heightSize-256", (int)heightOffset,
    // (int)heightSize);

    for (uint16_t i = heightOffset; i < heightOffset + heightSize; i++) {
      const unsigned int rowIndex = w + (s_CHUNK_SIZE * i);
      const unsigned int index = rowIndex * s_STEPS;

      __m256i widthSizeMask = GreedyMeshi256::mmr(widthSize, widthOffset);
      __m256i widthMask = _mm256_load_si256(
          reinterpret_cast<const __m256i *>(&widthMasks[index]));

      // LOG("i-widthSizeMask-256", std::bitset<64>(((uint64_t
      // *)&widthSizeMask)[0]),
      //             std::bitset<64>(((uint64_t *)&widthSizeMask)[1]),
      //             std::bitset<64>(((uint64_t *)&widthSizeMask)[2]),
      //             std::bitset<64>(((uint64_t *)&widthSizeMask)[3]));
      // LOG("i-widthMask-256", std::bitset<64>(((uint64_t *)&widthMask)[0]),
      //             std::bitset<64>(((uint64_t *)&widthMask)[1]),
      //             std::bitset<64>(((uint64_t *)&widthMask)[2]),
      //             std::bitset<64>(((uint64_t *)&widthMask)[3]));

      __m256i size = _mm256_and_si256(widthMask, widthSizeMask);

      __m256i sizeMask = _mm256_xor_si256(sr256(size, ffs256(size) - 1),
                                          _mm256_set1_epi64x(-1));

      // LOG(widthSize, widthOffset);
      // LOG_256(size);

      // LOG(!_mm256_testz_si256(size, size));

      if (_mm256_testz_si256(size, size) ||
          !_mm256_testc_si256(size, _mm256_set1_epi64x(~0ULL)) &&
              ctz256(sizeMask) != widthSize) {
        heightSize = i - heightOffset;
        break;
      }

      // LOG_TO_FILE("i-widthSizeMask-256",
      //     std::bitset<64>(((uint64_t *)&widthSizeMask)[0]),
      //     std::bitset<64>(((uint64_t *)&widthSizeMask)[1]),
      //     std::bitset<64>(((uint64_t *)&widthSizeMask)[2]),
      //     std::bitset<64>(((uint64_t *)&widthSizeMask)[3]));
      // LOG_TO_FILE("i-widthMask-256", std::bitset<64>(((uint64_t
      // *)&widthMask)[0]),
      //     std::bitset<64>(((uint64_t *)&widthMask)[1]),
      //     std::bitset<64>(((uint64_t *)&widthMask)[2]),
      //     std::bitset<64>(((uint64_t *)&widthMask)[3]));

      // LOG_256(size);

      widthMask = _mm256_xor_si256(size, _mm256_set1_epi64x(-1));

      _mm256_store_si256(reinterpret_cast<__m256i *>(&widthMasks[index]),
                         widthMask);
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

void GreedyMeshi256::GreedyMeshi256Axis(const glm::ivec3 &offsetPosition,
                                        uint64_t *bits, uint64_t *widthStart,
                                        uint64_t *heightStart,
                                        uint64_t *widthEnd, uint64_t *heightEnd,
                                        std::vector<Vertex> &vertices,
                                        FaceType startType, FaceType endType) {
  for (uint16_t a = 0; a < s_CHUNK_SIZE; a++)
    for (uint16_t b = 0; b < s_CHUNK_SIZE; b++) {
      // const uint64_t mask = bits[b + (s_CHUNK_SIZE * a)] & ~0ULL;
      unsigned int ri = b + (s_CHUNK_SIZE * a);
      unsigned int i = ri * s_STEPS;

      __m256i mask =
          _mm256_load_si256(reinterpret_cast<const __m256i *>(&bits[i]));

      // mask & ~(mask << 1)
      __m256i startMask = _mm256_and_si256(
          mask, _mm256_xor_si256(sl256(mask, 1), _mm256_set1_epi64x(-1)));
      // mask & ~(mask >> 1)
      __m256i endMask = _mm256_and_si256(
          mask, _mm256_xor_si256(sr256(mask, 1), _mm256_set1_epi64x(-1)));

      // for (size_t i = 0; i < s_MASK_LENGTH; i += s_STEPS) {
      //   LOG_TO_FILE("128", std::bitset<64>(widthStart[i]),
      //   std::bitset<64>(widthStart[i + 1]),
      //               std::bitset<64>(widthStart[i + 2]),
      //               std::bitset<64>(widthStart[i + 3]));
      // }

      GreedyMeshi256Face(offsetPosition, a, b, startMask, widthStart,
                         heightStart, vertices, startType);
      GreedyMeshi256Face(offsetPosition, a, b, endMask, widthEnd, heightEnd,
                         vertices, endType);
    }
}

/**
 * Count leading zeros
 */
int GreedyMeshi256::clz256(uint64_t *bits) {
  for (int i = 0; i < 4; ++i)
    if (bits[i] != 0)
      return __builtin_clzll(bits[i]) + (i * 64);
  return 256;
}

int GreedyMeshi256::clz256(__m256i &bits) {
  uint64_t *p = reinterpret_cast<uint64_t *>(&bits);
  for (int i = 0; i < 4; ++i)
    if (p[i] != 0)
      return __builtin_clzll(p[i]) + (i * 64);
  return 256;
}

/**
 * Count trailing zeros
 */
int GreedyMeshi256::ctz256(uint64_t *bits) {
  for (int i = 3; i >= 0; --i)
    if (bits[i] != 0)
      return __builtin_ctzll(bits[i]) + (3 - i) * 64;
  return 256;
}

int GreedyMeshi256::ctz256(__m256i &bits) {
  uint64_t *p = reinterpret_cast<uint64_t *>(&bits);
  for (int i = 3; i >= 0; --i)
    if (p[i] != 0)
      return __builtin_ctzll(p[i]) + (3 - i) * 64;
  return 256;
}

__m256i GreedyMeshi256::sl256(__m256i &bits, int n) {

  if (n == 0)
    return bits;

  if (n >= 256)
    return _mm256_setzero_si256();

  // Shift all lanes left
  __m256i shifted = _mm256_slli_epi64(bits, n % 64);

  // Shift all lanes right by 64 - n
  // This moves n MSB of each lane to the LSB side
  __m256i carry = _mm256_srli_epi64(bits, 64 - (n % 64));

  carry = _mm256_permute4x64_epi64(carry, _MM_SHUFFLE(0, 3, 2, 1));
  carry = _mm256_and_si256(carry, _mm256_set_epi64x(0LL, -1LL, -1LL, -1LL));

  __m256i result = _mm256_or_si256(shifted, carry);

  if ((n / 64) == 1) {
    result = _mm256_permute4x64_epi64(result, _MM_SHUFFLE(0, 3, 2, 1));
    result = _mm256_and_si256(result, _mm256_set_epi64x(0LL, -1LL, -1LL, -1LL));
  } else if ((n / 64) == 2) {
    result = _mm256_permute4x64_epi64(result, _MM_SHUFFLE(0, 0, 3, 2));
    result = _mm256_and_si256(result, _mm256_set_epi64x(0LL, 0LL, -1LL, -1LL));
  } else if ((n / 64) == 3) {
    result = _mm256_permute4x64_epi64(result, _MM_SHUFFLE(0, 0, 0, 3));
    result = _mm256_and_si256(result, _mm256_set_epi64x(0LL, 0LL, 0LL, -1LL));
  }

  return result;
}

__m256i GreedyMeshi256::sr256(__m256i &bits, int n) {

  if (n == 0)
    return bits;

  if (n >= 256)
    return _mm256_setzero_si256();

  // Shift all lanes right
  __m256i shifted = _mm256_srli_epi64(bits, n % 64);

  // Shift all lanes left by 64 - n
  // This moves n LSB of each lane to the MSB side
  __m256i carry = _mm256_slli_epi64(bits, 64 - (n % 64));

  carry = _mm256_permute4x64_epi64(carry, _MM_SHUFFLE(2, 1, 0, 0));
  carry = _mm256_and_si256(carry, _mm256_set_epi64x(-1LL, -1LL, -1LL, 0LL));
  __m256i result = _mm256_or_si256(shifted, carry);

  if ((n / 64) == 1) {
    result = _mm256_permute4x64_epi64(result, _MM_SHUFFLE(2, 1, 0, 0));
    result = _mm256_and_si256(result, _mm256_set_epi64x(-1LL, -1LL, -1LL, 0LL));
  } else if ((n / 64) == 2) {
    result = _mm256_permute4x64_epi64(result, _MM_SHUFFLE(1, 0, 0, 0));
    result = _mm256_and_si256(result, _mm256_set_epi64x(-1LL, -1LL, 0LL, 0LL));
  } else if ((n / 64) == 3) {
    result = _mm256_permute4x64_epi64(result, _MM_SHUFFLE(0, 0, 0, 0));
    result = _mm256_and_si256(result, _mm256_set_epi64x(-1LL, 0LL, 0LL, 0LL));
  }

  return result;
}

int GreedyMeshi256::ffs256(__m256i &bits) {
  uint64_t *p = reinterpret_cast<uint64_t *>(&bits);

  int result = 0;

  for (int i = 3; i >= 0; --i) {
    int current = __builtin_ffsll(p[i]);
    if (current != 0)
      return current + result;
    result += 64;
  }

  return result;
}

__m256i GreedyMeshi256::clb256(__m256i &bits, int n) {
  if (n >= 256)
    return _mm256_setzero_si256();

  alignas(32) uint64_t maskArr[4] = {~0ULL, ~0ULL, ~0ULL, ~0ULL};

  for (int i = 3; i >= 0; --i) {
    if (n >= 64) {
      maskArr[i] = 0ULL;
      n -= 64;
    } else {
      maskArr[i] = (maskArr[i] & ~((1ULL << n) - 1));
      break;
    }
  }

  __m256i mask = _mm256_load_si256(reinterpret_cast<const __m256i *>(&maskArr));
  return _mm256_and_si256(bits, mask);
}

__m256i GreedyMeshi256::mmr(int size, int offset) {
  if (size >= 256)
    return _mm256_set1_epi64x(~0ULL);

  alignas(32) uint64_t arr[4] = {0ULL, 0ULL, 0ULL, 0ULL};

  for (int i = 3; i >= 0; --i) {
    if (size >= 64) {
      arr[i] = ~0ULL;
      size -= 64;
    } else {
      arr[i] = ((1ULL << size) - 1);
      break;
    }
  }

  __m256i mask = _mm256_load_si256(reinterpret_cast<const __m256i *>(&arr));

  return sl256(mask, offset % 256);
}

// AND
// result = _mm256_and_si256(vec, vec);

// OR
// result = _mm256_or_si256(vec, vec);

// XOR
// result = _mm256_xor_si256(vec, vec);

// NOT
// result = _mm256_xor_si256(vec, _mm256_set1_epi64x(-1));

void GreedyMeshi256::Octree(SparseVoxelOctree *tree,
                            std::vector<Vertex> &vertices, int originX,
                            int originY, int originZ, int depth,
                            Voxel *filter) {
  glm::vec3 coord = {originX / s_CHUNK_SIZE, originY / s_CHUNK_SIZE,
                     originZ / s_CHUNK_SIZE};

  // LOG_IVEC3("coord", coord);

  /**
   * I align by 32 bytes here because I want to _mm256_load_si256 not
   * _mm256_loadu_si256 Since my offset is 4, it should work out. Otherwise I'll
   * just use _mm256_loadu_si256
   */
  uint64_t *rows =
      static_cast<uint64_t *>(_mm_malloc(sizeof(uint64_t) * s_MASK_LENGTH, 32));
  uint64_t *columns =
      static_cast<uint64_t *>(_mm_malloc(sizeof(uint64_t) * s_MASK_LENGTH, 32));
  uint64_t *layers =
      static_cast<uint64_t *>(_mm_malloc(sizeof(uint64_t) * s_MASK_LENGTH, 32));
  uint8_t *padding =
      static_cast<uint8_t *>(_mm_malloc(sizeof(uint8_t) * s_MASK_LENGTH, 32));

  memset(rows, 0, sizeof(uint64_t) * s_MASK_LENGTH);
  memset(columns, 0, sizeof(uint64_t) * s_MASK_LENGTH);
  memset(layers, 0, sizeof(uint64_t) * s_MASK_LENGTH);
  memset(padding, 0, sizeof(uint8_t) * s_MASK_LENGTH);

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

          // LOG("initial col index",
          //     FlipScalarIndex(static_cast<int>(columnIndex / s_BITS)));

          rows[FlipScalarIndex(static_cast<int>(rowIndex / s_BITS))] |=
              (1ULL << (rowIndex % s_BITS));
          columns[FlipScalarIndex(static_cast<int>(columnIndex / s_BITS))] |=
              (1ULL << (columnIndex % s_BITS));
          layers[FlipScalarIndex(static_cast<int>(layerIndex / s_BITS))] |=
              (1ULL << (layerIndex % s_BITS));
        }

  if (!hasVoxels)
    return;

  // for (size_t i = 0; i < s_MASK_LENGTH; i++) {
  //   LOG_TO_FILE("columns-256", std::bitset<64>(columns[i]),
  //               std::bitset<64>(columns[i + 1]),
  //               std::bitset<64>(columns[i + 2]),
  //               std::bitset<64>(columns[i + 3]));
  // }

  // for (size_t i = 0; i < s_MASK_LENGTH; i++) {
  //   LOG_TO_FILE("rows-256", std::bitset<64>(rows[i]),
  //               std::bitset<64>(rows[i + 1]), std::bitset<64>(rows[i + 2]),
  //               std::bitset<64>(rows[i + 3]));
  // }

  // for (size_t i = 0; i < s_MASK_LENGTH; i++) {
  //   LOG_TO_FILE("layers-256", std::bitset<64>(layers[i]),
  //               std::bitset<64>(layers[i + 1]), std::bitset<64>(layers[i +
  //               2]), std::bitset<64>(layers[i + 3]));
  // }

  /**
   * Here we capture the padding bit
   * Every chunk we need to get the neighbour chunks and check
   * if the bits at the 0 & 31st index are on. If so, we need to skip making
   * faces on that end.
   *
   * TODO: Needs to be tested with multiple chunks.
   */
  for (int i = 0; i < s_MASK_LENGTH; i += s_STEPS) {

    __m256i row =
        _mm256_load_si256(reinterpret_cast<const __m256i *>(&rows[i]));
    __m256i column =
        _mm256_load_si256(reinterpret_cast<const __m256i *>(&columns[i]));
    __m256i layer =
        _mm256_load_si256(reinterpret_cast<const __m256i *>(&layers[i]));

    int fast = i % s_CHUNK_SIZE;
    int slow = (i / s_CHUNK_SIZE) % s_CHUNK_SIZE;
    int pi = i / s_STEPS;

    if (!_mm256_testz_si256(row, row)) {
      int rMSB = (s_CHUNK_SIZE - 1) - clz256(&rows[i]) + 1;
      int rLSB = ctz256(&rows[i]) - 1;

      if (tree->get(originX + rMSB, fast + originY, slow + originZ, depth))
        padding[pi] |= (1ULL << 1);

      if (tree->get(originX + rLSB, fast + originY, slow + originZ, depth))
        padding[pi] |= (1ULL << 0);
    }

    if (!_mm256_testz_si256(column, column)) {
      int cMSB = (s_CHUNK_SIZE - 1) - clz256(&columns[i]) + 1;
      int cLSB = ctz256(&columns[i]) - 1;

      if (tree->get(fast + originX, originY + cMSB, slow + originZ, depth))
        padding[pi] |= (1ULL << 3);

      if (tree->get(fast + originX, originY + cLSB, slow + originZ, depth))
        padding[pi] |= (1ULL << 2);
    }

    if (!_mm256_testz_si256(layer, layer)) {
      int lMSB = (s_CHUNK_SIZE - 1) - clz256(&layers[i]) + 1;
      int lLSB = ctz256(&layers[i]) - 1;

      if (tree->get(slow + originX, fast + originY, originZ + lMSB, depth))
        padding[pi] |= (1ULL << 5);

      if (tree->get(slow + originX, fast + originY, originZ + lLSB, depth))
        padding[pi] |= (1ULL << 4);
    }
  }

  uint64_t *widthStart =
      static_cast<uint64_t *>(_mm_malloc(sizeof(uint64_t) * s_MASK_LENGTH, 32));
  uint64_t *heightStart =
      static_cast<uint64_t *>(_mm_malloc(sizeof(uint64_t) * s_MASK_LENGTH, 32));

  uint64_t *widthEnd =
      static_cast<uint64_t *>(_mm_malloc(sizeof(uint64_t) * s_MASK_LENGTH, 32));
  uint64_t *heightEnd =
      static_cast<uint64_t *>(_mm_malloc(sizeof(uint64_t) * s_MASK_LENGTH, 32));

  std::memset(widthStart, 0, sizeof(uint64_t) * s_MASK_LENGTH);
  std::memset(heightStart, 0, sizeof(uint64_t) * s_MASK_LENGTH);
  std::memset(widthEnd, 0, sizeof(uint64_t) * s_MASK_LENGTH);
  std::memset(heightEnd, 0, sizeof(uint64_t) * s_MASK_LENGTH);

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

  GreedyMeshi256Axis(coord, rows, widthStart, heightStart, widthEnd, heightEnd,
                     vertices, FaceType::LEFT, FaceType::RIGHT);

  std::memset(widthStart, 0, sizeof(uint64_t) * s_MASK_LENGTH);
  std::memset(heightStart, 0, sizeof(uint64_t) * s_MASK_LENGTH);
  std::memset(widthEnd, 0, sizeof(uint64_t) * s_MASK_LENGTH);
  std::memset(heightEnd, 0, sizeof(uint64_t) * s_MASK_LENGTH);

  PrepareWidthHeightMasks(columns, 2, padding, widthStart, heightStart,
                          widthEnd, heightEnd);

  GreedyMeshi256Axis(coord, columns, widthStart, heightStart, widthEnd,
                     heightEnd, vertices, FaceType::BOTTOM, FaceType::TOP);

  std::memset(widthStart, 0, sizeof(uint64_t) * s_MASK_LENGTH);
  std::memset(heightStart, 0, sizeof(uint64_t) * s_MASK_LENGTH);
  std::memset(widthEnd, 0, sizeof(uint64_t) * s_MASK_LENGTH);
  std::memset(heightEnd, 0, sizeof(uint64_t) * s_MASK_LENGTH);

  PrepareWidthHeightMasks(layers, 4, padding, widthStart, heightStart, widthEnd,
                          heightEnd);
  GreedyMeshi256Axis(coord, layers, widthStart, heightStart, widthEnd,
                     heightEnd, vertices, FaceType::FRONT, FaceType::BACK);

  _mm_free(rows);
  _mm_free(columns);
  _mm_free(layers);
  _mm_free(padding);
  _mm_free(widthStart);
  _mm_free(heightStart);
  _mm_free(widthEnd);
  _mm_free(heightEnd);
}