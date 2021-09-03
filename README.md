This repository provides a library for fast subdivision of polygonal surfaces.
Currently it supports Catmull-Clark and Loop subdivision, and has both CPU and GPU backends.
The CPU version runs using OpenMP parallelization, while the GPU version runs in OpenGL.
For most use cases, subdivision is real-time.

This library is the implementation of the following two papers:
* [A Halfedge Refinement Rule for Parallel Catmull-Clark Subdivision](http://kenneth.vanhoey.free.fr/index.php?page=research&lang=en#DV21), HPG 2021, by [Jonathan Dupuy](http://onrendering.com) and [Kenneth Vanhoey](http://kvanhoey.eu/)
* A Halfedge Refinement Rule for Parallel Loop Subdivision (under review)


# Organization
The lib/ folder contains the library files.
The root folder contains several usage examples that do the following:
* Catmull-Clark subdivision using the CPU backend
* Catmull-Clark subdivision using the GPU backend
* Loop subdivision using the CPU backend
* Loop subdivision using the GPU backend

In all cases, the result is written to disk as on OBJ file.
For Loop subdivision: the input mesh should be triangular.


