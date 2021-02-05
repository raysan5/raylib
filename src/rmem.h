/**********************************************************************************************
*
*   rmem - raylib memory pool and objects pool
*
*   A quick, efficient, and minimal free list and arena-based allocator
*
*   PURPOSE:
*     - A quicker, efficient memory allocator alternative to 'malloc' and friends.
*     - Reduce the possibilities of memory leaks for beginner developers using Raylib.
*     - Being able to flexibly range check memory if necessary.
*
*   CONFIGURATION:
*
*   #define RMEM_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2019 Kevin 'Assyrianic' Yonan (@assyrianic) and reviewed by Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef RMEM_H
#define RMEM_H

#include <inttypes.h>
#include <stdbool.h>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#if defined(_WIN32) && defined(BUILD_LIBTYPE_SHARED)
    #define RMEMAPI __declspec(dllexport)         // We are building library as a Win32 shared library (.dll)
#elif defined(_WIN32) && defined(USE_LIBTYPE_SHARED)
    #define RMEMAPI __declspec(dllimport)         // We are using library as a Win32 shared library (.dll)
#else
    #define RMEMAPI   // We are building or using library as a static library (or Linux shared library)
#endif

#define RMEM_VERSION    "v1.3"    // changelog at bottom of header.

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Memory Pool
typedef struct MemNode MemNode;
struct MemNode {
    size_t size;
    MemNode *next, *prev;
};

// Freelist implementation
typedef struct AllocList {
    MemNode *head, *tail;
    size_t len;
} AllocList;

// Arena allocator.
typedef struct Arena {
    uintptr_t mem, offs;
    size_t size;
} Arena;


enum {
    MEMPOOL_BUCKET_SIZE = 8,
    MEMPOOL_BUCKET_BITS = (sizeof(uintptr_t) >> 1) + 1,
    MEM_SPLIT_THRESHOLD = sizeof(uintptr_t) * 4
};

typedef struct MemPool {
    AllocList large, buckets[MEMPOOL_BUCKET_SIZE];
    Arena arena;
} MemPool;


// Object Pool
typedef struct ObjPool {
    uintptr_t mem, offs;
    size_t objSize, freeBlocks, memSize;
} ObjPool;


// Double-Ended Stack aka Deque
typedef struct BiStack {
    uintptr_t mem, front, back;
    size_t size;
} BiStack;


#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

//------------------------------------------------------------------------------------
// Functions Declaration - Memory Pool
//------------------------------------------------------------------------------------
RMEMAPI MemPool CreateMemPool(size_t bytes);
RMEMAPI MemPool CreateMemPoolFromBuffer(void *buf, size_t bytes);
RMEMAPI void DestroyMemPool(MemPool *mempool);

RMEMAPI void *MemPoolAlloc(MemPool *mempool, size_t bytes);
RMEMAPI void *MemPoolRealloc(MemPool *mempool, void *ptr, size_t bytes);
RMEMAPI void MemPoolFree(MemPool *mempool, void *ptr);
RMEMAPI void MemPoolCleanUp(MemPool *mempool, void **ptrref);
RMEMAPI void MemPoolReset(MemPool *mempool);
RMEMAPI size_t GetMemPoolFreeMemory(const MemPool mempool);

//------------------------------------------------------------------------------------
// Functions Declaration - Object Pool
//------------------------------------------------------------------------------------
RMEMAPI ObjPool CreateObjPool(size_t objsize, size_t len);
RMEMAPI ObjPool CreateObjPoolFromBuffer(void *buf, size_t objsize, size_t len);
RMEMAPI void DestroyObjPool(ObjPool *objpool);

RMEMAPI void *ObjPoolAlloc(ObjPool *objpool);
RMEMAPI void ObjPoolFree(ObjPool *objpool, void *ptr);
RMEMAPI void ObjPoolCleanUp(ObjPool *objpool, void **ptrref);

//------------------------------------------------------------------------------------
// Functions Declaration - Double-Ended Stack
//------------------------------------------------------------------------------------
RMEMAPI BiStack CreateBiStack(size_t len);
RMEMAPI BiStack CreateBiStackFromBuffer(void *buf, size_t len);
RMEMAPI void DestroyBiStack(BiStack *destack);

RMEMAPI void *BiStackAllocFront(BiStack *destack, size_t len);
RMEMAPI void *BiStackAllocBack(BiStack *destack, size_t len);

RMEMAPI void BiStackResetFront(BiStack *destack);
RMEMAPI void BiStackResetBack(BiStack *destack);
RMEMAPI void BiStackResetAll(BiStack *destack);

RMEMAPI intptr_t BiStackMargins(BiStack destack);

#ifdef __cplusplus
}
#endif

#endif // RMEM_H

/***********************************************************************************
*
*   RMEM IMPLEMENTATION
*
************************************************************************************/

#if defined(RMEM_IMPLEMENTATION)

#include <stdio.h>          // Required for:
#include <stdlib.h>         // Required for:
#include <string.h>         // Required for:

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------

// Make sure restrict type qualifier for pointers is defined
// NOTE: Not supported by C++, it is a C only keyword
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(_MSC_VER)
    #ifndef restrict
        #define restrict __restrict
    #endif
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
static inline size_t __AlignSize(const size_t size, const size_t align)
{
    return (size + (align - 1)) & -align;
}

static MemNode *__SplitMemNode(MemNode *const node, const size_t bytes)
{
    uintptr_t n = ( uintptr_t )node;
    MemNode *const r = ( MemNode* )(n + (node->size - bytes));
    node->size -= bytes;
    r->size = bytes;
    return r;
}

static void __InsertMemNodeBefore(AllocList *const list, MemNode *const insert, MemNode *const curr)
{
    insert->next = curr;
    if (curr->prev==NULL) list->head = insert;
    else
    {
        insert->prev = curr->prev;
        curr->prev->next = insert;
    }
    curr->prev = insert;
}

static void __ReplaceMemNode(MemNode *const old, MemNode *const replace)
{
    replace->prev = old->prev;
    replace->next = old->next;
    if( old->prev != NULL )
        old->prev->next = replace;
    if( old->next != NULL )
        old->next->prev = replace;
}


static MemNode *__RemoveMemNode(AllocList *const list, MemNode *const node)
{
    if (node->prev != NULL) node->prev->next = node->next;
    else
    {
        list->head = node->next;
        if (list->head != NULL) list->head->prev = NULL;
        else list->tail = NULL;
    }

    if (node->next != NULL) node->next->prev = node->prev;
    else
    {
        list->tail = node->prev;
        if (list->tail != NULL) list->tail->next = NULL;
        else list->head = NULL;
    }
    list->len--;
    return node;
}

static MemNode *__FindMemNode(AllocList *const list, const size_t bytes)
{
    for (MemNode *node = list->head; node != NULL; node = node->next)
    {
        if (node->size < bytes) continue;
        // close in size - reduce fragmentation by not splitting.
        else if (node->size <= bytes + MEM_SPLIT_THRESHOLD) return __RemoveMemNode(list, node);
        else return __SplitMemNode(node, bytes);
    }
    return NULL;
}

static void __InsertMemNode(MemPool *const mempool, AllocList *const list, MemNode *const node, const bool is_bucket)
{
    if (list->head == NULL)
    {
        list->head = node;
        list->len++;
    }
    else
    {
        for (MemNode *iter = list->head; iter != NULL; iter = iter->next)
        {
            if (( uintptr_t )iter == mempool->arena.offs)
            {
                mempool->arena.offs += iter->size;
                __RemoveMemNode(list, iter);
                iter = list->head;
            }
            const uintptr_t inode = ( uintptr_t )node;
            const uintptr_t iiter = ( uintptr_t )iter;
            const uintptr_t iter_end = iiter + iter->size;
            const uintptr_t node_end = inode + node->size;
            if (iter==node) return;
            else if (iter < node)
            {
                // node was coalesced prior.
                if (iter_end > inode) return;
                else if (iter_end==inode && !is_bucket)
                {
                    // if we can coalesce, do so.
                    iter->size += node->size;
                    return;
                }
            }
            else if (iter > node)
            {
                // Address sort, lowest to highest aka ascending order.
                if (iiter < node_end) return;
                else if (iter==list->head && !is_bucket)
                {
                    if (iter_end==inode) iter->size += node->size;
                    else if (node_end==iiter)
                    {
                        node->size += list->head->size;
                        node->next = list->head->next;
                        node->prev = NULL;
                        list->head = node;
                    }
                    else
                    {
                        node->next = iter;
                        node->prev = NULL;
                        iter->prev = node;
                        list->head = node;
                        list->len++;
                    }
                    return;
                }
                else if (iter_end==inode && !is_bucket)
                {
                    // if we can coalesce, do so.
                    iter->size += node->size;
                    return;
                }
                else
                {
                    __InsertMemNodeBefore(list, iter, node);
                    list->len++;
                    return;
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Memory Pool
//----------------------------------------------------------------------------------

MemPool CreateMemPool(const size_t size)
{
    MemPool mempool = { 0 };

    if (size == 0) return mempool;
    else
    {
        // Align the mempool size to at least the size of an alloc node.
        uint8_t *const restrict buf = malloc(size*sizeof *buf);
        if (buf==NULL) return mempool;
        else
        {
            mempool.arena.size = size;
            mempool.arena.mem = ( uintptr_t )buf;
            mempool.arena.offs = mempool.arena.mem + mempool.arena.size;
            return mempool;
        }
    }
}

MemPool CreateMemPoolFromBuffer(void *const restrict buf, const size_t size)
{
    MemPool mempool = { 0 };
    if ((size == 0) || (buf == NULL) || (size <= sizeof(MemNode))) return mempool;
    else
    {
        mempool.arena.size = size;
        mempool.arena.mem = ( uintptr_t )buf;
        mempool.arena.offs = mempool.arena.mem + mempool.arena.size;
        return mempool;
    }
}

void DestroyMemPool(MemPool *const restrict mempool)
{
    if (mempool->arena.mem == 0) return;
    else
    {
        void *const restrict ptr = ( void* )mempool->arena.mem;
        free(ptr);
        *mempool = (MemPool){ 0 };
    }
}

void *MemPoolAlloc(MemPool *const mempool, const size_t size)
{
    if ((size == 0) || (size > mempool->arena.size)) return NULL;
    else
    {
        MemNode *new_mem = NULL;
        const size_t ALLOC_SIZE = __AlignSize(size + sizeof *new_mem, sizeof(intptr_t));
        const size_t BUCKET_SLOT = (ALLOC_SIZE >> MEMPOOL_BUCKET_BITS) - 1;

        // If the size is small enough, let's check if our buckets has a fitting memory block.
        if (BUCKET_SLOT < MEMPOOL_BUCKET_SIZE)
        {
            new_mem = __FindMemNode(&mempool->buckets[BUCKET_SLOT], ALLOC_SIZE);
        }
        else if (mempool->large.head != NULL)
        {
            new_mem = __FindMemNode(&mempool->large, ALLOC_SIZE);
        }

        if (new_mem == NULL)
        {
            // not enough memory to support the size!
            if ((mempool->arena.offs - ALLOC_SIZE) < mempool->arena.mem) return NULL;
            else
            {
                // Couldn't allocate from a freelist, allocate from available mempool.
                // Subtract allocation size from the mempool.
                mempool->arena.offs -= ALLOC_SIZE;

                // Use the available mempool space as the new node.
                new_mem = ( MemNode* )mempool->arena.offs;
                new_mem->size = ALLOC_SIZE;
            }
        }

        // Visual of the allocation block.
        // --------------
        // | mem size   | lowest addr of block
        // | next node  | 12 byte (32-bit) header
        // | prev node  | 24 byte (64-bit) header
        // |------------|
        // |   alloc'd  |
        // |   memory   |
        // |   space    | highest addr of block
        // --------------
        new_mem->next = new_mem->prev = NULL;
        uint8_t *const restrict final_mem = ( uint8_t* )new_mem + sizeof *new_mem;
        return memset(final_mem, 0, new_mem->size - sizeof *new_mem);
    }
}

void *MemPoolRealloc(MemPool *const restrict mempool, void *const ptr, const size_t size)
{
    if (size > mempool->arena.size) return NULL;
    // NULL ptr should make this work like regular Allocation.
    else if (ptr == NULL) return MemPoolAlloc(mempool, size);
    else if ((uintptr_t)ptr - sizeof(MemNode) < mempool->arena.mem) return NULL;
    else
    {
        MemNode *const node = ( MemNode* )(( uint8_t* )ptr - sizeof *node);
        const size_t NODE_SIZE = sizeof *node;
        uint8_t *const resized_block = MemPoolAlloc(mempool, size);
        if (resized_block == NULL) return NULL;
        else
        {
            MemNode *const resized = ( MemNode* )(resized_block - sizeof *resized);
            memmove(resized_block, ptr, (node->size > resized->size)? (resized->size - NODE_SIZE) : (node->size - NODE_SIZE));
            MemPoolFree(mempool, ptr);
            return resized_block;
        }
    }
}

void MemPoolFree(MemPool *const restrict mempool, void *const ptr)
{
    const uintptr_t p = ( uintptr_t )ptr;
    if ((ptr == NULL) || (p - sizeof(MemNode) < mempool->arena.mem)) return;
    else
    {
        // Behind the actual pointer data is the allocation info.
        const uintptr_t block = p - sizeof(MemNode);
        MemNode *const mem_node = ( MemNode* )block;
        const size_t BUCKET_SLOT = (mem_node->size >> MEMPOOL_BUCKET_BITS) - 1;

        // Make sure the pointer data is valid.
        if ((block < mempool->arena.offs) ||
            ((block - mempool->arena.mem) > mempool->arena.size) ||
            (mem_node->size == 0) ||
            (mem_node->size > mempool->arena.size)) return;
        // If the mem_node is right at the arena offs, then merge it back to the arena.
        else if (block == mempool->arena.offs)
        {
            mempool->arena.offs += mem_node->size;
        }
        else
        {
            // try to place it into bucket or large freelist.
            struct AllocList *const l = (BUCKET_SLOT < MEMPOOL_BUCKET_SIZE) ? &mempool->buckets[BUCKET_SLOT] : &mempool->large;
            __InsertMemNode(mempool, l, mem_node, (BUCKET_SLOT < MEMPOOL_BUCKET_SIZE));
        }
    }
}

void MemPoolCleanUp(MemPool *const restrict mempool, void **const ptrref)
{
    if ((ptrref == NULL) || (*ptrref == NULL)) return;
    else
    {
        MemPoolFree(mempool, *ptrref);
        *ptrref = NULL;
    }
}

size_t GetMemPoolFreeMemory(const MemPool mempool)
{
    size_t total_remaining = mempool.arena.offs - mempool.arena.mem;

    for (MemNode *n=mempool.large.head; n != NULL; n = n->next) total_remaining += n->size;

    for (size_t i=0; i<MEMPOOL_BUCKET_SIZE; i++) for (MemNode *n = mempool.buckets[i].head; n != NULL; n = n->next) total_remaining += n->size;

    return total_remaining;
}

void MemPoolReset(MemPool *const mempool)
{
    mempool->large.head = mempool->large.tail = NULL;
    mempool->large.len = 0;
    for (size_t i = 0; i < MEMPOOL_BUCKET_SIZE; i++)
    {
        mempool->buckets[i].head = mempool->buckets[i].tail = NULL;
        mempool->buckets[i].len = 0;
    }
    mempool->arena.offs = mempool->arena.mem + mempool->arena.size;
}

//----------------------------------------------------------------------------------
// Module Functions Definition - Object Pool
//----------------------------------------------------------------------------------

ObjPool CreateObjPool(const size_t objsize, const size_t len)
{
    ObjPool objpool = { 0 };
    if ((len == 0) || (objsize == 0)) return objpool;
    else
    {
        const size_t aligned_size = __AlignSize(objsize, sizeof(size_t));
        uint8_t *const restrict buf = calloc(len, aligned_size);
        if (buf == NULL) return objpool;
        objpool.objSize = aligned_size;
        objpool.memSize = objpool.freeBlocks = len;
        objpool.mem = ( uintptr_t )buf;

        for (size_t i=0; i<objpool.freeBlocks; i++)
        {
            size_t *const restrict index = ( size_t* )(objpool.mem + (i*aligned_size));
            *index = i + 1;
        }

        objpool.offs = objpool.mem;
        return objpool;
    }
}

ObjPool CreateObjPoolFromBuffer(void *const restrict buf, const size_t objsize, const size_t len)
{
    ObjPool objpool = { 0 };

    // If the object size isn't large enough to align to a size_t, then we can't use it.
    const size_t aligned_size = __AlignSize(objsize, sizeof(size_t));
    if ((buf == NULL) || (len == 0) || (objsize < sizeof(size_t)) || (objsize*len != aligned_size*len)) return objpool;
    else
    {
        objpool.objSize = aligned_size;
        objpool.memSize = objpool.freeBlocks = len;
        objpool.mem = (uintptr_t)buf;

        for (size_t i=0; i<objpool.freeBlocks; i++)
        {
            size_t *const restrict index = ( size_t* )(objpool.mem + (i*aligned_size));
            *index = i + 1;
        }

        objpool.offs = objpool.mem;
        return objpool;
    }
}

void DestroyObjPool(ObjPool *const restrict objpool)
{
    if (objpool->mem == 0) return;
    else
    {
        void *const restrict ptr = ( void* )objpool->mem;
        free(ptr);
        *objpool = (ObjPool){0};
    }
}

void *ObjPoolAlloc(ObjPool *const objpool)
{
    if (objpool->freeBlocks > 0)
    {
        // For first allocation, head points to the very first index.
        // Head = &pool[0];
        // ret = Head == ret = &pool[0];
        size_t *const restrict block = ( size_t* )objpool->offs;
        objpool->freeBlocks--;

        // after allocating, we set head to the address of the index that *Head holds.
        // Head = &pool[*Head * pool.objsize];
        objpool->offs = (objpool->freeBlocks != 0)? objpool->mem + (*block*objpool->objSize) : 0;
        return memset(block, 0, objpool->objSize);
    }
    else return NULL;
}

void ObjPoolFree(ObjPool *const restrict objpool, void *const ptr)
{
    uintptr_t block = (uintptr_t)ptr;
    if ((ptr == NULL) || (block < objpool->mem) || (block > objpool->mem + objpool->memSize*objpool->objSize)) return;
    else
    {
        // When we free our pointer, we recycle the pointer space to store the previous index and then we push it as our new head.
        // *p = index of Head in relation to the buffer;
        // Head = p;
        size_t *const restrict index = ( size_t* )block;
        *index = (objpool->offs != 0)? (objpool->offs - objpool->mem)/objpool->objSize : objpool->memSize;
        objpool->offs = block;
        objpool->freeBlocks++;
    }
}

void ObjPoolCleanUp(ObjPool *const restrict objpool, void **const restrict ptrref)
{
    if (ptrref == NULL) return;
    else
    {
        ObjPoolFree(objpool, *ptrref);
        *ptrref = NULL;
    }
}


//----------------------------------------------------------------------------------
// Module Functions Definition - Double-Ended Stack
//----------------------------------------------------------------------------------
BiStack CreateBiStack(const size_t len)
{
    BiStack destack = { 0 };
    if (len == 0) return destack;

    uint8_t *const buf = malloc(len*sizeof *buf);
    if (buf==NULL) return destack;
    destack.size = len;
    destack.mem = ( uintptr_t )buf;
    destack.front = destack.mem;
    destack.back = destack.mem + len;
    return destack;
}

BiStack CreateBiStackFromBuffer(void *const buf, const size_t len)
{
    BiStack destack = { 0 };
    if (len == 0 || buf == NULL) return destack;
    else
    {
        destack.size = len;
        destack.mem = destack.front = ( uintptr_t )buf;
        destack.back = destack.mem + len;
        return destack;
    }
}

void DestroyBiStack(BiStack *const restrict destack)
{
    if (destack->mem == 0) return;
    else
    {
        uint8_t *const restrict buf = ( uint8_t* )destack->mem;
        free(buf);
        *destack = (BiStack){0};
    }
}

void *BiStackAllocFront(BiStack *const restrict destack, const size_t len)
{
    if (destack->mem == 0) return NULL;
    else
    {
        const size_t ALIGNED_LEN = __AlignSize(len, sizeof(uintptr_t));
        // front end arena is too high!
        if (destack->front + ALIGNED_LEN >= destack->back) return NULL;
        else
        {
            uint8_t *const restrict ptr = ( uint8_t* )destack->front;
            destack->front += ALIGNED_LEN;
            return ptr;
        }
    }
}

void *BiStackAllocBack(BiStack *const restrict destack, const size_t len)
{
    if (destack->mem == 0) return NULL;
    else
    {
        const size_t ALIGNED_LEN = __AlignSize(len, sizeof(uintptr_t));
        // back end arena is too low
        if (destack->back - ALIGNED_LEN <= destack->front) return NULL;
        else
        {
            destack->back -= ALIGNED_LEN;
            uint8_t *const restrict ptr = ( uint8_t* )destack->back;
            return ptr;
        }
    }
}

void BiStackResetFront(BiStack *const destack)
{
    if (destack->mem == 0) return;
    else destack->front = destack->mem;
}

void BiStackResetBack(BiStack *const destack)
{
    if (destack->mem == 0) return;
    else destack->back = destack->mem + destack->size;
}

void BiStackResetAll(BiStack *const destack)
{
    BiStackResetBack(destack);
    BiStackResetFront(destack);
}

inline intptr_t BiStackMargins(const BiStack destack)
{
    return destack.back - destack.front;
}

#endif  // RMEM_IMPLEMENTATION

/*******
 * Changelog
 * v1.0: First Creation.
 * v1.1: bug patches for the mempool and addition of object pool.
 * v1.2: addition of bidirectional arena.
 * v1.3:
    * optimizations of allocators.
    * renamed 'Stack' to 'Arena'.
    * replaced certain define constants with an anonymous enum.
    * refactored MemPool to no longer require active or deferred defragging.
 ********/
