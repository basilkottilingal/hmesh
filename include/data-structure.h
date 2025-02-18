/* _Flag : used for unsigned numbers in the range [0,256), like 
..  (a) counting numbers < 256, 
..  (b) error flags, 
..  etc.
*/
typedef uint8_t _Flag;

/* _DataType : Precision used in scientific computation.
.. 32 bits is Preferred data type precision for GPU vectors.
.. Everywhere else (and by default) it's used as 64.
*/
#ifdef _PRECISION32
typedef float _DataType;
#else 
typedef double _DataType;
#endif
typedef uint32_t _IndexType;

/* '_Index' : a node of the Linked list . 
.. A data node may represent a point, or a halfedge, 
.. or an orineted face (mostly triangle);
*/ 
typedef struct _Index {

  /* Global Index for a data node (of same kind) . 
  .. This is used to point to a scalar associated,
  .. with this data node. 
  .. fixme: See if you need 2^32 (~4 Billion) Nodes?
  _IndexType i;
  */

  /* 'flags' : Store flags related to the index.
    Ex: to store if this index is occupied 
  */
  Flag flags;

#ifdef MPI_VERSION
  /* 'pid' : processor ID 
  */
  uint32_t pid;
#endif
  /* 'next' : used to keep a linked list of indices 
  */
  _Index * next;

  /* 'prev' : previous of 'this' index.
  */
  _Index * prev;

}_Index;

/* '_IndexBlock' : Chunks of indices. 
.. (a) avoids repeated malloc/realloc,
.. (b) chunks of size close L2 cache (need optimization)
..  can help to maintain locality of 'next' node within
..  easily accessible range reducing overhead while
..  pointer traversal.
*/

typedef struct _IndexBlock {
  /* 'address' : the memory block which stores the node indices. 
  .. Used only for freeing when you destroy this block.
  */
  void * address;
 
  /* 'used' : starting node among the occuppied list
  */
  _Index * used;

  /* 'empty' : starting node in the empty/free list 
  .. All the occuppied+empty constitutes a memory block.
  */
  _Index * empty;

  /* 'next' : next index block */
  _IndexBlock * next; 

  /* 'nempty' : Number of empty nodes 
  */
  uint16_t nempty;

  /* object functions */
  Flag        (* init)     (_Points * p);
  (_Index * ) (* add)      (_points * p);
  Flag        (* remove)   (_Points * p, void * node);
  
} _IndexBlock;

typedef struct _VariableBlock {
  /* 'block' : the memory block which stores the scalar data. 
  .. Used only for freeing when you destroy this block.
  */
  void * address;

  /* 'u' : a memory block allocated for a scalar variable. 
  .. IMPORTANT. The number of double (or float or any other vartype) 
  .. a _VariableBlock block accomodates, SHOULD be same as the number of
  .. indices an _IndexBlock can accomodates.
  */
  void * u;

  /* 'next' :  next variable block corresponding to next index 
  .. block.
  */
  _IndexBlock * block;

}_VariableBlock;

typedef struct _VertexBlock {
  /* 'block' : the memory block which stores the indices of 
  .. vertices of a edge or a face. 
  .. Used only for freeing when you destroy this block.
  */
  DataBlock * block;

  /* 
  */
  _IndexType * i, * j, * k;

  /* 'next' :  next variable block corresponding to next index 
  .. block.
  */
  _IndexBlock * block;

}_VariableBlock;

typedef struct _Scalar {
  /* 'type' : Variable type. 
  .. Encodes information on variable type. is it
  .. (a) a constant ?,
  .. (b) is it a scalar, a vector or a tensor,
  .. (c) correspond for a vertex, edge center, face center,
  ..  volume center
  */
  _Flag type;

  /* Name of the variable */
  char * name;

  /* 'i' : i \in [0, VAR_MAX) */
  _Flag i;

}_Scalar;

/* Collection of points */
typedef struct _ManifoldCells {

  /* 'first' : first IndexBlock that stores indices of 
  .. of points */
  _IndexBlock * first; 

  /* 'type' : 0:point, 1:halfedge, 2:triangle, 3:terahedron
  */
  _Flag type;

  /* 'vertices' of cells. For points it will be empty.
  */
  _VertexBlock * vertices;

  /* 'twin' : it applies for a half-edge or a half-face.
  */
  _VertexBlock * twin;
  
  /* 'sub' : for a half-edge it represents points, and for a 
  .. face it represent half-edges. 
  */
  _ManifoldCells * sub;


  /* 'vars' : variable block corresponding to each variable */ 
  _VariableBlock * vars[NVAR_MAX];

  /* 's' : s[ivar] contains info of variable stored in indices 
  .. of vars[ivar] */ 
  _Scalar s[NVAR_MAX];

  /* object functions */
  Flag        (* init)   (_Points * p);
  (_Index * ) (* add)    (_points * p);
  Flag        (* remove) (_Points * p, void * node);
  (void *)    (* var)    (_Points * p, char * name, Flag type);
  (void *)  (* var_remove) (_Points * p, char * name);

}_ManifoldCell;

/* _Manifold: Mesh or a discretized manifold */
typedef struct {
  /* A large pool to accomodate blocks of indices of points,
  .. edges  and triangles (may be polygons too), index of 
  .. vertices of edges, edges of triangles (may be triangles 
  .. of tetrahedrons too), scalars of points, edges, triangles
  .. (may be polygons too).
  */
  _Mempool * pool;

  /* 'd' : dimension of manifold in [0,3]. They can be,
  .. 0: Collection of disconnected Points
  .. 1: Discretized curves in 2-D or 3-D
  .. 2: Discretized 3-D sufaces. Ex: sets of 4 faces of a tetrahedron.
  .. 3: Volume mesh in 3-D.
  .. Example. Surface of a sphere is a 2-D manifold.
  */
  _Flag d;

  /* 'D' : Dimension of space. For us, 'D' in [2,3].
  .. NOTE: It's always (d <= D)
  */
  _Flag D;
  
  /* set of manifold cells like faces, edges and points 
  .. NOTE: They can be empty, 
  .. ex: a curve, which is a 1-D manifold in 3D Eulerian space,
  .. will have empty face list ('f').
  */
  _ManifoldCells * points, 
                 * edges, 
                 * triangles, 
                 * tetrahedron; 

}_Manifold;
