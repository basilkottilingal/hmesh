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

#ifndef _HMESH_
#define _HMESH_

#ifdef __cplusplus
extern "C" {
#endif

  #include <common.h>
  #include <tree-pool.h>

  /*
  .. Some limits imposed in hmesh
  .. (a) Maximum 2^16 Blocks. Usually RAM will run much before that.
  .. (b) Maximum of 64 variables/attributes per manifold
  .. (c) Maximum of 32 characters including '\0' for variable name. 
  */
  #define HMESH_MAX_NBLOCKS UINT16_MAX
  #define HMESH_MAX_NVARS   64
  #define HMESH_MAX_VARNAME 31

  /*
  .. Access attribute or scalars of cells. The macros are global. Use these
  .. macros carefully, while using outside hmesh.c. Numbers like iattr,
  .. node.iblock, iscalar, ivertex, iedge, node.index should be in valid range
  .. HMESH_ATTR    : get 'iblk'-th block of 'iattr'-th attribute
  .. HMESH_REAL    : get 'iblk'-th block of a scalar
  .. HMESH_SCALAR  : Get scalar value of a node
  .. HMESH_SUBNODE : get subnodes like vertex of an edge/edge of a triangle/etc
  .. HMESH_IVERTEX : get i-th vertex
  .. HMESH_IEDGE   : get i-th edge
  */

  #define HMESH_ATTR(_c_, _iattr_, _iblk_)                                    \
    ( (_c_->mem[_iattr_])[_iblk_] )
  #define HMESH_REAL(_c_, _s_, _iblk_)                                        \
    ( (_Real *)(HMESH_ATTR(_c_, _s_, _iblk_)) )
  #define HMESH_SCALAR(_c_, _s_, _hnode_)                                     \
    ( HMESH_REAL(_c_, _s_, _hnode_.iblock)[_hnode_.index] )
  #define HMESH_SUBNODE(_c_,_node_, _isub_)                                   \
    ( ((_Node) HMESH_ATTR(_c_, 2 + _isub_, _node_.iblock))[_node_.index] )
  #define HMESH_IVERTEX(_edges_,_edge_,_iv_)                                  \
    HMESH_SUBNODE(_edges_, _edge_, _iv_)
  #define HMESH_IEDGE(_triangles_,_triangle_,_ie_)                            \
    HMESH_SUBNODE(_triangles_, _triangle_,_ie_)
  
  /*
  .. HmeshArray : a list of memory blocks. It can be used to store nodes, or
  .. attributes of nodes like scalars etc.
  .. 'name' : name of the attribute , in case this is an attribute to a node.
  .. Warning: Name of attribute limited to 31 characters!
  .. 'iblock' is the number corresponding to each block in the 'pool'. 'max'
  .. is the size of iblock array, and also address array
  .. To keep track of indices for which address[index] are in_use and
  .. free_list. you need to free the memblock before freeing the index
  */
  typedef struct
  {
    char name[HMESH_MAX_VARNAME + 1];
    Index * iblock, max, obj_size;
    IndexStack stack;
  } HmeshArray;

  typedef struct
  {
    Index index, iblock;
  } Node;

  /*
  .. cells of mesh : vertices/edges/triangle/tetrahedrons 
  .. Stack of indices in use for scalars, blocks
  .. attributes including 'prev', 'next', scalars etc
  .. address of blocks of each attribute 
  .. 'info' : head, free head, no: of nodes in use, .. no: of nodes free.
  .. 'max'  : blocks in [0,max) are (maybe) in use 
  .. 'maxs' : scalars in [0,maxs) are (maybe) in use 
  .. dimension.
  .. iscalars in [min, max) are in_use for scalars  
  */
  typedef struct 
  {
    IndexStack scalars, * blocks;
    void **  attr, *** mem;
    Index * info, max, maxs;
    int d, min;
  } HmeshCells;

  /*
  .. Hmesh: Mesh or a discretized manifold 
  .. 
  .. 'd' : dimension of manifold in [0,3]. They can be,
  .. 0: Collection of disconnected Points
  .. 1: Discretized curves in 2-D or 3-D
  .. 2: Discretized 3-D sufaces. 
  .. 3: Volume mesh in 3-D.
  ..
  .. 'D' : Dimension of space. For us, 'D' in [2,3].
  .. NOTE: It's always (d <= D)
  .. set of manifold cells like 
  .. points, edges, triangles, volumes 
  .. NOTE: They can be empty, 
  .. Ex: a curve, which is a 1-D manifold in 3D 
  .. Eulerian space will have empty face list ('triangles').
  ..
  .. 'p' ,'e', 't', 'v'  are sets of pointes, edges, triangles
  .. and volume cells 
  */
  typedef struct
  {
    int d, D;
    HmeshCells * p, * e, * t, * v; 
  } Hmesh;

  /*
  .. APIs
  .. (*) hmesh_cells () : Create cells of dim 'd' in Eulerain space R^D
  .. (*) hmesh_cells_destroy () : destroy HmeshCells
  .. (*) hmesh_scalar_new () :  add a scalar attribute (Real i.e float/double)
  .. to HmeshCells
  .. (*) hmesh_scalar_remove () :  remove a scalar attribute
  .. (*) hmesh () : to create a mesh of manidold dim 'd' in Eulerian space R^D
  .. Warning! it creates a mesh with no points, edges, ..
  .. (*) hmesh_destroy () : destroy a mesh
  .. (*) hmesh_array () : create HmeshArray
  .. (*) hmesh_array_remove () : remove HmeshArray
  .. Every time you manipulate with HmeshArray, you send 
  .. the pointer of 2D array of block address. 
  .. (*) add a node to the set of cells
  .. (*) remove a node from the set of cells
  */
  extern HmeshCells * hmesh_cells         ( int d, int D );
  extern int          hmesh_cells_destroy ( HmeshCells * );
  extern HmeshArray * hmesh_scalar_new    ( HmeshCells *, char * );
  extern int          hmesh_scalar_remove ( HmeshCells *, char * );
  extern Hmesh *      hmesh               ( int d, int D);
  extern int          hmesh_destroy       ( Hmesh *);
  extern HmeshArray * hmesh_array ( char * name, size_t size, void *** );
  extern int          hmesh_array_destroy ( HmeshArray *, void *** );
  extern Node         hmesh_node_new      ( HmeshCells *);
  extern int          hmesh_node_remove   ( HmeshCells *, Node);

  /* Make these 2 function local */
  extern void *       hmesh_array_add ( HmeshArray *, Index, void *** );
  extern int          hmesh_array_remove ( HmeshArray *, Index, void ***);

#ifdef __cplusplus
}
#endif

#endif
