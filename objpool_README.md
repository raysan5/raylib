Raylib Object Pool
By Kevin 'Assyrianic' Yonan @ https://github.com/assyrianic

About:
	The Raylib Object Pool is a fast and minimal fixed-size allocator.

Purpose:
	Raylib Object Pool was created as a complement to the Raylib Memory Pool.
	Due to the general purpose nature of Raylib Memory Pool, memory block fragmentations can affect allocation and deallocation speeds.
	Because of this, the Raylib Object pool succeeds by having no fragmentation and accomodating for allocating fixed-size data while the Raylib memory pool accomodates for allocating variadic/differently sized data.

Implementation:
	The object pool is implemented as a hybrid array-stack of cells that are large enough to hold the size of your data at initialization:
	```c
	typedef struct ObjPool {
		struct Stack stack;
		size_t objSize, freeBlocks;
	} ObjPool;
	```

Explanation & Usage:
	The object pool is designed to be used as a direct object.
	We have two constructor functions:
	```c
	struct ObjPool ObjPool_Create(size_t objsize, size_t len);
	struct ObjPool ObjPool_FromBuffer(void *buf, size_t objsize, size_t len);
	```
	
	To which you create a `struct ObjPool` instance and give the size of your object and how many objects for the pool to hold.
	So assume we have a vector struct like:
	```c
	typedef struct vec3D {
		float x,y,z;
	} vec3D_t;
	```
	which will have a size of 12 bytes.
	
	Now let's create a pool of 3D vectors that holds about 100 3D vectors.
	```c
	struct ObjPool vector_pool = ObjPool_Create(sizeof(struct vec3D), 100);
	```
	
	Alternatively, if for any reason that you cannot use dynamic memory allocation, you have the option of using an existing buffer for the object pool:
	```c
	struct vec3D vectors[100];
	struct ObjPool vector_pool = ObjPool_FromBuffer(vectors, sizeof(struct vec3D), 1[&vector] - 0[&vector]);
	```
	The buffer MUST be aligned to the size of `size_t` AND the object size must not be smaller than a `size_t` either.
	
	
	Next, we start our operations by allocating which will always allocate ONE object...
	If you need to allocate something like an array of these objects, then you'll have to make an object pool for the array of objects or use Raylib Memory Pool.
	Allocation is very simple nonetheless!
	```c
	struct vec3D *origin = ObjPool_Alloc(&vector_pool);
	origin->x = -0.5f;
	origin->y = +0.5f;
	origin->z = 0.f;
	```
	
	Deallocation itself is also very simple.
	There's two deallocation functions available:
	```c
	void ObjPool_Free(struct ObjPool *objpool, void *ptr);
	void ObjPool_CleanUp(struct ObjPool *objpool, void **ptrref);
	```
	
	`ObjPool_Free` will deallocate the object pointer data back to the memory pool.
	```c
	ObjPool_Free(&vector_pool, origin);
	```
	
	Like Raylib memory pool, the Raylib object pool also comes with a convenient clean up function that takes a pointer to an allocated pointer, frees it, and sets the pointer to NULL for you!
	```c
	ObjPool_CleanUp(&vector_pool, (void **)&origin);
	```
	
	Which of course is equivalent to:
	```c
	ObjPool_Free(&vector_pool, origin), origin = NULL;
	```
