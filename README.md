# glVoxel
- 21st December 2024

## Greedy meshing & Face culling

4th Feb 2025

I have created a meshing algorithm that uses bitwise operations to perform 
greedy meshing and face culling at incredible speeds.

To create a cube in a 32x32x32 grid it takes **~0.0207172ms** to generate vertices.
To create a sphere in a 32x32x32 grid it takes **~0.0521647ms** to generate vertices.

In the worst case scenario, where no faces are adjacent to each other. It takes **~0.476049ms** to generate vertices.

To build this project

```c++
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
```

To run the project:
```bash
build/glVoxel
```

To play around go to World/World.h. Check line 35, and play around!

## Benchmarks

64bit Greedy meshing for 128x128x128
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:175 (generateChunk): 58.5039 ms
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 39.9131 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 39.9274 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 39.8916 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 81.9613 ms (average) over 300 iterations

32bit Greedy meshing for 128x128x128
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:175 (generateChunk): 56.3936 ms
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 40.2014 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 40.1667 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 40.2461 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 89.3898 ms (average) over 300 iterations

Old 32bit Greedy meshing for 128x128x128
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:175 (generateChunk): 56.2263 ms
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 40.2369 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 40.2538 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 40.291 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 104.808 ms (average) over 300 iterations

64bit Greedy meshing for 128x128x128 Terrain
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:171 (generateChunk): 206.777 ms
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:192 (meshChunk): Took: 123.18 ms (average) over 100 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:192 (meshChunk): Took: 118.113 ms (average) over 100 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:192 (meshChunk): Took: 113.333 ms (average) over 100 iterations

32bit Greedy meshing for 128x128x128 Terrain
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:171 (generateChunk): 204.245 ms
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:192 (meshChunk): Took: 124.445 ms (average) over 100 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:192 (meshChunk): Took: 120.457 ms (average) over 100 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:192 (meshChunk): Took: 111.776 ms (average) over 100 iterations