/* TODO:
  Dynamic data structure architecture for gpu.
  Vectorization for gpu.
  Multithreading.
  May use a hashtable to faster insertion/deletion,
    ,, but memory of scalar should be taken care!
*/

#ifndef _HEDGE_MESH
#define _HEDGE_MESH

#ifdef __cplusplus
extern "C" {
#endif
  #define HMESH_MAX_NBLOCKS 16
  #define HMESH_MAX_NVARS   64
  #define HMESH_MAX_VARNAME 31
  
  /* '_HmeshArray' : a list of memory blocks.
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

    /*  'iblock' is the number corresponding to
    .. each block in the 'pool'. 'max' is the size of
    .. iblock array, and also address array
    */
    _Index * iblock, max;

    /* to keep track of indices for which 
    .. address[index] are in_use and free_list.
    .. you need to free the memblock before freeing 
    .. the index
    */
    _IndexStack stack;

  } _HmeshArray;
 
  struct _HmeshNode {
    /* Linked list of nodes. 
    .. Structure of Array (SoA) format 
    */
    _HmeshArray * prev, * next;

    /* For the block with index 'index' in array 
    .. 'info[4*index]' is the starting node of the used 
    .. nodes linked list and 'head[4*index+1]' is the 
    .. starting node of free nodes linked list.
    .. 'info[4*index+2]' is the number of nodes in use,
    .. 'info[4*index+3]' is the number of nodes free to use.
    .. 1 + nused + nfree = HMESH_BLOCK_SIZE.
    .. _Index '0' is reserved.
    */
    _Index * info;

#ifdef _HMESH_MPI
    /* In case of MPI, 
    .. pid : processor id,
    .. gid : global id of node
    */
    _HmeshArray * pid, * gid;
#endif

    /* number of blocks */
    _Index max;
  };

  /* Mapping of indices.. 
  .. Ex: (a) Half-edge mapping H = (V0, V1) . 
  ..     (b) Triangle mapping  T = (H0, H1, H2); 
  */
  struct _HmeshMap {
    /* identify mapped index by block and index. */
    _HmeshArray * iblock, * index;
  };

  struct _HmeshScalar {
    /* Scalar list of type double (or float in some cases)*/
    _HmeshArray ** s;

    /* Size of s array */
    _Flag max;

    /* Number of scalars in use*/
    _Flag n;

    /* To maintain a free list of scalars.
    .. fixme : This should be removed later,
    .. where flex is used to pop/push global and local vars*/
    _Flag nfree, * free_list;

  };

  /* cells of mesh : vertices/edges/triangle/tetrahedrons 
  */
  typedef struct _HmeshCells {
    /* Node : vertex/edge/face/tetrahedron */
    struct _HmeshNode * nodes;

    /* Vertices of edge. Apply only for d>0. */
    struct _HmeshMap ** v, * twin, * next;

    /* List of scalars */
    struct _HmeshScalar * scalars;

    /* type( = 'd' ) of manifoldcell  */
    _Flag type;

  } _HmeshCells;

  extern
  _HmeshCells * HmeshCells(_Flag d);

  extern
  _Flag HmeshCellsDestroy(_HmeshCells *);
  
  extern
  _Flag HmeshCellsAddScalar(_HmeshCells *, char *);
  
  extern
  _Flag HmeshCellsRemoveScalar(_HmeshCells *, char *);

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
