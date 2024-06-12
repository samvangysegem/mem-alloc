 # Memory Allocators
 A small memory allocation library for future reference and practice purposes

## Pool Allocator 
**Design decisions**
- Memory handle as opaque type - Memory handles in this library are designed as opaque types. This allows the creation and mixed use of multiple memory pools within the same program, each with different pool and block sizes. This design avoids issues related to global state by encapsulating the memory details, providing a cleaner and safer interface to the user.
- Dynamic and static variations - The `memp_create` function uses a call to `malloc` to dynamically create and initialize the memory pool. However, in embedded systems, it is common to pre-allocate buffers to manage memory more effectively. The `memp_create_static` function enables this use case by allowing users to provide pre-allocated buffers, ensuring compatibility with embedded system requirements.
- Single pool size - Currently, the pool allocator only supports memory pools where all blocks are of the same size. Future improvements may include the ability to create pools with blocks of varying sizes, enabling more complex memory allocation scenarios. This enhancement would provide greater flexibility at the cost of slightly increased time complexity.
