Raylib Memory Pool
By Kevin 'Assyrianic' Yonan @ https://github.com/assyrianic

About:
	The Raylib Memory Pool is a quick, efficient, and minimal free list & stack-based allocator.

Purpose:
	Raylib Memory Pool's purpose is the following list:
		* A quicker, efficient memory allocator alternative to `malloc` and friends.
		* Reduce the possibilities of memory leaks for beginner developers using Raylib.
		* Being able to flexibly range check memory if necessary.

Data Implementation:
	the memory pool encapsulates two public structs:
	`freeList` which is an abstracted doubly linked list consisting of a `head` and `tail` pointer to `struct MemNode`, a `len` that tracks the amount of nodes the linked list holds, `maxNodes` which is used for auto-defragging (explained below), and `autoDefrag` which controls whether auto-defragging will execute or not.
	```c
	typedef struct MemNode {
		size_t size;
		struct MemNode *next, *prev;
	} MemNode;

	typedef struct AllocList {
		struct MemNode *head, *tail;
		size_t len, maxNodes;
		bool autoDefrag : 1;
	} AllocList;
	```
	
	`Stack` which is an array-based stack consisting of a byte pointer to the entire array `mem`, the base pointer `base` which changes position when allocating, deallocating, or defragging, and `size` which tracks how large the entire stack buffer is.
	```c
	typedef struct Stack {
		uint8_t *mem, *base;
		size_t size;
	} Stack;
	```
	
	```c
	typedef struct MemPool {
		struct AllocList freeList;
		struct Stack stack;
	} MemPool;
	```

Explanation & Usage:
	The memory pool is designed to be used as a direct object.
	We have two constructor functions:
	```c
	struct MemPool CreateMemPool(size_t bytes);
	struct MemPool CreateMemPoolFromBuffer(void *buf, size_t bytes);
	```
	
	To which you create a `struct MemPool` instance and give the function a max amount of memory you wish or require for your data.
	Remember not to exceed that memory amount or the allocation functions of the allocator will give you a NULL pointer.
	
	
	So we create a pool that will malloc 10K bytes.
	```c
	struct MemPool pool = CreateMemPool(10000);
	```
	
	When we finish using the pool's memory, we clean it up by using `DestroyMemPool`.
	```c
	DestroyMemPool(&pool);
	```
	
	Alternatively, if you're not in a position to use any kind of dynamic allocation from the operating system, you have the option to utilize an existing buffer as memory for the mempool:
	```c
	char mem[64000];
	struct MemPool pool = CreateMemPoolFromBuffer(mem, sizeof mem);
	```
	
	To allocate from the pool, we have two functions:
	```c
	void *MemPoolAlloc(struct MemPool *mempool, size_t bytes);
	void *MemPoolRealloc(struct MemPool *mempool, void *ptr, size_t bytes);
	```
	
	`MemPoolAlloc` returns a (zeroed) pointer to a memory block.
	```c
	// allocate an int pointer.
	int *i = MemPoolAlloc(&pool, sizeof *i);
	```
	
	`MemPoolRealloc` works similar but it takes an existing pointers and resizes its data, it does NOT zero the memory as it exists for resizing existing data. Please note that if you resize a smaller size, the data WILL BE TRUNCATED/CUT OFF.
	If the `ptr` argument for `MemPoolRealloc`, it will work just like a call to `MemPoolAlloc`.
	```c
	// allocate an int pointer.
	int *i = MemPoolRealloc(&pool, NULL, sizeof *i);
	
	// resize the pointer into an int array of 10 cells!
	i = MemPoolRealloc(&pool, i, sizeof *i * 10);
	```
	
	To deallocate memory back to the pool, there's also two functions:
	```c
	void MemPoolFree(struct MemPool *mempool, void *ptr);
	void MemPoolCleanUp(struct MemPool *mempool, void **ptrref);
	```
	
	`MemPoolFree` will deallocate the pointer data back to the memory pool.
	```c
	// i is now deallocated! Remember that i is NOT NULL!
	MemPoolFree(&pool, i);
	```
	
	`MemPoolCleanUp` instead takes a pointer to an allocated pointer and then calls `MemPoolFree` for that pointer and then sets it to NULL.
	```c
	// deallocates i and sets the pointer to NULL.
	MemPoolCleanUp(&pool, (void **)&i);
	// i is now NULL.
	```
	
	Using `MemPoolCleanUp` is basically a shorthand way of doing this code:
	```c
	// i is now deallocated! Remember that i is NOT NULL!
	MemPoolFree(&pool, i);
	
	// i is now NULL obviously.
	i = NULL;
	```
	
	
	Given that the memory pool is based on a stack allocator, that means to refill the stack, you must free in the opposite order you allocate memory.
	You can still free memory out of order but that'll create memory block fragments.
	
	Freed memory blocks that are not absorbed into the stack allocator are placed into the allocator's free list. If you have too many nodes, the allocation functions might take a while to calculate as the allocating functions first check the free list for any memory blocks it can give you before falling back to the stack allocator.
	
	If you have too many nodes (You can check by accessing the `freeList` struct and its `len` member like: `pool.freeList.len`), then you'll have to defragment the free list.
	Defragmenting consists of merging together memory blocks that are physically near one another.
	
	you have two options in terms of defragging:
	You can manually call the defrag function:
	```c
	bool MemPoolDefrag(struct MemPool *mempool);
	```
	which will return true or false depending if it was able to merge any nodes.
	
	or you can set a node limit and enable auto defragging.
	Auto defragging is disabled by default, you can turn it on or off either by calling:
	```c
	void ToggleMemPoolAutoDefrag(struct MemPool *mempool);
	```
	or set it directly from the `freeList` struct member:
	```c
	pool.freeList.autoDefrag = true; // set to on.
	pool.freeList.autoDefrag = false; // set to off.
	pool.freeList.autoDefrag ^= true; // toggle on or off.
	```
	
	For auto defragging to work, you must also set a maximum memory block node limit.
	you can set it directly with the `maxNodes` member in the `freeList` struct member:
	```c
	// set free memory block node limit to 100!
	pool.freeList.maxNodes = 100UL;
	```
	If auto defragging is not enabled, then the node limit is ignored of course.
	
	Finally, to get the total amount of memory remaining (to make sure you don't accidentally over-allocate), you utilize this function:
	```c
	size_t GetMemPoolFreeMemory(const struct MemPool mempool);
	```
	
