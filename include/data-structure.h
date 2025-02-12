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
  typedef float _DataType
#else 
  typedef double _DataType
#endif

/* Alignment of memory block */


/* '_Index' : a node of the Linked list . 
.. A data node may represent a point, or a halfedge, 
.. or an orineted face (mostly triangle);
*/ 
typedef struct _Index {

  /* Index in the memory block . This is used to ..
  .. point to a scalar associated with this DataNode.
  .. fixme: See if you need 2^32 (~4 Billion) Nodes?
  */
  uint32_t i;

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

}_Index;


/* '_IndexBlock' : Chunks of indices. 
.. (a) avoids repeated malloc/realloc,
.. (b) chunks of size close L2 cache (need optimization)
..  can help locality of 'next' node within range,
*/


/* _List: linked list */
typedef struct _IndexList {

  /* memory block(s) allocated for the Linked list of  */
  _Mempool * index;
  
  /* List of memory blocks of each block. 
  .. Say there is variable is var \in [0, VARMAX],
  .. then vars[var] is the memory block corresponding to ..
  .. */ 
  _Mempool ** vars;

  /* List of object functions 
  .. (a) To add an index
  .. (b) To remove an occupied index 
  .. (c)
  .. respectively,
  */
  (void * ) (* add)      ( _List * list);
  Flag      (* remove)   (_List * list, void * node);
  (void *)  (* var)      (_List * list, char * name);
  Flag      (* var_free) (_List * list, char * name);

}_List;

enum VARIABLE_TYPE {
  VAR_IS_A_CONSTANT = 0;
  VAR_IS_A_SCALAR = 1;
  VAR_IS_A_VECTOR = 2;
  VAR_IS_A_TENSOR = 3;
};
typedef struct {
  /* Index in [0, max variables allowed) */
  Flag i;

  /* Variable type */
  
}_Scalar;

/* _Data : Datatype */
typedef struct _Data{

  /* List of scalars */
  _Scalar * s;

  /* List of vectors */
  _Vector * v;

  /* Function pointer to add a global scalar/vector . 
  .. NOTE: The varibale added here will have the scope till the,
  .. end of the program. 
  .. Input: 
  */
  (void * ) (*add) (_Data *, char * , Flag );

}_Data;

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
  
  /* List of indices of submanifolds like
  .. faces, edges and vertices. 
  .. NOTE: They can be empty, 
  .. ex: a curve, which is a 1-D manifold in 3D Eulerian space,
  .. will have empty face list ('f') */
  _List v, e, f; 
}_Manifold;
