typedef uint16_t _Index;

/* _List: linked list */
typedef struct _List {

  _Index i;

  /* List of object functions */
  /* 1: Add an(void * ) (* add) ( _List * );
  void 

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
