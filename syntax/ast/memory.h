#ifndef _H_AST_POOL_
#define _H_AST_POOL_
  
  #include <stdio.h>
  #include <stdlib.h>

  /* A memory node, _H_AST_PTR_, (kind of pointer) is an integer that stores
  .. adequate information (BLOCK <12 bits> + PAGE<8 bits> + NODE<12 bits>) to 
  .. locate the actual memory address. A uint32_t is used over unsigned int
  .. for the reason it's a 32 bit datatype irrespective of the architecture,
  .. unlike unsigned int.
  .. A block of size 1 MB (2^20) is divided into 256 pages (2^8) each of 
  .. size 4096 Bytes (2^12).
  .. Theoretically you can represent upto 4GB (2^12 Blocks x 2^20 Bytes) using
  .. _H_AST_PTR_ which we hope is more than adequate for our parser. 
  */

  #define _H_AST_PAGE_SIZE_ 4096
  typedef uint32_t _H_AST_PTR_ ;
  #define _H_AST_PTR_B_(_i_)        ( _i_ >> 20 )
  #define _H_AST_PTR_P_(_i_)        ( (_i_ >> 12) & 511 )
  #define _H_AST_PTR_N_(_i_)        ( _i_ & 8191 )
  #define _H_AST_PTR__(_b_,_p_,_n_) ( (_b_<<20) | (_p_<<12) | (_n_) )

  /*
  .. @ _Free : data type to store a linked list of unused blocks/pages/nodes
  ..    @ next   : to form a linked list of empty blocks
  ..    @ safety : is an encoded number to make sure,
  ..      you don't allocate or free same block twice
  */
  typedef struct _Free {
    struct _Free * next;
    uint32_t safety;
  } _Free;

  /*
  .. @ _MemoryHandler : Datatype that stores all root information 
  ..    related to memory handling
  ..    @ blocks : array of memory blocks
  ..    @ max    : size of blocks array
  ..    @ fhead  : head of the free list
  */
  typedef struct {
    _Block ** blocks;  
    _Free * fhead;
  } _MemoryHandler

  /*
  .. @ _Block : Datatype for a memory block which is ususally a big chunk
  ..  of memory of the order of MB. It's data members are
  ..    @ size    : size of the memory chunk.
  ..    @ npages  : number of pages the blocksize can accomodate
  ..        [ _H_AST_PAGE_SIZE x npages ] = size
  ..    @ nfree   : number of free pages available.
  ..    @ fhead : head of the linked list of free nodes
  */
  typedef struct _Block {
    size_t size;
    int npages, nfree;
    _Free * fhead;
  } _Block;
 
  /*
  .. @ _Page : A fixed size memory chunk like a memory page. 
  ..  [ As of now, set as 4k Bytes by default.
  ..    _Page data is stored at the head of the memory
  ..    allocated as a page. The usable memory per each _Page
  ..    is therefore, = 4KiB - sizeof(_Page).
  ..    The usable part of a page is divided to store 
  ..    nodes of same datatype.
  ..  ]
  ..
  ..    @ size  : size of datatype whose node is stored in page
  ..    @ nnodes, nfree : number of used & free nodes
  ..    @ fhead : head of the linked list of free nodes
  */
  typedef struct _Page {
    size_t size;
    int nnodes, nfree;
    _H_AST_PTR_ fhead;
  } _Page;

  /*
  .. Following are the api functions repsectively to
  .. (a) allocate a page of size 4k Bytes (default)
  .. (b) allocate a page & split it accomodate 
  ..       nodes each of size 'size'
  .. (c) allocate a node from specified page
  .. (d) deallocate a page back to the block
  .. (e) deallocate a node back to the page
  */
  extern _H_AST_PTR_ ast_allocate_page();
  extern _H_AST_PTR_ ast_split_page(size_t size);
  extern _H_AST_PTR_ ast_allocate_from(_H_AST_PTR_ page); 
  extern void ast_dealloacte_page(_H_AST_PTR_ page);
  extern void ast_deallocate_node(_H_AST_PTR_ node);
  
#endif
