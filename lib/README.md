# ParallelHalfedgeSubdivision library
The main part of the library is implemented in the C++ Mesh<...> classes that follow the following inheritence rules:

<img src="../img/class_hierarchy.png" alt="Library class hierarchy" width="800"/>

This scheme and detailed technical description of the classes are provided as a Doxygen documentation (see [../README.md](../README.md) for how to generate documentation).

# Class hierarchy
In the graph above, the root node (i.e., Mesh) and leave nodes can be instanciated, while the intermediate levels are pure virtual (or abstract) classes. Examples of how to use them are provided in the examples of this repository.
* *Mesh* is the base class representing a mesh. It can be instanciated as is.
* *Mesh_Subdiv* extends the Mesh class (i.e., it inherits from it) and is a virtual (or abstract). It defines the methods that necessarily have to be implemented for any class that proposes subdivision routines (and inherits from it).
* *Mesh_Subdiv_CPU* and *Mesh_Subdiv_GPU* extend *Mesh_Subdiv* and are pure virtual too. They add requirements and implementations for CPU-backend and GPU-backend subdivision methods, respectively.
* *Mesh_Subdiv_Loop* and *Mesh_Subdiv_Catmull-Clark* extend *Mesh_Subdiv* and are pure virtual too. They add add requirements and implementations for the Loop and Catmull-Clark subdivision schemes, respectively.
* Finally, the four leave classes derive from two pure virtual classes: one among *Mesh_Subdiv_Loop* and *Mesh_Subdiv_CatmullClark*, and one among *Mesh_Subdiv_CPU* and *Mesh_Subdiv_GPU* and fully implement a subdivision solution for either Loop or Catmull-Clark subdivision on either the CPU or GPU backend.

