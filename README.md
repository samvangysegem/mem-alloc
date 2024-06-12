 # Memory Allocators
 A small memory allocation library for future reference and practice purposes

## Pool Allocator 
**Design decisions**
- Memory handle as opaque type - Memory handles allow the creation and mixed use of multiple memory pool throughout the same program with different pool and block sizes, avoiding issues with global state. Hiding this type from the user
- Dynamic and static variations - `memp_create` utilises a call to `malloc` to create and initialise the buffer. However, in embedded systems it is common to pre-allocate buffers which is the use case covered by `memp_create_static`.
- Single pool size - For the moment, the pool allocator only supports pools where all blocks are of the same size. A possible improvement for the future is to allow a single pool to contain blocks of different sizes, enabling more complex memory allocation calls at the expense of a slightly higher time complexity.
