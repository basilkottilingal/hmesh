#Abstract Syntax Tree

AST is the tree data structure to represent a source code, whose leaf nodes are tokens:
keywords, identifiers, constants, operators, special symbols, and strings.
Each grammar rules like statement, expression, etc. corresponds to an internal
node of the AST tree. 

AST data structure requires a memory allocator every time an AST node is inserted,
which can be done using malloc/realloc or more efficiently using a memory pool
allocator.

You have to take care of these things while designing a parser that constructs an AST
| `aspect`  | things to be considered |
| --- | --- |
| `Grammar`       | Grammar	Clean, unambiguous grammar, precedence, associativity |
| `Node Design`	  | Clear node types, flexible child structure                    |
| `Tree`          | Construction	Simple, reusable node constructors              |
| `Memory Safety` |	Managed memory or smart cleanup strategy                      |
| `Traversals`	  | Traversal API, export format, error locations                 |
| `Debuggability`	| Print, graph, or serialize AST                                |

## Memory Pool of Ast Node
  
## Hash Table based identifier Stack

##    
