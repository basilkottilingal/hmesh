#include <common.h>
#include <tree-pool.h>

/* 
.. Use the HmeshTAllocate() function to allocate memory chunks
.. of size of 
.. {8 x PAGE_SIZE, 4 x PAGE_SIZE, 2 x PAGE_SIZE, 1 x PAGE_SIZE}
*/


static 
_HmeshTreepool HMESH_TREE_POOL = {0}; 

static inline char * 
HmeshTpoolAddress(void * start, _Index inode, _Flag depth) {
  _Index h = (1<<depth), H = (1 << HMESH_TREE_BLOCK_DEPTH),
    index = inode & HMESH_TREE_POOL_NODES,
    iroot = inode >> (HMESH_TREE_POOL_DEPTH + 1);
  return (char *) start + 
    (iroot*H + h*(index - h - 1)) * HMESH_TREE_BLOCK_SIZE;
}

static inline _Index HmeshTpoolParent(_Index inode) {
  return (((inode & HMESH_TREE_POOL_NODES) >> 1) - 1) |
    (inode & ~HMESH_TREE_POOL_NODES);    
}

static inline _Index 
HmeshTpoolChild(_Index inode, _Index ichild) {
  /*assert(ichild <2)*/
  return (inode & ~HMESH_TREE_POOL_NODES) |
    (((inode & HMESH_TREE_POOL_NODES) << 1) + ichild + 1);    
}

static inline _Index 
_HmeshTpoolDivide(_Index block, _Flag level, _Flag depth) {
  /* Divide a block till 'depth' */
        
  _Index inode = block & 8191, itree = block >> 13;
  _Flag * nodes = HMESH_TREE_POOL.trees[itree]->nodes;

  while (level++ < depth){
    /* Keep on dividing the chunk till the size 
    .. is of desirable size */
    _Index left = HmeshTpoolChild(inode,0),
      right = HmeshTpoolChild(inode,1);
    index[left]  = level;       /* In use */
    index[right] = level | 128; /* In free list */
    _FreeTBlock * fb = (_FreeTBlock *) 
        HmeshTpoolAddress(mem, right, level);
    fb = (_HmeshTBlock) 
      { .prev  = NULL, 
        .next = pool->free_list[level], 
        .block = right | (itree << 13)
      };
    if(fb->next)
      fb->next->prev = fb;
    pool->free_list[level] = fb;

    /* inode is divided and no more a leaf */
    index[inode] = (level - 1) | 64;
  
    /* Go to left child */ 
    inode = left; 
  }
          
  return inode | ((pool->ntrees-1) << 13);
}

static inline
_Flag HmeshTpoolDeallocate(_Index block){

  /* assumes max depth < 8 */
  _HmeshTpool * pool = &HMESH_TREE_POOL;

  /* last 13 bits are reserved to store node index,
  .. and the rest are used to store tree no (itree) */
  _Index itree = block >> 13, inode = block & 8191;

  if(itree >= pool->ntrees || inode >= 4096) {
    HmeshError("HmeshTpoolDeallocate() : out of bound");
    return HMESH_ERROR;
  }

  _Flag * info = pool->info[itree],
    /* last 3 bits are used to encode depth of the node,
    .. Flag 128 is used to see if node is free,
    .. Flag 64 is used in case the node is not a leaf */
    depth = info[inode] & 8;

  if( info[inode] & (128|64) ) {
    /* This node is already free */
    HmeshError("HmeshTpoolDeallocate() : "
      "tree pool node cannot be deallocated");
    return HMESH_ERROR;
  }

  do {
    _Index sibling = inode ^ 1; 
    /* Coalesce with 'buddy'/'sibling' if possible */
    if ( (128 & info[sibling]) && depth ) {

      /* Merge inode and sibling */
      info[sibling] = info[inode] = (depth | 64);

      _FreeTBlock * fb = (_FreeTBlock *) 
        HmeshTpoolAddress( pool->root[itree], sibling, depth);
      assert((fb->block & 8191) == sibling);

      /* Remove sibling out of free list */
      if( fb->prev )
        fb->prev->next = fb->next;
      else
        pool->free_list[depth] = fb->next;
      if( fb->next )
        fb->next->prev = fb->prev;
      
      /* Go to parent */
      inode = HmeshTpoolParent(inode);
    }
    else {
      /* Mark inode as free and add it to the free list*/
      info[inode] |= 128; 
      _FreeTBlock * fb = (_FreeTBlock *) 
        HmeshTpoolAddress( pool->root[itree], sibling, depth);
      fb->prev = NULL; 
      fb->next = pool->free_list[depth];
      fb->block = inode | (itree << 13);
      pool->free_list[depth] = fb;
      break;
     } 
  } while(depth--);

  return HMESH_ERROR;
}

void * HmeshTpoolAdd() {
  if(HMESH_TREE_POOL.ntrees == 8) {
    HmeshError("HmeshTpoolAdd() : _Index insufficient for "
      "block index. Rewrite using uin32_t");
    return NULL;
  }
  /*
  .. Create a new tree pool and add it to the list of pool
  .. HMESH_TREE_POOL.root[ntrees++] = mmap(..)
  .. 1 X PAGE_SIZE is used to stored information on tree_pool,
  .. 2047 X PAGE_SIZE is free to use (if successful).
  */

  /* 'psize' : or pool size = 8MB 
  .. 'rsize' : root size or the largest chunk that
  .. pool can allocate. rsize = 8 x PAGE_SIZE bytes.
  .. 'limit' : is the bare minimum tree pool size.
  .. limit = 16 x PAGE_SIZE
  */
  size_t psize = HMESH_TREE_POOL_SIZE,
    rsize = (1<<MESH_TREE_POOL_DEPTH) * HMESH_TREE_BLOCK_SIZE,
    limit = 2 * rsize;
 
  _Flag itree = pool->ntrees, depth = HMESH_TREE_POOL_DEPTH;

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
    _HmeshTreepool * pool = &HMESH_TREE_POOL;
    ++(pool->ntrees);
    pools->trees = (_HmeshTpool **) 
      realloc ( trees->root, itree * sizeof(_HmeshTpool *) );
    _HmeshTpool * tree = pools->trees + itree;
    *tree = (_HmeshTpool) 
      { .root = mem, 
        .nodes = (_Flag *) mem, 
        .size = psize
      };

    /* Let's use a part of the pool [0, PAGE_SIZE)
    .. to store the tree nodes info*/

    /* Update the info on the nodes */
    size_t rchunks = psize / rsize; assert(rchunks);
    _Flag * nodes = tree->nodes;
    for(_Index j = rchunks - 1; j >= 0; --j) {
      _Index inode = j * maxnodes;
      /* Adding chunk to free_list @ root (level = 0) 
      .. Encode level and the flag that node is not in use 
      .. _Flag NODE_IS_FREE = 128, NODE_IS_NOT_LEAF = 64;
      */
      _FreeTBlock * fb = 
        (_FreeTBlock *) HmeshTpoolAddress(mem, inode, 0);
      fb->prev  = NULL;
      fb->next  = pool->free_list[0];
      fb->block = (itree << 13) | inode;
      nodes[inode++] = 128; 

      _Flag level = 1;
      /* children of root nodes are not yet developed
      .. and not free*/
      do {
        for(_Index i = 0; i< (1<<level); ++i) 
          nodes[inode++] = level | 64; 
      } while(++level <= depth);
      /* to round off [2^(N+1)-1] to [2^(N+1)] */
      ++inode;
      assert(inode % maxnodes == 0);
    }

    /* Since we use only a (1/2)^N of the first rchunk,
    .. we need to divide it N times successively,
    .. each time adding the right block to free_list*/
    _Index infoNode = HmeshTpoolDivide(itree<<13, 0, depth);
    //assert( (infoNode & HMESH_TREE_POOL_NODES) )

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
    _HmeshTpool * pool = &HMESH_TREE_POOL;
    _Index * nodes = pool->ntrees ? pool->info[pool->ntrees - 1] : NULL;
    
    if(nodes) {
      _Flag level = depth;
      do {
        _FreeBlock * fb = pool->free_list[level];
        /* Cannot find free block @ this level; try larger chunks */
        if(!fb) continue;
   
        /* In case level < depth, divide till depth */ 
        return HmeshTpoolDivide(fb->block, level, depth);

      } while(depth--);
    }

    /* In case there is not= free blocks @ all levels,
    .. create a new pool */
    if(twice) {
      void * mem = HmeshTpoolAdd();
      if(!mem) break;
    }
  }
        
  HmeshError("HmeshTpoolAllocate() : no availability");
  return UINT16_MAX;
 
}

