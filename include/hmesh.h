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
  #include <mempool.h>

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

  /* cells of mesh : vertices/edges/triangle/tetrahedrons 
  */
  typedef struct _HmeshCells {
    _IndexStack scalars, * blocks;
    void ** attr;
    _Index * info, max;
    _Flag d;
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

  extern
  _HmeshArray * HmeshArray(char * name, size_t size);
  extern
  _Flag HmeshArrayDestroy(_HmeshArray *);

  extern
  void * HmeshArrayAdd(_HmeshArray *, _Index);
  extern
  _Flag HmeshArrayRemove(_HmeshArray *, _Index);

#ifdef __cplusplus
}
#endif

#endif
