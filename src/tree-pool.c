#include <common.h>
#include <tree-pool.h>

/* 
.. Use the HmeshTAllocate() function to allocate memory chunks
.. of size of 
.. {8 x PAGE_SIZE, 4 x PAGE_SIZE, 2 x PAGE_SIZE, 1 x PAGE_SIZE}
*/


static 
_HmeshTpool HMESH_TREE_POOL = 
  { .root = NULL, .info = NULL, 
    .ntrees = 0, .free_list = {NULL, } };

static inline char * 
HmeshTpoolAddress(void * start, _Index inode, _Flag depth) {
  _Index h = (1<<depth), H = (1 << HMESH_TREE_BLOCK_DEPTH),
    index = inode & HMESH_TREE_POOL_NODES,
    iroot = inode >> (HMESH_TREE_POOL_DEPTH + 1);
  return (char *) start + 
    ( iroot*H + h*(index - h - 1)) * HMESH_TREE_BLOCK_SIZE;
}

static inline _Index HmeshTpoolParent(_Index inode) {
  return ((inode & HMESH_TREE_POOL_NODES) >> 1) |
    (inode & ~HMESH_TREE_POOL_NODES);    
}

static inline
_Flag HmeshTpoolDeallocate(_Index block){

  /* assumes max depth < 8 */
  _HmeshTpool * pool = &HMESH_TREE_POOL;
  _Index itree = block >> 12, inode = block & 8191;

  if(itree >= pool->ntrees || inode >= 4096) {
    HmeshError("HmeshTpoolDeallocate() : out of bound");
    return HMESH_ERROR;
  }

  /* flag 128 is used to see if node is in use/free */
  _Flag * info = pool->info[itree],
    depth = info[inode] & 8;

  if( info[inode] & 128 ) {
    /* This node is already free */
    HmeshError("HmeshTpoolDeallocate() : "
      "tree pool node not in use");
    return HMESH_ERROR;
  }

  do {
    _Index sibling = inode ^ 1; 
    /* Coalesce with 'buddy'/'sibling' if possible */
    if ( (128 & info[sibling]) && depth ) {

      /* Remove sibling out of free list */
      info[sibling] &= ~128; 
      _FreeTBlock * fb = (_FreeTBlock *) 
        HmeshTpoolAddress( pool->root[itree], sibling, depth);
    
      fb->next->prev = fb->prev;
      fb->prev->next = fb->next;
      
      /* Go to parent */
      inode = HmeshTpoolParen(inode);
    }
    else {
      /* Mark inode as free and add it to the free list*/
      info[inode] |= 128; 
      _FreeTBlock * fb = (_FreeTBlock *) 
        HmeshTpoolAddress( pool->root[itree], sibling, depth);
      break;
    }
  } while(depth--);


  return HMESH_ERROR;
}

void * HmeshTpoolAdd() { 
  /*
  .. Create a new tree pool and add it to the list of pool
  .. HMESH_TREE_POOL.root[ntrees++] = mmap(..)
  */

  /* 'psize' : or pool size = 8MB 
  .. 'rsize' : root size or the largest chunk that
  .. pool can allocate. rsize = 8 x PAGE_SIZE bytes.
  .. 'limit' : is the bare minimum tree pool size.
  .. limit = 16 x PAGE_SIZE
  */
  size_t psize = HMESH_TREE_POOL_SIZE;

  size_t rsize = (1<<MESH_TREE_POOL_DEPTH) * HMESH_TREE_BLOCK_SIZE,
    limit = 2 * rsize;

  while(psize >= limit) {
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
    _HmeshTpool * pool = &HMESH_TREE_POOL;
    _Flag itree = ++(pool->ntrees);
    pool->root = (void **) 
      realloc ( pool->root, itree * sizeof(void *) );
    pool->chunk = (_Flag **) 
      realloc ( pool->chunk, itree * sizeof(_Flag *) );
    --itree; 
    pool->root[itree] = mem;

    if(!itree) 
      for(_Flag level = 0; level <= HMESH_POOL_DEPTH; ++level){
        pool->free_list[level]->next = 
          pool->free_list[level]->prev = pool->free_list[level];
      }
    

    /* Let's divide this memory block to chunks of 
    .. rsize (corresponding to depth 0) := 8 X PAGE_SIZE
    .. Let's put all these chunks into free_list[0].
    .. 'rchunks' no: of root chunks, 
    .. 'ichunk' is an iterator through [0,rchunks).
    .. 'nchunks' total number nodes of tree expanded to 
    .. it's maximum depth.
    */
    size_t rchunks = ichunk = psize / rsize,
      nchunks = rchunks *   
    assert(rchunks);
    char * rchunk = (char *) mem + ((rchunks - 1)*rsize);
    /* Update the free_list. NOTE : ichunk == 0 is used to store
    .. tree node information. */
    while(--ichunk) {
      /* Adding this chunk to free_list @ root (level = 0) */
      _FreeTBlock * fb = (_FreeTBlock) rchunk;
      fb->next = pool->free_list[0];
      fb->block = (_MemTBlock) 
        {.itree = itree, iblock = ichunk};
      pool->free_list[0] = fb;
      rchunk -= rsize;
    }

    /* Let's use a part of the pool [0, PAGE
    .. to store the tree nodes info*/
    _Flag * index = (_Flag *) mem;
    pool->chunk[itree] = index;

    /* Update the info on the nodes */
    ichunk = nchunks;
    while(ichunk--) {
      _Flag level = 0;
      while(level <= HMESH_TREE_POOL_DEPTH) {
        for(size_t inode = 0; inode< (1<<level); ++inode) {
          /* Encode level and the flag that node is not in use 
            _Flag NODE_IN_USE = 128;
            *index++ = level & ~NODE_IN_USE;  
          */
          *index++ = level;
        }
        level++; 
      }
      /* to round off [2^(N+1)-1] to [2^(N+1)] */
      *index++ = HMESH_TREE_POOL_DEPTH + 1; 
    }

    /* Since we use only a (1/2)^N of the first rchunk,
    .. we need to divide it N times successively,
    .. each time adding the right block to free_list*/
    _Flag level = HMESH_TREE_POOL_DEPTH;
    while(--level) {
    
    }

  }
      
  HmeshError("HmeshTpoolAdd() : failed to create a tree_pool");
  return NULL; 
}

