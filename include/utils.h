#ifdef _PRECISION32
  #define float _Type
#else 
  #define double _Type
#endif

typedef struct _Variable {
  char * name;
  _Mempool ** pools; 
  _Flag ipool; 
};


