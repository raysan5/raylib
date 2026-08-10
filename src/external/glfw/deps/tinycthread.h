/* -*- mode: c; tab-width: 2; indent-tabs-mode: nil; -*-
Copyright (c) 2012 Marcus Geelnard

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#ifndef _TINYCTHREAD_H_
#define _TINYCTHREAD_H_

/**
* @file
* @mainpage TinyCThread API Reference
*
* @section intro_sec Introduction
* TinyCThread is a minimal, portable implementation of basic threading
* classes for C.
*
* They closely mimic the functionality and naming of the C11 standard, and
* should be easily replaceable with the corresponding standard variants.
*
* @section port_sec Portability
* The Win32 variant uses the native Win32 API for implementing the thread
* classes, while for other systems, the POSIX threads API (pthread) is used.
*
* @section misc_sec Miscellaneous
* The following special keywords are available: #_Thread_local.
*
* For more detailed information, browse the different sections of this
* documentation. A good place to start is:
* tinycthread.h.
*/

/* Which platform are we on? */
#if !defined(_TTHREAD_PLATFORM_DEFINED_)
  #if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
    #define _TTHREAD_WIN32_
  #else
    #define _TTHREAD_POSIX_
  #endif
  #define _TTHREAD_PLATFORM_DEFINED_
#endif

/* Activate some POSIX functionality (e.g. clock_gettime and recursive mutexes) */
#if defined(_TTHREAD_POSIX_)
  #undef _FEATURES_H
  #if !defined(_GNU_SOURCE)
    #define _GNU_SOURCE
  #endif
  #if !defined(_POSIX_C_SOURCE) || ((_POSIX_C_SOURCE - 0) < 199309L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L
  #endif
  #if !defined(_XOPEN_SOURCE) || ((_XOPEN_SOURCE - 0) < 500)
    #undef _XOPEN_SOURCE
    #define _XOPEN_SOURCE 500
  #endif
#endif

/* Generic includes */
#include <time.h>

/* Platform specific includes */
#if defined(_TTHREAD_POSIX_)
  #include <sys/time.h>
  #include <pthread.h>
#elif defined(_TTHREAD_WIN32_)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #define __UNDEF_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #ifdef __UNDEF_LEAN_AND_MEAN
    #undef WIN32_LEAN_AND_MEAN
    #undef __UNDEF_LEAN_AND_MEAN
  #endif
#endif

/* Workaround for missing TIME_UTC: If time.h doesn't provide TIME_UTC,
   it's quite likely that libc does not support it either. Hence, fall back to
   the only other supported time specifier: CLOCK_REALTIME (and if that fails,
   we're probably emulating clock_gettime anyway, so anything goes). */
#ifndef TIME_UTC
  #ifdef CLOCK_REALTIME
    #define TIME_UTC CLOCK_REALTIME
  #else
    #define TIME_UTC 0
  #endif
#endif

/* Workaround for missing clock_gettime (most Windows compilers, afaik) */
#if defined(_TTHREAD_WIN32_) || defined(__APPLE_CC__)
#define _TTHREAD_EMULATE_CLOCK_GETTIME_
/* Emulate struct timespec */
#if defined(_TTHREAD_WIN32_)
struct _ttherad_timespec {
  time_t tv_sec;
  long   tv_nsec;
};
#define timespec _ttherad_timespec
#endif

/* Emulate clockid_t */
typedef int _tthread_clockid_t;
#define clockid_t _tthread_clockid_t

/* Emulate clock_gettime */
int _tthread_clock_gettime(clockid_t clk_id, struct timespec *ts);
#define clock_gettime _tthread_clock_gettime
#ifndef CLOCK_REALTIME
  #define CLOCK_REALTIME 0
#endif
#endif


/** TinyCThread version (major number). */
#define TINYCTHREAD_VERSION_MAJOR 1
/** TinyCThread version (minor number). */
#define TINYCTHREAD_VERSION_MINOR 1
/** TinyCThread version (full version). */
#define TINYCTHREAD_VERSION (TINYCTHREAD_VERSION_MAJOR * 100 + TINYCTHREAD_VERSION_MINOR)

/**
* @def _Thread_local
* Thread local storage keyword.
* A variable that is declared with the @c _Thread_local keyword makes the
* value of the variable local to each thread (known as thread-local storage,
* or TLS). Example usage:
* @code
* // This variable is local to each thread.
* _Thread_local int variable;
* @endcode
* @note The @c _Thread_local keyword is a macro that maps to the corresponding
* compiler directive (e.g. @c __declspec(thread)).
* @note This directive is currently not supported on Mac OS X (it will give
* a compiler error), since compile-time TLS is not supported in the Mac OS X
* executable format. Also, some older versions of MinGW (before GCC 4.x) do
* not support this directive.
* @hideinitializer
*/

/* FIXME: Check for a PROPER value of __STDC_VERSION__ to know if we have C11 */
#if !(defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201102L)) && !defined(_Thread_local)
 #if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__SUNPRO_CC) || defined(__IBMCPP__)
  #define _Thread_local __thread
 #else
  #define _Thread_local __declspec(thread)
 #endif
#endif

/* Macros */
#define TSS_DTOR_ITERATIONS 0

/* Function return values */
#define thrd_error    0 /**< The requested operation failed */
#define thrd_success  1 /**< The requested operation succeeded */
#define thrd_timeout  2 /**< The time specified in the call was reached without acquiring the requested resource */
#define thrd_busy     3 /**< The requested operation failed because a tesource requested by a test and return function is already in use */
#define thrd_nomem    4 /**< The requested operation failed because it was unable to allocate memory */

/* Mutex types */
#define mtx_plain     1
#define mtx_timed     2
#define mtx_try       4
#define mtx_recursive 8

/* Mutex */
#if defined(_TTHREAD_WIN32_)
typedef struct {
  CRITICAL_SECTION mHandle;   /* Critical section handle */
  int mAlreadyLocked;         /* TRUE if the mutex is already locked */
  int mRecursive;             /* TRUE if the mutex is recursive */
} mtx_t;
#else
typedef pthread_mutex_t mtx_t;
#endif

/** Create a mutex object.
* @param mtx A mutex object.
* @param type Bit-mask that must have one of the following six values:
*   @li @c mtx_plain for a simple non-recursive mutex
*   @li @c mtx_timed for a non-recursive mutex that supports timeout
*   @li @c mtx_try for a non-recursive mutex that supports test and return
*   @li @c mtx_plain | @c mtx_recursive (same as @c mtx_plain, but recursive)
*   @li @c mtx_timed | @c mtx_recursive (same as @c mtx_timed, but recursive)
*   @li @c mtx_try | @c mtx_recursive (same as @c mtx_try, but recursive)
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int mtx_init(mtx_t *mtx, int type);

/** Release any resources used by the given mutex.
* @param mtx A mutex object.
*/
void mtx_destroy(mtx_t *mtx);

/** Lock the given mutex.
* Blocks until the given mutex can be locked. If the mutex is non-recursive, and
* the calling thread already has a lock on the mutex, this call will block
* forever.
* @param mtx A mutex object.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int mtx_lock(mtx_t *mtx);

/** NOT YET IMPLEMENTED.
*/
int mtx_timedlock(mtx_t *mtx, const struct timespec *ts);

/** Try to lock the given mutex.
* The specified mutex shall support either test and return or timeout. If the
* mutex is already locked, the function returns without blocking.
* @param mtx A mutex object.
* @return @ref thrd_success on success, or @ref thrd_busy if the resource
* requested is already in use, or @ref thrd_error if the request could not be
* honored.
*/
int mtx_trylock(mtx_t *mtx);

/** Unlock the given mutex.
* @param mtx A mutex object.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int mtx_unlock(mtx_t *mtx);

/* Condition variable */
#if defined(_TTHREAD_WIN32_)
typedef struct {
  HANDLE mEvents[2];                  /* Signal and broadcast event HANDLEs. */
  unsigned int mWaitersCount;         /* Count of the number of waiters. */
  CRITICAL_SECTION mWaitersCountLock; /* Serialize access to mWaitersCount. */
} cnd_t;
#else
typedef pthread_cond_t cnd_t;
#endif

/** Create a condition variable object.
* @param cond A condition variable object.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int cnd_init(cnd_t *cond);

/** Release any resources used by the given condition variable.
* @param cond A condition variable object.
*/
void cnd_destroy(cnd_t *cond);

/** Signal a condition variable.
* Unblocks one of the threads that are blocked on the given condition variable
* at the time of the call. If no threads are blocked on the condition variable
* at the time of the call, the function does nothing and return success.
* @param cond A condition variable object.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int cnd_signal(cnd_t *cond);

/** Broadcast a condition variable.
* Unblocks all of the threads that are blocked on the given condition variable
* at the time of the call. If no threads are blocked on the condition variable
* at the time of the call, the function does nothing and return success.
* @param cond A condition variable object.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int cnd_broadcast(cnd_t *cond);

/** Wait for a condition variable to become signaled.
* The function atomically unlocks the given mutex and endeavors to block until
* the given condition variable is signaled by a call to cnd_signal or to
* cnd_broadcast. When the calling thread becomes unblocked it locks the mutex
* before it returns.
* @param cond A condition variable object.
* @param mtx A mutex object.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int cnd_wait(cnd_t *cond, mtx_t *mtx);

/** Wait for a condition variable to become signaled.
* The function atomically unlocks the given mutex and endeavors to block until
* the given condition variable is signaled by a call to cnd_signal or to
* cnd_broadcast, or until after the specified time. When the calling thread
* becomes unblocked it locks the mutex before it returns.
* @param cond A condition variable object.
* @param mtx A mutex object.
* @param xt A point in time at which the request will time out (absolute time).
* @return @ref thrd_success upon success, or @ref thrd_timeout if the time
* specified in the call was reached without acquiring the requested resource, or
* @ref thrd_error if the request could not be honored.
*/
int cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts);

/* Thread */
#if defined(_TTHREAD_WIN32_)
typedef HANDLE thrd_t;
#else
typedef pthread_t thrd_t;
#endif

/** Thread start function.
* Any thread that is started with the @ref thrd_create() function must be
* started through a function of this type.
* @param arg The thread argument (the @c arg argument of the corresponding
*        @ref thrd_create() call).
* @return The thread return value, which can be obtained by another thread
* by using the @ref thrd_join() function.
*/
typedef int (*thrd_start_t)(void *arg);

/** Create a new thread.
* @param thr Identifier of the newly created thread.
* @param func A function pointer to the function that will be executed in
*        the new thread.
* @param arg An argument to the thread function.
* @return @ref thrd_success on success, or @ref thrd_nomem if no memory could
* be allocated for the thread requested, or @ref thrd_error if the request
* could not be honored.
* @note A thread’s identifier may be reused for a different thread once the
* original thread has exited and either been detached or joined to another
* thread.
*/
int thrd_create(thrd_t *thr, thrd_start_t func, void *arg);

/** Identify the calling thread.
* @return The identifier of the calling thread.
*/
thrd_t thrd_current(void);

/** NOT YET IMPLEMENTED.
*/
int thrd_detach(thrd_t thr);

/** Compare two thread identifiers.
* The function determines if two thread identifiers refer to the same thread.
* @return Zero if the two thread identifiers refer to different threads.
* Otherwise a nonzero value is returned.
*/
int thrd_equal(thrd_t thr0, thrd_t thr1);

/** Terminate execution of the calling thread.
* @param res Result code of the calling thread.
*/
void thrd_exit(int res);

/** Wait for a thread to terminate.
* The function joins the given thread with the current thread by blocking
* until the other thread has terminated.
* @param thr The thread to join with.
* @param res If this pointer is not NULL, the function will store the result
*        code of the given thread in the integer pointed to by @c res.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int thrd_join(thrd_t thr, int *res);

/** Put the calling thread to sleep.
* Suspend execution of the calling thread.
* @param time_point A point in time at which the thread will resume (absolute time).
* @param remaining If non-NULL, this parameter will hold the remaining time until
*                  time_point upon return. This will typically be zero, but if
*                  the thread was woken up by a signal that is not ignored before
*                  time_point was reached @c remaining will hold a positive
*                  time.
* @return 0 (zero) on successful sleep, or -1 if an interrupt occurred.
*/
int thrd_sleep(const struct timespec *time_point, struct timespec *remaining);

/** Yield execution to another thread.
* Permit other threads to run, even if the current thread would ordinarily
* continue to run.
*/
void thrd_yield(void);

/* Thread local storage */
#if defined(_TTHREAD_WIN32_)
typedef DWORD tss_t;
#else
typedef pthread_key_t tss_t;
#endif

/** Destructor function for a thread-specific storage.
* @param val The value of the destructed thread-specific storage.
*/
typedef void (*tss_dtor_t)(void *val);

/** Create a thread-specific storage.
* @param key The unique key identifier that will be set if the function is
*        successful.
* @param dtor Destructor function. This can be NULL.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
* @note The destructor function is not supported under Windows. If @c dtor is
* not NULL when calling this function under Windows, the function will fail
* and return @ref thrd_error.
*/
int tss_create(tss_t *key, tss_dtor_t dtor);

/** Delete a thread-specific storage.
* The function releases any resources used by the given thread-specific
* storage.
* @param key The key that shall be deleted.
*/
void tss_delete(tss_t key);

/** Get the value for a thread-specific storage.
* @param key The thread-specific storage identifier.
* @return The value for the current thread held in the given thread-specific
* storage.
*/
void *tss_get(tss_t key);

/** Set the value for a thread-specific storage.
* @param key The thread-specific storage identifier.
* @param val The value of the thread-specific storage to set for the current
*        thread.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int tss_set(tss_t key, void *val);


#endif /* _TINYTHREAD_H_ */

