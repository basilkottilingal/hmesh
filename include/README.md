# Objectives. Outline.

  * The use of Structure of array (SoA) instead of Array of Structures (AoS) is preferred mainly for the reason of vectorization of array which guarantees.
    * Better Memory Alignment and Cache Utilization
    * Better for SIMD implementation (Multithreading/GPU)
    * GPU Performance
  * Readability.

  * Direct visualization using javascript. (Portability)
  * Should be able to run in CUDA, OpenMP+/MPI.
  * Mempooling to accomodate large dataset thus avoiding realloc and repeated malloc.
  * IMP: Need some sort of lexical for cleaner/faster writing.
  * Prefer Modular Programming approach where you use "separate compilation". 
  * reserve keywords "forEach", "meshes". 
  * Iterator example
    * forEach(m in meshes) //each mesh in meshes
    * forEach(p in m.vertices)  //each vertex of m
    * forEach(h in m.halfedges) //each halfedge of m
    * forEach(e in m.edges) //each edges of m
    * forEach(f in m.faces) //each triangle
    * forEach(s in m.vertex.scalars) // each vertex scalars
    * forEach(v in m.vertex.vectors) // each vertex vectors 
    * forEach(s in m.edge.scalars) // each edge scalars
    * forEach(v in m.edge.vectors) // each edge vectors 
    * forEach(s in m.face.scalars) // each face scalars
    * forEach(v in m.face.vectors) // each face vectors 
  * Global Variable;
    * m.face.vector.add("c"); // 
    * v = m.face.vector.get(); //allocate "v". will free before the end of the scope
  * requirement of vertices, vectors/scalars, 
    * Least sparse
    * Option to compact. Remove sparseness completely, by compacting the mempool
    * should be able to vectorize. 
    * If sparseness is minimal, GPU computatation on vectorized arrays neednot skip the sparse part of the array.
         (Same for multithreading.)
    * But in CPU, you can  iterate through only the occupied part of the linked list.
    
