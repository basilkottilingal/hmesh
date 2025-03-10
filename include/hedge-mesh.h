/* TODO:
  Dynamic data structure architecture for gpu.
  Vectorization for gpu.
  Multithreading.
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

    /*  'i' : or iblock is the number corresponding to
    .. each block in the 'pool'.
    .. iblock in [1,HMESH_MAX_NBLOCKS] are used and
    .. iblock '0' is reserved
    */
    _Flag i[HMESH_MAX_NBLOCKS+1];

    /* 'n' : size of 'address' array (in use).
    */
    int n, max; 

  } _HmeshArray;


  /* Create an new HmeshArray obj */
  extern _HmeshArray * 
  HmeshArray(char *, size_t);
 
  /* Destroy a HmeshArray obj*/
  extern
  _Flag _HmeshArrayDestroy(_HmeshArray *);
 
  /* _DataType : Precision used in scientific computation.
  .. 32 bits is Preferred data type precision for GPU vectors.
  .. Everywhere else (and by default) it's used as 64.
  */

  #ifdef _PRECISION32
  typedef float _Real;
  #else 
  typedef double _Real;
  #endif
  
  typedef uint16_t _Index;

  struct _HmeshNode {
    /* Linked list of nodes. 
    .. Structure of Array (SoA) format 
    */
    _HmeshArray * prev, * next;

    /* nodes blocks with atleast one empty node, and
    .. node blocks which is totally consumed. 
    .. Encode '0' as the end of array. 
    */
    _Index * free_blocks, * full_blocks;

    /* number of blocks */
    _Flag n, max; 

#ifdef _HMESH_MPI
    /* In case of MPI, 
    .. pid : processor id,
    .. gid : global id of node
    */
    _HmeshArray * pid, * gid;
#endif
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

    /* Number of scalars */
    _Flag n, max; 
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
