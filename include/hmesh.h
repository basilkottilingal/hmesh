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

  #define HMESH_ATTR(cells,iattr,iblock) ((cells->mem[iattr])[iblock])
  
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

  /* Every time you manipulate with HmeshArray, you send 
  .. the pointer of 2D array of block address. 
  */
  extern
  _HmeshArray * HmeshArray(char * name, size_t size, void ***);

  extern
  _Flag HmeshArrayDestroy(_HmeshArray *, void ***);

  extern
  void * HmeshArrayAdd(_HmeshArray *, _Index, void ***);

  extern
  _Flag HmeshArrayRemove(_HmeshArray *, _Index, void ***);

  extern
  _Node HmeshNodeNew(_HmeshCells *);

  extern
  _Flag HmeshNodeRemove(_HmeshCells *, _Node);

#ifdef __cplusplus
}
#endif

#endif
