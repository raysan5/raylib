#include "rmem.h"


typedef uintptr_t ptrcmp;

static size_t _AlignSize(const size_t size, const size_t align)
{
    return (size + (align-1)) & -align;
}


/************* Memory Pool *************/

static void _RemoveNode(struct MemNode **const node)
{
    ((*node)->prev != NULL)? ((*node)->prev->next = (*node)->next) : (*node = (*node)->next);
    ((*node)->next != NULL)? ((*node)->next->prev = (*node)->prev) : (*node = (*node)->prev);
}

struct MemPool MemPool_Create(const size_t size)
{
    struct MemPool mempool = {0};
    if( size==0UL )
        return mempool;
    else {
        // align the mempool size to at least the size of an alloc node.
        mempool.stack.size = _AlignSize(size, sizeof(struct MemNode));
        mempool.stack.mem = malloc(1 + mempool.stack.size*sizeof *mempool.stack.mem);
        if( mempool.stack.mem==NULL ) {
            mempool.stack.size = 0UL;
            return mempool;
        } else {
            mempool.stack.base = mempool.stack.mem + mempool.stack.size;
            return mempool;
        }
    }
}

struct MemPool MemPool_FromBuffer(const size_t size, void *buf)
{
    struct MemPool mempool = {0};
    if( size==0UL || buf==NULL || size<=sizeof(struct MemNode) )
        return mempool;
    else {
        mempool.stack.size = size;
        mempool.stack.mem = buf;
        mempool.stack.base = mempool.stack.mem + mempool.stack.size;
        return mempool;
    }
}

void MemPool_Destroy(struct MemPool *const mempool)
{
    if( mempool==NULL || mempool->stack.mem==NULL )
        return;
    else {
        free(mempool->stack.mem);
        *mempool = (struct MemPool){0};
    }
}

void *MemPool_Alloc(struct MemPool *const mempool, const size_t size)
{
    if( mempool==NULL || size==0UL || size > mempool->stack.size )
        return NULL;
    else {
        struct MemNode *new_mem = NULL;
        const size_t ALLOC_SIZE = size + sizeof *new_mem;
        if( mempool->freeList.head != NULL ) {
            const size_t MEM_SPLIT_THRESHOLD = sizeof(intptr_t);
            // if the freelist is valid, let's allocate FROM the freelist then!
            for( struct MemNode **inode = &mempool->freeList.head; *inode != NULL; inode = &(*inode)->next ) {
                if( (*inode)->size < ALLOC_SIZE )
                    continue;
                else if( (*inode)->size <= ALLOC_SIZE + MEM_SPLIT_THRESHOLD ) {
                    // close in size - reduce fragmentation by not splitting.
                    new_mem = *inode;
                    _RemoveNode(inode);
                    mempool->freeList.len--;
                    new_mem->next = new_mem->prev = NULL;
                    break;
                } else {
                    // split the memory chunk.
                    new_mem = (struct MemNode *)( (uint8_t *)*inode + ((*inode)->size - ALLOC_SIZE) );
                    (*inode)->size -= ALLOC_SIZE;
                    new_mem->size = ALLOC_SIZE;
                    new_mem->next = new_mem->prev = NULL;
                    break;
                }
            }
        }
        
        if( new_mem==NULL ) {
            // not enough memory to support the size!
            if( mempool->stack.base - ALLOC_SIZE < mempool->stack.mem )
                return NULL;
            else {
                // couldn't allocate from a freelist, allocate from available mempool.
                // subtract allocation size from the mempool.
                mempool->stack.base -= ALLOC_SIZE;
                
                // use the available mempool space as the new node.
                new_mem = (struct MemNode *)mempool->stack.base;
                new_mem->size = ALLOC_SIZE;
                new_mem->next = new_mem->prev = NULL;
            }
        }
        
        // visual of the allocation block.
        // --------------
        // | mem size   | lowest addr of block
        // | next node  |
        // --------------
        // |   alloc'd  |
        // |   memory   |
        // |   space    | highest addr of block
        // --------------
        uint8_t *const final_mem = (uint8_t *)new_mem + sizeof *new_mem;
        memset(final_mem, 0, new_mem->size - sizeof *new_mem);
        return final_mem;
    }
}

void *MemPool_Realloc(struct MemPool *const __restrict mempool, void *ptr, const size_t size)
{
    if( mempool==NULL || size > mempool->stack.size )
        return NULL;
    // NULL ptr should make this work like regular Allocation.
    else if( ptr==NULL )
        return MemPool_Alloc(mempool, size);
    else if( (ptrcmp)ptr <= (ptrcmp)mempool->stack.mem )
        return NULL;
    else {
        struct MemNode *node = (struct MemNode *)((uint8_t *)ptr - sizeof *node);
        const size_t NODE_SIZE = sizeof *node;
        uint8_t *resized_block = MemPool_Alloc(mempool, size);
        if( resized_block==NULL )
            return NULL;
        else {
            struct MemNode *resized = (struct MemNode *)(resized_block - sizeof *resized);
            memmove(resized_block, ptr, (node->size > resized->size)? (resized->size - NODE_SIZE) : (node->size - NODE_SIZE));
            MemPool_Free(mempool, ptr);
            return resized_block;
        }
    }
}

void MemPool_Free(struct MemPool *const __restrict mempool, void *ptr)
{
    if( mempool==NULL || ptr==NULL || (ptrcmp)ptr <= (ptrcmp)mempool->stack.mem )
        return;
    else {
        // behind the actual pointer data is the allocation info.
        struct MemNode *mem_node = (struct MemNode *)((uint8_t *)ptr - sizeof *mem_node);
        // make sure the pointer data is valid.
        if( (ptrcmp)mem_node < (ptrcmp)mempool->stack.base || ((ptrcmp)mem_node - (ptrcmp)mempool->stack.mem) > mempool->stack.size || mem_node->size==0UL || mem_node->size > mempool->stack.size )
            return;
        // if the mem_node is right at the stack base ptr, then add it to the stack.
        else if( (ptrcmp)mem_node == (ptrcmp)mempool->stack.base ) {
            mempool->stack.base += mem_node->size;
        }
        // otherwise, we add it to the free list.
        // We also check if the freelist already has the pointer so we can prevent double frees.
        else if( mempool->freeList.len==0UL || ((ptrcmp)mempool->freeList.head >= (ptrcmp)mempool->stack.mem && (ptrcmp)mempool->freeList.head - (ptrcmp)mempool->stack.mem < mempool->stack.size) ) {
            for( struct MemNode *n = mempool->freeList.head; n != NULL; n = n->next )
                if( n==mem_node )
                    return;
            
            // this code inserts at head.
            /*
            ( mempool->freeList.head==NULL )? (mempool->freeList.tail = mem_node) : (mempool->freeList.head->prev = mem_node);
            mem_node->next = mempool->freeList.head;
            mempool->freeList.head = mem_node;
            mempool->freeList.len++;
            */
            
            // this code insertion sorts where largest size is first.
            if( mempool->freeList.head==NULL ) {
                mempool->freeList.head = mempool->freeList.tail = mem_node;
                mempool->freeList.len++;
            } else if( mempool->freeList.head->size <= mem_node->size ) {
                mem_node->next = mempool->freeList.head;
                mem_node->next->prev = mem_node;
                mempool->freeList.head = mem_node;
                mempool->freeList.len++;
            } else if( mempool->freeList.tail->size > mem_node->size ) {
                mem_node->prev = mempool->freeList.tail;
                mempool->freeList.tail->next = mem_node;
                mempool->freeList.tail = mem_node;
                mempool->freeList.len++;
            } else {
                struct MemNode *n = mempool->freeList.head;
                while( n->next != NULL && n->next->size > mem_node->size )
                    n = n->next;
                
                mem_node->next = n->next;
                if( n->next != NULL )
                    mem_node->next->prev = mem_node;
                
                n->next = mem_node;
                mem_node->prev = n;
                mempool->freeList.len++;
            }
            
            if( mempool->freeList.autoDefrag && mempool->freeList.maxNodes != 0UL && mempool->freeList.len > mempool->freeList.maxNodes )
                MemPool_DeFrag(mempool);
        }
    }
}

void MemPool_CleanUp(struct MemPool *const __restrict mempool, void *ptrref)
{
    if( mempool==NULL || ptrref==NULL )
        return;
    else {
        void *__restrict *p = ptrref;
        if( *p==NULL ) {
            return;
        } else {
            MemPool_Free(mempool, *p), *p = NULL;
        }
    }
}

size_t MemPool_MemoryRemaining(const MemPool mempool)
{
    size_t total_remaining = (ptrcmp)mempool.stack.base - (ptrcmp)mempool.stack.mem;
    for( struct MemNode *n=mempool.freeList.head; n != NULL; n = n->next )
        total_remaining += n->size;
    return total_remaining;
}


bool MemPool_DeFrag(struct MemPool *const mempool)
{
    if( mempool==NULL )
        return false;
    else {
        // if the memory pool has been entirely released, fully defrag it.
        if( mempool->stack.size == MemPool_MemoryRemaining(*mempool) ) {
            memset(&mempool->freeList, 0, sizeof mempool->freeList);
            mempool->stack.base = mempool->stack.mem + mempool->stack.size;
            return true;
        } else {
            const size_t PRE_DEFRAG_LEN = mempool->freeList.len;
            struct MemNode **node = &mempool->freeList.head;
            while( *node != NULL ) {
                if( (ptrcmp)*node == (ptrcmp)mempool->stack.base ) {
                    // if node is right at the stack, merge it back into the stack.
                    mempool->stack.base += (*node)->size;
                    (*node)->size = 0UL;
                    _RemoveNode(node);
                    mempool->freeList.len--;
                    node = &mempool->freeList.head;
                } else if( (ptrcmp)*node + (*node)->size == (ptrcmp)(*node)->next ) {
                    // next node is at a higher address.
                    (*node)->size += (*node)->next->size;
                    (*node)->next->size = 0UL;
                    
                    // <-[P Curr N]-> <-[P Next N]-> <-[P NextNext N]->
                    // 
                    //           |--------------------|
                    // <-[P Curr N]-> <-[P Next N]-> [P NextNext N]->
                    if( (*node)->next->next != NULL )
                        (*node)->next->next->prev = *node;
                    
                    // <-[P Curr N]-> <-[P NextNext N]->
                    (*node)->next = (*node)->next->next;
                    
                    mempool->freeList.len--;
                    node = &mempool->freeList.head;
                } else if( (ptrcmp)*node + (*node)->size == (ptrcmp)(*node)->prev && (*node)->prev->prev != NULL ) {
                    // prev node is at a higher address.
                    (*node)->size += (*node)->prev->size;
                    (*node)->prev->size = 0UL;
                    
                    // <-[P PrevPrev N]-> <-[P Prev N]-> <-[P Curr N]->
                    //
                    //               |--------------------|
                    // <-[P PrevPrev N] <-[P Prev N]-> <-[P Curr N]->
                    (*node)->prev->prev->next = *node;
                    
                    // <-[P PrevPrev N]-> <-[P Curr N]->
                    (*node)->prev = (*node)->prev->prev;
                    
                    mempool->freeList.len--;
                    node = &mempool->freeList.head;
                } else if( (*node)->prev != NULL && (*node)->next != NULL && (ptrcmp)*node - (*node)->next->size == (ptrcmp)(*node)->next ) {
                    // next node is at a lower address.
                    (*node)->next->size += (*node)->size;
                    
                    (*node)->size = 0UL;
                    (*node)->next->prev = (*node)->prev;
                    (*node)->prev->next = (*node)->next;
                    
                    mempool->freeList.len--;
                    node = &mempool->freeList.head;
                } else if( (*node)->prev != NULL && (*node)->next != NULL && (ptrcmp)*node - (*node)->prev->size == (ptrcmp)(*node)->prev ) {
                    // prev node is at a lower address.
                    (*node)->prev->size += (*node)->size;
                    
                    (*node)->size = 0UL;
                    (*node)->next->prev = (*node)->prev;
                    (*node)->prev->next = (*node)->next;
                    
                    mempool->freeList.len--;
                    node = &mempool->freeList.head;
                } else {
                    node = &(*node)->next;
                }
            }
            return PRE_DEFRAG_LEN > mempool->freeList.len;
        }
    }
}


void MemPool_ToggleAutoDefrag(struct MemPool *const mempool)
{
    if( mempool==NULL )
        return;
    else mempool->freeList.autoDefrag ^= true;
}

#if 0

size_t MemPool_GetPoolSize(const MemPool mempool);
size_t MemPool_GetPoolSize(const MemPool mempool)
{
    return (mempool.stack.mem==NULL)? 0UL : mempool.stack.size;
}

void MemPool_SetMaxNodes(struct MemPool *mempool, size_t maxNodes);
void MemPool_SetMaxNodes(struct MemPool *const mempool, const size_t maxNodes)
{
    if( mempool==NULL )
        return;
    else mempool->freeList.maxNodes = maxNodes;
}

intptr_t MemPool_IsValidPtr(const MemPool mempool, void *ptr);
intptr_t MemPool_IsValidPtr(const MemPool mempool, void *ptr)
{
    if( mempool==NULL || mempool->stack.mem==NULL )
        return -1;
    else if( ptr==NULL || (ptrcmp)ptr < (ptrcmp)mempool->stack.mem || (ptrcmp)ptr > (ptrcmp)mempool->stack.mem + mempool->stack.size || (ptrcmp)ptr < (ptrcmp)mempool->stack.base )
        return false;
    else if( mempool->freeList.len != 0UL ) {
        struct MemNode *mem = (struct MemNode *)((uint8_t *)ptr - sizeof *mem);
        for( struct MemNode *n=mempool.freeList.head; n != NULL; n = n->next )
            if( mem==n )
                return false;
        return true;
    } else return true;
}

#endif
/***************************************************/


/************* Object Pool *************/
union ObjInfo {
    uint8_t *const byte;
    size_t *const size;
};

struct ObjPool ObjPool_Create(const size_t objsize, const size_t len)
{
    struct ObjPool objpool = {0};
    if( len==0UL || objsize==0UL )
        return objpool;
    else {
        objpool.objSize = _AlignSize(objsize, sizeof(size_t));
        objpool.stack.size = objpool.freeBlocks = len;
        objpool.stack.mem = calloc(objpool.stack.size, objpool.objSize);
        if( objpool.stack.mem==NULL ) {
            objpool.stack.size = 0UL;
            return objpool;
        } else {
            for( size_t i=0; i<objpool.freeBlocks; i++ ) {
                union ObjInfo block = { .byte = &objpool.stack.mem[i*objpool.objSize] };
                *block.size = i + 1;
            }
            objpool.stack.base = objpool.stack.mem;
            return objpool;
        }
    }
}

struct ObjPool ObjPool_FromBuffer(const size_t objsize, const size_t len, void *const buf)
{
    struct ObjPool objpool = {0};
    // If the object size isn't large enough to align to a size_t, then we can't use it.
    if( buf==NULL || len==0UL || objsize<sizeof(size_t) || objsize*len != _AlignSize(objsize, sizeof(size_t))*len )
        return objpool;
    else {
        objpool.objSize = _AlignSize(objsize, sizeof(size_t));
        objpool.stack.size = objpool.freeBlocks = len;
        objpool.stack.mem = buf;
        for( size_t i=0; i<objpool.freeBlocks; i++ ) {
            union ObjInfo block = { .byte = &objpool.stack.mem[i*objpool.objSize] };
            *block.size = i + 1;
        }
        objpool.stack.base = objpool.stack.mem;
        return objpool;
    }
}

void ObjPool_Destroy(struct ObjPool *const objpool)
{
    if( objpool==NULL || objpool->stack.mem==NULL )
        return;
    else {
        free(objpool->stack.mem);
        *objpool = (struct ObjPool){0};
    }
}

void *ObjPool_Alloc(struct ObjPool *const objpool)
{
    if( objpool==NULL )
        return NULL;
    else {
        if( objpool->freeBlocks>0UL ) {
            // for first allocation, head points to the very first index.
            // Head = &pool[0];
            // ret = Head == ret = &pool[0];
            union ObjInfo ret = { .byte = objpool->stack.base };
            objpool->freeBlocks--;
            
            // after allocating, we set head to the address of the index that *Head holds.
            // Head = &pool[*Head * pool.objsize];
            objpool->stack.base = (objpool->freeBlocks != 0UL)? objpool->stack.mem + ( *ret.size*objpool->objSize ) : NULL;
            memset(ret.byte, 0, objpool->objSize);
            return ret.byte;
        } else return NULL;
    }
}

void ObjPool_Free(struct ObjPool *const __restrict objpool, void *ptr)
{
    union ObjInfo p = { .byte = ptr };
    if( objpool==NULL || ptr==NULL || p.byte <= objpool->stack.mem || p.byte > objpool->stack.mem + objpool->stack.size*objpool->objSize )
        return;
    else {
        // when we free our Bointer, we recycle the pointer space to store the previous index
        // and then we push it as our new head.
        
        // *p = index of Head in relation to the buffer;
        // Head = p;
        *p.size = (objpool->stack.base != NULL)? (objpool->stack.base - objpool->stack.mem)/objpool->objSize : objpool->stack.size;
        objpool->stack.base = p.byte;
        objpool->freeBlocks++;
    }
}

void ObjPool_CleanUp(struct ObjPool *const __restrict objpool, void *ptrref)
{
    if( objpool==NULL || ptrref==NULL )
        return;
    else {
        void *__restrict *p = ptrref;
        if( *p==NULL ) {
            return;
        } else {
            ObjPool_Free(objpool, *p), *p = NULL;
        }
    }
}
/***************************************************/
