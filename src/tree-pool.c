#include <common.h>
#include <tree-pool.h>

/* 
.. Use the HmeshTAllocate() function to allocate memory chunks
.. of size of 
.. {8 x PAGE_SIZE, 4 x PAGE_SIZE, 2 x PAGE_SIZE, 1 x PAGE_SIZE}
*/


static 
_HmeshTpool HMESH_TREE_POOL = 
  { .root = NULL, .chunk = NULL, 
    .ntrees = 0, .free_list = {NULL} };

static inline
_Flag HmeshTpoolDeallocate(_Index block){
  /* assumes max depth < 8 */
  _HmeshTpool * pool = &HMESH_TREE_POOL;
  _Index itree = block >> 12, inode = block & 8191;
  if(itree >= pool->ntrees || inode >= 4096)
    return HMESH_ERROR;
  /* flag 128 is used to see if node is in use/free */
  _Flag * info = pool->chunk[itree],
    depth = info[inode] & 8;
  if( !(info[inode] & 128) )
    /* This node is already free */
    return HMESH_ERROR;
  do {
    info[inode] |= 128;
    /* Coalesce with 'buddy'/'sibling' if possible */
    if (128 & info[inode ^ 1]) 
      /* Go to parent */
      inode = (inode & ~15) | ((inode&15 - 1) >> 1);
    else 
      break;
  } while(depth--);


  return HMESH_ERROR;
}

void * HmeshTpoolAdd() { 
  /*
  .. Create a new tree pool and add it to the list of pool
  .. HMESH_TREE_POOL.root[ntrees++] = mmap(..)
  */

  /* 'psize' :  or pool size = 8MB 
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
        "memory not available for mmap() size of %zd bytes");
      /* Let's try with a smaller chunk size*/
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

    /* Let's divide this memory block to chunks of 
    .. rsize (corresponding to depth 0) := 8 X PAGE_SIZE
    .. Let's put all these chunks into free_list[0].
    .. 'rchunks' no: of root chunks, 
    .. 'ichunk' is an iterator through [0,rchunks).
    .. 'nchunks' total number nodes of tree expanded to 
    .. it's maximum depth.
    */
    size_t rchunks = ichunk = psize / limit,
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

// Global allocator
BiTreeAllocator allocator;

// Initialize memory allocator
void init_allocator() {
    allocator.memory = mmap(NULL, MEMORY_SIZE,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (allocator.memory == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }
    
    memset(allocator.tree, 0, sizeof(allocator.tree));  // Mark all blocks as free
}

// Find the first free block of requested size
int find_block(int index, int depth, int target_depth) {
    if (depth > MAX_DEPTH) return -1;  // Out of bounds
    if (allocator.tree[index] == 1) return -1;  // Block already allocated

    if (depth == target_depth) { // Found a suitable block
        allocator.tree[index] = 1;  // Mark as allocated
        return index;
    }

    int left_child = 2 * index + 1;
    int right_child = 2 * index + 2;

    // Try allocating in the left subtree
    int left_alloc = find_block(left_child, depth + 1, target_depth);
    if (left_alloc != -1) return left_alloc;

    // Try allocating in the right subtree
    return find_block(right_child, depth + 1, target_depth);
}

// Allocate memory of the requested size
void *allocate(size_t size) {
    if (size > MEMORY_SIZE || size < MIN_BLOCK_SIZE) {
        fprintf(stderr, "Invalid size requested: %zu bytes\n", size);
        return NULL;
    }

    // Find depth for the given size
    int target_depth = MAX_DEPTH;
    size_t block_size = MIN_BLOCK_SIZE;
    while (block_size < size) {
        block_size *= 2;
        target_depth--;
    }

    int block_index = find_block(0, 0, target_depth);
    if (block_index == -1) {
        fprintf(stderr, "Allocation failed: no free blocks\n");
        return NULL;
    }

    // Compute the memory address
    int offset = (block_index + 1 - (1 << target_depth)) * block_size;
    return (char *)allocator.memory + offset;
}

// Deallocate memory
void deallocate(void *ptr) {
    if (!ptr || ptr < allocator.memory || ptr >= (char *)allocator.memory + MEMORY_SIZE) {
        fprintf(stderr, "Invalid pointer to free\n");
        return;
    }

    int offset = (char *)ptr - (char *)allocator.memory;
    int block_size = MIN_BLOCK_SIZE;
    int target_depth = MAX_DEPTH;
    
    // Find the block size and depth
    while (offset % (block_size * 2) == 0 && target_depth > 0) {
        block_size *= 2;
        target_depth--;
    }

    int block_index = ((offset / block_size) + (1 << target_depth)) - 1;
    allocator.tree[block_index] = 0;  // Mark block as free

    // Merge with buddy if possible
    while (target_depth > 0) {
        int parent = (block_index - 1) / 2;
        int left_child = 2 * parent + 1;
        int right_child = 2 * parent + 2;

        // If both children are free, merge
        if (allocator.tree[left_child] == 0 && allocator.tree[right_child] == 0) {
            allocator.tree[parent] = 0;
            block_index = parent;
            target_depth--;
        } else {
            break;
        }
    }
}

// Print memory allocation tree (for debugging)
void print_tree() {
    int level = 0, count = 1;
    for (int i = 0; i < (1 << (MAX_DEPTH + 1)) - 1; i++) {
        printf("%d ", allocator.tree[i]);
        if (--count == 0) {
            printf("\n");
            level++;
            count = 1 << level;
        }
    }
    printf("\n");
}

// Cleanup memory allocator
void cleanup_allocator() {
    munmap(allocator.memory, MEMORY_SIZE);
}

int main() {
    init_allocator();
    
    void *ptr1 = allocate(512 * 1024);  // 512KB
    void *ptr2 = allocate(1 * 1024 * 1024);  // 1MB
    void *ptr3 = allocate(256 * 1024);  // 256KB
    print_tree();

    deallocate(ptr1);
    deallocate(ptr2);
    deallocate(ptr3);
    print_tree();
    
    cleanup_allocator();
    return 0;
}

