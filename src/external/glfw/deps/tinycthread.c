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

/* 2013-01-06 Camilla Löwy <elmindreda@glfw.org>
 *
 * Added casts from time_t to DWORD to avoid warnings on VC++.
 * Fixed time retrieval on POSIX systems.
 */

#include "tinycthread.h"
#include <stdlib.h>

/* Platform specific includes */
#if defined(_TTHREAD_POSIX_)
  #include <signal.h>
  #include <sched.h>
  #include <unistd.h>
  #include <sys/time.h>
  #include <errno.h>
#elif defined(_TTHREAD_WIN32_)
  #include <process.h>
  #include <sys/timeb.h>
#endif

/* Standard, good-to-have defines */
#ifndef NULL
  #define NULL (void*)0
#endif
#ifndef TRUE
  #define TRUE 1
#endif
#ifndef FALSE
  #define FALSE 0
#endif

int mtx_init(mtx_t *mtx, int type)
{
#if defined(_TTHREAD_WIN32_)
  mtx->mAlreadyLocked = FALSE;
  mtx->mRecursive = type & mtx_recursive;
  InitializeCriticalSection(&mtx->mHandle);
  return thrd_success;
#else
  int ret;
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  if (type & mtx_recursive)
  {
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  }
  ret = pthread_mutex_init(mtx, &attr);
  pthread_mutexattr_destroy(&attr);
  return ret == 0 ? thrd_success : thrd_error;
#endif
}

void mtx_destroy(mtx_t *mtx)
{
#if defined(_TTHREAD_WIN32_)
  DeleteCriticalSection(&mtx->mHandle);
#else
  pthread_mutex_destroy(mtx);
#endif
}

int mtx_lock(mtx_t *mtx)
{
#if defined(_TTHREAD_WIN32_)
  EnterCriticalSection(&mtx->mHandle);
  if (!mtx->mRecursive)
  {
    while(mtx->mAlreadyLocked) Sleep(1000); /* Simulate deadlock... */
    mtx->mAlreadyLocked = TRUE;
  }
  return thrd_success;
#else
  return pthread_mutex_lock(mtx) == 0 ? thrd_success : thrd_error;
#endif
}

int mtx_timedlock(mtx_t *mtx, const struct timespec *ts)
{
  /* FIXME! */
  (void)mtx;
  (void)ts;
  return thrd_error;
}

int mtx_trylock(mtx_t *mtx)
{
#if defined(_TTHREAD_WIN32_)
  int ret = TryEnterCriticalSection(&mtx->mHandle) ? thrd_success : thrd_busy;
  if ((!mtx->mRecursive) && (ret == thrd_success) && mtx->mAlreadyLocked)
  {
    LeaveCriticalSection(&mtx->mHandle);
    ret = thrd_busy;
  }
  return ret;
#else
  return (pthread_mutex_trylock(mtx) == 0) ? thrd_success : thrd_busy;
#endif
}

int mtx_unlock(mtx_t *mtx)
{
#if defined(_TTHREAD_WIN32_)
  mtx->mAlreadyLocked = FALSE;
  LeaveCriticalSection(&mtx->mHandle);
  return thrd_success;
#else
  return pthread_mutex_unlock(mtx) == 0 ? thrd_success : thrd_error;;
#endif
}

#if defined(_TTHREAD_WIN32_)
#define _CONDITION_EVENT_ONE 0
#define _CONDITION_EVENT_ALL 1
#endif

int cnd_init(cnd_t *cond)
{
#if defined(_TTHREAD_WIN32_)
  cond->mWaitersCount = 0;

  /* Init critical section */
  InitializeCriticalSection(&cond->mWaitersCountLock);

  /* Init events */
  cond->mEvents[_CONDITION_EVENT_ONE] = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (cond->mEvents[_CONDITION_EVENT_ONE] == NULL)
  {
    cond->mEvents[_CONDITION_EVENT_ALL] = NULL;
    return thrd_error;
  }
  cond->mEvents[_CONDITION_EVENT_ALL] = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (cond->mEvents[_CONDITION_EVENT_ALL] == NULL)
  {
    CloseHandle(cond->mEvents[_CONDITION_EVENT_ONE]);
    cond->mEvents[_CONDITION_EVENT_ONE] = NULL;
    return thrd_error;
  }

  return thrd_success;
#else
  return pthread_cond_init(cond, NULL) == 0 ? thrd_success : thrd_error;
#endif
}

void cnd_destroy(cnd_t *cond)
{
#if defined(_TTHREAD_WIN32_)
  if (cond->mEvents[_CONDITION_EVENT_ONE] != NULL)
  {
    CloseHandle(cond->mEvents[_CONDITION_EVENT_ONE]);
  }
  if (cond->mEvents[_CONDITION_EVENT_ALL] != NULL)
  {
    CloseHandle(cond->mEvents[_CONDITION_EVENT_ALL]);
  }
  DeleteCriticalSection(&cond->mWaitersCountLock);
#else
  pthread_cond_destroy(cond);
#endif
}

int cnd_signal(cnd_t *cond)
{
#if defined(_TTHREAD_WIN32_)
  int haveWaiters;

  /* Are there any waiters? */
  EnterCriticalSection(&cond->mWaitersCountLock);
  haveWaiters = (cond->mWaitersCount > 0);
  LeaveCriticalSection(&cond->mWaitersCountLock);

  /* If we have any waiting threads, send them a signal */
  if(haveWaiters)
  {
    if (SetEvent(cond->mEvents[_CONDITION_EVENT_ONE]) == 0)
    {
      return thrd_error;
    }
  }

  return thrd_success;
#else
  return pthread_cond_signal(cond) == 0 ? thrd_success : thrd_error;
#endif
}

int cnd_broadcast(cnd_t *cond)
{
#if defined(_TTHREAD_WIN32_)
  int haveWaiters;

  /* Are there any waiters? */
  EnterCriticalSection(&cond->mWaitersCountLock);
  haveWaiters = (cond->mWaitersCount > 0);
  LeaveCriticalSection(&cond->mWaitersCountLock);

  /* If we have any waiting threads, send them a signal */
  if(haveWaiters)
  {
    if (SetEvent(cond->mEvents[_CONDITION_EVENT_ALL]) == 0)
    {
      return thrd_error;
    }
  }

  return thrd_success;
#else
  return pthread_cond_signal(cond) == 0 ? thrd_success : thrd_error;
#endif
}

#if defined(_TTHREAD_WIN32_)
static int _cnd_timedwait_win32(cnd_t *cond, mtx_t *mtx, DWORD timeout)
{
  int result, lastWaiter;

  /* Increment number of waiters */
  EnterCriticalSection(&cond->mWaitersCountLock);
  ++ cond->mWaitersCount;
  LeaveCriticalSection(&cond->mWaitersCountLock);

  /* Release the mutex while waiting for the condition (will decrease
     the number of waiters when done)... */
  mtx_unlock(mtx);

  /* Wait for either event to become signaled due to cnd_signal() or
     cnd_broadcast() being called */
  result = WaitForMultipleObjects(2, cond->mEvents, FALSE, timeout);
  if (result == WAIT_TIMEOUT)
  {
    return thrd_timeout;
  }
  else if (result == (int)WAIT_FAILED)
  {
    return thrd_error;
  }

  /* Check if we are the last waiter */
  EnterCriticalSection(&cond->mWaitersCountLock);
  -- cond->mWaitersCount;
  lastWaiter = (result == (WAIT_OBJECT_0 + _CONDITION_EVENT_ALL)) &&
               (cond->mWaitersCount == 0);
  LeaveCriticalSection(&cond->mWaitersCountLock);

  /* If we are the last waiter to be notified to stop waiting, reset the event */
  if (lastWaiter)
  {
    if (ResetEvent(cond->mEvents[_CONDITION_EVENT_ALL]) == 0)
    {
      return thrd_error;
    }
  }

  /* Re-acquire the mutex */
  mtx_lock(mtx);

  return thrd_success;
}
#endif

int cnd_wait(cnd_t *cond, mtx_t *mtx)
{
#if defined(_TTHREAD_WIN32_)
  return _cnd_timedwait_win32(cond, mtx, INFINITE);
#else
  return pthread_cond_wait(cond, mtx) == 0 ? thrd_success : thrd_error;
#endif
}

int cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts)
{
#if defined(_TTHREAD_WIN32_)
  struct timespec now;
  if (clock_gettime(CLOCK_REALTIME, &now) == 0)
  {
    DWORD delta = (DWORD) ((ts->tv_sec - now.tv_sec) * 1000 +
                           (ts->tv_nsec - now.tv_nsec + 500000) / 1000000);
    return _cnd_timedwait_win32(cond, mtx, delta);
  }
  else
    return thrd_error;
#else
  int ret;
  ret = pthread_cond_timedwait(cond, mtx, ts);
  if (ret == ETIMEDOUT)
  {
    return thrd_timeout;
  }
  return ret == 0 ? thrd_success : thrd_error;
#endif
}


/** Information to pass to the new thread (what to run). */
typedef struct {
  thrd_start_t mFunction; /**< Pointer to the function to be executed. */
  void * mArg;            /**< Function argument for the thread function. */
} _thread_start_info;

/* Thread wrapper function. */
#if defined(_TTHREAD_WIN32_)
static unsigned WINAPI _thrd_wrapper_function(void * aArg)
#elif defined(_TTHREAD_POSIX_)
static void * _thrd_wrapper_function(void * aArg)
#endif
{
  thrd_start_t fun;
  void *arg;
  int  res;
#if defined(_TTHREAD_POSIX_)
  void *pres;
#endif

  /* Get thread startup information */
  _thread_start_info *ti = (_thread_start_info *) aArg;
  fun = ti->mFunction;
  arg = ti->mArg;

  /* The thread is responsible for freeing the startup information */
  free((void *)ti);

  /* Call the actual client thread function */
  res = fun(arg);

#if defined(_TTHREAD_WIN32_)
  return res;
#else
  pres = malloc(sizeof(int));
  if (pres != NULL)
  {
    *(int*)pres = res;
  }
  return pres;
#endif
}

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
  /* Fill out the thread startup information (passed to the thread wrapper,
     which will eventually free it) */
  _thread_start_info* ti = (_thread_start_info*)malloc(sizeof(_thread_start_info));
  if (ti == NULL)
  {
    return thrd_nomem;
  }
  ti->mFunction = func;
  ti->mArg = arg;

  /* Create the thread */
#if defined(_TTHREAD_WIN32_)
  *thr = (HANDLE)_beginthreadex(NULL, 0, _thrd_wrapper_function, (void *)ti, 0, NULL);
#elif defined(_TTHREAD_POSIX_)
  if(pthread_create(thr, NULL, _thrd_wrapper_function, (void *)ti) != 0)
  {
    *thr = 0;
  }
#endif

  /* Did we fail to create the thread? */
  if(!*thr)
  {
    free(ti);
    return thrd_error;
  }

  return thrd_success;
}

thrd_t thrd_current(void)
{
#if defined(_TTHREAD_WIN32_)
  return GetCurrentThread();
#else
  return pthread_self();
#endif
}

int thrd_detach(thrd_t thr)
{
  /* FIXME! */
  (void)thr;
  return thrd_error;
}

int thrd_equal(thrd_t thr0, thrd_t thr1)
{
#if defined(_TTHREAD_WIN32_)
  return thr0 == thr1;
#else
  return pthread_equal(thr0, thr1);
#endif
}

void thrd_exit(int res)
{
#if defined(_TTHREAD_WIN32_)
  ExitThread(res);
#else
  void *pres = malloc(sizeof(int));
  if (pres != NULL)
  {
    *(int*)pres = res;
  }
  pthread_exit(pres);
#endif
}

int thrd_join(thrd_t thr, int *res)
{
#if defined(_TTHREAD_WIN32_)
  if (WaitForSingleObject(thr, INFINITE) == WAIT_FAILED)
  {
    return thrd_error;
  }
  if (res != NULL)
  {
    DWORD dwRes;
    GetExitCodeThread(thr, &dwRes);
    *res = dwRes;
  }
#elif defined(_TTHREAD_POSIX_)
  void *pres;
  int ires = 0;
  if (pthread_join(thr, &pres) != 0)
  {
    return thrd_error;
  }
  if (pres != NULL)
  {
    ires = *(int*)pres;
    free(pres);
  }
  if (res != NULL)
  {
    *res = ires;
  }
#endif
  return thrd_success;
}

int thrd_sleep(const struct timespec *time_point, struct timespec *remaining)
{
  struct timespec now;
#if defined(_TTHREAD_WIN32_)
  DWORD delta;
#else
  long delta;
#endif

  /* Get the current time */
  if (clock_gettime(CLOCK_REALTIME, &now) != 0)
    return -2;  // FIXME: Some specific error code?

#if defined(_TTHREAD_WIN32_)
  /* Delta in milliseconds */
  delta = (DWORD) ((time_point->tv_sec - now.tv_sec) * 1000 +
                   (time_point->tv_nsec - now.tv_nsec + 500000) / 1000000);
  if (delta > 0)
  {
    Sleep(delta);
  }
#else
  /* Delta in microseconds */
  delta = (time_point->tv_sec - now.tv_sec) * 1000000L +
          (time_point->tv_nsec - now.tv_nsec + 500L) / 1000L;

  /* On some systems, the usleep argument must be < 1000000 */
  while (delta > 999999L)
  {
    usleep(999999);
    delta -= 999999L;
  }
  if (delta > 0L)
  {
    usleep((useconds_t)delta);
  }
#endif

  /* We don't support waking up prematurely (yet) */
  if (remaining)
  {
    remaining->tv_sec = 0;
    remaining->tv_nsec = 0;
  }
  return 0;
}

void thrd_yield(void)
{
#if defined(_TTHREAD_WIN32_)
  Sleep(0);
#else
  sched_yield();
#endif
}

int tss_create(tss_t *key, tss_dtor_t dtor)
{
#if defined(_TTHREAD_WIN32_)
  /* FIXME: The destructor function is not supported yet... */
  if (dtor != NULL)
  {
    return thrd_error;
  }
  *key = TlsAlloc();
  if (*key == TLS_OUT_OF_INDEXES)
  {
    return thrd_error;
  }
#else
  if (pthread_key_create(key, dtor) != 0)
  {
    return thrd_error;
  }
#endif
  return thrd_success;
}

void tss_delete(tss_t key)
{
#if defined(_TTHREAD_WIN32_)
  TlsFree(key);
#else
  pthread_key_delete(key);
#endif
}

void *tss_get(tss_t key)
{
#if defined(_TTHREAD_WIN32_)
  return TlsGetValue(key);
#else
  return pthread_getspecific(key);
#endif
}

int tss_set(tss_t key, void *val)
{
#if defined(_TTHREAD_WIN32_)
  if (TlsSetValue(key, val) == 0)
  {
    return thrd_error;
  }
#else
  if (pthread_setspecific(key, val) != 0)
  {
    return thrd_error;
  }
#endif
  return thrd_success;
}

#if defined(_TTHREAD_EMULATE_CLOCK_GETTIME_)
int _tthread_clock_gettime(clockid_t clk_id, struct timespec *ts)
{
#if defined(_TTHREAD_WIN32_)
  struct _timeb tb;
  _ftime(&tb);
  ts->tv_sec = (time_t)tb.time;
  ts->tv_nsec = 1000000L * (long)tb.millitm;
#else
  struct timeval tv;
  gettimeofday(&tv, NULL);
  ts->tv_sec = (time_t)tv.tv_sec;
  ts->tv_nsec = 1000L * (long)tv.tv_usec;
#endif
  return 0;
}
#endif // _TTHREAD_EMULATE_CLOCK_GETTIME_

