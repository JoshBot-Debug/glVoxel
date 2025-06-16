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

32bit Greedy meshing for 128x128x128
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 47.5163 ms (average) over 100 iterations
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 45.4442 ms (average) over 100 iterations
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 42.3203 ms (average) over 100 iterations
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 47.2106 ms (average) over 100 iterations
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 45.2261 ms (average) over 100 iterations
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 41.912 ms (average) over 100 iterations