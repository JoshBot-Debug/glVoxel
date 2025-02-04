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

To play around go to World/World.h. Check line 35, and play around!