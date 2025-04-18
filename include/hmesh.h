/* TODO:
  Dynamic data structure architecture for gpu.
  Vectorization for gpu.
  Multithreading.
  May use a hashtable to faster insertion/deletion,
    ,, but memory of scalar should be taken care!
  See list.h implementation in C++
    https://gcc.gnu.org/onlinedocs/gcc-4.7.2/libstdc++/api/a01472_source.html 
  Scalar should be identified by an _Index rather than
    a pointer. Also: It should be taken from a pool.
  Optimise the _Index implementation.
  There needs a lot of renaming of vars/files etc.
    Also, add more expln
  _Flag => _Index
  Pop() Push() scalar
  scalar => _Index
  Same Mempool
  compile with typecast warning
  memory tracing
  low level memory mgmt
  read https://github.com/exo-lang/exo
*/

#ifndef _HEDGE_MESH
#define _HEDGE_MESH

#ifdef __cplusplus
extern "C" {
#endif

  #include <common.h>
  #include <tree-pool.h>

  #define HMESH_MAX_NBLOCKS UINT16_MAX
  #define HMESH_MAX_NVARS   64
  #define HMESH_MAX_VARNAME 31

  /* Access attribute or scalars of cells .
  .. The macros are global. Use macros carefully,
  .. when using outside hmesh.c. 
  .. Numbers like iattr, node.iblock, iscalar, ivertex, iedge, node.index 
  .. should be in valid range*/

  /* get 'iblk'-th block of 'iattr'-th attribute */
  #define HMESH_ATTR(_c_, _iattr_, _iblk_)   \
    ( (_c_->mem[_iattr_])[_iblk_] )

  /* get 'iblk'-th block of a scalar */
  #define HMESH_REAL(_c_, _s_, _iblk_)       \
    ( (_Real *)(HMESH_ATTR(_c_, _s_, _iblk_)) )

  /* Get scalar value of a node*/
  #define HMESH_SCALAR(_c_, _s_, _hnode_)            \
    ( HMESH_REAL(_c_, _s_, _hnode_.iblock)[_hnode_.index] )

  /* get subnodes like vertex of an edge, edge of a triangle, etc */
  #define HMESH_SUBNODE(_c_,_node_, _isub_)      \
    ( ((_Node) HMESH_ATTR(_c_, 2 + _isub_, _node_.iblock))[_node_.index] )

  /* get i-th vertex */
  #define HMESH_IVERTEX(_edges_,_edge_,_iv_) \
    HMESH_SUBNODE(_edges_, _edge_, _iv_)

  /* get i-th edge */
  #define HMESH_IEDGE(_triangles_,_triangle_,_ie_) \
    HMESH_SUBNODE(_triangles_, _triangle_,_ie_)
  
  /* '_HmeshArray' : a list of memory blocks.
  .. It can be used to store nodes, or attributes of nodes
  .. like scalars etc.
  */
  typedef struct {

    /*'name' : name of the attribute ,
    .. in case this is an attribute to a node. 
    .. Warning: Name of attribute limited to 31 characters!*/
    char name[32];

    /* 'iblock' is the number corresponding to
    .. each block in the 'pool'. 'max' is the size of
    .. iblock array, and also address array
    */
    _Index * iblock, max, obj_size;

    /* to keep track of indices for which 
    .. address[index] are in_use and free_list.
    .. you need to free the memblock before freeing 
    .. the index
    */
    _IndexStack stack;

  } _HmeshArray;
 

  /* Mapping of indices.. 
  .. Ex: (a) Half-edge mapping H = (V0, V1) . 
  ..     (b) Triangle mapping  T = (H0, H1, H2); 
  */
  /* identify mapped index by block and index. 
  struct _HmeshMap {
    _HmeshArray * iblock, * index;
  };
  */

  typedef struct {
    _Index index, iblock;
  }_Node;

  /* cells of mesh : vertices/edges/triangle/tetrahedrons 
  */
  typedef struct _HmeshCells {
    /* Stack of indices in use for scalars, blocks */
    _IndexStack scalars, * blocks;

    /* attributes including 'prev', 'next', scalars etc */
    void ** attr;

    /* address of blocks of each attribute */
    void *** mem;

    /* 'info' : head, free head, no: of nodes in use, 
    .. no: of nodes free.
    .. 'max' : blocks in [0,max) are (maybe) in use 
    .. 'maxs' : scalars in [0,maxs) are (maybe) in use 
    */
    _Index * info, max, maxs;

    /* dimension.
    .. iscalars in [min, max) are in_use for scalars  
    */
    _Flag d, min;
  } _HmeshCells;

  extern
  _HmeshCells * HmeshCells(_Flag d, _Flag D);

  extern
  _Flag HmeshCellsDestroy(_HmeshCells *);
  
  extern
  _HmeshArray * HmeshScalarNew(_HmeshCells *, char *);
  
  extern
  _Flag HmeshScalarRemove(_HmeshCells *, char *);

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
  
    /* set of manifold cells like 
    .. points, edges, triangles, volumes 
    .. NOTE: They can be empty, 
    .. Ex: a curve, which is a 1-D manifold in 3D 
    .. Eulerian space will have empty face list ('triangles').
    */
    _HmeshCells * p, * e, * t, * v; 

  }_Hmesh;

  /* API to create a mesh.
  .. Warning! it creates a mesh
  .. with no points, edges, ..*/
  extern
  _Hmesh * Hmesh(_Flag d, _Flag D);
  
  extern
  _Flag HmeshDestroy(_Hmesh *);

  /* Every time you manipulate with HmeshArray, you send 
  .. the pointer of 2D array of block address. 
  */
  extern
  _HmeshArray * HmeshArray(char * name, size_t size, void ***);

  extern
  _Flag HmeshArrayDestroy(_HmeshArray *, void ***);

  /* Make these 2 function local */
  extern
  void * HmeshArrayAdd(_HmeshArray *, _Index, void ***);

  extern
  _Flag HmeshArrayRemove(_HmeshArray *, _Index, void ***);

  /* add a node or remove a node to the set of cells */
  extern
  _Node HmeshNodeNew(_HmeshCells *);

  extern
  _Flag HmeshNodeRemove(_HmeshCells *, _Node);

#ifdef __cplusplus
}
#endif

#endif
