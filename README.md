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

[OFFICE] 64bit Greedy meshing for 128x128x128
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:175 (generateChunk): 58.5039 ms
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 39.9131 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 39.9274 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 39.8916 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 81.9613 ms (average) over 300 iterations

[OFFICE] 32bit Greedy meshing for 128x128x128
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:175 (generateChunk): 56.3936 ms
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 40.2014 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 40.1667 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 40.2461 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 89.3898 ms (average) over 300 iterations

[OFFICE] Old 32bit Greedy meshing for 128x128x128
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:175 (generateChunk): 56.2263 ms
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 40.2369 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 40.2538 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 40.291 ms (average) over 300 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:196 (meshChunk): Took: 104.808 ms (average) over 300 iterations

[OFFICE] 64bit Greedy meshing for 128x128x128 Terrain
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:171 (generateChunk): 206.777 ms
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:192 (meshChunk): Took: 123.18 ms (average) over 100 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:192 (meshChunk): Took: 118.113 ms (average) over 100 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:192 (meshChunk): Took: 113.333 ms (average) over 100 iterations

[OFFICE] 32bit Greedy meshing for 128x128x128 Terrain
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:171 (generateChunk): 204.245 ms
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:192 (meshChunk): Took: 124.445 ms (average) over 100 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:192 (meshChunk): Took: 120.457 ms (average) over 100 iterations
LOG /home/joshua/TMP/glVoxel-main/src/World/VoxelManager.cpp:192 (meshChunk): Took: 111.776 ms (average) over 100 iterations

[HOME] 64bit Greedy meshing for 128x128x128 Terrain
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:171 (generateChunk): 87.7662 ms
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 47.8704 ms (average) over 200 iterations
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 44.3581 ms (average) over 200 iterations
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 43.0328 ms (average) over 200 iterations

[HOME] 32bit Greedy meshing for 128x128x128 Terrain
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:171 (generateChunk): 87.5471 ms
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 47.9275 ms (average) over 200 iterations
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 45.875 ms (average) over 200 iterations
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 42.8006 ms (average) over 200 iterations

[HOME] 64bit Greedy meshing for 128x128x128 Terrain \w parallelization
# Pratically this is not realistic because we will be building more than 8 chunks at a time so we
# cannot parallelize this. In fact performance degrades if we try to because more than 8 threads will be used.
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:171 (generateChunk): 88.5121 ms
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 14.0636 ms (average) over 200 iterations
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 15.9212 ms (average) over 200 iterations
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:192 (meshChunk): Took: 15.6241 ms (average) over 200 iterations


64bit
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:240 (meshChunk): Took: 262.552 ms (average) over 50 iterations

32bit
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:240 (meshChunk): Took: 283.879 ms (average) over 50 iterations

256bit
LOG /home/joshua/Projects/glVoxel/src/World/VoxelManager.cpp:240 (meshChunk): Took: 398.598 ms (average) over 50 iterations


# Performance Tool
valgrind --tool=callgrind ./build/glVoxel
gprof2dot --format=callgrind --output=out.dot ./callgrind.out.264922
dot -Tpng out.dot -o graph.png