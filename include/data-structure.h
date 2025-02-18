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
typedef uint32_t _Index;

/* '_Node' : a node of the Linked list . 
.. A data node may represent a point, or a halfedge, 
.. or an oriented face (mostly triangle);
*/ 
typedef struct _Node {

  /* Global Index for a data node (of same kind) . 
  .. This is used to point to a scalar associated,
  .. with this data node.
  .. NOTE: This pgm limits number of nodes to 
  .. UINT8_MAX x 2^15 = 8388608  per processor.
  */
  _Index i;

  /* 'flags' : Store flags related to the index.
    Ex: to store if this index is occupied 
  */
  _Flag flags;

#ifdef MPI_VERSION
  /* 'pid' : processor ID 
  */
  uint32_t pid;
#endif
  /* 'next' : used to keep a linked list of indices 
  */
  _Node * next;

  /* 'prev' : previous of 'this' index.
  .. NOTE: Why you need 'prev'? The linked list is 
  .. a 'doubly linked list' only which can let you
  .. delete any occuppied index.
  */
  _Node * prev;

}_Node;

/* '_NodeBlock' : Chunks of indices. 
.. (a) avoids repeated malloc/realloc,
.. (b) chunks of size close L2 cache (need optimization)
..  can help to maintain locality of 'next' node within
..  easily accessible range reducing overhead while
..  pointer traversal.
*/

typedef struct _NodeBlock {
  /* 'used' : starting node among the occuppied list
  */
  _Node * used;

  /* 'empty' : starting node in the empty/free list 
  .. All the occuppied+empty constitutes a memory block.
  */
  _Node * empty;

  /* 'nempty' : Number of empty nodes 
  */
  uint16_t nempty;

  /* object functions .
  .. fixme: remove add(), and remove() from here and 
  .. use inline functions wherever required. */
  _Flag       (* init)     (_NodeBlock *, _Index );
  (_Node * )  (* add)      (_NodeBlock * );
  Flag        (* remove)   (_NodeBlock * , void * node);
  
} _NodeBlock;

typedef struct _IndexMap {
  /* Map one kind of index to another kind.
  .. Ex1: map indices of triangles to their 0-th vertex.
  .. Ex2: map indices of halfedges to their respective twins
  */
  _Mempool * map;
}_IndexMap;

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
  /* A large pool to accomodate blocks of indices of points,
  .. edges  and triangles (may be polygons too), index of 
  .. vertices of edges, edges of triangles (may be triangles 
  .. of tetrahedrons too), scalars of points, edges, triangles
  .. (may be polygons too).
  */
  _Mempool * pool;

  /* Number of blocks (usually 1 block accomodate 1<<15 nodes)
  .. NOTE: Redundant. this->nblocks = pool->nblocks;
  */
  _Flag nblocks;

  /* Each NodeBlock corresponding to pool->blocks[iblock]
  */ 
  _NodeBlock ** indices;

  /* 'type' : 0:point, 1:halfedge, 2:triangle, 3:terahedron
  */
  _Flag type;

  /* vertices of cells. For points i,j,k = NULL. 
  .. For edges k = NULL.
  .. NOTE: vertices are ordered.
  .. Ex: (i,j) will be ordered set of half-edges.
  */
  _IndexMap i, j, k;

  /* 'twin' : it applies for a half-edge or a half-face.
  */
  _IndexMap twin;
  
  /* 'sub' : for a half-edge it represents points, and for a 
  .. face it represent half-edges. 
  */
  _ManifoldCells * sub;

  /* 'vars' : Mempool of blocks corresponding to each variable */ 
  _Mempool * vars[NVAR_MAX];

  /* 's' : s[ivar] contains info of variable stored in indices 
  .. of vars[ivar] */ 
  _Scalar s[NVAR_MAX];

  /* object functions */
  Flag       (* init)   (_ManifoldCells * );
  (_Node *)  (* add)    (_ManifoldCells * );
  Flag       (* remove) (_ManifoldCells *, void *);
  (void *)   (* var)    (_ManifoldCells *, char *, Flag);
  (void *)   (* var_remove) (_ManifoldCells *, char *);

}_ManifoldCells;

/* _Manifold: Mesh or a discretized manifold */
typedef struct {

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
