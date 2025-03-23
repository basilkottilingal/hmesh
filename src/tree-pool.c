#include <common.h>
#include <tree-pool.h>

/* MAP_ANONYMOUS not found in few compilers.
.. following is copied from glibc/malloc.c.
.. This file is expected to be compiled 
.. successfully w in 
.. linux/MacOS. (NOT VERIFIED)
*/
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif


#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
/* 
.. Use the HmeshTAllocate() function to allocate memory chunks
.. of size of 
.. \{ 8 x PAGE_SIZE, 4 x PAGE_SIZE, 2 x PAGE_SIZE, 1 x PAGE_SIZE \}
*/

typedef struct _HmeshTreepool {
  /* collection of pools */
  _HmeshTpool * trees;

  /* number of trees in use */
  _Flag ntrees; 

  /* Free blocks of each level */
  _FreeTBlock * free_list[HMESH_TREE_POOL_DEPTH + 1]; 

} _HmeshTreepool;

static
_HmeshTreepool HMESH_TREE_POOLS = {0};

_HmeshTpool * HmeshTpoolTree(_Flag itree) {
  return itree < HMESH_TREE_POOLS.ntrees ?
    &HMESH_TREE_POOLS.trees[itree] : NULL;
}

const size_t HMESH_TREE_POOL_NODES = 
  (1 << (HMESH_TREE_POOL_DEPTH+1)) - 1;

/* For the moment, let's fix the BLOCK_SIZE = no: of
.. nodes per block to 4096. So the the size of chunks
.. fall in 4096 * \{1,2,4,8\}, guaranteeing the minimum
.. of the chunk size is a PAGE_SIZE = 4096 Bytes.*/
const size_t HMESH_TREE_BLOCK_SIZE = HMESH_PAGE_SIZE;

size_t HmeshTpoolBlockSize() {
  return HMESH_TREE_BLOCK_SIZE;
}


/* We try to allocate 8MB of chunk */
#define HMESH_TREE_POOL_SIZE 1<<23

static inline void * 
HmeshTpoolAddressIs(void * start, _Index inode, _Flag depth){
  _Index h = 1<<depth, H = 1 << HMESH_TREE_POOL_DEPTH,
    D = 1 << (HMESH_TREE_POOL_DEPTH - depth),
    index = inode & HMESH_TREE_POOL_NODES,
    iroot = inode >> (HMESH_TREE_POOL_DEPTH + 1);
  return (void *) ((char *) start + 
    (iroot*H + (index - h + 1)*D) * HMESH_TREE_BLOCK_SIZE);
}

void * 
HmeshTpoolAddress(_Index block) {
  _Index itree = block >> 13, inode = block & 8191; 
  
  if( !(itree < HMESH_TREE_POOLS.ntrees && inode < 4096) )
    return NULL;
   
  return HmeshTpoolAddressIs(HMESH_TREE_POOLS.trees[itree].root,
    inode, HMESH_TREE_POOLS.trees[itree].flags[inode] & 7);
}

static inline _Index HmeshTpoolParent(_Index inode) {
  return (( (inode & HMESH_TREE_POOL_NODES) - 1) >> 1) |
    (inode & ~HMESH_TREE_POOL_NODES);    
}

static inline _Index 
HmeshTpoolChild(_Index inode, _Index ichild) {
  /*assert(ichild <2)*/
  return (inode & ~HMESH_TREE_POOL_NODES) |
    (((inode & HMESH_TREE_POOL_NODES) << 1) + ichild + 1);    
}

static inline void
HmeshTpoolPop(_FreeTBlock * fb, _Flag depth) {
  /* remove a free block from free list */
  if( fb->prev )
    fb->prev->next = fb->next;
  else
    HMESH_TREE_POOLS.free_list[depth] = fb->next;
  if( fb->next )
    fb->next->prev = fb->prev;
}

static inline void
HmeshTpoolPush(_FreeTBlock * fb, _Flag depth) {
  fb->prev = NULL;
  fb->next = HMESH_TREE_POOLS.free_list[depth]; 
  if(fb->next)
    fb->next->prev = fb;
  /* add the free block at the free list head*/
  HMESH_TREE_POOLS.free_list[depth] = fb;
}

static _Index 
HmeshTpoolDivide(_FreeTBlock * _fb, 
  _Flag level, _Flag depth) 
{
  /* Divide a freeblock till 'depth' */
  HmeshTpoolPop(_fb, level);
          
  _Index block = _fb->block, inode = block & 8191,
    itree = block >> 13;

  if(itree >= HMESH_TREE_POOLS.ntrees || inode >= 4096) {
    HmeshError("HmeshTpoolDeallocate() : out of bound");
    return UINT16_MAX;
  }

  _HmeshTpool * tree = &HMESH_TREE_POOLS.trees[itree];
  _Flag * flags = tree->flags;
  while (level++ < depth){
    /* Keep on dividing the chunk till the size 
    .. is of desirable size */
    _Index left = HmeshTpoolChild(inode,0),
      right = HmeshTpoolChild(inode,1);
    /*flags[left]  = level;        In use */
    flags[right] = level | 128; /* In free list */
    _FreeTBlock * fb = (_FreeTBlock *) 
        HmeshTpoolAddressIs(tree->root, right, level);
    *fb = (_FreeTBlock) { .block = right | (itree << 13) };
    /* pushing right half as a free block @ free list head*/
    HmeshTpoolPush(fb, level);

    /* inode is divided and no more a leaf */
    flags[inode] = (level - 1) | 64;
  
    /* Go to left child */ 
    inode = left; 
  }
          
  /* In use */
  flags[inode] = depth;
  return inode | (itree << 13);
}

_Flag HmeshTpoolDeallocate(_Index block){

  /* assumes max depth < 8 */

  /* last 13 bits are reserved to store node index,
  .. and the rest are used to store tree no (itree) */
  _Index itree = block >> 13, inode = block & 8191;

  if(itree >= HMESH_TREE_POOLS.ntrees || inode >= 4096) {
    HmeshError("HmeshTpoolDeallocate() : out of bound");
    return HMESH_ERROR;
  }
  
  _HmeshTpool * tree = &HMESH_TREE_POOLS.trees[itree];


  _Flag * flags = tree->flags,
    /* last 3 bits are used to encode depth of the node,
    .. Flag 128 is used to see if node is free,
    .. Flag 64 is used in case the node is not a leaf */
    depth = flags[inode] & 7;

  if( flags[inode] & (128|64) ) {
    /* This node is already free */
    HmeshError("HmeshTpoolDeallocate() : "
      "tree pool node cannot be deallocated");
    return HMESH_ERROR;
  }

  do {
    _Index sibling = inode + ((inode & 1) ? 1 : -1); 
    /* Coalesce with 'buddy'/'sibling' if possible */
    if ( (128 & flags[sibling]) && depth ) {

      /* Merge inode and sibling */
      flags[sibling] = flags[inode] = (depth | 64);
      _FreeTBlock * fb = (_FreeTBlock *) 
        HmeshTpoolAddressIs( tree->root, sibling, depth);
      assert((fb->block & 8191) == sibling);

      /* Remove sibling out of free list */
      HmeshTpoolPop(fb, depth);
      
      /* Go to parent */
      inode = HmeshTpoolParent(inode);
    }
    else {
      /* Mark inode as free and add it to the free list*/
      flags[inode] |= 128; 
      _FreeTBlock * fb = (_FreeTBlock *) 
        HmeshTpoolAddressIs( tree->root, inode, depth);
      *fb = (_FreeTBlock) { .block = inode | (itree << 13) };
      /* push to free list head */
      HmeshTpoolPush(fb, depth);
      break;
    } 
  } while(depth--);

  return HMESH_NO_ERROR;
}

static
void * HmeshTpoolAdd() {
  if(HMESH_TREE_POOLS.ntrees == 8) {
    HmeshError("HmeshTpoolAdd() : _Index insufficient for "
      "block index. Rewrite using uin32_t");
    return NULL;
  }
  /*
  .. Create a new tree pool and add it to the list of pool
  .. HMESH_TREE_POOLS.root[ntrees++] = mmap(..)
  .. 1 X PAGE_SIZE is used to stored flagsrmation on tree_pool,
  .. 2047 X PAGE_SIZE is free to use (if successful).
  */

  /* 'psize' : or pool size = 8MB 
  .. 'rsize' : root size or the largest chunk that
  .. pool can allocate. rsize = 8 x PAGE_SIZE bytes.
  .. 'limit' : is the bare minimum tree pool size.
  .. limit = 16 x PAGE_SIZE
  */
  size_t psize = HMESH_TREE_POOL_SIZE,
    rsize = (1<<HMESH_TREE_POOL_DEPTH) * HMESH_TREE_BLOCK_SIZE,
    limit = 2 * rsize, maxnodes = 1<<(HMESH_TREE_POOL_DEPTH+1);
 
  _Flag itree = HMESH_TREE_POOLS.ntrees, 
    depth = HMESH_TREE_POOL_DEPTH;

  while(psize >= limit) {
    /* Trying to pool memory directly from OS rather than
    .. using malloc(). */
    void * mem = mmap(NULL, psize, PROT_READ | PROT_WRITE, 
      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
      HmeshError("HmeshTpoolAdd() : Warning! "
        "mmap() failed for %zd bytes");
      /* Let's try with a smaller pool size*/
      psize = psize >> 1;
      continue;
    }

    /* Created a pool of size 'psize'. */
    _HmeshTreepool * pool = &HMESH_TREE_POOLS;
    ++(pool->ntrees);
    pool->trees = (_HmeshTpool *) realloc 
        ( pool->trees, (pool->ntrees) * sizeof(_HmeshTpool) );
    _HmeshTpool * tree = pool->trees + itree;
    *tree = (_HmeshTpool) 
      { 
        .root  = mem, 
        .flags = (_Flag *) mem, 
        .size  = psize
      };

    /* Let's use a part of the pool [0, PAGE_SIZE)
    .. to store the tree nodes flags*/

    /* Update the flags on the nodes */
    size_t rchunks = psize / rsize; assert(rchunks);
    _Flag * flags = tree->flags;
    _FreeTBlock _0b;
    for(int j = rchunks - 1; j >= 0; --j) {
      _Index inode = j * maxnodes;
      /* 
      .. Adding chunk to free_list @ root (level = 0) 
      .. Encode level and the flag that node is not in use 
      .. _Flag NODE_IS_FREE = 128, NODE_IS_NOT_LEAF = 64;
      */

      _FreeTBlock * fb = !j ? &_0b :
        (_FreeTBlock *) HmeshTpoolAddressIs(mem, inode, 0);

      *fb = (_FreeTBlock) { .block = (itree << 13) | inode };
      /* pushing @ free head of free_list[0]*/
      HmeshTpoolPush(fb, 0);
      flags[inode++] = 128;     /* 0 | 128 */ 

      _Flag level = 1;
      /* children of root nodes are not yet developed
      .. and not free*/
      do {
        for(_Index i = 0; i< (1<<level); ++i) 
          flags[inode++] = level | 64; 
      } while(++level <= depth);
      /* to round off [2^(N+1)-1] to [2^(N+1)] */
      ++inode;
      assert(inode % maxnodes == 0);
    }

    /* Since we use only a (1/2)^N of the first rchunk,
    .. we need to divide it N times successively,
    .. each time adding the right block to free_list*/
    _Index flagsNode = HmeshTpoolDivide(&_0b, 0, depth);
    assert( flagsNode == ((itree << 13) | ((1<<depth) - 1)) );

    return mem;    
  }
      
  HmeshError("HmeshTpoolAdd() : failed to create a tree_pool");
  return NULL; 
}

_Index HmeshTpoolAllocate(_Flag depth) {
  if(depth > HMESH_TREE_POOL_DEPTH) {
    HmeshError("HmeshTpoolAllocate() : depth out of bound");
    return UINT16_MAX;
  }

  int twice = 2;
  while (twice--) {
    _Flag level = depth;
    do {
      _FreeTBlock * fb = HMESH_TREE_POOLS.free_list[level];
      /* Cannot find free block @ this level; try larger chunks */
      if(!fb) continue;
   
      /* In case level < depth, divide till depth */ 
      return HmeshTpoolDivide(fb, level, depth);

    } while(level--);

    /* In case there is no free blocks @ all levels,
    .. create a new pool */
    if(twice) {
      void * mem = HmeshTpoolAdd();
      if(!mem) break;
    }
  }
        
  HmeshError("HmeshTpoolAllocate() : no availability");
  return UINT16_MAX;
 
}

_Index HmeshTpoolAllocateGeneral(size_t obj_size){
  size_t n = obj_size, r = 0;
  _Flag level = 0;
  while(n) {
    if(r) 
      break;
    r = n & 1;
    n = n >> 1;
    ++level;
  }
  if( (n && r) || (!obj_size)) {
    HmeshError("HmeshTpoolAllocateGeneral() : "
      "obj_size should be [1,2,..,2^HPOOL_DEPTH]");
    return UINT16_MAX;
  }
  --level;
  if( level > HMESH_TREE_POOL_DEPTH ) {
    HmeshError("HmeshTpoolAllocateGeneral() : "
      "out of bound");
    return UINT16_MAX;
  }
  return 
    HmeshTpoolAllocate(HMESH_TREE_POOL_DEPTH - level);
}

_Index HmeshTpoolAllocatePage(){
  return HmeshTpoolAllocate(HMESH_TREE_POOL_DEPTH);
}

void HmeshTpoolDestroy(){
  _Flag itree = HMESH_TREE_POOLS.ntrees;
  _HmeshTpool * tree = HMESH_TREE_POOLS.trees;
  while(itree--) {
    /* fixme: add error msg */
    munmap(tree->root, tree->size);
    ++tree;
  }
}

