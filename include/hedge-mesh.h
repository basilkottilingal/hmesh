#ifndef _HEDGE_MESH
#define _HEDGE_MESH

#ifdef __cplusplus
extern "C" {
#endif

  #define HMESH_MAX_NBLOCKS 16
  #define HMESH_MAX_NVARS   64
  
  /* '_HmeshAttribute' : a list of memory blocks.
  .. It can be used to store nodes, or attributes of nodes
  .. like scalars etc.
  */
  typedef struct {
    /* memory 'pool' from which blocks are allocated */
    _Mempool * pool;
    
    /*'address' of blocks. for easy access */
    void ** address;

    /*'name' : name of the attribute ,
    .. in case this is an attribute to a node. 
    .. Warning: Name of attribute limited to 31 characters!*/
    char name[32];

    /*  'i' : i is the block number corresponding to
    .. each block in the 'pool'.*/
    _Flag i[HMESH_MAX_NBLOCKS];

    /* 'n' : size of 'address' array
    */
    int n; 

  } _HmeshAttribute;

  /* Scalar is one type of attribute */
  typedef _HmeshAttribute _HmeshScalar;

  /* To add a block */
  extern 
  void * HmeshAttributeAdd(_HmeshAttribute *, _Flag);

  /* To delete a block */
  extern 
  _Flag HmeshAttributeRemove(_HmeshAttribute *, _Flag);

  /* Create an new HmeshAttribute obj */
  extern _HmeshAttribute * 
  HmeshAttribute(char *, size_t, Mempool *);
 
  /* Destroy a HmeshAttribute obj*/
  extern
  _Flag _HmeshAttributeDestroy(_HmeshAttribute *);
 
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

  /* '_HmeshNode' : a node in the Linked list . 
  .. A data node may represent a point, or a halfedge, 
  .. or an oriented face (mostly triangle);
  */ 
  typedef struct _HmeshNode {
    /* 'next' and 'prev' : for linked list.
    .. The linked list is a 'doubly linked list' 
    .. only which can let you delete any occuppied index.
    */
    struct _HmeshNode * next, * prev;

    /* Global Index for a data node (of same kind) . 
    .. This is used to point to a scalar associated,
    .. with this data node.
    .. NOTE: This pgm limits number of nodes to 
    .. UINT8_MAX x 2^15 = 8388608  per processor.
    */
    _Index i;

    /* 'pid' : processor ID 
    */
    _Index pid;

    /* 'flags' : Store flags related to the index.
    .. Ex: to store if this index is occupied.
    .. fixme: May store as a separate array to maintain 
    .. 8x Byte alignment for structure objects
    _Flag flags;
    */

  }_HmeshNode;

  /* '_HmeshNodeBlock' : Chunks of indices. 
  .. (a) avoids repeated malloc/realloc,
  .. (b) chunks of size close L2 cache (need optimization)
  ..  can help to maintain locality of 'next' node within
  ..  easily accessible range reducing overhead while
  ..  pointer traversal.
  */

  typedef struct _HmeshNodeBlock {
    /* 'used' : starting node among the occuppied list
    */
    _HmeshNode * used;

    /* 'empty' : starting node in the empty/free list 
    .. All the occuppied+empty constitutes a memory block.
    */
    _HmeshNode * empty;

    /* It's a linked list*/
    struct _HmeshNodeBlock * next;

  } _HmeshNodeBlock;

  /* Add a new node to the block */
  extern 
  _Node * NodeAdd(_HmeshNodeBlock * block);

  /* Remove an occuppied node from the block*/
  extern 
  _Flag NodeRemove(_HmeshNodeBlock *, _HmeshNode *);

  /* cells of mesh : vertices/edges/triangle/tetrahedrons 
  */
  typedef struct _HmeshCells {
    /* Node : vertex/edge/face/tetrahedron */
    _HmeshAttribute * nodes;

    /* Integer mapping. 
    .. Ex: mapping to vertices or twins etc.
    */
    _HmeshAttribute * maps[5];

    /* Scalar list of type double (or float in some cases)*/
    _HmeshScalar ** s;

    /* Number of scalars */
    _Flag nscalars; 

    /* type of manifoldcell */
    _Flag type;

  } _HmeshCells;

  extern
  _Flag _HmeshCellsInit(_HmeshCells *, _Flag);

  extern
  _Flag _HmeshCellsDestroy(_HmeshCells *);
  
  extern
  _Flag _HmeshCellsAddScalar(_HmeshCells *, char *);
  
  extern
  _Flag _HmeshCellsRemoveScalar(_HmeshCells *, char *);

  /* _Manifold: Mesh or a discretized manifold */
  typedef struct {

    /* 'd' : dimension of manifold in [0,3]. They can be,
    .. 0: Collection of disconnected Points
    .. 1: Discretized curves in 2-D or 3-D
    .. 2: Discretized 3-D sufaces. 
    .. 3: Volume mesh in 3-D.
    */
    _Flag d;

    /* 'D' : Dimension of space. For us, 'D' in [2,3].
    .. NOTE: It's always (d <= D)
    */
    _Flag D;
  
    /* set of manifold cells like faces, edges and points 
    .. NOTE: They can be empty, 
    .. Ex: a curve, which is a 1-D manifold in 3D 
    .. Eulerian space will have empty face list ('triangles').
    */
    _HmeshCells * points, * edges, * triangles, * tetrahedron; 

  }_Hmesh;

  extern
  _Hmesh * Hmesh(_Flag d, _Flag D);
  
  extern
  _Flag * HmeshDestroy(_Hmesh *);

#ifdef __cplusplus
}
#endif

#endif
