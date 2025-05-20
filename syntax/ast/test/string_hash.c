//cd ../ && make memory.o
//cd test/ 
//gcc -o test string_hash.c ../hash.o ../memory.o 

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../memory.h"
#include "../hash.h"

const char *keys[] = {
    "hello", "world", "murmur", "hash", "example", "test123", "foo", "bar", "baz",
    "openai", "chatgpt", "compiler", "gcc", "parser", "token", "symbol", "function",
    "macro", "define", "typedef", "variable", "constant", "memory", "pointer",
    "stack", "heap", "malloc", "free", "align", "struct", "union", "bitfield"
};

const uint32_t hashes[] = {
    613153351, 4220927227, 1945310157, 1455707387, 4028466757, 2983061069, 4138058784,
    1158584717, 4050152682, 2573582857, 3669011020, 891344557, 271620362, 69872214,
    3932035137, 1428905997, 1031134330, 2012850642, 1917923092, 2600531311,
    3256841337, 1322743076, 3107807967, 311699825, 3191576, 3193017409, 1249588206,
    1363043438, 2903753601, 1083080571, 599296923, 1316706014
};


int main() {
  _HashTable * t = hash_table_init();

  /*
  .. Murmurhash3 testing. See the list below:
  ..  (32 bit. little-endian. seed = 0)
  ..
  .. "hello"     => 613153351
  .. "world"     => 4220927227
  .. "murmur"    => 1945310157
  .. "hash"      => 1455707387
  .. "example"   => 4028466757
  .. "test123"   => 2983061069
  .. "foo"       => 4138058784
  .. "bar"       => 1158584717
  .. "baz"       => 4050152682
  .. "openai"    => 2573582857
  .. "chatgpt"   => 3669011020
  .. "compiler"  => 891344557
  .. "gcc"       => 271620362
  .. "parser"    => 69872214
  .. "token"     => 3932035137
  .. "symbol"    => 1428905997
  .. "function"  => 1031134330
  .. "macro"     => 2012850642
  .. "define"    => 1917923092
  .. "typedef"   => 2600531311
  .. "variable"  => 3256841337
  .. "constant"  => 1322743076
  .. "memory"    => 3107807967
  .. "pointer"   => 311699825
  .. "stack"     => 3191576
  .. "heap"      => 3193017409
  .. "malloc"    => 1249588206
  .. "free"      => 1363043438
  .. "align"     => 2903753601
  .. "struct"    => 1083080571
  .. "union"     => 599296923
  .. "bitfield"  => 1316706014
  */
  const size_t nkeys = sizeof(keys) / sizeof(keys[0]);

  for(size_t i=0; i<nkeys; ++i) { 
    _HashNode * node = hash_insert ( t, keys[i]);
  }

  for(size_t i=0; i<nkeys; ++i) {
    _HashNode * node = hash_lookup ( t, keys[i]);
      if(node)
        fprintf(stdout, "\nCompare hashes: \"%s\", hash :%u, is same as %u ?", 
          node->key, node->hash, hashes[i]);
  }

  /*
  .. analysing a large text file
  */
  FILE * fp = fopen("hash.txt", "r");
      
  if (!fp) {
    perror("fopen failed");
    return 1;
  }

  int nwords = 0;
  char word[128];
  while (fscanf(fp, "%127s", word) == 1) {
    _HashNode * node = hash_insert ( t, word );
    ++nwords;
  }
  fclose(fp);

  _HashNode ** table = t->table;
  fprintf(stdout, "\n %d scanned", nwords);
  for(size_t i=0; i<=t->bits; ++i)  {
    if(table[i]) {
      
      fprintf(stdout, "\n %zd : ", i); 
      _HashNode * node = table[i];
      while(node) {
        if ( (node->hash & t->bits) != i) {fprintf(stdout, "WW");}
        fprintf(stdout, " \"%s\"", node->key);
        node = node->next;
      }
    }
  }
  

  hash_table_free(t);
  ast_deallocate_all();
}
