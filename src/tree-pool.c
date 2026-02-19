#include <common.h>
#include <tree-pool.h>

/*
.. MAP_ANONYMOUS not found in few compilers. This file is expected to be
.. compiled successfully in linux/MacOS. (NOT VERIFIED)
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
.. Use the hmesh_tallocate () function to allocate memory chunksi of size of
.. { 8 x PAGE_SIZE, 4 x PAGE_SIZE, 2 x PAGE_SIZE, 1 x PAGE_SIZE }
*/

/*
.. "HmeshTpool" : collection of tree pools
.. 'trees' : collection of pools
.. 'ntree's : number of trees in use
.. 'free_list' : Free blocks of each level
*/
typedef struct
{
  HmeshTpool * trees;
  int ntrees;
  FreeTBlock * free_list [HMESH_TREE_POOL_DEPTH + 1];
} HmeshTpools;

/*
.. For the moment, let's fix the BLOCK_SIZE = no: of nodes per block to 4096.
.. So the size of chunks fall in 4096 * \{ 1, 2, 4, 8\}, guaranteeing the
.. minimum of the chunk size is a PAGE_SIZE = 4096 Bytes. We try to allocate
.. 8MB of chunk (by default).
*/
#define HMESH_TREE_POOL_SIZE 1<<23
static  HmeshTpools HMESH_TREE_POOLS = {0};
const   size_t HMESH_TREE_POOL_NODES = (1 << (HMESH_TREE_POOL_DEPTH+1)) - 1;
const   size_t HMESH_TREE_BLOCK_SIZE = HMESH_PAGE_SIZE;

size_t hmesh_tpool_block_size ()
{
  return HMESH_TREE_BLOCK_SIZE;
}

HmeshTpool * hmesh_tpool_tree (int itree)
{
  return itree < HMESH_TREE_POOLS.ntrees ?
    &HMESH_TREE_POOLS.trees[itree] : NULL;
}

static inline
void * hmesh_tpool_address_is (void * start, Index inode, int depth)
{
  Index
    h     = 1 << depth,
    H     = 1 << HMESH_TREE_POOL_DEPTH,
    D     = 1 << (HMESH_TREE_POOL_DEPTH - depth),
    index = inode & HMESH_TREE_POOL_NODES,
    iroot = inode >> (HMESH_TREE_POOL_DEPTH + 1);
  return (void *) ( (char *) start +
    (iroot*H + (index - h + 1)*D) * HMESH_TREE_BLOCK_SIZE);
}

void * hmesh_tpool_address (Index blockID)
{
  Index
    itree = blockID >> 13,
    inode = blockID & 8191;

  if ( ! (itree < HMESH_TREE_POOLS.ntrees && inode < 4096) )
    return NULL;

  return hmesh_tpool_address_is (HMESH_TREE_POOLS.trees[itree].root,
    inode, HMESH_TREE_POOLS.trees[itree].flags[inode] & 7);
}

static inline
Index hmesh_tpool_parent (Index inode)
{
  return ( ( (inode & HMESH_TREE_POOL_NODES) - 1) >> 1) |
    (inode & ~HMESH_TREE_POOL_NODES);
}

static inline
Index hmesh_tpool_child (Index inode, Index ichild)
{
  /*assert (ichild <2)*/
  return (inode & ~HMESH_TREE_POOL_NODES) |
    (((inode & HMESH_TREE_POOL_NODES) << 1) + ichild + 1);
}

static inline
void hmesh_tpool_pop (FreeTBlock * fb, int depth)
{
  /* remove a free block from free list */
  if ( fb->prev )
    fb->prev->next = fb->next;
  else
    HMESH_TREE_POOLS.free_list [depth] = fb->next;
  if ( fb->next )
    fb->next->prev = fb->prev;
}

static inline
void hmesh_tpool_push (FreeTBlock * fb, int depth)
{
  fb->prev = NULL;
  fb->next = HMESH_TREE_POOLS.free_list[depth];
  if (fb->next)
    fb->next->prev = fb;
  /* add the free block at the free list head*/
  HMESH_TREE_POOLS.free_list[depth] = fb;
}

static
Index  hmesh_tpool_divide (FreeTBlock * _fb, int level, int depth)
{
  /* Divide a freeblock till 'depth' */
  hmesh_tpool_pop (_fb, level);

  Index
    blockID = _fb->blockID,
    inode = blockID & 8191,
    itree = blockID >> 13;

  if (itree >= HMESH_TREE_POOLS.ntrees || inode >= 4096)
  {
    hmesh_error ("hmesh_tpool_deallocate () : out of bound");
    return UINT16_MAX;
  }

  HmeshTpool * tree = & HMESH_TREE_POOLS.trees [itree];
  uint8_t * flags = tree->flags;
  while (level++ < depth)
  {
    /*
    .. Keep on dividing the chunk till the size is of desirable size
    */
    Index
      left  = hmesh_tpool_child (inode, 0),
      right = hmesh_tpool_child (inode, 1);

    /*flags[left]  = level;        In use */
    flags[right] = level | 128;                              /* In free list */

    FreeTBlock * fb = (FreeTBlock *)
      hmesh_tpool_address_is (tree->root, right, level);
    *fb = (FreeTBlock) { .blockID = right | (itree << 13) };
    /* pushing right half as a free block @ free list head*/
    hmesh_tpool_push (fb, level);

    flags[inode] = (level - 1) | 64;  /* inode is divided and no more a leaf */

    inode = left;                                        /* Go to left child */
  }

  flags[inode] = depth;
  return inode | (itree << 13);
}

/* assumes max depth < 8 */
int hmesh_tpool_deallocate (Index blockID)
{

  /*
  .. last 13 bits are reserved to store node index,
  .. and the rest are used to store tree no (itree)
  */
  Index itree = blockID >> 13, inode = blockID & 8191;

  if (itree >= HMESH_TREE_POOLS.ntrees || inode >= 4096)
  {
    hmesh_error ("hmesh_tpool_deallocate () : out of bound");
    return HMESH_ERROR;
  }

  HmeshTpool * tree = & HMESH_TREE_POOLS.trees [itree];

  /*
  .. last 3 bits are used to encode depth of the node, Flag 128 is used to
  .. see if node is free, Flag 64 is used in case the node is not a leaf
  */
  uint8_t * flags = tree->flags,
    depth = flags [inode] & 7;

  if ( flags[inode] & (128|64) )
  {
    hmesh_error ("hmesh_tpool_deallocate () : "
      "tree pool node cannot be deallocated");
    return HMESH_ERROR;
  }

  /*
  .. While deallocating a block coalesce with 'buddy'/'sibling' if possible
  */
  do
  {
    Index sibling = inode + ((inode & 1) ? 1 : -1);
    if ( (128 & flags[sibling]) && depth )
    {
      flags [sibling] = flags[inode] = (depth | 64);
      FreeTBlock * fb = (FreeTBlock *)
        hmesh_tpool_address_is ( tree->root, sibling, depth);
      assert ((fb->blockID & 8191) == sibling);
      hmesh_tpool_pop (fb, depth);
      inode = hmesh_tpool_parent (inode);
    }
    else
    {
      flags[inode] |= 128;
      FreeTBlock * fb = (FreeTBlock *)
        hmesh_tpool_address_is ( tree->root, inode, depth);
      *fb = (FreeTBlock) { .blockID = inode | (itree << 13) };
      hmesh_tpool_push (fb, depth);
      break;
    }
  } while (depth--);

  return HMESH_NO_ERROR;
}

/*
.. Creating a memory block using mmap (). This function will be called when you
.. ask to allocate the first time, or you run out of the current block
*/
static
void * hmesh_tpool_add ()
{
  if (HMESH_TREE_POOLS.ntrees == 8)
  {
    hmesh_error ("hmesh_tpool_add () : Index insufficient for "
      "block index. Rewrite using uin32_t");
    return NULL;
  }
  /*
  .. Create a new tree pool and add it to the list of pool
  .. HMESH_TREE_POOLS.root[ntrees++] = mmap (..)
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
    limit = 2 * rsize, maxnodes = 1 << (HMESH_TREE_POOL_DEPTH+1);

  int itree = HMESH_TREE_POOLS.ntrees,
    depth = HMESH_TREE_POOL_DEPTH;

  while (psize >= limit)
  {
    /* Trying to pool memory directly from OS rather than
    .. using malloc (). */
    void * mem = mmap (NULL, psize, PROT_READ | PROT_WRITE,
      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED)
    {
      /*
      .. "hmesh_tpool_add () : Warning! mmap () failed for %zd bytes");
      */
      psize = psize >> 1;
      continue;
    }

    /* Created a pool of size 'psize'. */
    HmeshTpools * pool = & HMESH_TREE_POOLS;
    ++(pool->ntrees);
    pool->trees = realloc
        ( pool->trees, (pool->ntrees) * sizeof (HmeshTpool) );
    HmeshTpool * tree = pool->trees + itree;
    *tree = (HmeshTpool)
    {
      .root  = mem,
      .flags = (uint8_t *) mem,
      .size  = psize
    };

    /* Let's use a part of the pool [0, PAGE_SIZE)
    .. to store the tree nodes flags*/

    /* Update the flags on the nodes */
    size_t rchunks = psize / rsize; assert (rchunks);
    uint8_t * flags = tree->flags;
    FreeTBlock _0b;
    for (int j = rchunks - 1; j >= 0; --j)
    {
      Index inode = j * maxnodes;
      /*
      .. Adding chunk to free_list @ root (level = 0)
      .. Encode level and the flag that node is not in use
      .. _Flag NODE_IS_FREE = 128, NODE_IS_NOT_LEAF = 64;
      */

      FreeTBlock * fb = !j ? & _0b :
        (FreeTBlock *) hmesh_tpool_address_is (mem, inode, 0);

      *fb = (FreeTBlock) { .blockID = (itree << 13) | inode };
      /* pushing @ free head of free_list[0]*/
      hmesh_tpool_push (fb, 0);
      flags [inode++] = 128;

      int level = 1;
      /*
      .. children of root nodes are not yet developed and not free
      */
      do
      {
        for (Index i = 0; i< (1<<level); ++i)
          flags[inode++] = level | 64;
      } while (++level <= depth);
      /* to round off [2^(N+1)-1] to [2^(N+1)] */
      ++inode;
      assert (inode % maxnodes == 0);
    }

    /*
    .. Since we use only a (1/2)^N of the first rchunk, we need to divide it
    .. N times successively, each time adding the right block to free_list
    */
    Index flagsNode = hmesh_tpool_divide (&_0b, 0, depth);
    assert ( flagsNode == ((itree << 13) | ((1<<depth) - 1)) );

    return mem;
  }

  hmesh_error ("hmesh_tpool_add () : failed to create a tree_pool");
  return NULL;
}

/*
.. fixme : make this an internal routine.
*/
Index hmesh_tpool_allocate (int depth)
{
  if (depth > HMESH_TREE_POOL_DEPTH)
  {
    hmesh_error ("hmesh_tpool_allocate () : depth out of bound");
    return UINT16_MAX;
  }

  int twice = 2;
      /* Cannot find free block @ this level; try larger chunks */
      /* In case level < depth, divide till depth
    .. In case there is no free blocks @ all levels, create a new pool
    */
  while (twice--)
  {
    int level = depth;
    do
    {
      FreeTBlock * fb = HMESH_TREE_POOLS.free_list [level];
      if (!fb) continue;
      return hmesh_tpool_divide (fb, level, depth);
    } while (level--);

    if (twice)
    {
      void * mem = hmesh_tpool_add ();
      if (!mem) break;
    }
  }

  hmesh_error ("hmesh_tpool_allocate () : no availability");
  return UINT16_MAX;
}

/* This is the API to allocate a memory block of size
.. [obj_size X PAGE_SIZE].
.. NOTE : as of now obj_size in 1, 2, 4, 8 can be
.. handled.
*/
Index hmesh_tpool_allocate_general (size_t obj_size)
{
  size_t n = obj_size, r = 0;
  int level = 0;
  while (n)
  {
    if (r)
      break;
    r = n & 1;
    n = n >> 1;
    ++level;
  }
  if ( (n && r) || (!obj_size))
  {
    hmesh_error ("hmesh_tpool_allocate_general () : "
      "obj_size should be [1, 2, .., 2^HPOOL_DEPTH]");
    return UINT16_MAX;
  }
  --level;
  if ( level > HMESH_TREE_POOL_DEPTH )
  {
    hmesh_error ("hmesh_tpool_allocate_general () : out of bound");
    return UINT16_MAX;
  }
  return
    hmesh_tpool_allocate (HMESH_TREE_POOL_DEPTH - level);
}

/*
.. Allocate a page of size 4096 bytes
*/
Index hmesh_tpool_allocate_page ()
{
  return hmesh_tpool_allocate (HMESH_TREE_POOL_DEPTH);
}

/*
.. "hmesh_tpool_destroy ()" : cleans all tree pool allocated. Expected
.. to be called at teh end of pgm, after this you don't expect a tree
.. allocate
*/
void hmesh_tpool_destroy ()
{
  int itree = HMESH_TREE_POOLS.ntrees;
  HmeshTpool * tree = HMESH_TREE_POOLS.trees;
  while (itree--)
  {
    /* fixme: add error msg */
    munmap (tree->root, tree->size);
    ++tree;
  }
}
