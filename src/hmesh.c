#include <common.h>
#include <tree-pool.h>
#include <hmesh.h>
#include <ctype.h>

/* 
.. "hmesh_array_add" : Add a block @ iblock-th position.
.. Return memory address of the block, if successful
*/
void * hmesh_array_add (HmeshArray * a, Index iblock, void *** mem)
{

  IndexStack * stack = &a->stack;
  if ( index_stack_allocate (stack, iblock) )
  {
    hmesh_error ("hmesh_array_add () : iblock %d out of bound "
               "or already in use", iblock);
    return NULL;
  }

  /*
  .. Allocate memory for the block using tpool. The address is stored in
  .. (*mem)[iblock] for faster access. a->iblock[iblock] is updated (used
  .. only for deallocation. 
  */
  Index block = hmesh_tpool_allocate_general (a->obj_size);
  void * m = hmesh_tpool_address (block);
  if (!m)
  {
    hmesh_error ("hmesh_array_add () : memory pooling failed");
    index_stack_deallocate (stack, iblock);
    return NULL;
  }
  if (a->stack.max > a->max)
  {
    a->max = a->stack.max;
    a->iblock = realloc (a->iblock, a->max * sizeof (Index));
    *mem = realloc (*mem, a->max * sizeof (void *));
  }
  (*mem)[iblock] = m;
  a->iblock[iblock] = block;

  return m;
}

/*
.. "hmesh_array_remove" : deallocate an existing block @ iblock-th position
*/
int hmesh_array_remove (HmeshArray * a, Index iblock, void *** mem)
{

  if ( !((iblock < a->max) ? (*mem)[iblock] : NULL) )
  {
    hmesh_error ("hmesh_array_remove () : cannot locate memblock");
    return HMESH_ERROR;
  }

  Index status = hmesh_tpool_deallocate (a->iblock[iblock]);
  if (!status)
  {
    (*mem)[iblock] = NULL;
    status |= index_stack_deallocate (&a->stack, iblock);
  }
  if (status)
    hmesh_error ("hmesh_array_remove () : deallocation error");

  return status;
}

/*
.. "hmesh_array" : Create a new attribute. "name" is the name of the attribute,
.. "size" is the size of datatype in bytes. Note : only {1,2,4,8} are allowed.
.. (*mem) is for the array of memory blocks for the attribute.
*/
HmeshArray * hmesh_array (char * name, size_t size, void *** mem)
{
  if (!name || (name[0] == '\0')|| (strlen (name) > HMESH_MAX_VARNAME) )
  {
    hmesh_error ("hmesh_array () : Attribute name require [1,32) chars");
    return NULL;
  }
  HmeshArray * a = malloc (sizeof (HmeshArray));
  a->obj_size = size;
  a->stack    = index_stack (HMESH_MAX_NBLOCKS, 8, NULL);
  a->max      = a->stack.max;
  if (a->max)
  {
    a->iblock = malloc (a->max * sizeof (Index));
    *mem = malloc (a->max * sizeof (void *));
  }

  int s = 0;
  char * c = name;
  while (*c && s < HMESH_MAX_VARNAME)
    a->name[s++] = *c++;
  a->name[s] = '\0';

  return a;
}

/*
.. "hmesh_array_destroy ()" : Destroy attribute object "a",
.. and deallocate all of it's blocks stored in "*mem[]"
*/
int hmesh_array_destroy (HmeshArray * a, void *** mem)
{
  if ( !(a && *mem) )
    return HMESH_ERROR;

  int status = HMESH_NO_ERROR;
  IndexInfo * info = a->stack.info;
  for (int i = 0; i < a->stack.n; ++i)
  {
    Index iblock = info[i].in_use;
    if ( (*mem)[iblock] )
    {
      if ( hmesh_array_remove (a, iblock, mem) )
      {
        status = HMESH_ERROR;
        hmesh_error ("hmesh_array_destroy () : cannot free memblock");
      }
    }
  }

  if (*mem)
  {
    free (*mem);
    *mem = NULL;
  }
  if ( index_stack_destroy (&a->stack) != HMESH_NO_ERROR )
  {
    hmesh_error ("hmesh_array_destroy () : index stack cannot be cleaned."
      " (Memory Leak) ");
  }
  free (a);

  return status;
}

/*
.. "hmesh_cells_expand () " Add 1 block to all the attributes of "cells"
*/
static
int hmesh_cells_expand (HmeshCells * cells)
{
  Index iblock = index_stack_free_head (cells->blocks, 0);
  Index nattr = cells->scalars.n;
  while (nattr--)
  {
    Index iattr = cells->scalars.info[nattr].in_use;
    HmeshArray * attr = (HmeshArray *) cells->attr[iattr];
    if ( !attr )
    {
      hmesh_error ("hmesh_cells_expand () : attr[%d] not found", iattr);
      return HMESH_ERROR;
    }
    void * m = hmesh_array_add (attr, iblock, &cells->mem[iattr]);
    if ( !m )
    {
      hmesh_error ("hmesh_cells_expand () : cannot add block to "
        "attr '%s'", attr->name);
      return HMESH_ERROR;
    }
    if (!iattr)
    {
      Index * prev = (Index *) m;
      prev[0] = 0;
    }
    else if (iattr == 1)
    {
      /* Free index list. Index '0' is reserved node*/
      Index * next = (Index *) m, bsize = hmesh_tpool_block_size ();
      for (Index index = 1; index < bsize-1; ++index)
        next[index] = index + 1;
      next[bsize-1] = 0;
      /* Head of used list*/
      next[0] = 0;
    }
  }

  if (cells->max < cells->blocks->max)
  {
    cells->max = cells->blocks->max;
    cells->info = 
      realloc (cells->info, 4 * cells->max * sizeof (Index));
  }

  Index * info = cells->info + (4*iblock),
    bsize = hmesh_tpool_block_size ();

  /* head of in-use and free-list linked list respectively*/
  info[0] = 0;
  info[1] = 1;
  /* number of nodes in-use and free */
  info[2] = 0;
  info[3] = bsize - 1;

  return HMESH_NO_ERROR;
}

/*
.. Create a HmeshCells (list of vertices, edges, etc ..)
*/
HmeshCells * hmesh_cells (int d, int D)
{
  HmeshCells * cells = malloc  (sizeof (HmeshCells));

  if ( (!cells) || (d>3) || (d>D) )
    return NULL;

  cells->attr = NULL;
  IndexStack * scalars = &cells->scalars;
  *scalars = index_stack (HMESH_MAX_NVARS, 5, &cells->attr);

  /* Default attributes */
  int * nattr = &cells->min;
  *nattr = 2 + (d ? (d + 3) : D);
  for (int iattr = 0; iattr < *nattr; ++iattr)
    index_stack_allocate (scalars, iattr);
  cells->mem = malloc (scalars->max * sizeof (void **));

  /* warning: do warning reallocs carefully*/
  void *** mem = cells->mem,
       ** attr = cells->attr;

  cells->maxs = scalars->max;

  /* 'prev' and 'next' is used to keep double linked
  .. list of nodes */
  HmeshArray * prev =
    hmesh_array ("prev_nodes", sizeof (Index), &mem[0]);
  attr[0] = prev;
  attr[1] = hmesh_array ("next_nodes", sizeof (Index), &mem[1]);

  cells->blocks = prev ? &prev->stack : NULL;
  cells->d = d;
  cells->max = 0;
  cells->info = NULL;

  if (d)
  {
    char v[] = "v0";
    /* sub cells. triangles/edges/vertices */
    v[0] = (d == 3) ? 't' : (d == 2) ? 'e' : 'v';
    for (int iattr = 2; iattr < 2 + (d+1); ++iattr)
    {
      attr[iattr] = hmesh_array (v, sizeof (Node), &mem[iattr]);
      v[1]++;
    }
    /* in half-edge meshes, we keep 'next' & 'twin' */
    attr[d + 3] = hmesh_array ("next", sizeof (Node), &mem[d + 3]);
    attr[d + 4] = hmesh_array ("twin", sizeof (Node), &mem[d + 4]);
  }
  else
  {
    char x[] = "x.x";
    /* Vector 'x' */
    for (int iattr = 2; iattr < *nattr; ++iattr)
    {
      attr[iattr] = hmesh_array (x, sizeof (Real), &mem[iattr]);
      x[2]++;
    }
  }

  for (Index iattr = 0; iattr < *nattr; ++iattr)
  {
    if ( attr[iattr] == NULL )
      hmesh_error ("hmesh_cells () : attribute missing");
  }

  /* Expand all attributes array by 1 block */
  hmesh_cells_expand (cells);

  return cells;
}

/* Destroy cells with all it's attributes */
int hmesh_cells_destroy (HmeshCells * cells)
{
  IndexStack * stack = &cells->scalars;
  void ** attr = cells->attr, *** mem = cells->mem;
  Index n = stack->n;
  int status = HMESH_NO_ERROR;
  while (n--)
  {
    Index iattr = stack->info[n].in_use;
    HmeshArray * s = (HmeshArray *) attr[iattr];
    status |= hmesh_array_destroy (s, &mem[iattr]);
  }
  free (cells->attr);
  free (cells->mem);
  free (stack->info);
  free (cells->info);
  free (cells);

  if (status)
    hmesh_error ("hmesh_cells_destroy () : "
      "Warning! ignoring error in destroying scalars");
  return status;
}

/*
.. Add a scalar with name 'name' to the list of
.. attributes of 'cells'. NOTE: name length should be < 32,
.. and should follow C naming rules
*/
HmeshArray * hmesh_scalar_new (HmeshCells * cells, char * name)
{
  if (!name)
  {
    hmesh_error ("hmesh_scalar_new () : no name specified");
    return NULL;
  }

  if (strlen (name) > HMESH_MAX_VARNAME)
  {
    hmesh_error ("hmesh_scalar_new () : very long name");
    return NULL;
  }

  if (! (isalpha (name[0]) || name[0] == '_') )
  {
    hmesh_error ("hmesh_scalar_new () : wrong naming");
    return NULL;
  }

  for (int i=1; name[i] != '\0'; ++i)
  {
    if ( ! (isalnum (name[0]) || name[0] == '_') )
    {
      hmesh_error ("hmesh_scalar_new () : wrong naming");
      return NULL;
    }
  }

  IndexStack * stack = &cells->scalars;
  void ** attr = cells->attr;
  for (int i=cells->min; i< stack->n; ++i)
  {
    Index iscalar = stack->info[i].in_use;
    HmeshArray * s = (HmeshArray *) attr[iscalar];
    if (!strcmp (s->name, name))
    {
      hmesh_error ("hmesh_scalar_new () : scalar '%s' exists", name);
      return NULL;
    }
  }

  Index iscalar = index_stack_free_head (stack, 1);
  if (iscalar == UINT16_MAX)
  {
    hmesh_error ("hmesh_scalar_new () : out of scalar index");
    return NULL;
  }

  if (stack->max > cells->maxs)
  {
    cells->mem =
      realloc (cells->mem, stack->max * sizeof (void**));
    cells->maxs = stack->max;
  }
  void *** mem = &cells->mem[iscalar];

  HmeshArray * s = hmesh_array (name, sizeof (Real), mem);
  if (!s)
  {
    hmesh_error ("hmesh_scalar_new () : hmesh_array () failed");
    return NULL;
  }
  IndexStack * blocks = cells->blocks;
  for (int i=0; i < blocks->n; ++i)
  {
    Index iblock = blocks->info[i].in_use;
    if ( !hmesh_array_add (s, iblock, mem) )
    {
      hmesh_error ("hmesh_scalar_new () : hmesh_array_add() : failed");
      hmesh_array_destroy (s, mem);
      return NULL;
    }
  }

  cells->attr[iscalar] = s;

  return s;
}

/*
.. Remove the scalar 'name' from the attributes of 'cells'
*/
int hmesh_scalar_remove (HmeshCells * cells, char * name)
{
  if ( ! (name||cells) )
  {
    hmesh_error ("hmesh_scalar_remove () : aborted");
    return HMESH_ERROR;
  }

  IndexStack * stack = &cells->scalars;
  void ** attr = cells->attr, *** mem = cells->mem;
  Index n = stack->n;
  while (n-- > cells->min)
  {
    Index iscalar = stack->info[n].in_use;
    HmeshArray * s = (HmeshArray *) attr[iscalar];
    if ( !strcmp (s->name, name))
    {
      if (hmesh_array_destroy (s, &mem[iscalar]))
      {
        hmesh_error ("hmesh_scalar_remove () : "
          "cannot remove scalar '%s'", name);
        return HMESH_ERROR;
      }
      attr[iscalar] = NULL;
      mem[iscalar]  = NULL;
      index_stack_deallocate (stack, iscalar);
      return HMESH_NO_ERROR;
    }
  }

  hmesh_error ("hmesh_scalar_remove () : scalar '%s' not found", name);
  return HMESH_ERROR;
}

/*
.. Add a node to the 'cells'
*/
Node hmesh_node_new (HmeshCells * cells)
{

  IndexStack * blocks = cells->blocks;
  Index iblock = blocks->info[blocks->n-1].in_use,
    * prev = (Index *) HMESH_ATTR (cells, 0, iblock),
    * next = (Index *) HMESH_ATTR (cells, 1, iblock),
    head = cells->info[4*iblock], fhead = cells->info[4*iblock + 1];

  /* Make sure free head is not empty */
  if (!fhead)
    return (Node) {.index = 0, .iblock = 0};

  /* If this is the last free node in the block, add block */
  if (!next[fhead])
    /* fixme : Optimize a bit to reuse freed nodes in previous blocks */
    hmesh_cells_expand (cells);

  /* Remove fhead from free list, update free head*/
  cells->info [4*iblock+1] = next[fhead];

  /* adding fhead to used list */
  next[fhead] = next[head];
  prev[fhead] = head;
  next[head]  = prev[next[head]] = fhead;
  cells->info[4*iblock] = next[fhead];

  /* Update count*/
  cells->info[4*iblock+2]++;
  cells->info[4*iblock+3]--;

  return (Node) {.index = fhead, .iblock = iblock};
}

/*
.. Add a node to the 'cells'
*/
int hmesh_node_remove (HmeshCells * cells, Node node)
{
  Index iblock = node.iblock, index = node.index,
    * prev = (Index *) HMESH_ATTR (cells, 0, iblock),
    * next = (Index *) HMESH_ATTR (cells, 1, iblock);

  /* Error : Index is already a free index, or the reserved index '0'
  .. or out of bound */
  if ( (prev[index] == UINT16_MAX) || (!index) ||
      (index >= hmesh_tpool_block_size ()) )
    return HMESH_ERROR;

  /* remove 'index' from used list */
  next[prev[index]] = next[index];
  prev[next[index]] = prev[index];
  cells->info[4*iblock] = prev[index];

  /* add 'index' to free list, update free head*/
  next[index] = cells->info[4*iblock+1];
  prev[index] = UINT16_MAX;
  cells->info[4*iblock+1] = index;

  /* Update count*/
  cells->info[4*iblock+2]--;
  cells->info[4*iblock+3]++;

  return HMESH_NO_ERROR;
}

/* API to create, and free a mesh */
int hmesh_destroy (Hmesh * h)
{
  if (!h)
    return HMESH_ERROR;

  HmeshCells ** c[4] = { &h->p, &h->e, &h->t, &h->v };

  int err = HMESH_NO_ERROR;

  /* destroy list of cells */
  for (int _d = 0; _d < 4; ++_d)
    if (*c[_d])
      err |= hmesh_cells_destroy (*c[_d]);

  free (h);

  return err;
}

/*
.. "hmesh (int d, int D)" creates a mesh in Eulerina space R^D.
.. NOTE : volume meshes are not yet implemented
*/
Hmesh * hmesh (int d, int D)
{
  if ( (d > D) || (D > 3) || (!D) )
  {
    hmesh_error ("hmesh () :  Incompatible dim. d%d, D%D", d, D);
    return NULL;
  }
  if (d == 3)
  {
    hmesh_error ("hmesh () :  Volume meshes not available");
    return NULL;
  }

  Hmesh * h = malloc (sizeof (Hmesh));

  /* setting dimension of mesh */
  h->d = d;
  h->D = D;

  HmeshCells ** c[4] = { &h->p, &h->e, &h->t, &h->v };

  for (int _d = 0; _d < 4; ++_d)
    *c[_d] = NULL;

  /* create cells of points, edges, etc .. */
  for (int _d = 0; _d <= d; ++_d)
  {
    HmeshCells * cells = hmesh_cells (_d, D);
    if (!cells)
    {
      hmesh_error ("hmesh () : hmesh_cells (%d, %d) failed", _d, D);
      hmesh_destroy (h);
      return NULL;
    }
    *c[_d] = cells;
  }

  return h;
}
