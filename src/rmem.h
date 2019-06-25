#ifndef RAYLIB_MEMORY_INCLUDED
#    define RAYLIB_MEMORY_INCLUDED


#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

/************* Memory Pool (mempool.c) *************/
typedef struct MemNode {
    size_t size;
    struct MemNode *next, *prev;
} MemNode;

typedef struct AllocList {
    struct MemNode *head, *tail;
    size_t len, maxNodes;
    bool autoDefrag : 1;
} AllocList;

typedef struct Stack {
    uint8_t *mem, *base;
    size_t size;
} Stack;

typedef struct MemPool {
    struct AllocList freeList;
    struct Stack stack;
} MemPool;
/***************************************************/


/************* Object Pool *************/
typedef struct ObjPool {
    struct Stack stack;
    size_t objSize, freeBlocks;
} ObjPool;
/***************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/************* Memory Pool *************/
struct MemPool MemPool_Create(size_t bytes);
struct MemPool MemPool_FromBuffer(void *buf, size_t bytes);
void MemPool_Destroy(struct MemPool *mempool);

void *MemPool_Alloc(struct MemPool *mempool, size_t bytes);
void *MemPool_Realloc(struct MemPool *mempool, void *ptr, size_t bytes);
void MemPool_Free(struct MemPool *mempool, void *ptr);
void MemPool_CleanUp(struct MemPool *mempool, void *ptrref);

size_t MemPool_MemoryRemaining(const struct MemPool mempool);
bool MemPool_DeFrag(struct MemPool *mempool);
void MemPool_ToggleAutoDefrag(struct MemPool *mempool);
/***************************************************/

/************* Object Pool (objpool.c) *************/
struct ObjPool ObjPool_Create(size_t objsize, size_t len);
struct ObjPool ObjPool_FromBuffer(void *buf, size_t objsize, size_t len);
void ObjPool_Destroy(struct ObjPool *objpool);

void *ObjPool_Alloc(struct ObjPool *objpool);
void ObjPool_Free(struct ObjPool *objpool, void *ptr);
void ObjPool_CleanUp(struct ObjPool *objpool, void *ptrref);
/***************************************************/

#ifdef __cplusplus
}
#endif

#endif /* RAYLIB_MEMORY_INCLUDED */
