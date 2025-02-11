# Objectives. Outline.
  (a) The use of Structure of array (SoA) instead of Array of Structures (AoS) is preferred mainly for the reason of vectorization of array which guarantees.
    (i) Better Memory Alignment and Cache Utilization
    (ii) Better for SIMD implementation (Multithreading/GPU)
    (iii) GPU Performance
  (b) Readability.
  (c) Direct visualization using javascript. (Portability)
  (d) Should be able to run in CUDA, OpenMP+/MPI.
  (e) Mempooling to accomodate large dataset thus avoiding realloc and repeated malloc.
  (f) IMP: Need some sort of lexical for cleaner/faster writing.
  (g) Prefer Modular Programming approach where you use "separate compilation". 
  (h) reserve keywords "forEach", "meshes". 
  (i) Iterator example
    (i) forEach(m in meshes) //each mesh in meshes
    (ii) forEach(p in m.vertices)  //each vertex of m
    (iii) forEach(h in m.halfedges) //each halfedge of m
    (iii) forEach(e in m.edges) //each edges of m
    (iv) forEach(f in m.faces) //each triangle
    (v) forEach(s in m.vertex.scalars) // each vertex scalars
    (vi) forEach(v in m.vertex.vectors) // each vertex vectors 
    (vii) forEach(s in m.edge.scalars) // each edge scalars
    (viii) forEach(v in m.edge.vectors) // each edge vectors 
    (vii) forEach(s in m.face.scalars) // each face scalars
    (viii) forEach(v in m.face.vectors) // each face vectors 
  (j) Global Variable;
      m.face.vector.add("c"); // 
      v = m.face.vector.get(); //allocate "v". will free before the end of the scope
  (k) requirement of vertices, vectors/scalars, 
    (i) Least sparse
    (ii) Option to compact. Remove sparseness completely, by compacting the mempool
    (iii) should be able to vectorize. 
    (iv) If sparseness is minimal, GPU computatation on vectorized arrays neednot skip the sparse part of the array.
         Same for multithreading.
    (v) But in CPU, you can  iterate through only the occupied part of the linked list.
    
