# Objectives. Outline.

  * The use of **Structure of array (SoA)** instead of Array of Structures (AoS) is preferred mainly for the reason of vectorization of array which guarantees.
    * Better **Memory Alignment** and Cache Utilization
    * Better for SIMD implementation (Multithreading/GPU)
    * GPU Performance
  * Readability.
  * Direct visualization using javascript. (Portability).
  * Standardize mesh input/output.
  * Should be able to run in CUDA, OpenMP+/MPI.
  * Mempooling to accomodate large dataset thus avoiding realloc and repeated malloc.
    * IMPORTANT : Need to read on mmap()
    * as of now, tree-pool.h is using mmap() which is not std C99, but available in >= POSIX.1-2001.
 It might be good idea to make it portable. (if not available, use malloc.)
  * Prefer Modular Programming approach where you use "separate compilation". 
  * Need a lexical preprocessor or in-house compiler that takes care of
    * Linear operations ( for discrete calculus or discrete exterior calculus (DEC) ) that involves scalars, vectors and tensors. 
    * additional syntax for easier mesh manipulation. (easier in writing)
    * 
  * **IMP: addition C syntax/grammar**: Need some sort of lexical for cleaner/faster writing. reserve keywords "forEach", "meshes".  Iterator example :
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
  * In-house compiler is required for adding syntax.
    * preprocessing can be done by gcc itself.
    * flex/bison may be employed for creating parser.
    * **It might be a practical idea: write back to C and compile rather than writing a full compiler**. (unrelated: \_\_free\_\_ \_\_attribute\_\_ in gcc)
  * In house parser had to take care of mesh variables.
    * scalar/vector/mesh\_id maybe identified by unsigned int,
    * globlal/local stack size. Initial analysis (pre running / during compilation)  of memory requirement for mesh.
  * Global Variable;
    * m.face.vector.add("c"); // 
    * v = m.face.vector.get(); //allocate "v". will free before the end of the scope
  * requirement of vertices, vectors/scalars, 
    * Least sparse. BUT : reducing sparseness increases overhead in insertion!!. So how can you optimize
    * Option to compact. Remove sparseness completely, by compacting the mempool
    * should be able to vectorize. 
    * If sparseness is minimal, GPU computatation on vectorized arrays neednot skip the sparse part of the array.
         (Same for multithreading.)
    * But in CPU, you can  iterate through only the occupied part of the linked list.
  * challenge in using many blocks to represent a non-partitioned manifold in a single CPU memory, 
    * while searching for the pointer in memory (vertex/edge/etc..) is the locality of searched memory pointer. If locality is not guaranteed TLB searching will fail and start manually searching for memory in the memory hardware which might be  
  * Error handling. (type \_Flag) 
    * Return 0 : success. 
    * > 0 : failed. Identify error type. 
    * NOTE : For pointer returns , returning NULL means failure, and success otherwise.
    * may include an error.h
  * Coding Style
    * typedef datatypes starts with underscore followed by caps letter, and the rest are small letters. Ex: \_Mempool, \_Flag
    * Function name. Starts with word that represent the datatype, each word starts with caps, no underscore in b/w. 
Example : Mempool(\_Mempool \*);
    * Good intendation. 
