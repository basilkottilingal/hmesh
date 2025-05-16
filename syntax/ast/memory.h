#ifndef _H_AST_POOL_
#define _H_AST_POOL_

  /* A memory node, _H_AST_PTR_, (kind of pointer) is an integer that stores
  .. adequate information (BLOCK NUMBER <12 bits> + BIT LOCATION<20 bits>) to 
  .. locate the actual memory address. A uint32_t is used over unsigned int
  .. for the reason it's a 32 bit datatype irrespective of the architecture,
  .. unlike unsigned int.
  .. 
  .. The memory pool is an arena allocator, where you cannot deallocate.
  .. Each allocated node survive till the end of the pgm.
  .. You can only delete the entire block(s) which you should do
  .. only at the end of the program.
  ..
  .. A block of size 1 MB (2^20) is allocated each time you run out of memory. 
  .. Theoretically you can represent upto 4GB (2^12 Blocks x 2^20 Bytes) using
  .. _H_AST_PTR_ which we hope is more than adequate for our parser.
  ..
  .. Thread Safety : NOT THREAD SAFE
  */

  #define _H_AST_BLOCK_SIZE_ 1<<20    /* 1 MB     */
  #define _H_AST_20_BITS_    1048575  /* 2^20 - 1 */

  #ifndef _AST_LONG_PRECISION_
    typedef uint32_t _H_AST_PTR_ ;
  #else
    typedef uint64_t _H_AST_PTR_ ;
  #endif

  #define _H_AST_PTR_B_(_i_)            ( _i_ >> 20 )
  #define _H_AST_PTR_N_(_i_)            ( _i_ & _H_AST_20_BITS_ )
  #define _H_AST_PTR_ENCODE_(_b_,_n_)   (  (_b_<<20) | (_n_)  )

  /*
  .. @ _MemoryHandler : Datatype that stores all root information 
  ..    related to memory handling
  ..    @ blocks : array of memory blocks
  ..    @ head  : head of the available memory stack.
  */
  typedef struct {
    void ** blocks;
    _H_AST_PTR_ head; 
  } _MemoryHandler;
  _MemoryHandler _H_AST_POOL_ = {0};

  /*
  .. Following are the api functions repsectively to
  .. (a) allocate a memory chunk of size 'size'.
  ..     NOTE : don't use this to allocate large chunks,
  ..     say, > 64 Bytes.
  .. (c) deallocate all memory block.
  */
  extern _H_AST_PTR_ ast_allocate(size);
  extern void ast_deallocate_all();

  /*
  .. Type Cast Memory Location to the specified type.
  */
  #define _H_AST_PTR_ADDRESS_(_i_, _type_)         \
    ( (_type_) ( ((char *) _H_AST_POOL_.blocks[ _H_AST_PTR_B_(_i_) ] )  +  \
          _H_AST_PTR_N_(_i_)  )  )
  
#endif
