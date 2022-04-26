[![Build Status](https://app.travis-ci.com/kvanhoey/ParallelHalfedgeSubdivision.svg?token=Hduw3SXVyXsHpjsbgt9p&branch=main)](https://app.travis-ci.com/kvanhoey/ParallelHalfedgeSubdivision)
[![Build status](https://ci.appveyor.com/api/projects/status/irhuj5vnb9s8k0db?svg=true)](https://ci.appveyor.com/project/kvanhoey/parallelhalfedgesubdivision)


This repository provides a library for fast subdivision of polygonal surfaces.
Currently it supports Catmull-Clark and Loop subdivision, and has both CPU and GPU backends with parallel implementations.
For most use cases, subdivision is real-time.
It is the implementation of the paper
* [A Halfedge Refinement Rule for Parallel Loop Subdivision](http://kenneth.vanhoey.free.fr/index.php?page=research&lang=en#VD22), Eurographics 2022 short paper, by [Kenneth Vanhoey](http://kvanhoey.eu/) and [Jonathan Dupuy](http://onrendering.com).

and an alternate implementation (for the official reference see [here](https://github.com/jdupuy/HalfedgeCatmullClark)) of
* [A Halfedge Refinement Rule for Parallel Catmull-Clark Subdivision](http://kenneth.vanhoey.free.fr/index.php?page=research&lang=en#DV21), HPG 2021, by [Jonathan Dupuy](http://onrendering.com) and [Kenneth Vanhoey](http://kvanhoey.eu/)


<img src="img/loop_subdiv.jpg" alt="Example of Loop subdivision" width="500"/>

## Setup
### License
Apart from the submodules folder, the code from this repository is released with the CC-BY licence.

### Cloning

Clone the repository and all its submodules using the following command:
```sh
git clone --recursive git@github.com/kvanhoey/ParallelHalfedgeSubdivision.git
```

If you accidentally omitted the `--recursive` flag when cloning the repository you can retrieve the submodules like so:
```sh
git submodule update --init --recursive
```

### Compilation
This is a CMake project. The root folder contains the main CMakeLists.txt file. Tested on Window 10 and Linux Ubuntu.

To compile manually:
```sh
mkdir build/  # create compilation folder
cd build/     # move into it
cmake ..      # Call cmake on the folder containing CMakeLists.txt
make [<name_of_executable>]  # Compile one or all of the executables
```
#### Optional
By default, documentation is not compiled. Please set the CMake variable `BUILD_DOC` to `ON` using `ccmake` or by directly editing `CMakeLists.txt` before hitting `cmake` and `make doc`, which will generate the `html/index.html` file.




## Content

### Organization
The `lib/` folder contains the library files: see [lib/README.md](lib/README.md)

The `root` folder contains several usage examples that do the following:
* `catmull-clark_cpu` Catmull-Clark subdivision using the CPU backend
* `catmull-clark_gpu` Catmull-Clark subdivision using the GPU backend
* `loop_cpu` Loop subdivision using the CPU backend
* `loop_gpu` Loop subdivision using the GPU backend
* `stats` provide statistics of a loaded Mesh.

Notes:
* The CPU backend relies on OpenMP for parallelization. By default, it uses as many threads as there are CPU cores available. This can be altered by setting the environment variable `OMP_NUM_THREADS` to another value. For example: `export OMP_NUM_THREADS=2`
* The GPU backend relies on OpenGL (library provided under [`lib/gpu_dependencies`](lib/gpu_dependencies)). Shader files are loaded using relative paths, so the executable has to be launched from a subfolder of the root folder, e.g., `build/`.
* All executables take for input an OBJ file (note: for Loop subdivision, the mesh should be triangle-only) and a subdivision depth.
* The resulting subdivision is written to disk as an OBJ file. It is triangular for Loop subdivision, and quad-only for Catmull-Clark subdivision.

The `meshes` folder contains example meshes that can be used as inputs.

The `doc` folder contains a doxygen file to generate documentation.

