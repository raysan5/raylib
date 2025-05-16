/* MIT License

  mate.h - A single-header library for compiling your C code in C
  Version - 2025-05-16 (0.1.10):
  https://github.com/TomasBorquez/mate.h

  Guide on the `README.md`
*/
#pragma once

#ifdef MATE_IMPLEMENTATION
#  define BASE_IMPLEMENTATION
#endif

// --- BASE.H START ---
/* MIT License

  base.h - Better cross-platform STD
  Version - 2025-05-15 (0.1.18):
  https://github.com/TomasBorquez/base.h

  Usage:
    #define BASE_IMPLEMENTATION
    #include "base.h"

  More on the the `README.md`
*/

#ifdef __cplusplus
extern "C" {
#endif

/* --- Platform MACROS and includes --- */
#if defined(__clang__)
#  define COMPILER_CLANG
#elif defined(_MSC_VER)
#  define COMPILER_MSVC
#elif defined(__GNUC__)
#  define COMPILER_GCC
#elif defined(__TINYC__)
#  define COMPILER_TCC
#else
#  error "The codebase only supports GCC, Clang, TCC and MSVC"
#endif

#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#  define NORETURN __attribute__((noreturn))
#  define UNLIKELY(x) __builtin_expect(!!(x), 0)
#  define ALLOC_ATTR2(sz, al) __attribute__((malloc, alloc_size(sz), alloc_align(al)))
#  define ALLOC_ATTR(sz) __attribute__((malloc, alloc_size(sz)))
#  define FORMAT_CHECK(fmt_pos, args_pos) __attribute__((format(printf, fmt_pos, args_pos)))
#  define WARN_UNUSED __attribute__((warn_unused_result))
#elif defined(COMPILER_MSVC)
#  define NORETURN __declspec(noreturn)
#  define UNLIKELY(x) x
#  define ALLOC_ATTR2(sz, al)
#  define ALLOC_ATTR(sz)
#  define FORMAT_CHECK(fmt_pos, args_pos)
#  define WARN_UNUSED _Check_return_
#else
#  define NORETURN __declspec(noreturn)
#  define UNLIKELY(x) x
#  define ALLOC_ATTR2(sz, al)
#  define ALLOC_ATTR(sz)
#  define FORMAT_CHECK(fmt_pos, args_pos)
#  define WARN_UNUSED
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#  define PLATFORM_WIN
#elif defined(__linux__) || defined(__gnu_linux__)
#  define PLATFORM_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
#  define PLATFORM_MACOS
#else
#  error "The codebase only supports linux, macos and windows"
#endif

#if defined(COMPILER_CLANG)
#  define FILE_NAME __FILE_NAME__
#else
#  define FILE_NAME __FILE__
#endif

#if defined(PLATFORM_WIN)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  define _POSIX_C_SOURCE 200809L
#  define _GNU_SOURCE
#  include <dirent.h>
#  include <errno.h>
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <unistd.h>
#endif

#if defined(__STDC_VERSION__)
#  if (__STDC_VERSION__ >= 202311L)
#    define C_STANDARD_C23
#    define C_STANDARD "C23"
#  elif (__STDC_VERSION__ >= 201710L)
#    define C_STANDARD_C17
#    define C_STANDARD "C17"
#  elif (__STDC_VERSION__ >= 201112L)
#    define C_STANDARD_C11
#    define C_STANDARD "C11"
#  elif (__STDC_VERSION__ >= 199901L)
#    define C_STANDARD_C99
#    define C_STANDARD "C99"
#  else
#    error "Why C89 if you have C99"
#  endif
#else
#  if defined(COMPILER_MSVC)
#    if defined(_MSC_VER) && _MSC_VER >= 1920 // >= Visual Studio 2019
#      define C_STANDARD_C17
#      define C_STANDARD "C17"
#    else
#      define C_STANDARD_C11
#      define C_STANDARD "C11"
#    endif
#  endif
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* --- Platform Specific --- */
#if defined(PLATFORM_WIN)
/* Process functions */
#  define popen _popen
#  define pclose _pclose

/* File I/O functions */
#  define fileno _fileno
#  define fdopen _fdopen
#  define access _access
#  define unlink _unlink
#  define isatty _isatty
#  define dup _dup
#  define dup2 _dup2
#  define ftruncate _chsize
#  define fsync _commit

/* Directory functions */
#  define mkdir(path, mode) _mkdir(path)
#  define rmdir _rmdir
#  define getcwd _getcwd
#  define chdir _chdir

/* Process/Threading */
#  define getpid _getpid
#  define execvp _execvp
#  define execve _execve
#  define sleep(x) Sleep((x) * 1000)
#  define usleep(x) Sleep((x) / 1000)

/* String functions */
#  define strcasecmp _stricmp
#  define strncasecmp _strnicmp
#  define strdup _strdup

/* File modes */
#  define R_OK 4
#  define W_OK 2
#  define X_OK 0 /* Windows doesn't have explicit X_OK */
#  define F_OK 0

/* File descriptors */
#  define STDIN_FILENO 0
#  define STDOUT_FILENO 1
#  define STDERR_FILENO 2

/* Some functions need complete replacements */
#  if defined(COMPILER_MSVC)
#    define snprintf _snprintf
#    define vsnprintf _vsnprintf
#  endif
#endif

/* --- Types and MACRO types --- */
// Unsigned int types
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Signed int types
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

// Regular int types
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

// Floating point types
typedef float f32;
typedef double f64;

typedef struct {
  size_t length; // Does not include null terminator
  char *data;
} String;

// Maximum values for integer types
#define U8_MAX UINT8_MAX
#define U16_MAX UINT16_MAX
#define U32_MAX UINT32_MAX
#define U64_MAX UINT64_MAX

#define S8_MAX INT8_MAX
#define S8_MIN INT8_MIN
#define S16_MAX INT16_MAX
#define S16_MIN INT16_MIN
#define S32_MAX INT32_MAX
#define S32_MIN INT32_MIN
#define S64_MAX INT64_MAX
#define S64_MIN INT64_MIN

#define I8_MAX INT8_MAX
#define I8_MIN INT8_MIN
#define I16_MAX INT16_MAX
#define I16_MIN INT16_MIN
#define I32_MAX INT32_MAX
#define I32_MIN INT32_MIN
#define I64_MAX INT64_MAX
#define I64_MIN INT64_MIN

#define TYPE_INIT(type) (type)

void _custom_assert(const char *expr, const char *file, unsigned line, const char *format, ...) FORMAT_CHECK(4, 5);
#define Assert(expression, ...) (void)((!!(expression)) || (_custom_assert(#expression, __FILE__, __LINE__, __VA_ARGS__), 0))

/* --- Vector --- */
// TODO: `VecSort` implement some sorting algorithm for sorting the vector
#define VEC_TYPE(typeName, valueType) \
  typedef struct {                    \
    valueType *data;                  \
    size_t length;                    \
    size_t capacity;                  \
  } typeName

#define VecReserve(vector, count)                       \
  do {                                                  \
    vector.capacity = count;                            \
    vector.data = Malloc(count * sizeof(*vector.data)); \
  } while (0)

#define VecPush(vector, value) vecPush((void **)&(vector).data, &(vector).length, &(vector).capacity, sizeof(*vector.data), &(value));

#define VecPop(vector) vecPop((vector).data, &(vector).length, sizeof(*vector.data));

#define VecShift(vector) vecShift((void **)&(vector).data, &(vector).length, sizeof(*vector.data))

#define VecUnshift(vector, value) vecUnshift((void **)&(vector).data, &(vector).length, &(vector).capacity, sizeof(*vector.data), &(value))

#define VecInsert(vector, value, index) vecInsert((void **)&(vector).data, &(vector).length, &(vector).capacity, sizeof(*vector.data), &(value), index)

#define VecAt(vector, index) (*(__typeof__(*vector.data) *)vecAt((void **)&(vector).data, &(vector).length, index, sizeof(*vector.data)))

#define VecAtPtr(vector, index) (vecAt((void **)&(vector).data, &(vector).length, index, sizeof(*vector.data)))

#define VecFree(vector) vecFree((void **)&(vector).data, &(vector).length, &(vector).capacity)

#define VecForEach(vector, it) for (__typeof__(*vector.data) *it = vector.data; it < vector.data + vector.length; it++)

/* --- Time and Platforms --- */
i64 TimeNow();
void WaitTime(i64 ms);
String GetCompiler();
String GetPlatform();

/* --- Error --- */
typedef i32 errno_t;

enum GeneralError {
  SUCCESS = 0,
};

String ErrToStr(errno_t err);
/* --- Arena --- */
typedef struct __ArenaChunk {
  struct __ArenaChunk *next;
  size_t cap;
  char buffer[];
} __ArenaChunk;

typedef struct {
  __ArenaChunk *current;
  size_t offset;
  __ArenaChunk *root;
  size_t chunkSize;
} Arena;

// This makes sure right alignment on 86/64 bits
#define DEFAULT_ALIGNMENT (2 * sizeof(void *))

Arena *ArenaCreate(size_t chunkSize);
ALLOC_ATTR2(2, 3) void *ArenaAllocAligned(Arena *arena, size_t size, size_t align);
ALLOC_ATTR(2) char *ArenaAllocChars(Arena *arena, size_t count);
ALLOC_ATTR(2) void *ArenaAlloc(Arena *arena, size_t size);
void ArenaFree(Arena *arena);
void ArenaReset(Arena *arena);

/* --- Memory Allocations --- */
void *Realloc(void *block, size_t size);
void *Malloc(size_t size);
void Free(void *address);

/* --- String and Macros --- */
#define STRING_LENGTH(s) ((sizeof(s) / sizeof((s)[0])) - sizeof((s)[0])) // NOTE: Inspired from clay.h
#define ENSURE_STRING_LITERAL(x) ("" x "")

// NOTE: If an error led you here, it's because `S` can only be used with string literals, i.e. `S("SomeString")` and not `S(yourString)` - for that use `s()`
#define S(string) (TYPE_INIT(String){.length = STRING_LENGTH(ENSURE_STRING_LITERAL(string)), .data = (string)})
String s(char *msg);

String F(Arena *arena, const char *format, ...) FORMAT_CHECK(2, 3);

VEC_TYPE(StringVector, String);
#define StringVectorPushMany(vector, ...)           \
  do {                                              \
    char *values[] = {__VA_ARGS__};                 \
    size_t count = sizeof(values) / sizeof(char *); \
    for (size_t i = 0; i < count; i++) {            \
      String value = s(values[i]);                  \
      VecPush(vector, value);                       \
    }                                               \
  } while (0)

void SetMaxStrSize(size_t size);
String StrNew(Arena *arena, char *str);
String StrNewSize(Arena *arena, char *str, size_t len); // Without null terminator

void StrCopy(String destination, String source);
StringVector StrSplit(Arena *arena, String string, String delimiter);
StringVector StrSplitNewLine(Arena *arena, String str);
bool StrEq(String string1, String string2);
String StrConcat(Arena *arena, String string1, String string2);

void StrToLower(String string1);
void StrToUpper(String string1);

bool StrIsNull(String string);
void StrTrim(String *string);

String StrSlice(Arena *arena, String str, size_t start, size_t end);

String NormalizePath(Arena *arena, String path);
String NormalizeExePath(Arena *arena, String path);
String NormalizeExtension(Arena *arena, String path);
String NormalizeStaticLibPath(Arena *arena, String path);
String NormalizePathStart(Arena *arena, String path);
String NormalizePathEnd(Arena *arena, String path);

typedef struct {
  size_t capacity;
  String buffer;
} StringBuilder;

StringBuilder StringBuilderCreate(Arena *arena);
StringBuilder StringBuilderReserve(Arena *arena, size_t capacity);
void StringBuilderAppend(Arena *arena, StringBuilder *builder, String *string);

/* --- Random --- */
void RandomInit();
u64 RandomGetSeed();
void RandomSetSeed(u64 newSeed);
i32 RandomInteger(i32 min, i32 max);
f32 RandomFloat(f32 min, f32 max);

/* --- File System --- */
#define MAX_FILES 200

typedef struct {
  char *name;
  char *extension;
  i64 size;
  i64 createTime;
  i64 modifyTime;
} File;

char *GetCwd();
void SetCwd(char *destination);
bool Mkdir(String path); // NOTE: Mkdir if not exist
StringVector ListDir(Arena *arena, String path);

enum FileStatsError { FILE_GET_ATTRIBUTES_FAILED = 100, FILE_STATS_FILE_NOT_EXIST };
WARN_UNUSED errno_t FileStats(String path, File *file);

enum FileReadError { FILE_NOT_EXIST = 200, FILE_OPEN_FAILED, FILE_GET_SIZE_FAILED, FILE_READ_FAILED };
WARN_UNUSED errno_t FileRead(Arena *arena, String path, String *result);

enum FileWriteError { FILE_WRITE_OPEN_FAILED = 300, FILE_WRITE_ACCESS_DENIED, FILE_WRITE_NO_MEMORY, FILE_WRITE_NOT_FOUND, FILE_WRITE_DISK_FULL, FILE_WRITE_IO_ERROR };
WARN_UNUSED errno_t FileWrite(String path, String data);

// enum FileWriteError - Same error enum since it uses `FileWrite("")` under the hood
WARN_UNUSED errno_t FileReset(String path);

enum FileAddError { FILE_ADD_OPEN_FAILED = 400, FILE_ADD_ACCESS_DENIED, FILE_ADD_NO_MEMORY, FILE_ADD_NOT_FOUND, FILE_ADD_DISK_FULL, FILE_ADD_IO_ERROR };
WARN_UNUSED errno_t FileAdd(String path, String data); // NOTE: Adds `\n` at the end always

enum FileDeleteError { FILE_DELETE_ACCESS_DENIED = 500, FILE_DELETE_NOT_FOUND, FILE_DELETE_IO_ERROR };
WARN_UNUSED errno_t FileDelete(String path);

enum FileRenameError { FILE_RENAME_ACCESS_DENIED = 600, FILE_RENAME_NOT_FOUND, FILE_RENAME_IO_ERROR };
WARN_UNUSED errno_t FileRename(String oldPath, String newPath);

/* --- Logger --- */
#define _RESET "\x1b[0m"
#define _GRAY "\x1b[0;36m"
#define _RED "\x1b[0;31m"
#define _GREEN "\x1b[0;32m"
#define _ORANGE "\x1b[0;33m"

void LogInit();
void LogInfo(const char *format, ...) FORMAT_CHECK(1, 2);
void LogWarn(const char *format, ...) FORMAT_CHECK(1, 2);
void LogError(const char *format, ...) FORMAT_CHECK(1, 2);
void LogSuccess(const char *format, ...) FORMAT_CHECK(1, 2);

/* --- Math --- */
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define Clamp(a, x, b) (((x) < (a)) ? (a) : ((b) < (x)) ? (b) : (x))
#define Swap(a, b) \
  do {             \
    a ^= b;        \
    b ^= a;        \
    a ^= b;        \
  } while (0);

/* --- Defer Macros --- */
#if defined(DEFER_MACRO) // NOTE: Optional since not all compilers support it and not all C versions do either
/* - GCC implementation -
  NOTE: Must use C23 (depending on the platform)
*/
#  if defined(COMPILER_GCC)
#    define defer __DEFER(__COUNTER__)
#    define __DEFER(N) __DEFER_(N)
#    define __DEFER_(N) __DEFER__(__DEFER_FUNCTION_##N, __DEFER_VARIABLE_##N)
#    define __DEFER__(F, V)      \
      auto void F(int *);        \
      [[gnu::cleanup(F)]] int V; \
      auto void F(int *)

/* - Clang implementation -
  NOTE: Must compile with flag `-fblocks`
*/
#  elif defined(COMPILER_CLANG)
typedef void (^const __df_t)(void);

[[maybe_unused]]
static inline void __df_cb(__df_t *__fp) {
  (*__fp)();
}

#    define defer __DEFER(__COUNTER__)
#    define __DEFER(N) __DEFER_(N)
#    define __DEFER_(N) __DEFER__(__DEFER_VARIABLE_##N)
#    define __DEFER__(V) [[gnu::cleanup(__df_cb)]] __df_t V = ^void(void)

/* -- MSVC implementation --
  NOTE: Not available yet in MSVC, use `_try/_finally`
*/
#  elif defined(COMPILER_MSVC)
#    error "Not available yet in MSVC, use `_try/_finally`"
#  endif
#endif

/* --- INI Parser --- */
typedef struct {
  String key;
  String value;
} IniEntry;

VEC_TYPE(IniEntryVector, IniEntry);

typedef struct {
  IniEntryVector data;
  Arena *arena;
} IniFile;

WARN_UNUSED errno_t IniParse(String path, IniFile *result);
WARN_UNUSED errno_t IniWrite(String path, IniFile *iniFile); // NOTE: Updates/Creates .ini file

void IniFree(IniFile *iniFile);

String IniGet(IniFile *ini, String key);
String IniSet(IniFile *ini, String key, String value);
i32 IniGetInt(IniFile *ini, String key);
i64 IniGetLong(IniFile *ini, String key);
f64 IniGetDouble(IniFile *ini, String key);
bool IniGetBool(IniFile *ini, String key);

/* MIT License
   base.h - Implementation of base.h
   https://github.com/TomasBorquez/base.h
*/

#if defined(BASE_IMPLEMENTATION)
// --- Vector Implementation ---
static void vecPush(void **data, size_t *length, size_t *capacity, size_t element_size, void *value) {
  // WARNING: Vector must always be initialized to zero `Vector vector = {0}`
  Assert(*length <= *capacity, "VecPush: Possible memory corruption or vector not initialized, `Vector vector = {0}`");
  Assert(!(*length > 0 && *data == NULL), "VecPush: Possible memory corruption, data should be NULL only if length == 0");

  if (*length >= *capacity) {
    if (*capacity == 0) *capacity = 128;
    else *capacity *= 2;

    *data = Realloc(*data, *capacity * element_size);
  }

  void *address = (char *)(*data) + (*length * element_size);
  memcpy(address, value, element_size);

  (*length)++;
}

static void *vecPop(void *data, size_t *length, size_t element_size) {
  Assert(*length > 0, "VecPop: Cannot pop from empty vector");
  (*length)--;
  return (char *)data + (*length * element_size);
}

static void vecShift(void **data, size_t *length, size_t element_size) {
  Assert(*length != 0, "VecShift: Length should at least be >= 1");
  memmove(*data, (char *)(*data) + element_size, ((*length) - 1) * element_size);
  (*length)--;
}

static void vecUnshift(void **data, size_t *length, size_t *capacity, size_t element_size, const void *value) {
  if (*length >= *capacity) {
    if (*capacity == 0) *capacity = 2;
    else *capacity *= 2;
    *data = Realloc(*data, *capacity * element_size);
  }

  if (*length > 0) {
    memmove((char *)(*data) + element_size, *data, (*length) * element_size);
  }

  memcpy(*data, value, element_size);
  (*length)++;
}

static void vecInsert(void **data, size_t *length, size_t *capacity, size_t element_size, void *value, size_t index) {
  Assert(index <= *length, "VecInsert: Index out of bounds for insertion");

  if (*length >= *capacity) {
    if (*capacity == 0) *capacity = 2;
    else *capacity *= 2;
    *data = Realloc(*data, *capacity * element_size);
  }

  if (index < *length) {
    memmove((char *)(*data) + ((index + 1) * element_size), (char *)(*data) + (index * element_size), (*length - index) * element_size);
  }

  memcpy((char *)(*data) + (index * element_size), value, element_size);
  (*length)++;
}

static void *vecAt(void **data, size_t *length, size_t index, size_t elementSize) {
  Assert(index >= 0 && index < *length, "VecAt: Index out of bounds");
  void *address = (char *)(*data) + (index * elementSize);
  return address;
}

static void vecFree(void **data, size_t *length, size_t *capacity) {
  free(*data);
  *data = NULL;
  *length = 0;
  *capacity = 0;
}

// --- Time and Platforms Implementation ---
#  if !defined(PLATFORM_WIN)

#    if !defined(EINVAL)
#      define EINVAL 22 // Invalid argument
#    endif

#    if !defined(ERANGE)
#      define ERANGE 34 // Result too large
#    endif

WARN_UNUSED errno_t memcpy_s(void *dest, size_t destSize, const void *src, size_t count) {
  if (dest == NULL) {
    return EINVAL;
  }

  if (src == NULL || destSize < count) {
    memset(dest, 0, destSize);
    return EINVAL;
  }

  memcpy(dest, src, count);
  return 0;
}

WARN_UNUSED errno_t fopen_s(FILE **streamptr, const char *filename, const char *mode) {
  if (streamptr == NULL) {
    return EINVAL;
  }

  *streamptr = NULL;
  if (filename == NULL || mode == NULL) {
    return EINVAL;
  }

  if (*filename == '\0') {
    return EINVAL;
  }

  const char *valid_modes = "rwa+btx";
  size_t mode_len = strlen(mode);

  if (mode_len == 0) {
    return EINVAL;
  }

  for (size_t i = 0; i < mode_len; i++) {
    if (strchr(valid_modes, mode[i]) == NULL) {
      return EINVAL;
    }
  }

  *streamptr = fopen(filename, mode);
  if (*streamptr == NULL) {
    return errno;
  }

  return 0;
}
#  endif

String GetCompiler() {
#  if defined(COMPILER_CLANG)
  return S("clang");
#  elif defined(COMPILER_GCC)
  return S("gcc");
#  elif defined(COMPILER_TCC)
  return S("tcc");
#  elif defined(COMPILER_MSVC)
  return S("cl.exe");
#  endif
}

String GetPlatform() {
#  if defined(PLATFORM_WIN)
  return S("windows");
#  elif defined(PLATFORM_LINUX)
  return S("linux");
#  elif defined(PLATFORM_MACOS)
  return S("macos");
#  endif
}

i64 TimeNow() {
#  if defined(PLATFORM_WIN)
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  LARGE_INTEGER li;
  li.LowPart = ft.dwLowDateTime;
  li.HighPart = ft.dwHighDateTime;
  // Convert Windows FILETIME (100-nanosecond intervals since January 1, 1601)
  // to UNIX timestamp in milliseconds
  i64 currentTime = (li.QuadPart - 116444736000000000LL) / 10000;
#  else
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  i64 currentTime = (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000000LL);
#  endif
  Assert(currentTime != -1, "TimeNow: currentTime should never be -1");
  return currentTime;
}

void WaitTime(i64 ms) {
#  if defined(PLATFORM_WIN)
  sleep(ms);
#  else
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, NULL);
#  endif
}

/* --- Error Implementation --- */
String FileErrToStr(errno_t err) {
  if (err < 100) {
#  if defined(PLATFORM_WIN)
    char buffer[256];
    DWORD msgLen = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof(buffer), NULL);

    // Trim message
    if (msgLen > 0) {
      while (msgLen > 0 && (buffer[msgLen - 1] == '\r' || buffer[msgLen - 1] == '\n')) {
        buffer[--msgLen] = '\0';
      }

      if (msgLen > 0 && buffer[msgLen - 1] == '.') {
        buffer[--msgLen] = '\0';
      }

      return s(buffer);
    }
    return S("Unknown system error");
#  else
    return s(strerror(err));
#  endif
  }

  switch (err) {
  // FileStats errors (100-199)
  case FILE_GET_ATTRIBUTES_FAILED:
    return S("Failed to get file attributes");
  case FILE_STATS_FILE_NOT_EXIST:
    return S("File does not exist");

  // FileRead errors (200-299)
  case FILE_NOT_EXIST:
    return S("File does not exist");
  case FILE_OPEN_FAILED:
    return S("Failed to open file for reading");
  case FILE_GET_SIZE_FAILED:
    return S("Failed to get file size");
  case FILE_READ_FAILED:
    return S("Failed to read file");

  // FileWrite errors (300-399)
  case FILE_WRITE_OPEN_FAILED:
    return S("Failed to open file for writing");
  case FILE_WRITE_ACCESS_DENIED:
    return S("Access denied when writing file");
  case FILE_WRITE_NO_MEMORY:
    return S("Not enough memory to write file");
  case FILE_WRITE_NOT_FOUND:
    return S("File not found for writing");
  case FILE_WRITE_DISK_FULL:
    return S("Disk full when writing file");
  case FILE_WRITE_IO_ERROR:
    return S("I/O error when writing file");

  // FileAdd errors (400-499)
  case FILE_ADD_OPEN_FAILED:
    return S("Failed to open file for appending");
  case FILE_ADD_ACCESS_DENIED:
    return S("Access denied when appending to file");
  case FILE_ADD_NO_MEMORY:
    return S("Not enough memory to append to file");
  case FILE_ADD_NOT_FOUND:
    return S("File not found for appending");
  case FILE_ADD_DISK_FULL:
    return S("Disk full when appending to file");
  case FILE_ADD_IO_ERROR:
    return S("I/O error when appending to file");

  // FileDelete errors (500-599)
  case FILE_DELETE_ACCESS_DENIED:
    return S("Access denied when deleting file");
  case FILE_DELETE_NOT_FOUND:
    return S("File not found for deletion");
  case FILE_DELETE_IO_ERROR:
    return S("I/O error when deleting file");

  // FileRename errors (600-699)
  case FILE_RENAME_ACCESS_DENIED:
    return S("Access denied when renaming file");
  case FILE_RENAME_NOT_FOUND:
    return S("File not found for renaming");
  case FILE_RENAME_IO_ERROR:
    return S("I/O error when renaming file");

  default:
    return S("Unknown file error");
  }
}

void _custom_assert(const char *expr, const char *file, unsigned line, const char *format, ...) {
  printf("%sAssertion failed: %s, file %s, line %u %s\n", _RED, expr, file, line, _RESET);

  if (format) {
    va_list args;
    va_start(args, format);
    LogError(format, args);
    va_start(args, format);
  }

  abort();
}

/* --- Arena Implementation --- */
// Allocate or iterate to next chunk that can fit `bytes`
void __ArenaNextChunk(Arena *arena, size_t bytes) {
  __ArenaChunk *next = arena->current ? arena->current->next : NULL;
  while (next) {
    arena->current = next;
    if (arena->current->cap > bytes) {
      return;
    }
    next = arena->current->next;
  }
  next = (__ArenaChunk *)Malloc(sizeof(__ArenaChunk) + bytes);
  next->cap = bytes;
  next->next = NULL;
  if (arena->current) arena->current->next = next;
  arena->current = next;
}

void *ArenaAllocAligned(Arena *arena, size_t size, size_t al) {
  void *current_pos = arena->current->buffer + arena->offset;
  intptr_t mask = al - 1;
  intptr_t misalignment = ((intptr_t)current_pos & mask);
  intptr_t padding = misalignment ? al - misalignment : 0;

  arena->offset += padding;

  void *result;
  if (arena->offset + size > arena->current->cap) {
    size_t bytes = size > arena->chunkSize ? size : arena->chunkSize;
    __ArenaNextChunk(arena, bytes);

    current_pos = arena->current->buffer;
    misalignment = ((intptr_t)current_pos & mask);
    padding = misalignment ? al - misalignment : 0;
    arena->offset = padding;

    result = arena->current->buffer + arena->offset;
    arena->offset += size;
  } else {
    result = arena->current->buffer + arena->offset;
    arena->offset += size;
  }

  if (size) memset(result, 0, size);
  return result;
}

char *ArenaAllocChars(Arena *arena, size_t count) {
  return (char *)ArenaAllocAligned(arena, count, 1);
}

void *ArenaAlloc(Arena *arena, const size_t size) {
  return ArenaAllocAligned(arena, size, DEFAULT_ALIGNMENT);
}

void ArenaFree(Arena *arena) {
  __ArenaChunk *chunk = arena->root;
  while (chunk) {
    __ArenaChunk *next = chunk->next;
    free(chunk);
    chunk = next;
  }
  free(arena);
}

void ArenaReset(Arena *arena) {
  arena->current = arena->root;
  arena->offset = 0;
}

Arena *ArenaCreate(size_t chunkSize) {
  Arena *res = (Arena *)Malloc(sizeof(Arena));
  memset(res, 0, sizeof(*res));
  res->chunkSize = chunkSize;
  __ArenaNextChunk(res, chunkSize);
  res->root = res->current;
  return res;
}

/* Memory Allocations */
// TODO: Add hashmap that checks for unfreed values only on DEBUG, __FILE__ and __LINE__
void *Malloc(size_t size) {
  Assert(size > 0, "Malloc: size cant be negative");

  void *address = malloc(size);
  Assert(address != NULL, "Malloc: failed, returned address should never be NULL");
  return address;
}

void *Realloc(void *block, size_t size) {
  Assert(size > 0, "Realloc: size cant be negative");

  void *address = realloc(block, size);
  Assert(address != NULL, "Realloc: failed, returned address should never be NULL");
  return address;
}

void Free(void *address) {
  free(address);
}

/* String Implementation */
static size_t maxStringSize = 10000;

static size_t strLength(char *str, size_t maxSize) {
  if (str == NULL) {
    return 0;
  }

  size_t len = 0;
  while (len < maxSize && str[len] != '\0') {
    len++;
  }

  return len;
}

static void addNullTerminator(char *str, size_t len) {
  str[len] = '\0';
}

bool StrIsNull(String str) {
  return str.data == NULL;
}

void SetMaxStrSize(size_t size) {
  maxStringSize = size;
}

String StrNewSize(Arena *arena, char *str, size_t len) {
  Assert(str != NULL, "StrNewSize: failed, can't give a NULL str");

  const size_t memorySize = sizeof(char) * len + 1; // NOTE: Includes null terminator
  char *allocatedString = ArenaAllocChars(arena, memorySize);

  memcpy(allocatedString, str, memorySize);
  addNullTerminator(allocatedString, len);
  return (String){len, allocatedString};
}

String StrNew(Arena *arena, char *str) {
  const size_t len = strLength(str, maxStringSize);
  if (len == 0) {
    return (String){0, NULL};
  }
  const size_t memorySize = sizeof(char) * len + 1; // NOTE: Includes null terminator
  char *allocatedString = ArenaAllocChars(arena, memorySize);

  memcpy(allocatedString, str, memorySize);
  addNullTerminator(allocatedString, len);
  return (String){len, allocatedString};
}

String s(char *msg) {
  if (msg == NULL) {
    return (String){0};
  }

  return (String){
      .length = strlen(msg),
      .data = msg,
  };
}

String StrConcat(Arena *arena, String string1, String string2) {
  if (StrIsNull(string1)) {
    const size_t len = string2.length;
    const size_t memorySize = sizeof(char) * len;
    char *allocatedString = ArenaAllocChars(arena, memorySize);

    errno_t err = memcpy_s(allocatedString, memorySize, string2.data, string2.length);
    Assert(err == SUCCESS, "StrConcat: memcpy_s failed, err: %d", err);

    addNullTerminator(allocatedString, len);
    return (String){len, allocatedString};
  }

  if (StrIsNull(string2)) {
    const size_t len = string1.length;
    const size_t memorySize = sizeof(char) * len;
    char *allocatedString = ArenaAllocChars(arena, memorySize);
    errno_t err = memcpy_s(allocatedString, memorySize, string1.data, string1.length);
    Assert(err == SUCCESS, "StrConcat: memcpy_s failed, err: %d", err);

    addNullTerminator(allocatedString, len);
    return (String){len, allocatedString};
  }

  const size_t len = string1.length + string2.length;
  const size_t memorySize = sizeof(char) * len + 1; // NOTE: Includes null terminator
  char *allocatedString = ArenaAllocChars(arena, memorySize);

  errno_t err = memcpy_s(allocatedString, memorySize, string1.data, string1.length);
  Assert(err == SUCCESS, "StrConcat: memcpy_s failed, err: %d", err);

  err = memcpy_s(allocatedString + string1.length, memorySize, string2.data, string2.length);
  Assert(err == SUCCESS, "StrConcat: memcpy_s failed, err: %d", err);

  addNullTerminator(allocatedString, len);
  return (String){len, allocatedString};
}

void StrCopy(String destination, String source) {
  Assert(!StrIsNull(destination), "StrCopy: destination should never be NULL");
  Assert(!StrIsNull(source), "StrCopy: source should never be NULL");
  Assert(destination.length >= source.length, "destination length should never smaller than source length");

  errno_t err = memcpy_s(destination.data, destination.length, source.data, source.length);
  Assert(err == SUCCESS, "StrCopy: memcpy_s failed, err: %d", err);

  destination.length = source.length;
  addNullTerminator(destination.data, destination.length);
}

bool StrEq(String string1, String string2) {
  if (string1.length != string2.length) {
    return false;
  }

  if (memcmp(string1.data, string2.data, string1.length) != 0) {
    return false;
  }

  return true;
}

StringVector StrSplit(Arena *arena, String str, String delimiter) {
  Assert(!StrIsNull(str), "StrSplit: str should never be NULL");
  Assert(!StrIsNull(delimiter), "StrSplit: delimiter should never be NULL");

  char *start = str.data;
  const char *end = str.data + str.length;
  char *curr = start;
  StringVector result = {0};
  if (delimiter.length == 0) {
    for (size_t i = 0; i < str.length; i++) {
      String currString = StrNewSize(arena, str.data + i, 1);
      VecPush(result, currString);
    }
    return result;
  }

  while (curr < end) {
    char *match = NULL;
    for (char *search = curr; search <= end - delimiter.length; search++) {
      if (memcmp(search, delimiter.data, delimiter.length) == 0) {
        match = search;
        break;
      }
    }

    if (!match) {
      String currString = StrNewSize(arena, curr, end - curr);
      VecPush(result, currString);
      break;
    }

    size_t len = match - curr;
    String currString = StrNewSize(arena, curr, len);
    VecPush(result, currString);

    curr = match + delimiter.length;
  }

  return result;
}

StringVector StrSplitNewLine(Arena *arena, String str) {
  Assert(!StrIsNull(str), "SplitNewLine: str should never be NULL");
  char *start = str.data;
  const char *end = str.data + str.length;
  char *curr = start;
  StringVector result = {0};

  while (curr < end) {
    char *pos = curr;

    while (pos < end && *pos != '\n') {
      pos++;
    }

    size_t len = pos - curr;

    if (pos < end && pos > curr && *(pos - 1) == '\r') {
      len--;
    }

    String currString = StrNewSize(arena, curr, len);
    VecPush(result, currString);

    if (pos < end) {
      curr = pos + 1;
    } else {
      break;
    }
  }

  return result;
}

void StrToUpper(String str) {
  for (size_t i = 0; i < str.length; ++i) {
    char currChar = str.data[i];
    str.data[i] = toupper(currChar);
  }
}

void StrToLower(String str) {
  for (size_t i = 0; i < str.length; ++i) {
    char currChar = str.data[i];
    str.data[i] = tolower(currChar);
  }
}

bool isSpace(char character) {
  return character == ' ' || character == '\n' || character == '\t' || character == '\r';
}

void StrTrim(String *str) {
  char *firstChar = NULL;
  char *lastChar = NULL;

  if (str->length == 0) {
    return;
  }

  if (str->length == 1) {
    if (isSpace(str->data[0])) {
      str->data[0] = '\0';
      str->length = 0;
    }
    return;
  }

  for (size_t i = 0; i < str->length; ++i) {
    char *currChar = &str->data[i];
    if (isSpace(*currChar)) {
      continue;
    }

    if (firstChar == NULL) {
      firstChar = currChar;
    }
    lastChar = currChar;
  }

  if (firstChar == NULL || lastChar == NULL) {
    str->data[0] = '\0';
    str->length = 0;
    addNullTerminator(str->data, 0);
    return;
  }

  size_t len = (lastChar - firstChar) + 1;
  errno_t err = memcpy_s(str->data, str->length, firstChar, len);
  Assert(err == SUCCESS, "str->rim: memcpy_s failed, err: %d", err);

  str->length = len;
  addNullTerminator(str->data, len);
}

String StrSlice(Arena *arena, String str, size_t start, size_t end) {
  Assert(start >= 0, "StrSlice: start index must be non-negative");
  Assert(start <= str.length, "StrSlice: start index out of bounds");

  if (end < 0) {
    end = str.length + end;
  }

  Assert(end >= start, "StrSlice: end must be greater than or equal to start");
  Assert(end <= str.length, "StrSlice: end index out of bounds");

  size_t len = end - start;
  return StrNewSize(arena, str.data + start, len);
}

String F(Arena *arena, const char *format, ...) {
  va_list args;
  va_start(args, format);
  size_t size = vsnprintf(NULL, 0, format, args) + 1; // +1 for null terminator
  va_end(args);

  char *buffer = ArenaAllocChars(arena, size);
  va_start(args, format);
  vsnprintf(buffer, size, format, args);
  va_end(args);

  return (String){.length = size - 1, .data = buffer};
}

String normSlashes(String path) {
#  if defined(PLATFORM_WIN)
  for (size_t i = 0; i < path.length; i++) {
    if (path.data[i] == '/') {
      path.data[i] = '\\';
    }
  }
#  else
  for (size_t i = 0; i < path.length; i++) {
    if (path.data[i] == '\\') {
      path.data[i] = '/';
    }
  }
#  endif
  return path;
}

String NormalizePath(Arena *arena, String path) {
  String result;
  if (path.length >= 2 && path.data[0] == '.' && (path.data[1] == '/' || path.data[1] == '\\')) {
    result = StrNewSize(arena, path.data + 2, path.length - 2);
  } else {
    result = StrNewSize(arena, path.data, path.length);
  }

  return normSlashes(result);
}

String NormalizeExePath(Arena *arena, String path) {
  String platform = GetPlatform();
  String result;

  if (path.length >= 2 && path.data[0] == '.' && (path.data[1] == '/' || path.data[1] == '\\')) {
    result = StrNewSize(arena, path.data + 2, path.length - 2);
  } else {
    result = StrNewSize(arena, path.data, path.length);
  }

  bool hasExe = false;
  String exeExtension = S(".exe");
  if (result.length >= exeExtension.length) {
    String resultEnd = StrSlice(arena, result, result.length - exeExtension.length, result.length);
    if (StrEq(resultEnd, exeExtension)) {
      hasExe = true;
    }
  }

  if (StrEq(platform, S("windows"))) {
    if (!hasExe) {
      result = StrConcat(arena, result, exeExtension);
    }

    return normSlashes(result);
  }

  if (hasExe) {
    result = StrSlice(arena, result, 0, result.length - exeExtension.length);
  }

  return normSlashes(result);
}

String NormalizeExtension(Arena *arena, String path) {
  String result;

  if (path.length >= 2 && path.data[0] == '.' && (path.data[1] == '/' || path.data[1] == '\\')) {
    result = StrNewSize(arena, path.data + 2, path.length - 2);
  } else {
    result = StrNewSize(arena, path.data, path.length);
  }

  size_t filenameStart = 0;
  for (size_t i = 0; i < result.length; i++) {
    if (result.data[i] == '/' || result.data[i] == '\\') {
      filenameStart = i + 1;
    }
  }

  size_t lastDotIndex = 0;
  for (size_t i = 0; i < result.length; i++) {
    if (result.data[i] == '.') {
      lastDotIndex = i;
    }
  }

  if (lastDotIndex <= filenameStart) {
    return normSlashes(result);
  }

  result = StrSlice(arena, result, filenameStart, lastDotIndex);
  return normSlashes(result);
}

String NormalizeStaticLibPath(Arena *arena, String path) {
  String platform = GetPlatform();
  String result;

  if (path.length >= 2 && path.data[0] == '.' && (path.data[1] == '/' || path.data[1] == '\\')) {
    result = StrNewSize(arena, path.data + 2, path.length - 2);
  } else {
    result = StrNewSize(arena, path.data, path.length);
  }

  bool hasLibExt = false;
  String libExtension;
  String aExtension = S(".a");
  String libWinExtension = S(".lib");
  if (result.length >= aExtension.length) {
    String resultEnd = StrSlice(arena, result, result.length - aExtension.length, result.length);
    if (StrEq(resultEnd, aExtension)) {
      hasLibExt = true;
      libExtension = aExtension;
    }
  }

  if (!hasLibExt && result.length >= libWinExtension.length) {
    String resultEnd = StrSlice(arena, result, result.length - libWinExtension.length, result.length);
    if (StrEq(resultEnd, libWinExtension)) {
      hasLibExt = true;
      libExtension = libWinExtension;
    }
  }

  if (StrEq(platform, S("windows"))) {
    if (hasLibExt && !StrEq(libExtension, libWinExtension)) {
      result = StrSlice(arena, result, 0, result.length - libExtension.length);
      hasLibExt = false;
    }

    if (!hasLibExt) {
      result = StrConcat(arena, result, libWinExtension);
    }

    return normSlashes(result);
  }

  if (hasLibExt && !StrEq(libExtension, aExtension)) {
    result = StrSlice(arena, result, 0, result.length - libExtension.length);
    hasLibExt = false;
  }

  if (!hasLibExt) {
    result = StrConcat(arena, result, aExtension);
  }

  return normSlashes(result);
}

String NormalizePathStart(Arena *arena, String path) {
  String result;

  if (path.length >= 2 && path.data[0] == '.' && (path.data[1] == '/' || path.data[1] == '\\')) {
    result = StrNewSize(arena, path.data + 2, path.length - 2);
  } else {
    result = StrNewSize(arena, path.data, path.length);
  }

  return result;
}

String NormalizePathEnd(Arena *arena, String path) {
  size_t lastSlashIndex = 0;
  for (size_t i = 0; i < path.length; i++) {
    if (path.data[i] == '/' || path.data[i] == '\\') {
      lastSlashIndex = i + 1;
    }
  }

  return StrNewSize(arena, path.data + lastSlashIndex, path.length - lastSlashIndex);
}

StringBuilder StringBuilderCreate(Arena *arena) {
  StringBuilder result = {0};
  char *data = ArenaAllocChars(arena, 128);

  result.capacity = 128;
  result.buffer = (String){.data = data, .length = 0};
  return result;
}

StringBuilder StringBuilderReserve(Arena *arena, size_t capacity) {
  StringBuilder result = {0};
  char *data = ArenaAllocChars(arena, capacity);

  result.capacity = capacity;
  result.buffer = (String){.data = data, .length = 0};
  return result;
}

void StringBuilderAppend(Arena *arena, StringBuilder *builder, String *string) {
  size_t newLength = builder->buffer.length + string->length;
  if (newLength + 1 >= builder->capacity) {
    size_t newCapacity = (newLength + 1) * 2;
    char *data = ArenaAllocChars(arena, newCapacity);

    memcpy(data, builder->buffer.data, builder->buffer.length);
    builder->buffer.data = data;
    builder->capacity = newCapacity;
  }

  memcpy(builder->buffer.data + builder->buffer.length, string->data, string->length);
  builder->buffer.length = newLength;
  builder->buffer.data[builder->buffer.length] = '\0';
}

/* Random Implemenation */
static u64 seed = 0;

u64 RandomGetSeed() {
  return seed;
}

void RandomSetSeed(u64 newSeed) {
  seed = newSeed;
  srand(seed);
}

i32 RandomInteger(i32 min, i32 max) {
  Assert(min <= max, "RandomInteger: min should always be less than or equal to max");
  Assert(max - min <= INT32_MAX - 1, "RandomInteger: range too large");

  i32 range = max - min + 1;

  // Calculate scaling factor to avoid modulo bias
  u32 buckets = RAND_MAX / range;
  u32 limit = buckets * range;

  // Reject numbers that would create bias
  u32 r;
  do {
    r = rand();
  } while (r >= limit);

  return min + (r / buckets);
}

f32 RandomFloat(f32 min, f32 max) {
  Assert(min <= max, "RandomFloat: min must be less than or equal to max");
  f32 normalized = (f32)rand() / (f32)RAND_MAX;
  return min + normalized * (max - min);
}

/* File System Implementation */
#  if defined(PLATFORM_WIN)
char *GetCwd() {
  static char currentPath[MAX_PATH];
  DWORD length = GetCurrentDirectory(MAX_PATH, currentPath);
  if (length == 0) {
    LogError("GetCwd: failed getting current directory, err: %lu", GetLastError());
    currentPath[0] = '\0';
  }
  return currentPath;
}

void SetCwd(char *destination) {
  bool result = SetCurrentDirectory(destination);
  if (!result) {
    LogError("SetCwd: failed setting cwd for %s, err: %lu", destination, GetLastError());
  }
  GetCwd();
}

errno_t FileStats(String path, File *result) {
  if (GetFileAttributesA(path.data) == INVALID_FILE_ATTRIBUTES) {
    DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
      return FILE_STATS_FILE_NOT_EXIST;
    }
    return FILE_GET_ATTRIBUTES_FAILED;
  }

  WIN32_FILE_ATTRIBUTE_DATA fileAttr = {0};
  if (!GetFileAttributesExA(path.data, GetFileExInfoStandard, &fileAttr)) {
    return FILE_GET_ATTRIBUTES_FAILED;
  }

  char *nameStart = strrchr(path.data, '\\');
  if (!nameStart) {
    nameStart = strrchr(path.data, '/');
  }
  if (nameStart) {
    nameStart++;
  } else {
    nameStart = path.data;
  }

  result->name = strdup(nameStart);
  char *extStart = strrchr(nameStart, '.');
  if (extStart) {
    result->extension = strdup(extStart + 1);
  } else {
    result->extension = strdup("");
  }

  LARGE_INTEGER fileSize;
  fileSize.HighPart = fileAttr.nFileSizeHigh;
  fileSize.LowPart = fileAttr.nFileSizeLow;
  result->size = fileSize.QuadPart;

  LARGE_INTEGER createTime, modifyTime;
  createTime.LowPart = fileAttr.ftCreationTime.dwLowDateTime;
  createTime.HighPart = fileAttr.ftCreationTime.dwHighDateTime;
  modifyTime.LowPart = fileAttr.ftLastWriteTime.dwLowDateTime;
  modifyTime.HighPart = fileAttr.ftLastWriteTime.dwHighDateTime;

  const i64 WINDOWS_TICK = 10000000;
  const i64 SEC_TO_UNIX_EPOCH = 11644473600LL;
  result->createTime = createTime.QuadPart / WINDOWS_TICK - SEC_TO_UNIX_EPOCH;
  result->modifyTime = modifyTime.QuadPart / WINDOWS_TICK - SEC_TO_UNIX_EPOCH;

  return SUCCESS;
}

errno_t FileRead(Arena *arena, String path, String *result) {
  HANDLE hFile = INVALID_HANDLE_VALUE;

  hFile = CreateFileA(path.data, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hFile == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();

    if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
      return FILE_NOT_EXIST;
    }

    return FILE_OPEN_FAILED;
  }

  LARGE_INTEGER fileSize;
  if (!GetFileSizeEx(hFile, &fileSize)) {
    CloseHandle(hFile);
    return FILE_GET_SIZE_FAILED;
  }

  DWORD bytesRead;
  char *buffer = ArenaAllocChars(arena, fileSize.QuadPart);
  if (!ReadFile(hFile, buffer, (DWORD)fileSize.QuadPart, &bytesRead, NULL) || bytesRead != fileSize.QuadPart) {
    CloseHandle(hFile);
    return FILE_READ_FAILED;
  }

  *result = (String){.length = bytesRead, .data = buffer};

  CloseHandle(hFile);
  return SUCCESS;
}

errno_t FileWrite(String path, String data) {
  HANDLE hFile = INVALID_HANDLE_VALUE;

  hFile = CreateFileA(path.data, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hFile == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();

    switch (error) {
    case ERROR_ACCESS_DENIED:
      return FILE_WRITE_ACCESS_DENIED;
    case ERROR_FILE_NOT_FOUND:
      return FILE_WRITE_NOT_FOUND;
    default:
      return FILE_WRITE_OPEN_FAILED;
    }
  }

  DWORD bytesWritten;
  if (!WriteFile(hFile, data.data, (DWORD)data.length, &bytesWritten, NULL) || bytesWritten != data.length) {
    DWORD error = GetLastError();
    CloseHandle(hFile);

    switch (error) {
    case ERROR_DISK_FULL:
      return FILE_WRITE_DISK_FULL;
    default:
      return FILE_WRITE_IO_ERROR;
    }
  }

  CloseHandle(hFile);

  return SUCCESS;
}

errno_t FileAdd(String path, String data) {
  HANDLE hFile = INVALID_HANDLE_VALUE;
  hFile = CreateFileA(path.data, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();

    switch (error) {
    case ERROR_ACCESS_DENIED:
      return FILE_ADD_ACCESS_DENIED;
    case ERROR_FILE_NOT_FOUND:
      return FILE_ADD_NOT_FOUND;
    default:
      return FILE_ADD_OPEN_FAILED;
    }
  }

  if (SetFilePointer(hFile, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER) {
    CloseHandle(hFile);
    return FILE_ADD_IO_ERROR;
  }

  char *newData = Malloc(data.length + 1); // NOTE: +1 for \n
  memcpy(newData, data.data, data.length);
  newData[data.length] = '\n';
  DWORD newLength = (DWORD)data.length + 1;
  DWORD bytesWritten;
  if (!WriteFile(hFile, newData, newLength, &bytesWritten, NULL) || bytesWritten != newLength) {
    DWORD error = GetLastError();
    CloseHandle(hFile);
    Free(newData);
    switch (error) {
    case ERROR_DISK_FULL:
      return FILE_ADD_DISK_FULL;
    default:
      return FILE_ADD_IO_ERROR;
    }
  }

  CloseHandle(hFile);
  Free(newData);
  return SUCCESS;
}

errno_t FileDelete(String path) {
  if (!DeleteFileA(path.data)) {
    DWORD error = GetLastError();
    switch (error) {
    case ERROR_ACCESS_DENIED:
      return FILE_DELETE_ACCESS_DENIED;
    case ERROR_FILE_NOT_FOUND:
      return FILE_DELETE_NOT_FOUND;
    default:
      return FILE_DELETE_IO_ERROR;
    }
  }

  return SUCCESS;
}

errno_t FileRename(String oldPath, String newPath) {
  if (!MoveFileEx(oldPath.data, newPath.data, MOVEFILE_REPLACE_EXISTING)) {
    DWORD error = GetLastError();
    switch (error) {
    case ERROR_ACCESS_DENIED:
      return FILE_RENAME_ACCESS_DENIED;
    case ERROR_FILE_NOT_FOUND:
      return FILE_RENAME_NOT_FOUND;
    default:
      return FILE_RENAME_IO_ERROR;
    }
  }

  return SUCCESS;
}

bool Mkdir(String path) {
  bool result = CreateDirectory(path.data, NULL);
  if (result != false) {
    return true;
  }

  u64 error = GetLastError();
  if (error == ERROR_ALREADY_EXISTS) {
    return true;
  }

  LogError("Mkdir: failed for %s, err: %llu", path.data, error);
  return false;
}

StringVector ListDir(Arena *arena, String path) {
  StringVector result = {0};
  WIN32_FIND_DATA findData;
  HANDLE hFind = INVALID_HANDLE_VALUE;
  char searchPath[MAX_PATH];

  snprintf(searchPath, MAX_PATH, "%s\\*", path.data);
  hFind = FindFirstFile(searchPath, &findData);

  if (hFind == INVALID_HANDLE_VALUE) {
    DWORD error = GetLastError();
    LogError("ListDir: failed for %s, err: %lu", path.data, error);
    return result;
  }

  do {
    if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
      continue;
    }

    String entry = StrNew(arena, findData.cFileName);
    VecPush(result, entry);

  } while (FindNextFile(hFind, &findData) != 0);

  DWORD error = GetLastError();
  if (error != ERROR_NO_MORE_FILES) {
    LogError("ListDir: failed reading directory %s, err: %lu", path.data, error);
  }

  FindClose(hFind);
  return result;
}
#  else
char *GetCwd() {
  static char currentPath[PATH_MAX];
  if (getcwd(currentPath, PATH_MAX) == NULL) {
    LogError("GetCwd: failed getting current directory, err: %s", strerror(errno));
    currentPath[0] = '\0';
  }
  return currentPath;
}

void SetCwd(char *destination) {
  if (chdir(destination) != 0) {
    LogError("SetCwd: failed setting cwd for %s, err: %s", destination, strerror(errno));
  }
  GetCwd();
}

errno_t FileStats(String path, File *result) {
  struct stat fileStat;
  if (stat(path.data, &fileStat) != 0) {
    if (errno == ENOENT) {
      return FILE_STATS_FILE_NOT_EXIST;
    }
    return FILE_GET_ATTRIBUTES_FAILED;
  }

  char *nameStart = strrchr(path.data, '/');
  if (nameStart) {
    nameStart++;
  } else {
    nameStart = path.data;
  }
  result->name = strdup(nameStart);

  char *extStart = strrchr(nameStart, '.');
  if (extStart) {
    result->extension = strdup(extStart + 1);
  } else {
    result->extension = strdup("");
  }

  result->size = fileStat.st_size;
  result->createTime = fileStat.st_ctime; // Creation time (may be change time on some Unix systems)
  result->modifyTime = fileStat.st_mtime; // Modification time

  return SUCCESS;
}

errno_t FileRead(Arena *arena, String path, String *result) {
  FILE *file = fopen(path.data, "r");
  if (!file) {
    int error = errno;
    if (error == ENOENT) {
      return FILE_NOT_EXIST;
    }
    return FILE_OPEN_FAILED;
  }

  if (fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    return FILE_GET_SIZE_FAILED;
  }

  long fileSize = ftell(file);
  if (fileSize == -1) {
    fclose(file);
    return FILE_GET_SIZE_FAILED;
  }

  rewind(file);

  char *buffer = ArenaAllocChars(arena, fileSize);
  size_t bytesRead = fread(buffer, 1, fileSize, file);
  if (bytesRead != (size_t)fileSize) {
    fclose(file);
    return FILE_READ_FAILED;
  }

  *result = (String){.length = bytesRead, .data = buffer};
  fclose(file);
  return SUCCESS;
}

errno_t FileWrite(String path, String data) {
  i32 fd = -1;

  fd = open(path.data, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    i32 error = errno;

    switch (error) {
    case EACCES:
      return FILE_WRITE_ACCESS_DENIED;
    case ENOENT:
      return FILE_WRITE_NOT_FOUND;
    default:
      return FILE_WRITE_OPEN_FAILED;
    }
  }

  ssize_t bytesWritten = write(fd, data.data, data.length);
  if (bytesWritten != data.length) {
    int error = errno;
    close(fd);

    switch (error) {
    case ENOSPC:
      return FILE_WRITE_DISK_FULL;
    default:
      return FILE_WRITE_IO_ERROR;
    }
  }

  close(fd);

  return SUCCESS;
}

errno_t FileAdd(String path, String data) {
  i32 fd = -1;

  fd = open(path.data, O_WRONLY | O_APPEND | O_CREAT, 0644);
  if (fd == -1) {
    int error = errno;

    switch (error) {
    case EACCES:
      return FILE_ADD_ACCESS_DENIED;
    case ENOENT:
      return FILE_ADD_NOT_FOUND;
    default:
      return FILE_ADD_OPEN_FAILED;
    }
  }

  char *newData = Malloc(data.length + 1); // +1 for \n
  memcpy(newData, data.data, data.length);
  newData[data.length] = '\n';
  size_t newLength = data.length + 1;

  ssize_t bytesWritten = write(fd, newData, newLength);
  if (bytesWritten != newLength) {
    int error = errno;
    close(fd);
    Free(newData);

    switch (error) {
    case ENOSPC:
      return FILE_ADD_DISK_FULL;
    default:
      return FILE_ADD_IO_ERROR;
    }
  }

  close(fd);
  Free(newData);
  return SUCCESS;
}

errno_t FileDelete(String path) {
  if (unlink(path.data) != 0) {
    int error = errno;

    switch (error) {
    case EACCES:
      return FILE_DELETE_ACCESS_DENIED;
    case ENOENT:
      return FILE_DELETE_NOT_FOUND;
    default:
      return FILE_DELETE_IO_ERROR;
    }
  }

  return SUCCESS;
}

errno_t FileRename(String oldPath, String newPath) {
  if (rename(oldPath.data, newPath.data) != 0) {
    errno_t error = errno;

    switch (error) {
    case EACCES:
      return FILE_RENAME_ACCESS_DENIED;
    case ENOENT:
      return FILE_RENAME_NOT_FOUND;
    default:
      return FILE_RENAME_IO_ERROR;
    }
  }

  return SUCCESS;
}

bool Mkdir(String path) {
  struct stat st;
  if (stat(path.data, &st) == 0 && S_ISDIR(st.st_mode)) {
    return true; // Directory already exists
  }

  if (mkdir(path.data, 0755) != 0) {
    if (errno != EEXIST) {
      LogError("Mkdir: failed to create directory for %s, err: %s", path.data, strerror(errno));
      return false;
    }
  }

  return true;
}

StringVector ListDir(Arena *arena, String path) {
  StringVector result = {0};
  DIR *dir = opendir(path.data);

  if (dir == NULL) {
    errno_t error = errno;
    LogError("ListDir: failed opening directory for %s, err: %d", path.data, error);
    return result;
  }

  errno = 0;
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    String entryStr = StrNew(arena, entry->d_name);
    VecPush(result, entryStr);

    errno = 0;
  }

  if (errno != 0) {
    errno_t error = errno;
    LogError("ListDir: failed reading directory %s, err: %d", path.data, error);
  }

  closedir(dir);
  return result;
}
#  endif

errno_t FileReset(String path) {
  return FileWrite(path, S(""));
}

/* Logger Implemenation */
void LogInfo(const char *format, ...) {
  printf("%s[INFO]: ", _GRAY);
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("%s\n", _RESET);
}

void LogWarn(const char *format, ...) {
  printf("%s[WARN]: ", _ORANGE);
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("%s\n", _RESET);
}

void LogError(const char *format, ...) {
  printf("%s[ERROR]: ", _RED);
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("%s\n", _RESET);
}

void LogSuccess(const char *format, ...) {
  printf("%s[SUCCESS]: ", _GREEN);
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("%s\n", _RESET);
}

void LogInit() {
#  if defined(PLATFORM_WIN)
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dwMode = 0;
  GetConsoleMode(hOut, &dwMode);
  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hOut, dwMode);
#  endif
}

/* --- INI Parser Implementation --- */
errno_t IniParse(String path, IniFile *result) {
  File stats = {0};
  errno_t err = FileStats(path, &stats);
  if (err == FILE_STATS_FILE_NOT_EXIST) {
    LogWarn("IniParse: %s does not exist, creating...", path.data);
    errno_t fileResetErr = FileReset(path);
    Assert(fileResetErr == SUCCESS, "IniParse: Failed creating file for path %s, err: %d", path.data, fileResetErr);

    result->arena = ArenaCreate(sizeof(String) * 10); // Initialize arena
    return SUCCESS;
  }

  if (err != SUCCESS) {
    return err;
  }

  String buffer;
  result->arena = ArenaCreate(stats.size * 4);
  err = FileRead(result->arena, path, &buffer);
  if (err != SUCCESS) {
    return err;
  }

  StringVector iniSplit = StrSplitNewLine(result->arena, buffer);
  VecForEach(iniSplit, currLine) {
    if (currLine->length == 0 || currLine->data[0] == ';') {
      continue;
    }

    size_t equalPos = (size_t)-1;
    for (size_t j = 0; j < currLine->length; j++) {
      if (currLine->data[j] == '=') {
        equalPos = j;
        break;
      }
    }

    if (equalPos == (size_t)-1) {
      continue;
    }

    String key = StrSlice(result->arena, *currLine, 0, equalPos);
    String value = StrSlice(result->arena, *currLine, equalPos + 1, currLine->length);

    IniEntry entry = {.key = key, .value = value};
    VecPush(result->data, entry);
  }

  VecFree(iniSplit);
  return SUCCESS;
}

errno_t IniWrite(String path, IniFile *iniFile) {
  errno_t err = FileReset(path);
  if (err != SUCCESS) {
    return err;
  }

  VecForEach(iniFile->data, entry) {
    String value = F(iniFile->arena, "%s=%s", entry->key.data, entry->value.data);
    err = FileAdd(path, value);
    if (err != SUCCESS) {
      return err;
    }
  }

  return SUCCESS;
}

void IniFree(IniFile *iniFile) {
  ArenaFree(iniFile->arena);
  VecFree(iniFile->data);
}

String IniGet(IniFile *ini, String key) {
  VecForEach(ini->data, entry) {
    if (StrEq(entry->key, key)) {
      return entry->value;
    }
  }

  return (String){0};
}

String IniSet(IniFile *ini, String key, String value) {
  VecForEach(ini->data, entry) {
    if (StrEq(entry->key, key)) {
      entry->value = value;
      return entry->value;
    }
  }

  IniEntry newEntry;
  newEntry.key = key;
  newEntry.value = value;
  VecPush(ini->data, newEntry);

  return newEntry.value;
}

i32 IniGetInt(IniFile *ini, String key) {
  String value = IniGet(ini, key);
  if (StrIsNull(value)) {
    return 0;
  }

  char *endPtr;
  i32 result = (i32)strtol(value.data, &endPtr, 10);
  if (endPtr == value.data) {
    LogWarn("IniGetLong: Failed to convert [key: %s, value: %s] to int", key.data, value.data);
    return 0;
  }

  return result;
}

i64 IniGetLong(IniFile *ini, String key) {
  String value = IniGet(ini, key);
  if (StrIsNull(value)) {
    return 0;
  }

  char *endPtr;
  i64 result = strtoll(value.data, &endPtr, 10);
  if (endPtr == value.data) {
    LogWarn("IniGetLong: Failed to convert [key: %s, value: %s] to long", key.data, value.data);
    return 0;
  }

  return result;
}

f64 IniGetDouble(IniFile *ini, String key) {
  String value = IniGet(ini, key);
  if (StrIsNull(value)) {
    return 0.0;
  }

  char *endPtr;
  f64 result = strtod(value.data, &endPtr);
  if (endPtr == value.data) {
    LogWarn("IniGetLong: Failed to convert [key: %s, value: %s] to double", key.data, value.data);
    return 0.0;
  }

  return result;
}

bool IniGetBool(IniFile *ini, String key) {
  String value = IniGet(ini, key);
  if (StrIsNull(value)) {
    return false;
  }

  return StrEq(value, S("true"));
}
#endif

#ifdef __cplusplus
}
#endif
// --- BASE.H END ---

// --- MATE.H START ---
/* MIT License
   mate.h - Mate Definitions start here
   Guide on the `README.md`
*/
typedef struct {
  i64 lastBuild;
  bool samuraiBuild;
  bool firstBuild;
} MateCache;

typedef struct {
  char *compiler;
  char *buildDirectory;
  char *mateSource;
  char *mateExe;
} MateOptions;

typedef struct {
  String compiler;

  // Paths
  String buildDirectory;
  String mateSource;
  String mateExe;

  // Cache
  MateCache mateCache;
  IniFile cache;

  // Misc
  Arena *arena;
  bool initConfig;

  i64 startTime;
  i64 totalTime;
} MateConfig;

typedef struct {
  String output;
  String flags;
  String arFlags;
  String ninjaBuildPath;
} StaticLib;

typedef struct {
  String output;
  String flags;
  String linkerFlags;
  String ninjaBuildPath;
} Executable;

enum WarningsFlags {
  FLAG_WARNINGS_MINIMAL = 1, // -Wall
  FLAG_WARNINGS,             // -Wall -Wextra
  FLAG_WARNINGS_VERBOSE,     // -Wall -Wextra -Wpedantic
};

enum DebugFlags {
  FLAG_DEBUG_MINIMAL = 1, // -g1
  FLAG_DEBUG_MEDIUM,      // -g/g2
  FLAG_DEBUG,             // -g3
};

enum OptimizationFlags {
  FLAG_OPTIMIZATION_NONE = 1,  // -O0
  FLAG_OPTIMIZATION_BASIC,     // -O1
  FLAG_OPTIMIZATION,           // -O2
  FLAG_OPTIMIZATION_SIZE,      // -Os
  FLAG_OPTIMIZATION_AGGRESSIVE // -O3
};

enum StandardFlags {
  FLAG_STD_C99 = 1, // -std=c99
  FLAG_STD_C11,     // -std=c11
  FLAG_STD_C17,     // -std=c17
  FLAG_STD_C23,     // -std=c23
  FLAG_STD_C2X      // -std=c2x
};

typedef struct {
  char *output;
  char *flags;
  char *linkerFlags;
  char *includes;
  char *libs;
  u8 warnings;
  u8 debug;
  u8 optimization;
  u8 std;
} ExecutableOptions;

typedef struct {
  char *output;
  char *flags;
  char *arFlags;
  char *includes;
  char *libs;
  u8 warnings;
  u8 debug;
  u8 optimization;
  u8 std;
} StaticLibOptions;

void StartBuild();
void EndBuild();

void CreateConfig(MateOptions options);

String CreateExecutable(ExecutableOptions executableOptions);
String InstallExecutable();
void ResetExecutable();

String CreateStaticLib(StaticLibOptions staticLibOptions);
String InstallStaticLib();
void ResetStaticLib();

WARN_UNUSED errno_t RunCommand(String command);

enum CreateCompileCommandsError { COMPILE_COMMANDS_FAILED_OPEN_FILE = 1000, COMPILE_COMMANDS_FAILED_COMPDB };
WARN_UNUSED errno_t CreateCompileCommands(String ninjaBuildPath);

#define AddLibraryPaths(...)                   \
  do {                                         \
    StringVector vector = {0};                 \
    StringVectorPushMany(vector, __VA_ARGS__); \
    addLibraryPaths(&vector);                  \
  } while (0)
static void addLibraryPaths(StringVector *vector);

#define AddIncludePaths(...)                   \
  do {                                         \
    StringVector vector = {0};                 \
    StringVectorPushMany(vector, __VA_ARGS__); \
    addIncludePaths(&vector);                  \
  } while (0)
static void addIncludePaths(StringVector *vector);

#define LinkSystemLibraries(...)               \
  do {                                         \
    StringVector vector = {0};                 \
    StringVectorPushMany(vector, __VA_ARGS__); \
    linkSystemLibraries(&vector);              \
  } while (0)
static void linkSystemLibraries(StringVector *vector);

#define AddFile(source) addFile(S(source));
static void addFile(String source);

#define RemoveFile(source) removeFile(S(source));
static bool removeFile(String source);

static void reBuild();
static bool needRebuild();
static void setDefaultState();

// Utility platform/compiler functions
bool isWindows();
bool isLinux();

bool isMSVC();
bool isGCC();
bool isClang();
bool isTCC();

// --- SAMURAI START ---
/*
* This code comes from Samurai (https://github.com/michaelforney/samurai)
* Copyright © 2017-2021 Michael Forney
* Licensed under ISC license, with portions under Apache License 2.0 and MIT licenses.
* See LICENSE-SAMURAI.txt for the full license text.
*/
#define SAMURAI_AMALGAM "#define _POSIX_C_SOURCE 200809L\n"  \
            "#include <errno.h>\n"\
            "#include <fcntl.h>\n"\
            "#include <inttypes.h>\n"\
            "#include <stdbool.h>\n"\
            "#include <stdint.h>\n"\
            "#include <ctype.h>\n"\
            "#include <stdio.h>\n"\
            "#include <stdlib.h>\n"\
            "#include <assert.h>\n"\
            "#include <stdarg.h>\n"\
            "#include <string.h>\n"\
            "#include <sys/stat.h>\n"\
            "#include <poll.h>\n"\
            "#include <signal.h>\n"\
            "#include <spawn.h>\n"\
            "#include <sys/wait.h>\n"\
            "#include <time.h>\n"\
            "#include <unistd.h>\n"\
            "struct node;\n"\
            "struct buildoptions {\n"\
            "  size_t maxjobs, maxfail;\n"\
            "  _Bool verbose, explain, keepdepfile, keeprsp, dryrun;\n"\
            "  const char *statusfmt;\n"\
            "  double maxload;\n"\
            "};\n"\
            "extern struct buildoptions buildopts;\n"\
            "void buildreset(void);\n"\
            "void buildadd(struct node *);\n"\
            "void build(void);\n"\
            "struct edge;\n"\
            "void depsinit(const char *);\n"\
            "void depsclose(void);\n"\
            "void depsload(struct edge *);\n"\
            "void depsrecord(struct edge *);\n"\
            "struct evalstring;\n"\
            "struct string;\n"\
            "struct rule {\n"\
            "  char *name;\n"\
            "  struct treenode *bindings;\n"\
            "};\n"\
            "struct pool {\n"\
            "  char *name;\n"\
            "  int numjobs, maxjobs;\n"\
            "  struct edge *work;\n"\
            "};\n"\
            "void envinit(void);\n"\
            "struct environment *mkenv(struct environment *);\n"\
            "struct string *envvar(struct environment *, char *);\n"\
            "void envaddvar(struct environment *, char *, struct string *);\n"\
            "struct string *enveval(struct environment *, struct evalstring *);\n"\
            "struct rule *envrule(struct environment *, char *);\n"\
            "void envaddrule(struct environment *, struct rule *);\n"\
            "struct rule *mkrule(char *);\n"\
            "void ruleaddvar(struct rule *, char *, struct evalstring *);\n"\
            "struct pool *mkpool(char *);\n"\
            "struct pool *poolget(char *);\n"\
            "struct string *edgevar(struct edge *, char *, _Bool);\n"\
            "extern struct environment *rootenv;\n"\
            "extern struct rule phonyrule;\n"\
            "extern struct pool consolepool;\n"\
            "enum {\n"\
            "  MTIME_UNKNOWN = -1,\n"\
            "  MTIME_MISSING = -2,\n"\
            "};\n"\
            "struct node {\n"\
            "  struct string *path, *shellpath;\n"\
            "  int64_t mtime, logmtime;\n"\
            "  struct edge *gen, **use;\n"\
            "  size_t nuse;\n"\
            "  uint64_t hash;\n"\
            "  int32_t id;\n"\
            "  _Bool dirty;\n"\
            "};\n"\
            "struct edge {\n"\
            "  struct rule *rule;\n"\
            "  struct pool *pool;\n"\
            "  struct environment *env;\n"\
            "  struct node **out, **in;\n"\
            "  size_t nout, nin;\n"\
            "  size_t outimpidx;\n"\
            "  size_t inimpidx, inorderidx;\n"\
            "  uint64_t hash;\n"\
            "  size_t nblock;\n"\
            "  size_t nprune;\n"\
            "  enum {\n"\
            "    FLAG_WORK = 1 << 0,\n"\
            "    FLAG_HASH = 1 << 1,\n"\
            "    FLAG_DIRTY_IN = 1 << 3,\n"\
            "    FLAG_DIRTY_OUT = 1 << 4,\n"\
            "    FLAG_DIRTY = FLAG_DIRTY_IN | FLAG_DIRTY_OUT,\n"\
            "    FLAG_CYCLE = 1 << 5,\n"\
            "    FLAG_DEPS = 1 << 6,\n"\
            "  } flags;\n"\
            "  struct edge *worknext;\n"\
            "  struct edge *allnext;\n"\
            "};\n"\
            "void graphinit(void);\n"\
            "struct node *mknode(struct string *);\n"\
            "struct node *nodeget(const char *, size_t);\n"\
            "void nodestat(struct node *);\n"\
            "struct string *nodepath(struct node *, _Bool);\n"\
            "void nodeuse(struct node *, struct edge *);\n"\
            "struct edge *mkedge(struct environment *parent);\n"\
            "void edgehash(struct edge *);\n"\
            "void edgeadddeps(struct edge *e, struct node **deps, size_t ndeps);\n"\
            "extern struct edge *alledges;\n"\
            "struct hashtablekey {\n"\
            "  uint64_t hash;\n"\
            "  const char *str;\n"\
            "  size_t len;\n"\
            "};\n"\
            "void htabkey(struct hashtablekey *, const char *, size_t);\n"\
            "struct hashtable *mkhtab(size_t);\n"\
            "void delhtab(struct hashtable *, void(void *));\n"\
            "void **htabput(struct hashtable *, struct hashtablekey *);\n"\
            "void *htabget(struct hashtable *, struct hashtablekey *);\n"\
            "uint64_t murmurhash64a(const void *, size_t);\n"\
            "struct node;\n"\
            "void loginit(const char *);\n"\
            "void logclose(void);\n"\
            "void logrecord(struct node *);\n"\
            "struct environment;\n"\
            "struct node;\n"\
            "struct parseoptions {\n"\
            "  _Bool dupbuildwarn;\n"\
            "};\n"\
            "void parseinit(void);\n"\
            "void parse(const char *, struct environment *);\n"\
            "extern struct parseoptions parseopts;\n"\
            "enum {\n"\
            "  ninjamajor = 1,\n"\
            "  ninjaminor = 9,\n"\
            "};\n"\
            "void defaultnodes(void(struct node *));\n"\
            "extern const char *argv0;\n"\
            "#define ARGBEGIN                                                         \\\n"\
            "  for (;;) {                                                             \\\n"\
            "    if (argc > 0) ++argv, --argc;                                        \\\n"\
            "    if (argc == 0 || (*argv)[0] != '-') break;                           \\\n"\
            "    if ((*argv)[1] == '-' && !(*argv)[2]) {                              \\\n"\
            "      ++argv, --argc;                                                    \\\n"\
            "      break;                                                             \\\n"\
            "    }                                                                    \\\n"\
            "    for (char *opt_ = &(*argv)[1], done_ = 0; !done_ && *opt_; ++opt_) { \\\n"\
            "      switch (*opt_)\n"\
            "#define ARGEND \\\n"\
            "  }            \\\n"\
            "  }\n"\
            "#define EARGF(x) (done_ = 1, *++opt_ ? opt_ : argv[1] ? --argc, *++argv : ((x), abort(), (char *)0))\n"\
            "enum token {\n"\
            "  BUILD,\n"\
            "  DEFAULT,\n"\
            "  INCLUDE,\n"\
            "  POOL,\n"\
            "  RULE,\n"\
            "  SUBNINJA,\n"\
            "  VARIABLE,\n"\
            "};\n"\
            "struct scanner {\n"\
            "  FILE *f;\n"\
            "  const char *path;\n"\
            "  int chr, line, col;\n"\
            "};\n"\
            "extern struct evalstring **paths;\n"\
            "extern size_t npaths;\n"\
            "void scaninit(struct scanner *, const char *);\n"\
            "void scanclose(struct scanner *);\n"\
            "void scanerror(struct scanner *, const char *, ...);\n"\
            "int scankeyword(struct scanner *, char **);\n"\
            "char *scanname(struct scanner *);\n"\
            "struct evalstring *scanstring(struct scanner *, _Bool);\n"\
            "void scanpaths(struct scanner *);\n"\
            "void scanchar(struct scanner *, int);\n"\
            "int scanpipe(struct scanner *, int);\n"\
            "_Bool scanindent(struct scanner *);\n"\
            "void scannewline(struct scanner *);\n"\
            "struct tool {\n"\
            "  const char *name;\n"\
            "  int (*run)(int, char *[]);\n"\
            "};\n"\
            "const struct tool *toolget(const char *);\n"\
            "struct treenode {\n"\
            "  char *key;\n"\
            "  void *value;\n"\
            "  struct treenode *child[2];\n"\
            "  int height;\n"\
            "};\n"\
            "void deltree(struct treenode *, void(void *), void(void *));\n"\
            "struct treenode *treefind(struct treenode *, const char *);\n"\
            "void *treeinsert(struct treenode **, char *, void *);\n"\
            "struct buffer {\n"\
            "  char *data;\n"\
            "  size_t len, cap;\n"\
            "};\n"\
            "struct string {\n"\
            "  size_t n;\n"\
            "  char s[];\n"\
            "};\n"\
            "struct evalstring {\n"\
            "  char *var;\n"\
            "  struct string *str;\n"\
            "  struct evalstring *next;\n"\
            "};\n"\
            "#define LEN(a) (sizeof(a) / sizeof((a)[0]))\n"\
            "void warn(const char *, ...);\n"\
            "void fatal(const char *, ...);\n"\
            "void *xmalloc(size_t);\n"\
            "void *xreallocarray(void *, size_t, size_t);\n"\
            "char *xmemdup(const char *, size_t);\n"\
            "int xasprintf(char **, const char *, ...);\n"\
            "void bufadd(struct buffer *buf, char c);\n"\
            "struct string *mkstr(size_t n);\n"\
            "void delevalstr(void *);\n"\
            "void canonpath(struct string *);\n"\
            "int writefile(const char *, struct string *);\n"\
            "struct string;\n"\
            "void osgetcwd(char *, size_t);\n"\
            "void oschdir(const char *);\n"\
            "int osmkdirs(struct string *, _Bool);\n"\
            "int64_t osmtime(const char *);\n"\
            "struct job {\n"\
            "  struct string *cmd;\n"\
            "  struct edge *edge;\n"\
            "  struct buffer buf;\n"\
            "  size_t next;\n"\
            "  pid_t pid;\n"\
            "  int fd;\n"\
            "  bool failed;\n"\
            "};\n"\
            "struct buildoptions buildopts = {.maxfail = 1};\n"\
            "static struct edge *work;\n"\
            "static size_t nstarted, nfinished, ntotal;\n"\
            "static bool consoleused;\n"\
            "static struct timespec starttime;\n"\
            "void buildreset(void) {\n"\
            "  struct edge *e;\n"\
            "  for (e = alledges; e; e = e->allnext)\n"\
            "    e->flags &= ~FLAG_WORK;\n"\
            "}\n"\
            "static bool isnewer(struct node *n1, struct node *n2) {\n"\
            "  return n1 && n1->mtime > n2->mtime;\n"\
            "}\n"\
            "static bool isdirty(struct node *n, struct node *newest, bool generator, bool restat) {\n"\
            "  struct edge *e;\n"\
            "  e = n->gen;\n"\
            "  if (e->rule == &phonyrule) {\n"\
            "    if (e->nin > 0 || n->mtime != MTIME_MISSING) return false;\n"\
            "    if (buildopts.explain) warn(\"explain %s: phony and no inputs\", n->path->s);\n"\
            "    return true;\n"\
            "  }\n"\
            "  if (n->mtime == MTIME_MISSING) {\n"\
            "    if (buildopts.explain) warn(\"explain %s: missing\", n->path->s);\n"\
            "    return true;\n"\
            "  }\n"\
            "  if (isnewer(newest, n) && (!restat || n->logmtime == MTIME_MISSING)) {\n"\
            "    if (buildopts.explain) {\n"\
            "      warn(\"explain %s: older than input '%s': %\" PRId64 \" vs %\" PRId64, n->path->s, newest->path->s, n->mtime, newest->mtime);\n"\
            "    }\n"\
            "    return true;\n"\
            "  }\n"\
            "  if (n->logmtime == MTIME_MISSING) {\n"\
            "    if (!generator) {\n"\
            "      if (buildopts.explain) warn(\"explain %s: no record in .ninja_log\", n->path->s);\n"\
            "      return true;\n"\
            "    }\n"\
            "  } else if (newest && n->logmtime < newest->mtime) {\n"\
            "    if (buildopts.explain) {\n"\
            "      warn(\"explain %s: recorded mtime is older than input '%s': %\" PRId64 \" vs %\" PRId64, n->path->s, newest->path->s, n->logmtime, newest->mtime);\n"\
            "    }\n"\
            "    return true;\n"\
            "  }\n"\
            "  if (generator) return false;\n"\
            "  edgehash(e);\n"\
            "  if (e->hash == n->hash) return false;\n"\
            "  if (buildopts.explain) warn(\"explain %s: command line changed\", n->path->s);\n"\
            "  return true;\n"\
            "}\n"\
            "static void queue(struct edge *e) {\n"\
            "  struct edge **front = &work;\n"\
            "  if (e->pool && e->rule != &phonyrule) {\n"\
            "    if (e->pool->numjobs == e->pool->maxjobs) front = &e->pool->work;\n"\
            "    else ++e->pool->numjobs;\n"\
            "  }\n"\
            "  e->worknext = *front;\n"\
            "  *front = e;\n"\
            "}\n"\
            "void buildadd(struct node *n) {\n"\
            "  struct edge *e;\n"\
            "  struct node *newest;\n"\
            "  size_t i;\n"\
            "  bool generator, restat;\n"\
            "  e = n->gen;\n"\
            "  if (!e) {\n"\
            "    if (n->mtime == MTIME_UNKNOWN) nodestat(n);\n"\
            "    if (n->mtime == MTIME_MISSING) fatal(\"file is missing and not created by any action: '%s'\", n->path->s);\n"\
            "    n->dirty = false;\n"\
            "    return;\n"\
            "  }\n"\
            "  if (e->flags & FLAG_CYCLE) fatal(\"dependency cycle involving '%s'\", n->path->s);\n"\
            "  if (e->flags & FLAG_WORK) return;\n"\
            "  e->flags |= FLAG_CYCLE | FLAG_WORK;\n"\
            "  for (i = 0; i < e->nout; ++i) {\n"\
            "    n = e->out[i];\n"\
            "    n->dirty = false;\n"\
            "    if (n->mtime == MTIME_UNKNOWN) nodestat(n);\n"\
            "  }\n"\
            "  depsload(e);\n"\
            "  e->nblock = 0;\n"\
            "  newest = NULL;\n"\
            "  for (i = 0; i < e->nin; ++i) {\n"\
            "    n = e->in[i];\n"\
            "    buildadd(n);\n"\
            "    if (i < e->inorderidx) {\n"\
            "      if (n->dirty) e->flags |= FLAG_DIRTY_IN;\n"\
            "      if (n->mtime != MTIME_MISSING && !isnewer(newest, n)) newest = n;\n"\
            "    }\n"\
            "    if (n->dirty || (n->gen && n->gen->nblock > 0)) ++e->nblock;\n"\
            "  }\n"\
            "  generator = edgevar(e, \"generator\", true);\n"\
            "  restat = edgevar(e, \"restat\", true);\n"\
            "  for (i = 0; i < e->nout && !(e->flags & FLAG_DIRTY_OUT); ++i) {\n"\
            "    n = e->out[i];\n"\
            "    if (isdirty(n, newest, generator, restat)) {\n"\
            "      n->dirty = true;\n"\
            "      e->flags |= FLAG_DIRTY_OUT;\n"\
            "    }\n"\
            "  }\n"\
            "  if (e->flags & FLAG_DIRTY) {\n"\
            "    for (i = 0; i < e->nout; ++i) {\n"\
            "      n = e->out[i];\n"\
            "      if (buildopts.explain && !n->dirty) {\n"\
            "        if (e->flags & FLAG_DIRTY_IN) warn(\"explain %s: input is dirty\", n->path->s);\n"\
            "        else if (e->flags & FLAG_DIRTY_OUT) warn(\"explain %s: output of generating action is dirty\", n->path->s);\n"\
            "      }\n"\
            "      n->dirty = true;\n"\
            "    }\n"\
            "  }\n"\
            "  if (!(e->flags & FLAG_DIRTY_OUT)) e->nprune = e->nblock;\n"\
            "  if (e->flags & FLAG_DIRTY) {\n"\
            "    if (e->nblock == 0) queue(e);\n"\
            "    if (e->rule != &phonyrule) ++ntotal;\n"\
            "  }\n"\
            "  e->flags &= ~FLAG_CYCLE;\n"\
            "}\n"\
            "static size_t formatstatus(char *buf, size_t len) {\n"\
            "  const char *fmt;\n"\
            "  size_t ret = 0;\n"\
            "  int n;\n"\
            "  struct timespec endtime;\n"\
            "  for (fmt = buildopts.statusfmt; *fmt; ++fmt) {\n"\
            "    if (*fmt != '%' || *++fmt == '%') {\n"\
            "      if (len > 1) {\n"\
            "        *buf++ = *fmt;\n"\
            "        --len;\n"\
            "      }\n"\
            "      ++ret;\n"\
            "      continue;\n"\
            "    }\n"\
            "    n = 0;\n"\
            "    switch (*fmt) {\n"\
            "    case 's':\n"\
            "      n = snprintf(buf, len, \"%zu\", nstarted);\n"\
            "      break;\n"\
            "    case 'f':\n"\
            "      n = snprintf(buf, len, \"%zu\", nfinished);\n"\
            "      break;\n"\
            "    case 't':\n"\
            "      n = snprintf(buf, len, \"%zu\", ntotal);\n"\
            "      break;\n"\
            "    case 'r':\n"\
            "      n = snprintf(buf, len, \"%zu\", nstarted - nfinished);\n"\
            "      break;\n"\
            "    case 'u':\n"\
            "      n = snprintf(buf, len, \"%zu\", ntotal - nstarted);\n"\
            "      break;\n"\
            "    case 'p':\n"\
            "      n = snprintf(buf, len, \"%3zu%%\", 100 * nfinished / ntotal);\n"\
            "      break;\n"\
            "    case 'o':\n"\
            "      if (clock_gettime(CLOCK_MONOTONIC, &endtime) != 0) {\n"\
            "        warn(\"clock_gettime:\");\n"\
            "        break;\n"\
            "      }\n"\
            "      n = snprintf(buf, len, \"%.1f\", nfinished / ((endtime.tv_sec - starttime.tv_sec) + 0.000000001 * (endtime.tv_nsec - starttime.tv_nsec)));\n"\
            "      break;\n"\
            "    case 'e':\n"\
            "      if (clock_gettime(CLOCK_MONOTONIC, &endtime) != 0) {\n"\
            "        warn(\"clock_gettime:\");\n"\
            "        break;\n"\
            "      }\n"\
            "      n = snprintf(buf, len, \"%.3f\", (endtime.tv_sec - starttime.tv_sec) + 0.000000001 * (endtime.tv_nsec - starttime.tv_nsec));\n"\
            "      break;\n"\
            "    default:\n"\
            "      fatal(\"unknown placeholder '%%%c' in $NINJA_STATUS\", *fmt);\n"\
            "      continue;\n"\
            "    }\n"\
            "    if (n < 0) fatal(\"snprintf:\");\n"\
            "    ret += n;\n"\
            "    if ((size_t)n > len) n = len;\n"\
            "    buf += n;\n"\
            "    len -= n;\n"\
            "  }\n"\
            "  if (len > 0) *buf = '\\0';\n"\
            "  return ret;\n"\
            "}\n"\
            "static void printstatus(struct edge *e, struct string *cmd) {\n"\
            "  struct string *description;\n"\
            "  char status[256];\n"\
            "  description = buildopts.verbose ? NULL : edgevar(e, \"description\", true);\n"\
            "  if (!description || description->n == 0) description = cmd;\n"\
            "  formatstatus(status, sizeof(status));\n"\
            "  fputs(status, stdout);\n"\
            "  puts(description->s);\n"\
            "}\n"\
            "static int jobstart(struct job *j, struct edge *e) {\n"\
            "  extern char **environ;\n"\
            "  size_t i;\n"\
            "  struct node *n;\n"\
            "  struct string *rspfile, *content;\n"\
            "  int fd[2];\n"\
            "  posix_spawn_file_actions_t actions;\n"\
            "  char *argv[] = {\"/bin/sh\", \"-c\", NULL, NULL};\n"\
            "  ++nstarted;\n"\
            "  for (i = 0; i < e->nout; ++i) {\n"\
            "    n = e->out[i];\n"\
            "    if (n->mtime == MTIME_MISSING) {\n"\
            "      if (osmkdirs(n->path, true) < 0) goto err0;\n"\
            "    }\n"\
            "  }\n"\
            "  rspfile = edgevar(e, \"rspfile\", false);\n"\
            "  if (rspfile) {\n"\
            "    content = edgevar(e, \"rspfile_content\", true);\n"\
            "    if (writefile(rspfile->s, content) < 0) goto err0;\n"\
            "  }\n"\
            "  if (pipe(fd) < 0) {\n"\
            "    warn(\"pipe:\");\n"\
            "    goto err1;\n"\
            "  }\n"\
            "  j->edge = e;\n"\
            "  j->cmd = edgevar(e, \"command\", true);\n"\
            "  j->fd = fd[0];\n"\
            "  argv[2] = j->cmd->s;\n"\
            "  if (!consoleused) printstatus(e, j->cmd);\n"\
            "  if ((errno = posix_spawn_file_actions_init(&actions))) {\n"\
            "    warn(\"posix_spawn_file_actions_init:\");\n"\
            "    goto err2;\n"\
            "  }\n"\
            "  if ((errno = posix_spawn_file_actions_addclose(&actions, fd[0]))) {\n"\
            "    warn(\"posix_spawn_file_actions_addclose:\");\n"\
            "    goto err3;\n"\
            "  }\n"\
            "  if (e->pool != &consolepool) {\n"\
            "    if ((errno = posix_spawn_file_actions_addopen(&actions, 0, \"/dev/null\", O_RDONLY, 0))) {\n"\
            "      warn(\"posix_spawn_file_actions_addopen:\");\n"\
            "      goto err3;\n"\
            "    }\n"\
            "    if ((errno = posix_spawn_file_actions_adddup2(&actions, fd[1], 1))) {\n"\
            "      warn(\"posix_spawn_file_actions_adddup2:\");\n"\
            "      goto err3;\n"\
            "    }\n"\
            "    if ((errno = posix_spawn_file_actions_adddup2(&actions, fd[1], 2))) {\n"\
            "      warn(\"posix_spawn_file_actions_adddup2:\");\n"\
            "      goto err3;\n"\
            "    }\n"\
            "    if ((errno = posix_spawn_file_actions_addclose(&actions, fd[1]))) {\n"\
            "      warn(\"posix_spawn_file_actions_addclose:\");\n"\
            "      goto err3;\n"\
            "    }\n"\
            "  }\n"\
            "  if ((errno = posix_spawn(&j->pid, argv[0], &actions, NULL, argv, environ))) {\n"\
            "    warn(\"posix_spawn %s:\", j->cmd->s);\n"\
            "    goto err3;\n"\
            "  }\n"\
            "  posix_spawn_file_actions_destroy(&actions);\n"\
            "  close(fd[1]);\n"\
            "  j->failed = false;\n"\
            "  if (e->pool == &consolepool) consoleused = true;\n"\
            "  return j->fd;\n"\
            "err3:\n"\
            "  posix_spawn_file_actions_destroy(&actions);\n"\
            "err2:\n"\
            "  close(fd[0]);\n"\
            "  close(fd[1]);\n"\
            "err1:\n"\
            "  if (rspfile && !buildopts.keeprsp) remove(rspfile->s);\n"\
            "err0:\n"\
            "  return -1;\n"\
            "}\n"\
            "static void nodedone(struct node *n, bool prune) {\n"\
            "  struct edge *e;\n"\
            "  size_t i, j;\n"\
            "  for (i = 0; i < n->nuse; ++i) {\n"\
            "    e = n->use[i];\n"\
            "    if (!(e->flags & FLAG_WORK)) continue;\n"\
            "    if (!(e->flags & (prune ? FLAG_DIRTY_OUT : FLAG_DIRTY)) && --e->nprune == 0) {\n"\
            "      for (j = 0; j < e->nout; ++j)\n"\
            "        nodedone(e->out[j], true);\n"\
            "      if (e->flags & FLAG_DIRTY && e->rule != &phonyrule) --ntotal;\n"\
            "    } else if (--e->nblock == 0) {\n"\
            "      queue(e);\n"\
            "    }\n"\
            "  }\n"\
            "}\n"\
            "static bool shouldprune(struct edge *e, struct node *n, int64_t old) {\n"\
            "  struct node *in, *newest;\n"\
            "  size_t i;\n"\
            "  if (old != n->mtime) return false;\n"\
            "  newest = NULL;\n"\
            "  for (i = 0; i < e->inorderidx; ++i) {\n"\
            "    in = e->in[i];\n"\
            "    nodestat(in);\n"\
            "    if (in->mtime != MTIME_MISSING && !isnewer(newest, in)) newest = in;\n"\
            "  }\n"\
            "  if (newest) n->logmtime = newest->mtime;\n"\
            "  return true;\n"\
            "}\n"\
            "static void edgedone(struct edge *e) {\n"\
            "  struct node *n;\n"\
            "  size_t i;\n"\
            "  struct string *rspfile;\n"\
            "  bool restat;\n"\
            "  int64_t old;\n"\
            "  restat = edgevar(e, \"restat\", true);\n"\
            "  for (i = 0; i < e->nout; ++i) {\n"\
            "    n = e->out[i];\n"\
            "    old = n->mtime;\n"\
            "    nodestat(n);\n"\
            "    n->logmtime = n->mtime == MTIME_MISSING ? 0 : n->mtime;\n"\
            "    nodedone(n, restat && shouldprune(e, n, old));\n"\
            "  }\n"\
            "  rspfile = edgevar(e, \"rspfile\", false);\n"\
            "  if (rspfile && !buildopts.keeprsp) remove(rspfile->s);\n"\
            "  edgehash(e);\n"\
            "  depsrecord(e);\n"\
            "  for (i = 0; i < e->nout; ++i) {\n"\
            "    n = e->out[i];\n"\
            "    n->hash = e->hash;\n"\
            "    logrecord(n);\n"\
            "  }\n"\
            "}\n"\
            "static void jobdone(struct job *j) {\n"\
            "  int status;\n"\
            "  struct edge *e, *new;\n"\
            "  struct pool *p;\n"\
            "  ++nfinished;\n"\
            "  if (waitpid(j->pid, &status, 0) < 0) {\n"\
            "    warn(\"waitpid %d:\", j->pid);\n"\
            "    j->failed = true;\n"\
            "  } else if (WIFEXITED(status)) {\n"\
            "    if (WEXITSTATUS(status) != 0) {\n"\
            "      warn(\"job failed with status %d: %s\", WEXITSTATUS(status), j->cmd->s);\n"\
            "      j->failed = true;\n"\
            "    }\n"\
            "  } else if (WIFSIGNALED(status)) {\n"\
            "    warn(\"job terminated due to signal %d: %s\", WTERMSIG(status), j->cmd->s);\n"\
            "    j->failed = true;\n"\
            "  } else {\n"\
            "    warn(\"job status unknown: %s\", j->cmd->s);\n"\
            "    j->failed = true;\n"\
            "  }\n"\
            "  close(j->fd);\n"\
            "  if (j->buf.len && (!consoleused || j->failed)) fwrite(j->buf.data, 1, j->buf.len, stdout);\n"\
            "  j->buf.len = 0;\n"\
            "  e = j->edge;\n"\
            "  if (e->pool) {\n"\
            "    p = e->pool;\n"\
            "    if (p == &consolepool) consoleused = false;\n"\
            "    if (p->work) {\n"\
            "      new = p->work;\n"\
            "      p->work = p->work->worknext;\n"\
            "      new->worknext = work;\n"\
            "      work = new;\n"\
            "    } else {\n"\
            "      --p->numjobs;\n"\
            "    }\n"\
            "  }\n"\
            "  if (!j->failed) edgedone(e);\n"\
            "}\n"\
            "static bool jobwork(struct job *j) {\n"\
            "  char *newdata;\n"\
            "  size_t newcap;\n"\
            "  ssize_t n;\n"\
            "  if (j->buf.cap - j->buf.len < BUFSIZ / 2) {\n"\
            "    newcap = j->buf.cap + BUFSIZ;\n"\
            "    newdata = realloc(j->buf.data, newcap);\n"\
            "    if (!newdata) {\n"\
            "      warn(\"realloc:\");\n"\
            "      goto kill;\n"\
            "    }\n"\
            "    j->buf.cap = newcap;\n"\
            "    j->buf.data = newdata;\n"\
            "  }\n"\
            "  n = read(j->fd, j->buf.data + j->buf.len, j->buf.cap - j->buf.len);\n"\
            "  if (n > 0) {\n"\
            "    j->buf.len += n;\n"\
            "    return true;\n"\
            "  }\n"\
            "  if (n == 0) goto done;\n"\
            "  warn(\"read:\");\n"\
            "kill:\n"\
            "  kill(j->pid, SIGTERM);\n"\
            "  j->failed = true;\n"\
            "done:\n"\
            "  jobdone(j);\n"\
            "  return false;\n"\
            "}\n"\
            "static double queryload(void) {\n"\
            "#ifdef HAVE_GETLOADAVG\n"\
            "  double load;\n"\
            "  if (getloadavg(&load, 1) == -1) {\n"\
            "    warn(\"getloadavg:\");\n"\
            "    load = 100.0;\n"\
            "  }\n"\
            "  return load;\n"\
            "#else\n"\
            "  return 0;\n"\
            "#endif\n"\
            "}\n"\
            "void build(void) {\n"\
            "  struct job *jobs = NULL;\n"\
            "  struct pollfd *fds = NULL;\n"\
            "  size_t i, next = 0, jobslen = 0, maxjobs = buildopts.maxjobs, numjobs = 0, numfail = 0;\n"\
            "  struct edge *e;\n"\
            "  if (ntotal == 0) {\n"\
            "    warn(\"nothing to do\");\n"\
            "    return;\n"\
            "  }\n"\
            "  clock_gettime(CLOCK_MONOTONIC, &starttime);\n"\
            "  formatstatus(NULL, 0);\n"\
            "  nstarted = 0;\n"\
            "  for (;;) {\n"\
            "    if (buildopts.maxload) maxjobs = queryload() > buildopts.maxload ? 1 : buildopts.maxjobs;\n"\
            "    while (work && numjobs < maxjobs && numfail < buildopts.maxfail) {\n"\
            "      e = work;\n"\
            "      work = work->worknext;\n"\
            "      if (e->rule != &phonyrule && buildopts.dryrun) {\n"\
            "        ++nstarted;\n"\
            "        printstatus(e, edgevar(e, \"command\", true));\n"\
            "        ++nfinished;\n"\
            "      }\n"\
            "      if (e->rule == &phonyrule || buildopts.dryrun) {\n"\
            "        for (i = 0; i < e->nout; ++i)\n"\
            "          nodedone(e->out[i], false);\n"\
            "        continue;\n"\
            "      }\n"\
            "      if (next == jobslen) {\n"\
            "        jobslen = jobslen ? jobslen * 2 : 8;\n"\
            "        if (jobslen > buildopts.maxjobs) jobslen = buildopts.maxjobs;\n"\
            "        jobs = xreallocarray(jobs, jobslen, sizeof(jobs[0]));\n"\
            "        fds = xreallocarray(fds, jobslen, sizeof(fds[0]));\n"\
            "        for (i = next; i < jobslen; ++i) {\n"\
            "          jobs[i].buf.data = NULL;\n"\
            "          jobs[i].buf.len = 0;\n"\
            "          jobs[i].buf.cap = 0;\n"\
            "          jobs[i].next = i + 1;\n"\
            "          fds[i].fd = -1;\n"\
            "          fds[i].events = POLLIN;\n"\
            "        }\n"\
            "      }\n"\
            "      fds[next].fd = jobstart(&jobs[next], e);\n"\
            "      if (fds[next].fd < 0) {\n"\
            "        warn(\"job failed to start\");\n"\
            "        ++numfail;\n"\
            "      } else {\n"\
            "        next = jobs[next].next;\n"\
            "        ++numjobs;\n"\
            "      }\n"\
            "    }\n"\
            "    if (numjobs == 0) break;\n"\
            "    if (poll(fds, jobslen, 5000) < 0) fatal(\"poll:\");\n"\
            "    for (i = 0; i < jobslen; ++i) {\n"\
            "      if (!fds[i].revents || jobwork(&jobs[i])) continue;\n"\
            "      --numjobs;\n"\
            "      jobs[i].next = next;\n"\
            "      fds[i].fd = -1;\n"\
            "      next = i;\n"\
            "      if (jobs[i].failed) ++numfail;\n"\
            "    }\n"\
            "  }\n"\
            "  for (i = 0; i < jobslen; ++i)\n"\
            "    free(jobs[i].buf.data);\n"\
            "  free(jobs);\n"\
            "  free(fds);\n"\
            "  if (numfail > 0) {\n"\
            "    if (numfail < buildopts.maxfail) fatal(\"cannot make progress due to previous errors\");\n"\
            "    else if (numfail > 1) fatal(\"subcommands failed\");\n"\
            "    else fatal(\"subcommand failed\");\n"\
            "  }\n"\
            "  ntotal = 0;\n"\
            "}\n"\
            "#define MAX_RECORD_SIZE (1 << 19)\n"\
            "struct nodearray {\n"\
            "  struct node **node;\n"\
            "  size_t len;\n"\
            "};\n"\
            "struct entry {\n"\
            "  struct node *node;\n"\
            "  struct nodearray deps;\n"\
            "  int64_t mtime;\n"\
            "};\n"\
            "static const char depsname[] = \".ninja_deps\";\n"\
            "static const char depstmpname[] = \".ninja_deps.tmp\";\n"\
            "static const char depsheader[] = \"# ninjadeps\\n\";\n"\
            "static const uint32_t depsver = 4;\n"\
            "static FILE *depsfile;\n"\
            "static struct entry *entries;\n"\
            "static size_t entrieslen, entriescap;\n"\
            "static void depswrite(const void *p, size_t n, size_t m) {\n"\
            "  if (fwrite(p, n, m, depsfile) != m) fatal(\"deps log write:\");\n"\
            "}\n"\
            "static bool recordid(struct node *n) {\n"\
            "  uint32_t sz, chk;\n"\
            "  if (n->id != -1) return false;\n"\
            "  if (entrieslen == INT32_MAX) fatal(\"too many nodes\");\n"\
            "  n->id = entrieslen++;\n"\
            "  sz = (n->path->n + 7) & ~3;\n"\
            "  if (sz + 4 >= MAX_RECORD_SIZE) fatal(\"ID record too large\");\n"\
            "  depswrite(&sz, 4, 1);\n"\
            "  depswrite(n->path->s, 1, n->path->n);\n"\
            "  depswrite((char[4]){0}, 1, sz - n->path->n - 4);\n"\
            "  chk = ~n->id;\n"\
            "  depswrite(&chk, 4, 1);\n"\
            "  return true;\n"\
            "}\n"\
            "static void recorddeps(struct node *out, struct nodearray *deps, int64_t mtime) {\n"\
            "  uint32_t sz, m;\n"\
            "  size_t i;\n"\
            "  sz = 12 + deps->len * 4;\n"\
            "  if (sz + 4 >= MAX_RECORD_SIZE) fatal(\"deps record too large\");\n"\
            "  sz |= 0x80000000;\n"\
            "  depswrite(&sz, 4, 1);\n"\
            "  depswrite(&out->id, 4, 1);\n"\
            "  m = mtime & 0xffffffff;\n"\
            "  depswrite(&m, 4, 1);\n"\
            "  m = (mtime >> 32) & 0xffffffff;\n"\
            "  depswrite(&m, 4, 1);\n"\
            "  for (i = 0; i < deps->len; ++i)\n"\
            "    depswrite(&deps->node[i]->id, 4, 1);\n"\
            "}\n"\
            "void depsinit(const char *builddir) {\n"\
            "  char *depspath = (char *)depsname, *depstmppath = (char *)depstmpname;\n"\
            "  uint32_t *buf, cap, ver, sz, id;\n"\
            "  size_t len, i, j, nrecord;\n"\
            "  bool isdep;\n"\
            "  struct string *path;\n"\
            "  struct node *n;\n"\
            "  struct edge *e;\n"\
            "  struct entry *entry, *oldentries;\n"\
            "  if (depsfile) fclose(depsfile);\n"\
            "  entrieslen = 0;\n"\
            "  cap = BUFSIZ;\n"\
            "  buf = xmalloc(cap);\n"\
            "  if (builddir) xasprintf(&depspath, \"%s/%s\", builddir, depsname);\n"\
            "  depsfile = fopen(depspath, \"r+\");\n"\
            "  if (!depsfile) {\n"\
            "    if (errno != ENOENT) fatal(\"open %s:\", depspath);\n"\
            "    goto rewrite;\n"\
            "  }\n"\
            "  if (!fgets((char *)buf, sizeof(depsheader), depsfile)) goto rewrite;\n"\
            "  if (strcmp((char *)buf, depsheader) != 0) {\n"\
            "    warn(\"invalid deps log header\");\n"\
            "    goto rewrite;\n"\
            "  }\n"\
            "  if (fread(&ver, sizeof(ver), 1, depsfile) != 1) {\n"\
            "    warn(ferror(depsfile) ? \"deps log read:\" : \"deps log truncated\");\n"\
            "    goto rewrite;\n"\
            "  }\n"\
            "  if (ver != depsver) {\n"\
            "    warn(\"unknown deps log version\");\n"\
            "    goto rewrite;\n"\
            "  }\n"\
            "  for (nrecord = 0;; ++nrecord) {\n"\
            "    if (fread(&sz, sizeof(sz), 1, depsfile) != 1) break;\n"\
            "    isdep = sz & 0x80000000;\n"\
            "    sz &= 0x7fffffff;\n"\
            "    if (sz > MAX_RECORD_SIZE) {\n"\
            "      warn(\"deps record too large\");\n"\
            "      goto rewrite;\n"\
            "    }\n"\
            "    if (sz > cap) {\n"\
            "      do\n"\
            "        cap *= 2;\n"\
            "      while (sz > cap);\n"\
            "      free(buf);\n"\
            "      buf = xmalloc(cap);\n"\
            "    }\n"\
            "    if (fread(buf, sz, 1, depsfile) != 1) {\n"\
            "      warn(ferror(depsfile) ? \"deps log read:\" : \"deps log truncated\");\n"\
            "      goto rewrite;\n"\
            "    }\n"\
            "    if (sz % 4) {\n"\
            "      warn(\"invalid size, must be multiple of 4: %\" PRIu32, sz);\n"\
            "      goto rewrite;\n"\
            "    }\n"\
            "    if (isdep) {\n"\
            "      if (sz < 12) {\n"\
            "        warn(\"invalid size, must be at least 12: %\" PRIu32, sz);\n"\
            "        goto rewrite;\n"\
            "      }\n"\
            "      sz -= 12;\n"\
            "      id = buf[0];\n"\
            "      if (id >= entrieslen) {\n"\
            "        warn(\"invalid node ID: %\" PRIu32, id);\n"\
            "        goto rewrite;\n"\
            "      }\n"\
            "      entry = &entries[id];\n"\
            "      entry->mtime = (int64_t)buf[2] << 32 | buf[1];\n"\
            "      e = entry->node->gen;\n"\
            "      if (!e || !edgevar(e, \"deps\", true)) continue;\n"\
            "      sz /= 4;\n"\
            "      free(entry->deps.node);\n"\
            "      entry->deps.len = sz;\n"\
            "      entry->deps.node = xreallocarray(NULL, sz, sizeof(n));\n"\
            "      for (i = 0; i < sz; ++i) {\n"\
            "        id = buf[3 + i];\n"\
            "        if (id >= entrieslen) {\n"\
            "          warn(\"invalid node ID: %\" PRIu32, id);\n"\
            "          goto rewrite;\n"\
            "        }\n"\
            "        entry->deps.node[i] = entries[id].node;\n"\
            "      }\n"\
            "    } else {\n"\
            "      if (sz <= 4) {\n"\
            "        warn(\"invalid size, must be greater than 4: %\" PRIu32, sz);\n"\
            "        goto rewrite;\n"\
            "      }\n"\
            "      if (entrieslen != ~buf[sz / 4 - 1]) {\n"\
            "        warn(\"corrupt deps log, bad checksum\");\n"\
            "        goto rewrite;\n"\
            "      }\n"\
            "      if (entrieslen == INT32_MAX) {\n"\
            "        warn(\"too many nodes in deps log\");\n"\
            "        goto rewrite;\n"\
            "      }\n"\
            "      len = sz - 4;\n"\
            "      while (((char *)buf)[len - 1] == '\\0')\n"\
            "        --len;\n"\
            "      path = mkstr(len);\n"\
            "      memcpy(path->s, buf, len);\n"\
            "      path->s[len] = '\\0';\n"\
            "      n = mknode(path);\n"\
            "      if (entrieslen >= entriescap) {\n"\
            "        entriescap = entriescap ? entriescap * 2 : 1024;\n"\
            "        entries = xreallocarray(entries, entriescap, sizeof(entries[0]));\n"\
            "      }\n"\
            "      n->id = entrieslen;\n"\
            "      entries[entrieslen++] = (struct entry){.node = n};\n"\
            "    }\n"\
            "  }\n"\
            "  if (ferror(depsfile)) {\n"\
            "    warn(\"deps log read:\");\n"\
            "    goto rewrite;\n"\
            "  }\n"\
            "  if (nrecord <= 1000 || nrecord < 3 * entrieslen) {\n"\
            "    if (builddir) free(depspath);\n"\
            "    free(buf);\n"\
            "    return;\n"\
            "  }\n"\
            "rewrite:\n"\
            "  free(buf);\n"\
            "  if (depsfile) fclose(depsfile);\n"\
            "  if (builddir) xasprintf(&depstmppath, \"%s/%s\", builddir, depstmpname);\n"\
            "  depsfile = fopen(depstmppath, \"w\");\n"\
            "  if (!depsfile) fatal(\"open %s:\", depstmppath);\n"\
            "  depswrite(depsheader, 1, sizeof(depsheader) - 1);\n"\
            "  depswrite(&depsver, 1, sizeof(depsver));\n"\
            "  for (i = 0; i < entrieslen; ++i)\n"\
            "    entries[i].node->id = -1;\n"\
            "  oldentries = xreallocarray(NULL, entrieslen, sizeof(entries[0]));\n"\
            "  memcpy(oldentries, entries, entrieslen * sizeof(entries[0]));\n"\
            "  len = entrieslen;\n"\
            "  entrieslen = 0;\n"\
            "  for (i = 0; i < len; ++i) {\n"\
            "    entry = &oldentries[i];\n"\
            "    if (!entry->deps.len) continue;\n"\
            "    recordid(entry->node);\n"\
            "    entries[entry->node->id] = *entry;\n"\
            "    for (j = 0; j < entry->deps.len; ++j)\n"\
            "      recordid(entry->deps.node[j]);\n"\
            "    recorddeps(entry->node, &entry->deps, entry->mtime);\n"\
            "  }\n"\
            "  free(oldentries);\n"\
            "  fflush(depsfile);\n"\
            "  if (ferror(depsfile)) fatal(\"deps log write failed\");\n"\
            "  if (rename(depstmppath, depspath) < 0) fatal(\"deps log rename:\");\n"\
            "  if (builddir) {\n"\
            "    free(depstmppath);\n"\
            "    free(depspath);\n"\
            "  }\n"\
            "}\n"\
            "void depsclose(void) {\n"\
            "  fflush(depsfile);\n"\
            "  if (ferror(depsfile)) fatal(\"deps log write failed\");\n"\
            "  fclose(depsfile);\n"\
            "}\n"\
            "static struct nodearray *depsparse(const char *name, bool allowmissing) {\n"\
            "  static struct buffer buf;\n"\
            "  static struct nodearray deps;\n"\
            "  static size_t depscap;\n"\
            "  struct string *in, *out = NULL;\n"\
            "  FILE *f;\n"\
            "  int c, n;\n"\
            "  bool sawcolon;\n"\
            "  deps.len = 0;\n"\
            "  f = fopen(name, \"r\");\n"\
            "  if (!f) {\n"\
            "    if (errno == ENOENT && allowmissing) return &deps;\n"\
            "    return NULL;\n"\
            "  }\n"\
            "  sawcolon = false;\n"\
            "  buf.len = 0;\n"\
            "  c = getc(f);\n"\
            "  for (;;) {\n"\
            "    while (isalnum(c) || strchr(\"$+,-./@\\\\_\", c)) {\n"\
            "      switch (c) {\n"\
            "      case '\\\\':\n"\
            "        n = 0;\n"\
            "        do {\n"\
            "          c = getc(f);\n"\
            "          if (++n % 2 == 0) bufadd(&buf, '\\\\');\n"\
            "        } while (c == '\\\\');\n"\
            "        if ((c == ' ' || c == '\\t') && n % 2 != 0) break;\n"\
            "        for (; n > 2; n -= 2)\n"\
            "          bufadd(&buf, '\\\\');\n"\
            "        switch (c) {\n"\
            "        case '#':\n"\
            "          break;\n"\
            "        case '\\n':\n"\
            "          c = ' ';\n"\
            "          continue;\n"\
            "        default:\n"\
            "          bufadd(&buf, '\\\\');\n"\
            "          continue;\n"\
            "        }\n"\
            "        break;\n"\
            "      case '$':\n"\
            "        c = getc(f);\n"\
            "        if (c != '$') {\n"\
            "          warn(\"bad depfile '%s': contains variable reference\", name);\n"\
            "          goto err;\n"\
            "        }\n"\
            "        break;\n"\
            "      }\n"\
            "      bufadd(&buf, c);\n"\
            "      c = getc(f);\n"\
            "    }\n"\
            "    if (sawcolon) {\n"\
            "      if (!isspace(c) && c != EOF) {\n"\
            "        warn(\"bad depfile '%s': '%c' is not a valid target character\", name, c);\n"\
            "        goto err;\n"\
            "      }\n"\
            "      if (buf.len > 0) {\n"\
            "        if (deps.len == depscap) {\n"\
            "          depscap = deps.node ? depscap * 2 : 32;\n"\
            "          deps.node = xreallocarray(deps.node, depscap, sizeof(deps.node[0]));\n"\
            "        }\n"\
            "        in = mkstr(buf.len);\n"\
            "        memcpy(in->s, buf.data, buf.len);\n"\
            "        in->s[buf.len] = '\\0';\n"\
            "        deps.node[deps.len++] = mknode(in);\n"\
            "      }\n"\
            "      if (c == '\\n') {\n"\
            "        sawcolon = false;\n"\
            "        do\n"\
            "          c = getc(f);\n"\
            "        while (c == '\\n');\n"\
            "      }\n"\
            "      if (c == EOF) break;\n"\
            "    } else {\n"\
            "      while (isblank(c))\n"\
            "        c = getc(f);\n"\
            "      if (c == EOF) break;\n"\
            "      if (c != ':') {\n"\
            "        warn(\"bad depfile '%s': expected ':', saw '%c'\", name, c);\n"\
            "        goto err;\n"\
            "      }\n"\
            "      if (!out) {\n"\
            "        out = mkstr(buf.len);\n"\
            "        memcpy(out->s, buf.data, buf.len);\n"\
            "        out->s[buf.len] = '\\0';\n"\
            "      } else if (out->n != buf.len || memcmp(buf.data, out->s, buf.len) != 0) {\n"\
            "        warn(\"bad depfile '%s': multiple outputs: %.*s != %s\", name, (int)buf.len, buf.data, out->s);\n"\
            "        goto err;\n"\
            "      }\n"\
            "      sawcolon = true;\n"\
            "      c = getc(f);\n"\
            "    }\n"\
            "    buf.len = 0;\n"\
            "    for (;;) {\n"\
            "      if (c == '\\\\') {\n"\
            "        if (getc(f) != '\\n') {\n"\
            "          warn(\"bad depfile '%s': '\\\\' only allowed before newline\", name);\n"\
            "          goto err;\n"\
            "        }\n"\
            "      } else if (!isblank(c)) {\n"\
            "        break;\n"\
            "      }\n"\
            "      c = getc(f);\n"\
            "    }\n"\
            "  }\n"\
            "  if (ferror(f)) {\n"\
            "    warn(\"depfile read '%s':\", name);\n"\
            "    goto err;\n"\
            "  }\n"\
            "  fclose(f);\n"\
            "  return &deps;\n"\
            "err:\n"\
            "  fclose(f);\n"\
            "  return NULL;\n"\
            "}\n"\
            "void depsload(struct edge *e) {\n"\
            "  struct string *deptype, *depfile;\n"\
            "  struct nodearray *deps = NULL;\n"\
            "  struct node *n;\n"\
            "  if (e->flags & FLAG_DEPS) return;\n"\
            "  e->flags |= FLAG_DEPS;\n"\
            "  n = e->out[0];\n"\
            "  deptype = edgevar(e, \"deps\", true);\n"\
            "  if (deptype) {\n"\
            "    if (n->id != -1 && n->mtime <= entries[n->id].mtime) deps = &entries[n->id].deps;\n"\
            "    else if (buildopts.explain) warn(\"explain %s: missing or outdated record in .ninja_deps\", n->path->s);\n"\
            "  } else {\n"\
            "    depfile = edgevar(e, \"depfile\", false);\n"\
            "    if (!depfile) return;\n"\
            "    deps = depsparse(depfile->s, false);\n"\
            "    if (buildopts.explain && !deps) warn(\"explain %s: missing or invalid depfile\", n->path->s);\n"\
            "  }\n"\
            "  if (deps) {\n"\
            "    edgeadddeps(e, deps->node, deps->len);\n"\
            "  } else {\n"\
            "    n->dirty = true;\n"\
            "    e->flags |= FLAG_DIRTY_OUT;\n"\
            "  }\n"\
            "}\n"\
            "void depsrecord(struct edge *e) {\n"\
            "  struct string *deptype, *depfile;\n"\
            "  struct nodearray *deps;\n"\
            "  struct node *out, *n;\n"\
            "  struct entry *entry;\n"\
            "  size_t i;\n"\
            "  bool update;\n"\
            "  deptype = edgevar(e, \"deps\", true);\n"\
            "  if (!deptype || deptype->n == 0) return;\n"\
            "  if (strcmp(deptype->s, \"gcc\") != 0) {\n"\
            "    warn(\"unsuported deps type: %s\", deptype->s);\n"\
            "    return;\n"\
            "  }\n"\
            "  depfile = edgevar(e, \"depfile\", false);\n"\
            "  if (!depfile || depfile->n == 0) {\n"\
            "    warn(\"deps but no depfile\");\n"\
            "    return;\n"\
            "  }\n"\
            "  out = e->out[0];\n"\
            "  deps = depsparse(depfile->s, true);\n"\
            "  if (!buildopts.keepdepfile) remove(depfile->s);\n"\
            "  if (!deps) return;\n"\
            "  update = false;\n"\
            "  entry = NULL;\n"\
            "  if (recordid(out)) {\n"\
            "    update = true;\n"\
            "  } else {\n"\
            "    entry = &entries[out->id];\n"\
            "    if (entry->mtime != out->mtime || entry->deps.len != deps->len) update = true;\n"\
            "    for (i = 0; i < deps->len && !update; ++i) {\n"\
            "      if (entry->deps.node[i] != deps->node[i]) update = true;\n"\
            "    }\n"\
            "  }\n"\
            "  for (i = 0; i < deps->len; ++i) {\n"\
            "    n = deps->node[i];\n"\
            "    if (recordid(n)) update = true;\n"\
            "  }\n"\
            "  if (update) {\n"\
            "    recorddeps(out, deps, out->mtime);\n"\
            "    if (fflush(depsfile) < 0) fatal(\"deps log flush:\");\n"\
            "  }\n"\
            "}\n"\
            "struct environment {\n"\
            "  struct environment *parent;\n"\
            "  struct treenode *bindings;\n"\
            "  struct treenode *rules;\n"\
            "  struct environment *allnext;\n"\
            "};\n"\
            "struct environment *rootenv;\n"\
            "struct rule phonyrule = {.name = \"phony\"};\n"\
            "struct pool consolepool = {.name = \"console\", .maxjobs = 1};\n"\
            "static struct treenode *pools;\n"\
            "static struct environment *allenvs;\n"\
            "static void addpool(struct pool *);\n"\
            "static void delpool(void *);\n"\
            "static void delrule(void *);\n"\
            "void envinit(void) {\n"\
            "  struct environment *env;\n"\
            "  while (allenvs) {\n"\
            "    env = allenvs;\n"\
            "    allenvs = env->allnext;\n"\
            "    deltree(env->bindings, free, free);\n"\
            "    deltree(env->rules, NULL, delrule);\n"\
            "    free(env);\n"\
            "  }\n"\
            "  deltree(pools, NULL, delpool);\n"\
            "  rootenv = mkenv(NULL);\n"\
            "  envaddrule(rootenv, &phonyrule);\n"\
            "  pools = NULL;\n"\
            "  addpool(&consolepool);\n"\
            "}\n"\
            "static void addvar(struct treenode **tree, char *var, void *val) {\n"\
            "  char *old;\n"\
            "  old = treeinsert(tree, var, val);\n"\
            "  if (old) free(old);\n"\
            "}\n"\
            "struct environment *mkenv(struct environment *parent) {\n"\
            "  struct environment *env;\n"\
            "  env = xmalloc(sizeof(*env));\n"\
            "  env->parent = parent;\n"\
            "  env->bindings = NULL;\n"\
            "  env->rules = NULL;\n"\
            "  env->allnext = allenvs;\n"\
            "  allenvs = env;\n"\
            "  return env;\n"\
            "}\n"\
            "struct string *envvar(struct environment *env, char *var) {\n"\
            "  struct treenode *n;\n"\
            "  do {\n"\
            "    n = treefind(env->bindings, var);\n"\
            "    if (n) return n->value;\n"\
            "    env = env->parent;\n"\
            "  } while (env);\n"\
            "  return NULL;\n"\
            "}\n"\
            "void envaddvar(struct environment *env, char *var, struct string *val) {\n"\
            "  addvar(&env->bindings, var, val);\n"\
            "}\n"\
            "static struct string *merge(struct evalstring *str, size_t n) {\n"\
            "  struct string *result;\n"\
            "  struct evalstring *p;\n"\
            "  char *s;\n"\
            "  result = mkstr(n);\n"\
            "  s = result->s;\n"\
            "  for (p = str; p; p = p->next) {\n"\
            "    if (!p->str) continue;\n"\
            "    memcpy(s, p->str->s, p->str->n);\n"\
            "    s += p->str->n;\n"\
            "  }\n"\
            "  *s = '\\0';\n"\
            "  return result;\n"\
            "}\n"\
            "struct string *enveval(struct environment *env, struct evalstring *str) {\n"\
            "  size_t n;\n"\
            "  struct evalstring *p;\n"\
            "  struct string *res;\n"\
            "  n = 0;\n"\
            "  for (p = str; p; p = p->next) {\n"\
            "    if (p->var) p->str = envvar(env, p->var);\n"\
            "    if (p->str) n += p->str->n;\n"\
            "  }\n"\
            "  res = merge(str, n);\n"\
            "  delevalstr(str);\n"\
            "  return res;\n"\
            "}\n"\
            "void envaddrule(struct environment *env, struct rule *r) {\n"\
            "  if (treeinsert(&env->rules, r->name, r)) fatal(\"rule '%s' redefined\", r->name);\n"\
            "}\n"\
            "struct rule *envrule(struct environment *env, char *name) {\n"\
            "  struct treenode *n;\n"\
            "  do {\n"\
            "    n = treefind(env->rules, name);\n"\
            "    if (n) return n->value;\n"\
            "    env = env->parent;\n"\
            "  } while (env);\n"\
            "  return NULL;\n"\
            "}\n"\
            "static struct string *pathlist(struct node **nodes, size_t n, char sep, bool escape) {\n"\
            "  size_t i, len;\n"\
            "  struct string *path, *result;\n"\
            "  char *s;\n"\
            "  if (n == 0) return NULL;\n"\
            "  if (n == 1) return nodepath(nodes[0], escape);\n"\
            "  for (i = 0, len = 0; i < n; ++i)\n"\
            "    len += nodepath(nodes[i], escape)->n;\n"\
            "  result = mkstr(len + n - 1);\n"\
            "  s = result->s;\n"\
            "  for (i = 0; i < n; ++i) {\n"\
            "    path = nodepath(nodes[i], escape);\n"\
            "    memcpy(s, path->s, path->n);\n"\
            "    s += path->n;\n"\
            "    *s++ = sep;\n"\
            "  }\n"\
            "  *--s = '\\0';\n"\
            "  return result;\n"\
            "}\n"\
            "struct rule *mkrule(char *name) {\n"\
            "  struct rule *r;\n"\
            "  r = xmalloc(sizeof(*r));\n"\
            "  r->name = name;\n"\
            "  r->bindings = NULL;\n"\
            "  return r;\n"\
            "}\n"\
            "static void delrule(void *ptr) {\n"\
            "  struct rule *r = ptr;\n"\
            "  if (r == &phonyrule) return;\n"\
            "  deltree(r->bindings, free, delevalstr);\n"\
            "  free(r->name);\n"\
            "  free(r);\n"\
            "}\n"\
            "void ruleaddvar(struct rule *r, char *var, struct evalstring *val) {\n"\
            "  addvar(&r->bindings, var, val);\n"\
            "}\n"\
            "struct string *edgevar(struct edge *e, char *var, bool escape) {\n"\
            "  static void *const cycle = (void *)&cycle;\n"\
            "  struct evalstring *str, *p;\n"\
            "  struct treenode *n;\n"\
            "  size_t len;\n"\
            "  if (strcmp(var, \"in\") == 0) return pathlist(e->in, e->inimpidx, ' ', escape);\n"\
            "  if (strcmp(var, \"in_newline\") == 0) return pathlist(e->in, e->inimpidx, '\\n', escape);\n"\
            "  if (strcmp(var, \"out\") == 0) return pathlist(e->out, e->outimpidx, ' ', escape);\n"\
            "  n = treefind(e->env->bindings, var);\n"\
            "  if (n) return n->value;\n"\
            "  n = treefind(e->rule->bindings, var);\n"\
            "  if (!n) return envvar(e->env->parent, var);\n"\
            "  if (n->value == cycle) fatal(\"cycle in rule variable involving '%s'\", var);\n"\
            "  str = n->value;\n"\
            "  n->value = cycle;\n"\
            "  len = 0;\n"\
            "  for (p = str; p; p = p->next) {\n"\
            "    if (p->var) p->str = edgevar(e, p->var, escape);\n"\
            "    if (p->str) len += p->str->n;\n"\
            "  }\n"\
            "  n->value = str;\n"\
            "  return merge(str, len);\n"\
            "}\n"\
            "static void addpool(struct pool *p) {\n"\
            "  if (treeinsert(&pools, p->name, p)) fatal(\"pool '%s' redefined\", p->name);\n"\
            "}\n"\
            "struct pool *mkpool(char *name) {\n"\
            "  struct pool *p;\n"\
            "  p = xmalloc(sizeof(*p));\n"\
            "  p->name = name;\n"\
            "  p->numjobs = 0;\n"\
            "  p->maxjobs = 0;\n"\
            "  p->work = NULL;\n"\
            "  addpool(p);\n"\
            "  return p;\n"\
            "}\n"\
            "static void delpool(void *ptr) {\n"\
            "  struct pool *p = ptr;\n"\
            "  if (p == &consolepool) return;\n"\
            "  free(p->name);\n"\
            "  free(p);\n"\
            "}\n"\
            "struct pool *poolget(char *name) {\n"\
            "  struct treenode *n;\n"\
            "  n = treefind(pools, name);\n"\
            "  if (!n) fatal(\"unknown pool '%s'\", name);\n"\
            "  return n->value;\n"\
            "}\n"\
            "static struct hashtable *allnodes;\n"\
            "struct edge *alledges;\n"\
            "static void delnode(void *p) {\n"\
            "  struct node *n = p;\n"\
            "  if (n->shellpath != n->path) free(n->shellpath);\n"\
            "  free(n->use);\n"\
            "  free(n->path);\n"\
            "  free(n);\n"\
            "}\n"\
            "void graphinit(void) {\n"\
            "  struct edge *e;\n"\
            "  delhtab(allnodes, delnode);\n"\
            "  while (alledges) {\n"\
            "    e = alledges;\n"\
            "    alledges = e->allnext;\n"\
            "    free(e->out);\n"\
            "    free(e->in);\n"\
            "    free(e);\n"\
            "  }\n"\
            "  allnodes = mkhtab(1024);\n"\
            "}\n"\
            "struct node *mknode(struct string *path) {\n"\
            "  void **v;\n"\
            "  struct node *n;\n"\
            "  struct hashtablekey k;\n"\
            "  htabkey(&k, path->s, path->n);\n"\
            "  v = htabput(allnodes, &k);\n"\
            "  if (*v) {\n"\
            "    free(path);\n"\
            "    return *v;\n"\
            "  }\n"\
            "  n = xmalloc(sizeof(*n));\n"\
            "  n->path = path;\n"\
            "  n->shellpath = NULL;\n"\
            "  n->gen = NULL;\n"\
            "  n->use = NULL;\n"\
            "  n->nuse = 0;\n"\
            "  n->mtime = MTIME_UNKNOWN;\n"\
            "  n->logmtime = MTIME_MISSING;\n"\
            "  n->hash = 0;\n"\
            "  n->id = -1;\n"\
            "  *v = n;\n"\
            "  return n;\n"\
            "}\n"\
            "struct node *nodeget(const char *path, size_t len) {\n"\
            "  struct hashtablekey k;\n"\
            "  if (!len) len = strlen(path);\n"\
            "  htabkey(&k, path, len);\n"\
            "  return htabget(allnodes, &k);\n"\
            "}\n"\
            "void nodestat(struct node *n) {\n"\
            "  n->mtime = osmtime(n->path->s);\n"\
            "}\n"\
            "struct string *nodepath(struct node *n, bool escape) {\n"\
            "  char *s, *d;\n"\
            "  int nquote;\n"\
            "  if (!escape) return n->path;\n"\
            "  if (n->shellpath) return n->shellpath;\n"\
            "  escape = false;\n"\
            "  nquote = 0;\n"\
            "  for (s = n->path->s; *s; ++s) {\n"\
            "    if (!isalnum(*(unsigned char *)s) && !strchr(\"_+-./\", *s)) escape = true;\n"\
            "    if (*s == '\\'') ++nquote;\n"\
            "  }\n"\
            "  if (escape) {\n"\
            "    n->shellpath = mkstr(n->path->n + 2 + 3 * nquote);\n"\
            "    d = n->shellpath->s;\n"\
            "    *d++ = '\\'';\n"\
            "    for (s = n->path->s; *s; ++s) {\n"\
            "      *d++ = *s;\n"\
            "      if (*s == '\\'') {\n"\
            "        *d++ = '\\\\';\n"\
            "        *d++ = '\\'';\n"\
            "        *d++ = '\\'';\n"\
            "      }\n"\
            "    }\n"\
            "    *d++ = '\\'';\n"\
            "  } else {\n"\
            "    n->shellpath = n->path;\n"\
            "  }\n"\
            "  return n->shellpath;\n"\
            "}\n"\
            "void nodeuse(struct node *n, struct edge *e) {\n"\
            "  if (!(n->nuse & (n->nuse - 1))) n->use = xreallocarray(n->use, n->nuse ? n->nuse * 2 : 1, sizeof(e));\n"\
            "  n->use[n->nuse++] = e;\n"\
            "}\n"\
            "struct edge *mkedge(struct environment *parent) {\n"\
            "  struct edge *e;\n"\
            "  e = xmalloc(sizeof(*e));\n"\
            "  e->env = mkenv(parent);\n"\
            "  e->pool = NULL;\n"\
            "  e->out = NULL;\n"\
            "  e->nout = 0;\n"\
            "  e->in = NULL;\n"\
            "  e->nin = 0;\n"\
            "  e->flags = 0;\n"\
            "  e->allnext = alledges;\n"\
            "  alledges = e;\n"\
            "  return e;\n"\
            "}\n"\
            "void edgehash(struct edge *e) {\n"\
            "  static const char sep[] = \";rspfile=\";\n"\
            "  struct string *cmd, *rsp, *s;\n"\
            "  if (e->flags & FLAG_HASH) return;\n"\
            "  e->flags |= FLAG_HASH;\n"\
            "  cmd = edgevar(e, \"command\", true);\n"\
            "  if (!cmd) fatal(\"rule '%s' has no command\", e->rule->name);\n"\
            "  rsp = edgevar(e, \"rspfile_content\", true);\n"\
            "  if (rsp && rsp->n > 0) {\n"\
            "    s = mkstr(cmd->n + sizeof(sep) - 1 + rsp->n);\n"\
            "    memcpy(s->s, cmd->s, cmd->n);\n"\
            "    memcpy(s->s + cmd->n, sep, sizeof(sep) - 1);\n"\
            "    memcpy(s->s + cmd->n + sizeof(sep) - 1, rsp->s, rsp->n);\n"\
            "    s->s[s->n] = '\\0';\n"\
            "    e->hash = murmurhash64a(s->s, s->n);\n"\
            "    free(s);\n"\
            "  } else {\n"\
            "    e->hash = murmurhash64a(cmd->s, cmd->n);\n"\
            "  }\n"\
            "}\n"\
            "static struct edge *mkphony(struct node *n) {\n"\
            "  struct edge *e;\n"\
            "  e = mkedge(rootenv);\n"\
            "  e->rule = &phonyrule;\n"\
            "  e->inimpidx = 0;\n"\
            "  e->inorderidx = 0;\n"\
            "  e->outimpidx = 1;\n"\
            "  e->nout = 1;\n"\
            "  e->out = xmalloc(sizeof(n));\n"\
            "  e->out[0] = n;\n"\
            "  return e;\n"\
            "}\n"\
            "void edgeadddeps(struct edge *e, struct node **deps, size_t ndeps) {\n"\
            "  struct node **order, *n;\n"\
            "  size_t norder, i;\n"\
            "  for (i = 0; i < ndeps; ++i) {\n"\
            "    n = deps[i];\n"\
            "    if (!n->gen) n->gen = mkphony(n);\n"\
            "    nodeuse(n, e);\n"\
            "  }\n"\
            "  e->in = xreallocarray(e->in, e->nin + ndeps, sizeof(e->in[0]));\n"\
            "  order = e->in + e->inorderidx;\n"\
            "  norder = e->nin - e->inorderidx;\n"\
            "  memmove(order + ndeps, order, norder * sizeof(e->in[0]));\n"\
            "  memcpy(order, deps, ndeps * sizeof(e->in[0]));\n"\
            "  e->inorderidx += ndeps;\n"\
            "  e->nin += ndeps;\n"\
            "}\n"\
            "struct hashtable {\n"\
            "  size_t len, cap;\n"\
            "  struct hashtablekey *keys;\n"\
            "  void **vals;\n"\
            "};\n"\
            "void htabkey(struct hashtablekey *k, const char *s, size_t n) {\n"\
            "  k->str = s;\n"\
            "  k->len = n;\n"\
            "  k->hash = murmurhash64a(s, n);\n"\
            "}\n"\
            "struct hashtable *mkhtab(size_t cap) {\n"\
            "  struct hashtable *h;\n"\
            "  size_t i;\n"\
            "  assert(!(cap & (cap - 1)));\n"\
            "  h = xmalloc(sizeof(*h));\n"\
            "  h->len = 0;\n"\
            "  h->cap = cap;\n"\
            "  h->keys = xreallocarray(NULL, cap, sizeof(h->keys[0]));\n"\
            "  h->vals = xreallocarray(NULL, cap, sizeof(h->vals[0]));\n"\
            "  for (i = 0; i < cap; ++i)\n"\
            "    h->keys[i].str = NULL;\n"\
            "  return h;\n"\
            "}\n"\
            "void delhtab(struct hashtable *h, void del(void *)) {\n"\
            "  size_t i;\n"\
            "  if (!h) return;\n"\
            "  if (del) {\n"\
            "    for (i = 0; i < h->cap; ++i) {\n"\
            "      if (h->keys[i].str) del(h->vals[i]);\n"\
            "    }\n"\
            "  }\n"\
            "  free(h->keys);\n"\
            "  free(h->vals);\n"\
            "  free(h);\n"\
            "}\n"\
            "static bool keyequal(struct hashtablekey *k1, struct hashtablekey *k2) {\n"\
            "  if (k1->hash != k2->hash || k1->len != k2->len) return false;\n"\
            "  return memcmp(k1->str, k2->str, k1->len) == 0;\n"\
            "}\n"\
            "static size_t keyindex(struct hashtable *h, struct hashtablekey *k) {\n"\
            "  size_t i;\n"\
            "  i = k->hash & (h->cap - 1);\n"\
            "  while (h->keys[i].str && !keyequal(&h->keys[i], k))\n"\
            "    i = (i + 1) & (h->cap - 1);\n"\
            "  return i;\n"\
            "}\n"\
            "void **htabput(struct hashtable *h, struct hashtablekey *k) {\n"\
            "  struct hashtablekey *oldkeys;\n"\
            "  void **oldvals;\n"\
            "  size_t i, j, oldcap;\n"\
            "  if (h->cap / 2 < h->len) {\n"\
            "    oldkeys = h->keys;\n"\
            "    oldvals = h->vals;\n"\
            "    oldcap = h->cap;\n"\
            "    h->cap *= 2;\n"\
            "    h->keys = xreallocarray(NULL, h->cap, sizeof(h->keys[0]));\n"\
            "    h->vals = xreallocarray(NULL, h->cap, sizeof(h->vals[0]));\n"\
            "    for (i = 0; i < h->cap; ++i)\n"\
            "      h->keys[i].str = NULL;\n"\
            "    for (i = 0; i < oldcap; ++i) {\n"\
            "      if (oldkeys[i].str) {\n"\
            "        j = keyindex(h, &oldkeys[i]);\n"\
            "        h->keys[j] = oldkeys[i];\n"\
            "        h->vals[j] = oldvals[i];\n"\
            "      }\n"\
            "    }\n"\
            "    free(oldkeys);\n"\
            "    free(oldvals);\n"\
            "  }\n"\
            "  i = keyindex(h, k);\n"\
            "  if (!h->keys[i].str) {\n"\
            "    h->keys[i] = *k;\n"\
            "    h->vals[i] = NULL;\n"\
            "    ++h->len;\n"\
            "  }\n"\
            "  return &h->vals[i];\n"\
            "}\n"\
            "void *htabget(struct hashtable *h, struct hashtablekey *k) {\n"\
            "  size_t i;\n"\
            "  i = keyindex(h, k);\n"\
            "  return h->keys[i].str ? h->vals[i] : NULL;\n"\
            "}\n"\
            "uint64_t murmurhash64a(const void *ptr, size_t len) {\n"\
            "  const uint64_t seed = 0xdecafbaddecafbadull;\n"\
            "  const uint64_t m = 0xc6a4a7935bd1e995ull;\n"\
            "  uint64_t h, k, n;\n"\
            "  const uint8_t *p, *end;\n"\
            "  int r = 47;\n"\
            "  h = seed ^ (len * m);\n"\
            "  n = len & ~0x7ull;\n"\
            "  end = ptr;\n"\
            "  end += n;\n"\
            "  for (p = ptr; p != end; p += 8) {\n"\
            "    memcpy(&k, p, sizeof(k));\n"\
            "    k *= m;\n"\
            "    k ^= k >> r;\n"\
            "    k *= m;\n"\
            "    h ^= k;\n"\
            "    h *= m;\n"\
            "  }\n"\
            "  switch (len & 0x7) {\n"\
            "  case 7:\n"\
            "    h ^= (uint64_t)p[6] << 48;\n"\
            "  case 6:\n"\
            "    h ^= (uint64_t)p[5] << 40;\n"\
            "  case 5:\n"\
            "    h ^= (uint64_t)p[4] << 32;\n"\
            "  case 4:\n"\
            "    h ^= (uint64_t)p[3] << 24;\n"\
            "  case 3:\n"\
            "    h ^= (uint64_t)p[2] << 16;\n"\
            "  case 2:\n"\
            "    h ^= (uint64_t)p[1] << 8;\n"\
            "  case 1:\n"\
            "    h ^= (uint64_t)p[0];\n"\
            "    h *= m;\n"\
            "  }\n"\
            "  h ^= h >> r;\n"\
            "  h *= m;\n"\
            "  h ^= h >> r;\n"\
            "  return h;\n"\
            "}\n"\
            "static FILE *logfile;\n"\
            "static const char *logname = \".ninja_log\";\n"\
            "static const char *logtmpname = \".ninja_log.tmp\";\n"\
            "static const char *logfmt = \"# ninja log v%d\\n\";\n"\
            "static const int logver = 5;\n"\
            "static char *nextfield(char **end) {\n"\
            "  char *s = *end;\n"\
            "  if (!*s) {\n"\
            "    warn(\"corrupt build log: missing field\");\n"\
            "    return NULL;\n"\
            "  }\n"\
            "  *end += strcspn(*end, \"\\t\\n\");\n"\
            "  if (**end) *(*end)++ = '\\0';\n"\
            "  return s;\n"\
            "}\n"\
            "void loginit(const char *builddir) {\n"\
            "  int ver;\n"\
            "  char *logpath = (char *)logname, *logtmppath = (char *)logtmpname, *p, *s;\n"\
            "  size_t nline, nentry, i;\n"\
            "  struct edge *e;\n"\
            "  struct node *n;\n"\
            "  int64_t mtime;\n"\
            "  struct buffer buf = {0};\n"\
            "  nline = 0;\n"\
            "  nentry = 0;\n"\
            "  if (logfile) fclose(logfile);\n"\
            "  if (builddir) xasprintf(&logpath, \"%s/%s\", builddir, logname);\n"\
            "  logfile = fopen(logpath, \"r+\");\n"\
            "  if (!logfile) {\n"\
            "    if (errno != ENOENT) fatal(\"open %s:\", logpath);\n"\
            "    goto rewrite;\n"\
            "  }\n"\
            "  setvbuf(logfile, NULL, _IOLBF, 0);\n"\
            "  if (fscanf(logfile, logfmt, &ver) < 1) goto rewrite;\n"\
            "  if (ver != logver) goto rewrite;\n"\
            "  for (;;) {\n"\
            "    if (buf.cap - buf.len < BUFSIZ) {\n"\
            "      buf.cap = buf.cap ? buf.cap * 2 : BUFSIZ;\n"\
            "      buf.data = xreallocarray(buf.data, buf.cap, 1);\n"\
            "    }\n"\
            "    buf.data[buf.cap - 2] = '\\0';\n"\
            "    if (!fgets(buf.data + buf.len, buf.cap - buf.len, logfile)) break;\n"\
            "    if (buf.data[buf.cap - 2] && buf.data[buf.cap - 2] != '\\n') {\n"\
            "      buf.len = buf.cap - 1;\n"\
            "      continue;\n"\
            "    }\n"\
            "    ++nline;\n"\
            "    p = buf.data;\n"\
            "    buf.len = 0;\n"\
            "    if (!nextfield(&p)) continue;\n"\
            "    if (!nextfield(&p)) continue;\n"\
            "    s = nextfield(&p);\n"\
            "    if (!s) continue;\n"\
            "    mtime = strtoll(s, &s, 10);\n"\
            "    if (*s) {\n"\
            "      warn(\"corrupt build log: invalid mtime\");\n"\
            "      continue;\n"\
            "    }\n"\
            "    s = nextfield(&p);\n"\
            "    if (!s) continue;\n"\
            "    n = nodeget(s, 0);\n"\
            "    if (!n || !n->gen) continue;\n"\
            "    if (n->logmtime == MTIME_MISSING) ++nentry;\n"\
            "    n->logmtime = mtime;\n"\
            "    s = nextfield(&p);\n"\
            "    if (!s) continue;\n"\
            "    n->hash = strtoull(s, &s, 16);\n"\
            "    if (*s) {\n"\
            "      warn(\"corrupt build log: invalid hash for '%s'\", n->path->s);\n"\
            "      continue;\n"\
            "    }\n"\
            "  }\n"\
            "  free(buf.data);\n"\
            "  if (ferror(logfile)) {\n"\
            "    warn(\"build log read:\");\n"\
            "    goto rewrite;\n"\
            "  }\n"\
            "  if (nline <= 100 || nline <= 3 * nentry) {\n"\
            "    if (builddir) free(logpath);\n"\
            "    return;\n"\
            "  }\n"\
            "rewrite:\n"\
            "  if (logfile) fclose(logfile);\n"\
            "  if (builddir) xasprintf(&logtmppath, \"%s/%s\", builddir, logtmpname);\n"\
            "  logfile = fopen(logtmppath, \"w\");\n"\
            "  if (!logfile) fatal(\"open %s:\", logtmppath);\n"\
            "  setvbuf(logfile, NULL, _IOLBF, 0);\n"\
            "  fprintf(logfile, logfmt, logver);\n"\
            "  if (nentry > 0) {\n"\
            "    for (e = alledges; e; e = e->allnext) {\n"\
            "      for (i = 0; i < e->nout; ++i) {\n"\
            "        n = e->out[i];\n"\
            "        if (!n->hash) continue;\n"\
            "        logrecord(n);\n"\
            "      }\n"\
            "    }\n"\
            "  }\n"\
            "  fflush(logfile);\n"\
            "  if (ferror(logfile)) fatal(\"build log write failed\");\n"\
            "  if (rename(logtmppath, logpath) < 0) fatal(\"build log rename:\");\n"\
            "  if (builddir) {\n"\
            "    free(logpath);\n"\
            "    free(logtmppath);\n"\
            "  }\n"\
            "}\n"\
            "void logclose(void) {\n"\
            "  fflush(logfile);\n"\
            "  if (ferror(logfile)) fatal(\"build log write failed\");\n"\
            "  fclose(logfile);\n"\
            "}\n"\
            "void logrecord(struct node *n) {\n"\
            "  fprintf(logfile, \"0\\t0\\t%\" PRId64 \"\\t%s\\t%\" PRIx64 \"\\n\", n->logmtime, n->path->s, n->hash);\n"\
            "}\n"\
            "struct parseoptions parseopts;\n"\
            "static struct node **deftarg;\n"\
            "static size_t ndeftarg;\n"\
            "void parseinit(void) {\n"\
            "  free(deftarg);\n"\
            "  deftarg = NULL;\n"\
            "  ndeftarg = 0;\n"\
            "}\n"\
            "static void parselet(struct scanner *s, struct evalstring **val) {\n"\
            "  scanchar(s, '=');\n"\
            "  *val = scanstring(s, false);\n"\
            "  scannewline(s);\n"\
            "}\n"\
            "static void parserule(struct scanner *s, struct environment *env) {\n"\
            "  struct rule *r;\n"\
            "  char *var;\n"\
            "  struct evalstring *val;\n"\
            "  bool hascommand = false, hasrspfile = false, hasrspcontent = false;\n"\
            "  r = mkrule(scanname(s));\n"\
            "  scannewline(s);\n"\
            "  while (scanindent(s)) {\n"\
            "    var = scanname(s);\n"\
            "    parselet(s, &val);\n"\
            "    ruleaddvar(r, var, val);\n"\
            "    if (!val) continue;\n"\
            "    if (strcmp(var, \"command\") == 0) hascommand = true;\n"\
            "    else if (strcmp(var, \"rspfile\") == 0) hasrspfile = true;\n"\
            "    else if (strcmp(var, \"rspfile_content\") == 0) hasrspcontent = true;\n"\
            "  }\n"\
            "  if (!hascommand) fatal(\"rule '%s' has no command\", r->name);\n"\
            "  if (hasrspfile != hasrspcontent) fatal(\"rule '%s' has rspfile and no rspfile_content or vice versa\", r->name);\n"\
            "  envaddrule(env, r);\n"\
            "}\n"\
            "static void parseedge(struct scanner *s, struct environment *env) {\n"\
            "  struct edge *e;\n"\
            "  struct evalstring *str, **path;\n"\
            "  char *name;\n"\
            "  struct string *val;\n"\
            "  struct node *n;\n"\
            "  size_t i;\n"\
            "  int p;\n"\
            "  e = mkedge(env);\n"\
            "  scanpaths(s);\n"\
            "  e->outimpidx = npaths;\n"\
            "  if (scanpipe(s, 1)) scanpaths(s);\n"\
            "  e->nout = npaths;\n"\
            "  if (e->nout == 0) scanerror(s, \"expected output path\");\n"\
            "  scanchar(s, ':');\n"\
            "  name = scanname(s);\n"\
            "  e->rule = envrule(env, name);\n"\
            "  if (!e->rule) fatal(\"undefined rule '%s'\", name);\n"\
            "  free(name);\n"\
            "  scanpaths(s);\n"\
            "  e->inimpidx = npaths - e->nout;\n"\
            "  p = scanpipe(s, 1 | 2);\n"\
            "  if (p == 1) {\n"\
            "    scanpaths(s);\n"\
            "    p = scanpipe(s, 2);\n"\
            "  }\n"\
            "  e->inorderidx = npaths - e->nout;\n"\
            "  if (p == 2) scanpaths(s);\n"\
            "  e->nin = npaths - e->nout;\n"\
            "  scannewline(s);\n"\
            "  while (scanindent(s)) {\n"\
            "    name = scanname(s);\n"\
            "    parselet(s, &str);\n"\
            "    val = enveval(env, str);\n"\
            "    envaddvar(e->env, name, val);\n"\
            "  }\n"\
            "  e->out = xreallocarray(NULL, e->nout, sizeof(e->out[0]));\n"\
            "  for (i = 0, path = paths; i < e->nout; ++path) {\n"\
            "    val = enveval(e->env, *path);\n"\
            "    canonpath(val);\n"\
            "    n = mknode(val);\n"\
            "    if (n->gen) {\n"\
            "      if (!parseopts.dupbuildwarn) fatal(\"multiple rules generate '%s'\", n->path->s);\n"\
            "      warn(\"multiple rules generate '%s'\", n->path->s);\n"\
            "      --e->nout;\n"\
            "      if (i < e->outimpidx) --e->outimpidx;\n"\
            "    } else {\n"\
            "      n->gen = e;\n"\
            "      e->out[i] = n;\n"\
            "      ++i;\n"\
            "    }\n"\
            "  }\n"\
            "  e->in = xreallocarray(NULL, e->nin, sizeof(e->in[0]));\n"\
            "  for (i = 0; i < e->nin; ++i, ++path) {\n"\
            "    val = enveval(e->env, *path);\n"\
            "    canonpath(val);\n"\
            "    n = mknode(val);\n"\
            "    e->in[i] = n;\n"\
            "    nodeuse(n, e);\n"\
            "  }\n"\
            "  npaths = 0;\n"\
            "  val = edgevar(e, \"pool\", true);\n"\
            "  if (val) e->pool = poolget(val->s);\n"\
            "}\n"\
            "static void parseinclude(struct scanner *s, struct environment *env, bool newscope) {\n"\
            "  struct evalstring *str;\n"\
            "  struct string *path;\n"\
            "  str = scanstring(s, true);\n"\
            "  if (!str) scanerror(s, \"expected include path\");\n"\
            "  scannewline(s);\n"\
            "  path = enveval(env, str);\n"\
            "  if (newscope) env = mkenv(env);\n"\
            "  parse(path->s, env);\n"\
            "  free(path);\n"\
            "}\n"\
            "static void parsedefault(struct scanner *s, struct environment *env) {\n"\
            "  struct string *path;\n"\
            "  struct node *n;\n"\
            "  size_t i;\n"\
            "  scanpaths(s);\n"\
            "  deftarg = xreallocarray(deftarg, ndeftarg + npaths, sizeof(*deftarg));\n"\
            "  for (i = 0; i < npaths; ++i) {\n"\
            "    path = enveval(env, paths[i]);\n"\
            "    canonpath(path);\n"\
            "    n = nodeget(path->s, path->n);\n"\
            "    if (!n) fatal(\"unknown target '%s'\", path->s);\n"\
            "    free(path);\n"\
            "    deftarg[ndeftarg++] = n;\n"\
            "  }\n"\
            "  scannewline(s);\n"\
            "  npaths = 0;\n"\
            "}\n"\
            "static void parsepool(struct scanner *s, struct environment *env) {\n"\
            "  struct pool *p;\n"\
            "  struct evalstring *val;\n"\
            "  struct string *str;\n"\
            "  char *var, *end;\n"\
            "  p = mkpool(scanname(s));\n"\
            "  scannewline(s);\n"\
            "  while (scanindent(s)) {\n"\
            "    var = scanname(s);\n"\
            "    parselet(s, &val);\n"\
            "    if (strcmp(var, \"depth\") == 0) {\n"\
            "      str = enveval(env, val);\n"\
            "      p->maxjobs = strtol(str->s, &end, 10);\n"\
            "      if (*end) fatal(\"invalid pool depth '%s'\", str->s);\n"\
            "      free(str);\n"\
            "    } else {\n"\
            "      fatal(\"unexpected pool variable '%s'\", var);\n"\
            "    }\n"\
            "  }\n"\
            "  if (!p->maxjobs) fatal(\"pool '%s' has no depth\", p->name);\n"\
            "}\n"\
            "static void checkversion(const char *ver) {\n"\
            "  int major, minor = 0;\n"\
            "  if (sscanf(ver, \"%d.%d\", &major, &minor) < 1) fatal(\"invalid ninja_required_version\");\n"\
            "  if (major > ninjamajor || (major == ninjamajor && minor > ninjaminor)) fatal(\"ninja_required_version %s is newer than %d.%d\", ver, ninjamajor, ninjaminor);\n"\
            "}\n"\
            "void parse(const char *name, struct environment *env) {\n"\
            "  struct scanner s;\n"\
            "  char *var;\n"\
            "  struct string *val;\n"\
            "  struct evalstring *str;\n"\
            "  scaninit(&s, name);\n"\
            "  for (;;) {\n"\
            "    switch (scankeyword(&s, &var)) {\n"\
            "    case RULE:\n"\
            "      parserule(&s, env);\n"\
            "      break;\n"\
            "    case BUILD:\n"\
            "      parseedge(&s, env);\n"\
            "      break;\n"\
            "    case INCLUDE:\n"\
            "      parseinclude(&s, env, false);\n"\
            "      break;\n"\
            "    case SUBNINJA:\n"\
            "      parseinclude(&s, env, true);\n"\
            "      break;\n"\
            "    case DEFAULT:\n"\
            "      parsedefault(&s, env);\n"\
            "      break;\n"\
            "    case POOL:\n"\
            "      parsepool(&s, env);\n"\
            "      break;\n"\
            "    case VARIABLE:\n"\
            "      parselet(&s, &str);\n"\
            "      val = enveval(env, str);\n"\
            "      if (strcmp(var, \"ninja_required_version\") == 0) checkversion(val->s);\n"\
            "      envaddvar(env, var, val);\n"\
            "      break;\n"\
            "    case EOF:\n"\
            "      scanclose(&s);\n"\
            "      return;\n"\
            "    }\n"\
            "  }\n"\
            "}\n"\
            "void defaultnodes(void fn(struct node *)) {\n"\
            "  struct edge *e;\n"\
            "  struct node *n;\n"\
            "  size_t i;\n"\
            "  if (ndeftarg > 0) {\n"\
            "    for (i = 0; i < ndeftarg; ++i)\n"\
            "      fn(deftarg[i]);\n"\
            "  } else {\n"\
            "    for (e = alledges; e; e = e->allnext) {\n"\
            "      for (i = 0; i < e->nout; ++i) {\n"\
            "        n = e->out[i];\n"\
            "        if (n->nuse == 0) fn(n);\n"\
            "      }\n"\
            "    }\n"\
            "  }\n"\
            "}\n"\
            "const char *argv0;\n"\
            "static void usage(void) {\n"\
            "  fprintf(stderr, \"usage: %s [-C dir] [-f buildfile] [-j maxjobs] [-k maxfail] [-l maxload] [-n]\\n\", argv0);\n"\
            "  exit(2);\n"\
            "}\n"\
            "static char *getbuilddir(void) {\n"\
            "  struct string *builddir;\n"\
            "  builddir = envvar(rootenv, \"builddir\");\n"\
            "  if (!builddir) return NULL;\n"\
            "  if (osmkdirs(builddir, false) < 0) exit(1);\n"\
            "  return builddir->s;\n"\
            "}\n"\
            "static void debugflag(const char *flag) {\n"\
            "  if (strcmp(flag, \"explain\") == 0) buildopts.explain = true;\n"\
            "  else if (strcmp(flag, \"keepdepfile\") == 0) buildopts.keepdepfile = true;\n"\
            "  else if (strcmp(flag, \"keeprsp\") == 0) buildopts.keeprsp = true;\n"\
            "  else fatal(\"unknown debug flag '%s'\", flag);\n"\
            "}\n"\
            "static void loadflag(const char *flag) {\n"\
            "#ifdef HAVE_GETLOADAVG\n"\
            "  double value;\n"\
            "  char *end;\n"\
            "  errno = 0;\n"\
            "  value = strtod(flag, &end);\n"\
            "  if (*end || value < 0 || errno != 0) fatal(\"invalid -l parameter\");\n"\
            "  buildopts.maxload = value;\n"\
            "#else\n"\
            "  warn(\"job scheduling based on load average is not supported\");\n"\
            "#endif\n"\
            "}\n"\
            "static void warnflag(const char *flag) {\n"\
            "  if (strcmp(flag, \"dupbuild=err\") == 0) parseopts.dupbuildwarn = false;\n"\
            "  else if (strcmp(flag, \"dupbuild=warn\") == 0) parseopts.dupbuildwarn = true;\n"\
            "  else fatal(\"unknown warning flag '%s'\", flag);\n"\
            "}\n"\
            "static void jobsflag(const char *flag) {\n"\
            "  long num;\n"\
            "  char *end;\n"\
            "  num = strtol(flag, &end, 10);\n"\
            "  if (*end || num < 0) fatal(\"invalid -j parameter\");\n"\
            "  buildopts.maxjobs = num > 0 ? num : -1;\n"\
            "}\n"\
            "static void parseenvargs(char *env) {\n"\
            "  char *arg, *argvbuf[64], **argv = argvbuf;\n"\
            "  int argc;\n"\
            "  if (!env) return;\n"\
            "  env = xmemdup(env, strlen(env) + 1);\n"\
            "  argc = 1;\n"\
            "  argv[0] = NULL;\n"\
            "  arg = strtok(env, \" \");\n"\
            "  while (arg) {\n"\
            "    if ((size_t)argc >= LEN(argvbuf) - 1) fatal(\"too many arguments in SAMUFLAGS\");\n"\
            "    argv[argc++] = arg;\n"\
            "    arg = strtok(NULL, \" \");\n"\
            "  }\n"\
            "  argv[argc] = NULL;\n"\
            "  ARGBEGIN {\n"\
            "  case 'j':\n"\
            "    jobsflag(EARGF(usage()));\n"\
            "    break;\n"\
            "  case 'v':\n"\
            "    buildopts.verbose = true;\n"\
            "    break;\n"\
            "  case 'l':\n"\
            "    loadflag(EARGF(usage()));\n"\
            "    break;\n"\
            "  default:\n"\
            "    fatal(\"invalid option in SAMUFLAGS\");\n"\
            "  }\n"\
            "  ARGEND\n"\
            "  free(env);\n"\
            "}\n"\
            "static const char *progname(const char *arg, const char *def) {\n"\
            "  const char *slash;\n"\
            "  if (!arg) return def;\n"\
            "  slash = strrchr(arg, '/');\n"\
            "  return slash ? slash + 1 : arg;\n"\
            "}\n"\
            "int main(int argc, char *argv[]) {\n"\
            "  char *builddir, *manifest = \"build.ninja\", *end, *arg;\n"\
            "  const struct tool *tool = NULL;\n"\
            "  struct node *n;\n"\
            "  long num;\n"\
            "  int tries;\n"\
            "  argv0 = progname(argv[0], \"samu\");\n"\
            "  parseenvargs(getenv(\"SAMUFLAGS\"));\n"\
            "  ARGBEGIN {\n"\
            "  case '-':\n"\
            "    arg = EARGF(usage());\n"\
            "    if (strcmp(arg, \"version\") == 0) {\n"\
            "      printf(\"%d.%d.0\\n\", ninjamajor, ninjaminor);\n"\
            "      return 0;\n"\
            "    } else if (strcmp(arg, \"verbose\") == 0) {\n"\
            "      buildopts.verbose = true;\n"\
            "    } else {\n"\
            "      usage();\n"\
            "    }\n"\
            "    break;\n"\
            "  case 'C':\n"\
            "    arg = EARGF(usage());\n"\
            "    warn(\"entering directory '%s'\", arg);\n"\
            "    oschdir(arg);\n"\
            "    break;\n"\
            "  case 'd':\n"\
            "    debugflag(EARGF(usage()));\n"\
            "    break;\n"\
            "  case 'f':\n"\
            "    manifest = EARGF(usage());\n"\
            "    break;\n"\
            "  case 'j':\n"\
            "    jobsflag(EARGF(usage()));\n"\
            "    break;\n"\
            "  case 'k':\n"\
            "    num = strtol(EARGF(usage()), &end, 10);\n"\
            "    if (*end) fatal(\"invalid -k parameter\");\n"\
            "    buildopts.maxfail = num > 0 ? num : -1;\n"\
            "    break;\n"\
            "  case 'l':\n"\
            "    loadflag(EARGF(usage()));\n"\
            "    break;\n"\
            "  case 'n':\n"\
            "    buildopts.dryrun = true;\n"\
            "    break;\n"\
            "  case 't':\n"\
            "    tool = toolget(EARGF(usage()));\n"\
            "    goto argdone;\n"\
            "  case 'v':\n"\
            "    buildopts.verbose = true;\n"\
            "    break;\n"\
            "  case 'w':\n"\
            "    warnflag(EARGF(usage()));\n"\
            "    break;\n"\
            "  default:\n"\
            "    usage();\n"\
            "  }\n"\
            "  ARGEND\n"\
            "argdone:\n"\
            "  if (!buildopts.maxjobs) {\n"\
            "#ifdef _SC_NPROCESSORS_ONLN\n"\
            "    long nproc = sysconf(_SC_NPROCESSORS_ONLN);\n"\
            "    switch (nproc) {\n"\
            "    case -1:\n"\
            "    case 0:\n"\
            "    case 1:\n"\
            "      buildopts.maxjobs = 2;\n"\
            "      break;\n"\
            "    case 2:\n"\
            "      buildopts.maxjobs = 3;\n"\
            "      break;\n"\
            "    default:\n"\
            "      buildopts.maxjobs = nproc + 2;\n"\
            "      break;\n"\
            "    }\n"\
            "#else\n"\
            "    buildopts.maxjobs = 2;\n"\
            "#endif\n"\
            "  }\n"\
            "  buildopts.statusfmt = getenv(\"NINJA_STATUS\");\n"\
            "  if (!buildopts.statusfmt) buildopts.statusfmt = \"[%s/%t] \";\n"\
            "  setvbuf(stdout, NULL, _IOLBF, 0);\n"\
            "  tries = 0;\n"\
            "retry:\n"\
            "  graphinit();\n"\
            "  envinit();\n"\
            "  parseinit();\n"\
            "  parse(manifest, rootenv);\n"\
            "  if (tool) return tool->run(argc, argv);\n"\
            "  builddir = getbuilddir();\n"\
            "  loginit(builddir);\n"\
            "  depsinit(builddir);\n"\
            "  n = nodeget(manifest, 0);\n"\
            "  if (n && n->gen) {\n"\
            "    buildadd(n);\n"\
            "    if (n->dirty) {\n"\
            "      build();\n"\
            "      if (n->gen->flags & FLAG_DIRTY_OUT || n->gen->nprune > 0) {\n"\
            "        if (++tries > 100) fatal(\"manifest '%s' dirty after 100 tries\", manifest);\n"\
            "        if (!buildopts.dryrun) goto retry;\n"\
            "      }\n"\
            "      buildreset();\n"\
            "    }\n"\
            "  }\n"\
            "  if (argc) {\n"\
            "    for (; *argv; ++argv) {\n"\
            "      n = nodeget(*argv, 0);\n"\
            "      if (!n) fatal(\"unknown target '%s'\", *argv);\n"\
            "      buildadd(n);\n"\
            "    }\n"\
            "  } else {\n"\
            "    defaultnodes(buildadd);\n"\
            "  }\n"\
            "  build();\n"\
            "  logclose();\n"\
            "  depsclose();\n"\
            "  return 0;\n"\
            "}\n"\
            "struct evalstring **paths;\n"\
            "size_t npaths;\n"\
            "static struct buffer buf;\n"\
            "void scaninit(struct scanner *s, const char *path) {\n"\
            "  s->path = path;\n"\
            "  s->line = 1;\n"\
            "  s->col = 1;\n"\
            "  s->f = fopen(path, \"r\");\n"\
            "  if (!s->f) fatal(\"open %s:\", path);\n"\
            "  s->chr = getc(s->f);\n"\
            "}\n"\
            "void scanclose(struct scanner *s) {\n"\
            "  fclose(s->f);\n"\
            "}\n"\
            "void scanerror(struct scanner *s, const char *fmt, ...) {\n"\
            "  extern const char *argv0;\n"\
            "  va_list ap;\n"\
            "  fprintf(stderr, \"%s: %s:%d:%d: \", argv0, s->path, s->line, s->col);\n"\
            "  va_start(ap, fmt);\n"\
            "  vfprintf(stderr, fmt, ap);\n"\
            "  va_end(ap);\n"\
            "  putc('\\n', stderr);\n"\
            "  exit(1);\n"\
            "}\n"\
            "static int next(struct scanner *s) {\n"\
            "  if (s->chr == '\\n') {\n"\
            "    ++s->line;\n"\
            "    s->col = 1;\n"\
            "  } else {\n"\
            "    ++s->col;\n"\
            "  }\n"\
            "  s->chr = getc(s->f);\n"\
            "  return s->chr;\n"\
            "}\n"\
            "static int issimplevar(int c) {\n"\
            "  return isalnum(c) || c == '_' || c == '-';\n"\
            "}\n"\
            "static int isvar(int c) {\n"\
            "  return issimplevar(c) || c == '.';\n"\
            "}\n"\
            "static bool newline(struct scanner *s) {\n"\
            "  switch (s->chr) {\n"\
            "  case '\\r':\n"\
            "    if (next(s) != '\\n') scanerror(s, \"expected '\\\\n' after '\\\\r'\");\n"\
            "  case '\\n':\n"\
            "    next(s);\n"\
            "    return true;\n"\
            "  }\n"\
            "  return false;\n"\
            "}\n"\
            "static bool singlespace(struct scanner *s) {\n"\
            "  switch (s->chr) {\n"\
            "  case '$':\n"\
            "    next(s);\n"\
            "    if (newline(s)) return true;\n"\
            "    ungetc(s->chr, s->f);\n"\
            "    s->chr = '$';\n"\
            "    return false;\n"\
            "  case ' ':\n"\
            "    next(s);\n"\
            "    return true;\n"\
            "  }\n"\
            "  return false;\n"\
            "}\n"\
            "static bool space(struct scanner *s) {\n"\
            "  if (!singlespace(s)) return false;\n"\
            "  while (singlespace(s))\n"\
            "    ;\n"\
            "  return true;\n"\
            "}\n"\
            "static bool comment(struct scanner *s) {\n"\
            "  if (s->chr != '#') return false;\n"\
            "  do\n"\
            "    next(s);\n"\
            "  while (!newline(s));\n"\
            "  return true;\n"\
            "}\n"\
            "static void name(struct scanner *s) {\n"\
            "  buf.len = 0;\n"\
            "  while (isvar(s->chr)) {\n"\
            "    bufadd(&buf, s->chr);\n"\
            "    next(s);\n"\
            "  }\n"\
            "  if (!buf.len) scanerror(s, \"expected name\");\n"\
            "  bufadd(&buf, '\\0');\n"\
            "  space(s);\n"\
            "}\n"\
            "int scankeyword(struct scanner *s, char **var) {\n"\
            "  static const struct {\n"\
            "    const char *name;\n"\
            "    int value;\n"\
            "  } keywords[] = {\n"\
            "      {\"build\", BUILD},\n"\
            "      {\"default\", DEFAULT},\n"\
            "      {\"include\", INCLUDE},\n"\
            "      {\"pool\", POOL},\n"\
            "      {\"rule\", RULE},\n"\
            "      {\"subninja\", SUBNINJA},\n"\
            "  };\n"\
            "  int low = 0, high = LEN(keywords) - 1, mid, cmp;\n"\
            "  for (;;) {\n"\
            "    switch (s->chr) {\n"\
            "    case ' ':\n"\
            "      space(s);\n"\
            "      if (!comment(s) && !newline(s)) scanerror(s, \"unexpected indent\");\n"\
            "      break;\n"\
            "    case '#':\n"\
            "      comment(s);\n"\
            "      break;\n"\
            "    case '\\r':\n"\
            "    case '\\n':\n"\
            "      newline(s);\n"\
            "      break;\n"\
            "    case EOF:\n"\
            "      return EOF;\n"\
            "    default:\n"\
            "      name(s);\n"\
            "      while (low <= high) {\n"\
            "        mid = (low + high) / 2;\n"\
            "        cmp = strcmp(buf.data, keywords[mid].name);\n"\
            "        if (cmp == 0) return keywords[mid].value;\n"\
            "        if (cmp < 0) high = mid - 1;\n"\
            "        else low = mid + 1;\n"\
            "      }\n"\
            "      *var = xmemdup(buf.data, buf.len);\n"\
            "      return VARIABLE;\n"\
            "    }\n"\
            "  }\n"\
            "}\n"\
            "char *scanname(struct scanner *s) {\n"\
            "  name(s);\n"\
            "  return xmemdup(buf.data, buf.len);\n"\
            "}\n"\
            "static void addstringpart(struct evalstring ***end, bool var) {\n"\
            "  struct evalstring *p;\n"\
            "  p = xmalloc(sizeof(*p));\n"\
            "  p->next = NULL;\n"\
            "  **end = p;\n"\
            "  if (var) {\n"\
            "    bufadd(&buf, '\\0');\n"\
            "    p->var = xmemdup(buf.data, buf.len);\n"\
            "  } else {\n"\
            "    p->var = NULL;\n"\
            "    p->str = mkstr(buf.len);\n"\
            "    memcpy(p->str->s, buf.data, buf.len);\n"\
            "    p->str->s[buf.len] = '\\0';\n"\
            "  }\n"\
            "  *end = &p->next;\n"\
            "  buf.len = 0;\n"\
            "}\n"\
            "static void escape(struct scanner *s, struct evalstring ***end) {\n"\
            "  switch (s->chr) {\n"\
            "  case '$':\n"\
            "  case ' ':\n"\
            "  case ':':\n"\
            "    bufadd(&buf, s->chr);\n"\
            "    next(s);\n"\
            "    break;\n"\
            "  case '{':\n"\
            "    if (buf.len > 0) addstringpart(end, false);\n"\
            "    while (isvar(next(s)))\n"\
            "      bufadd(&buf, s->chr);\n"\
            "    if (s->chr != '}') scanerror(s, \"invalid variable name\");\n"\
            "    next(s);\n"\
            "    addstringpart(end, true);\n"\
            "    break;\n"\
            "  case '\\r':\n"\
            "  case '\\n':\n"\
            "    newline(s);\n"\
            "    space(s);\n"\
            "    break;\n"\
            "  default:\n"\
            "    if (buf.len > 0) addstringpart(end, false);\n"\
            "    while (issimplevar(s->chr)) {\n"\
            "      bufadd(&buf, s->chr);\n"\
            "      next(s);\n"\
            "    }\n"\
            "    if (!buf.len) scanerror(s, \"invalid $ escape\");\n"\
            "    addstringpart(end, true);\n"\
            "  }\n"\
            "}\n"\
            "struct evalstring *scanstring(struct scanner *s, bool path) {\n"\
            "  struct evalstring *str = NULL, **end = &str;\n"\
            "  buf.len = 0;\n"\
            "  for (;;) {\n"\
            "    switch (s->chr) {\n"\
            "    case '$':\n"\
            "      next(s);\n"\
            "      escape(s, &end);\n"\
            "      break;\n"\
            "    case ':':\n"\
            "    case '|':\n"\
            "    case ' ':\n"\
            "      if (path) goto out;\n"\
            "    default:\n"\
            "      bufadd(&buf, s->chr);\n"\
            "      next(s);\n"\
            "      break;\n"\
            "    case '\\r':\n"\
            "    case '\\n':\n"\
            "    case EOF:\n"\
            "      goto out;\n"\
            "    }\n"\
            "  }\n"\
            "out:\n"\
            "  if (buf.len > 0) addstringpart(&end, 0);\n"\
            "  if (path) space(s);\n"\
            "  return str;\n"\
            "}\n"\
            "void scanpaths(struct scanner *s) {\n"\
            "  static size_t max;\n"\
            "  struct evalstring *str;\n"\
            "  while ((str = scanstring(s, true))) {\n"\
            "    if (npaths == max) {\n"\
            "      max = max ? max * 2 : 32;\n"\
            "      paths = xreallocarray(paths, max, sizeof(paths[0]));\n"\
            "    }\n"\
            "    paths[npaths++] = str;\n"\
            "  }\n"\
            "}\n"\
            "void scanchar(struct scanner *s, int c) {\n"\
            "  if (s->chr != c) scanerror(s, \"expected '%c'\", c);\n"\
            "  next(s);\n"\
            "  space(s);\n"\
            "}\n"\
            "int scanpipe(struct scanner *s, int n) {\n"\
            "  if (s->chr != '|') return 0;\n"\
            "  next(s);\n"\
            "  if (s->chr != '|') {\n"\
            "    if (!(n & 1)) scanerror(s, \"expected '||'\");\n"\
            "    space(s);\n"\
            "    return 1;\n"\
            "  }\n"\
            "  if (!(n & 2)) scanerror(s, \"unexpected '||'\");\n"\
            "  next(s);\n"\
            "  space(s);\n"\
            "  return 2;\n"\
            "}\n"\
            "bool scanindent(struct scanner *s) {\n"\
            "  bool indent;\n"\
            "  for (;;) {\n"\
            "    indent = space(s);\n"\
            "    if (!comment(s)) return indent && !newline(s);\n"\
            "  }\n"\
            "}\n"\
            "void scannewline(struct scanner *s) {\n"\
            "  if (!newline(s)) scanerror(s, \"expected newline\");\n"\
            "}\n"\
            "static int cleanpath(struct string *path) {\n"\
            "  if (path) {\n"\
            "    if (remove(path->s) == 0) {\n"\
            "      printf(\"remove %s\\n\", path->s);\n"\
            "    } else if (errno != ENOENT) {\n"\
            "      warn(\"remove %s:\", path->s);\n"\
            "      return -1;\n"\
            "    }\n"\
            "  }\n"\
            "  return 0;\n"\
            "}\n"\
            "static int cleanedge(struct edge *e) {\n"\
            "  int ret = 0;\n"\
            "  size_t i;\n"\
            "  for (i = 0; i < e->nout; ++i) {\n"\
            "    if (cleanpath(e->out[i]->path) < 0) ret = -1;\n"\
            "  }\n"\
            "  if (cleanpath(edgevar(e, \"rspfile\", false)) < 0) ret = -1;\n"\
            "  if (cleanpath(edgevar(e, \"depfile\", false)) < 0) ret = -1;\n"\
            "  return ret;\n"\
            "}\n"\
            "static int cleantarget(struct node *n) {\n"\
            "  int ret = 0;\n"\
            "  size_t i;\n"\
            "  if (!n->gen || n->gen->rule == &phonyrule) return 0;\n"\
            "  if (cleanpath(n->path) < 0) ret = -1;\n"\
            "  for (i = 0; i < n->gen->nin; ++i) {\n"\
            "    if (cleantarget(n->gen->in[i]) < 0) ret = -1;\n"\
            "  }\n"\
            "  return ret;\n"\
            "}\n"\
            "static int clean(int argc, char *argv[]) {\n"\
            "  int ret = 0;\n"\
            "  bool cleangen = false, cleanrule = false;\n"\
            "  struct edge *e;\n"\
            "  struct node *n;\n"\
            "  struct rule *r;\n"\
            "  ARGBEGIN {\n"\
            "  case 'g':\n"\
            "    cleangen = true;\n"\
            "    break;\n"\
            "  case 'r':\n"\
            "    cleanrule = true;\n"\
            "    break;\n"\
            "  default:\n"\
            "    fprintf(stderr, \"usage: %s ... -t clean [-gr] [targets...]\\n\", argv0);\n"\
            "    return 2;\n"\
            "  }\n"\
            "  ARGEND\n"\
            "  if (cleanrule) {\n"\
            "    if (!argc) fatal(\"expected a rule to clean\");\n"\
            "    for (; *argv; ++argv) {\n"\
            "      r = envrule(rootenv, *argv);\n"\
            "      if (!r) {\n"\
            "        warn(\"unknown rule '%s'\", *argv);\n"\
            "        ret = 1;\n"\
            "        continue;\n"\
            "      }\n"\
            "      for (e = alledges; e; e = e->allnext) {\n"\
            "        if (e->rule != r) continue;\n"\
            "        if (cleanedge(e) < 0) ret = 1;\n"\
            "      }\n"\
            "    }\n"\
            "  } else if (argc > 0) {\n"\
            "    for (; *argv; ++argv) {\n"\
            "      n = nodeget(*argv, 0);\n"\
            "      if (!n) {\n"\
            "        warn(\"unknown target '%s'\", *argv);\n"\
            "        ret = 1;\n"\
            "        continue;\n"\
            "      }\n"\
            "      if (cleantarget(n) < 0) ret = 1;\n"\
            "    }\n"\
            "  } else {\n"\
            "    for (e = alledges; e; e = e->allnext) {\n"\
            "      if (e->rule == &phonyrule) continue;\n"\
            "      if (!cleangen && edgevar(e, \"generator\", true)) continue;\n"\
            "      if (cleanedge(e) < 0) ret = 1;\n"\
            "    }\n"\
            "  }\n"\
            "  return ret;\n"\
            "}\n"\
            "static void targetcommands(struct node *n) {\n"\
            "  struct edge *e = n->gen;\n"\
            "  struct string *command;\n"\
            "  size_t i;\n"\
            "  if (!e || (e->flags & FLAG_WORK)) return;\n"\
            "  e->flags |= FLAG_WORK;\n"\
            "  for (i = 0; i < e->nin; ++i)\n"\
            "    targetcommands(e->in[i]);\n"\
            "  command = edgevar(e, \"command\", true);\n"\
            "  if (command && command->n) puts(command->s);\n"\
            "}\n"\
            "static int commands(int argc, char *argv[]) {\n"\
            "  struct node *n;\n"\
            "  if (argc > 1) {\n"\
            "    while (*++argv) {\n"\
            "      n = nodeget(*argv, 0);\n"\
            "      if (!n) fatal(\"unknown target '%s'\", *argv);\n"\
            "      targetcommands(n);\n"\
            "    }\n"\
            "  } else {\n"\
            "    defaultnodes(targetcommands);\n"\
            "  }\n"\
            "  if (fflush(stdout) || ferror(stdout)) fatal(\"write failed\");\n"\
            "  return 0;\n"\
            "}\n"\
            "static void printquoted(const char *s, size_t n, bool join) {\n"\
            "  size_t i;\n"\
            "  char c;\n"\
            "  for (i = 0; i < n; ++i) {\n"\
            "    c = s[i];\n"\
            "    switch (c) {\n"\
            "    case '\"':\n"\
            "    case '\\\\':\n"\
            "      putchar('\\\\');\n"\
            "      break;\n"\
            "    case '\\n':\n"\
            "      if (join) c = ' ';\n"\
            "      break;\n"\
            "    case '\\0':\n"\
            "      return;\n"\
            "    }\n"\
            "    putchar(c);\n"\
            "  }\n"\
            "}\n"\
            "static int compdb(int argc, char *argv[]) {\n"\
            "  char dir[1024], *p;\n"\
            "  struct edge *e;\n"\
            "  struct string *cmd, *rspfile, *content;\n"\
            "  bool expandrsp = false, first = true;\n"\
            "  int i;\n"\
            "  size_t off;\n"\
            "  ARGBEGIN {\n"\
            "  case 'x':\n"\
            "    expandrsp = true;\n"\
            "    break;\n"\
            "  default:\n"\
            "    fprintf(stderr, \"usage: %s ... -t compdb [-x] [rules...]\\n\", argv0);\n"\
            "    return 2;\n"\
            "  }\n"\
            "  ARGEND\n"\
            "  osgetcwd(dir, sizeof(dir));\n"\
            "  putchar('[');\n"\
            "  for (e = alledges; e; e = e->allnext) {\n"\
            "    if (e->nin == 0) continue;\n"\
            "    for (i = 0; i < argc; ++i) {\n"\
            "      if (strcmp(e->rule->name, argv[i]) == 0) {\n"\
            "        if (first) first = false;\n"\
            "        else putchar(',');\n"\
            "        printf(\"\\n  {\\n    \\\"directory\\\": \\\"\");\n"\
            "        printquoted(dir, -1, false);\n"\
            "        printf(\"\\\",\\n    \\\"command\\\": \\\"\");\n"\
            "        cmd = edgevar(e, \"command\", true);\n"\
            "        rspfile = expandrsp ? edgevar(e, \"rspfile\", true) : NULL;\n"\
            "        p = rspfile ? strstr(cmd->s, rspfile->s) : NULL;\n"\
            "        if (!p || p == cmd->s || p[-1] != '@') {\n"\
            "          printquoted(cmd->s, cmd->n, false);\n"\
            "        } else {\n"\
            "          off = p - cmd->s;\n"\
            "          printquoted(cmd->s, off - 1, false);\n"\
            "          content = edgevar(e, \"rspfile_content\", true);\n"\
            "          printquoted(content->s, content->n, true);\n"\
            "          off += rspfile->n;\n"\
            "          printquoted(cmd->s + off, cmd->n - off, false);\n"\
            "        }\n"\
            "        printf(\"\\\",\\n    \\\"file\\\": \\\"\");\n"\
            "        printquoted(e->in[0]->path->s, -1, false);\n"\
            "        printf(\"\\\",\\n    \\\"output\\\": \\\"\");\n"\
            "        printquoted(e->out[0]->path->s, -1, false);\n"\
            "        printf(\"\\\"\\n  }\");\n"\
            "        break;\n"\
            "      }\n"\
            "    }\n"\
            "  }\n"\
            "  puts(\"\\n]\");\n"\
            "  if (fflush(stdout) || ferror(stdout)) fatal(\"write failed\");\n"\
            "  return 0;\n"\
            "}\n"\
            "static void graphnode(struct node *n) {\n"\
            "  struct edge *e = n->gen;\n"\
            "  size_t i;\n"\
            "  const char *style;\n"\
            "  printf(\"\\\"%p\\\" [label=\\\"\", (void *)n);\n"\
            "  printquoted(n->path->s, n->path->n, false);\n"\
            "  printf(\"\\\"]\\n\");\n"\
            "  if (!e || (e->flags & FLAG_WORK)) return;\n"\
            "  e->flags |= FLAG_WORK;\n"\
            "  for (i = 0; i < e->nin; ++i)\n"\
            "    graphnode(e->in[i]);\n"\
            "  if (e->nin == 1 && e->nout == 1) {\n"\
            "    printf(\"\\\"%p\\\" -> \\\"%p\\\" [label=\\\"%s\\\"]\\n\", (void *)e->in[0], (void *)e->out[0], e->rule->name);\n"\
            "  } else {\n"\
            "    printf(\"\\\"%p\\\" [label=\\\"%s\\\", shape=ellipse]\\n\", (void *)e, e->rule->name);\n"\
            "    for (i = 0; i < e->nout; ++i)\n"\
            "      printf(\"\\\"%p\\\" -> \\\"%p\\\"\\n\", (void *)e, (void *)e->out[i]);\n"\
            "    for (i = 0; i < e->nin; ++i) {\n"\
            "      style = i >= e->inorderidx ? \" style=dotted\" : \"\";\n"\
            "      printf(\"\\\"%p\\\" -> \\\"%p\\\" [arrowhead=none%s]\\n\", (void *)e->in[i], (void *)e, style);\n"\
            "    }\n"\
            "  }\n"\
            "}\n"\
            "static int graph(int argc, char *argv[]) {\n"\
            "  struct node *n;\n"\
            "  puts(\"digraph ninja {\");\n"\
            "  puts(\"rankdir=\\\"LR\\\"\");\n"\
            "  puts(\"node [fontsize=10, shape=box, height=0.25]\");\n"\
            "  puts(\"edge [fontsize=10]\");\n"\
            "  if (argc > 1) {\n"\
            "    while (*++argv) {\n"\
            "      n = nodeget(*argv, 0);\n"\
            "      if (!n) fatal(\"unknown target '%s'\", *argv);\n"\
            "      graphnode(n);\n"\
            "    }\n"\
            "  } else {\n"\
            "    defaultnodes(graphnode);\n"\
            "  }\n"\
            "  puts(\"}\");\n"\
            "  if (fflush(stdout) || ferror(stdout)) fatal(\"write failed\");\n"\
            "  return 0;\n"\
            "}\n"\
            "static int query(int argc, char *argv[]) {\n"\
            "  struct node *n;\n"\
            "  struct edge *e;\n"\
            "  char *path;\n"\
            "  int i;\n"\
            "  size_t j, k;\n"\
            "  if (argc == 1) {\n"\
            "    fprintf(stderr, \"usage: %s ... -t query target...\\n\", argv0);\n"\
            "    exit(2);\n"\
            "  }\n"\
            "  for (i = 1; i < argc; ++i) {\n"\
            "    path = argv[i];\n"\
            "    n = nodeget(path, 0);\n"\
            "    if (!n) fatal(\"unknown target '%s'\", path);\n"\
            "    printf(\"%s:\\n\", argv[i]);\n"\
            "    e = n->gen;\n"\
            "    if (e) {\n"\
            "      printf(\"  input: %s\\n\", e->rule->name);\n"\
            "      for (j = 0; j < e->nin; ++j)\n"\
            "        printf(\"    %s\\n\", e->in[j]->path->s);\n"\
            "    }\n"\
            "    puts(\"  outputs:\");\n"\
            "    for (j = 0; j < n->nuse; ++j) {\n"\
            "      e = n->use[j];\n"\
            "      for (k = 0; k < e->nout; ++k)\n"\
            "        printf(\"    %s\\n\", e->out[k]->path->s);\n"\
            "    }\n"\
            "  }\n"\
            "  return 0;\n"\
            "}\n"\
            "static void targetsdepth(struct node *n, size_t depth, size_t indent) {\n"\
            "  struct edge *e = n->gen;\n"\
            "  size_t i;\n"\
            "  for (i = 0; i < indent; ++i)\n"\
            "    printf(\"  \");\n"\
            "  if (e) {\n"\
            "    printf(\"%s: %s\\n\", n->path->s, e->rule->name);\n"\
            "    if (depth != 1) {\n"\
            "      for (i = 0; i < e->nin; ++i)\n"\
            "        targetsdepth(e->in[i], depth - 1, indent + 1);\n"\
            "    }\n"\
            "  } else {\n"\
            "    puts(n->path->s);\n"\
            "  }\n"\
            "}\n"\
            "static void targetsusage(void) {\n"\
            "  fprintf(stderr,\n"\
            "          \"usage: %s ... -t targets [depth [maxdepth]]\\n\"\n"\
            "          \"       %s ... -t targets rule [rulename]\\n\"\n"\
            "          \"       %s ... -t targets all\\n\",\n"\
            "          argv0,\n"\
            "          argv0,\n"\
            "          argv0);\n"\
            "  exit(2);\n"\
            "}\n"\
            "static int targets(int argc, char *argv[]) {\n"\
            "  struct edge *e;\n"\
            "  size_t depth = 1, i;\n"\
            "  char *end, *mode, *name;\n"\
            "  if (argc > 3) targetsusage();\n"\
            "  mode = argv[1];\n"\
            "  if (!mode || strcmp(mode, \"depth\") == 0) {\n"\
            "    if (argc == 3) {\n"\
            "      depth = strtol(argv[2], &end, 10);\n"\
            "      if (*end) targetsusage();\n"\
            "    }\n"\
            "    for (e = alledges; e; e = e->allnext) {\n"\
            "      for (i = 0; i < e->nout; ++i) {\n"\
            "        if (e->out[i]->nuse == 0) targetsdepth(e->out[i], depth, 0);\n"\
            "      }\n"\
            "    }\n"\
            "  } else if (strcmp(mode, \"rule\") == 0) {\n"\
            "    name = argv[2];\n"\
            "    for (e = alledges; e; e = e->allnext) {\n"\
            "      if (!name) {\n"\
            "        for (i = 0; i < e->nin; ++i) {\n"\
            "          if (!e->in[i]->gen) puts(e->in[i]->path->s);\n"\
            "        }\n"\
            "      } else if (strcmp(e->rule->name, name) == 0) {\n"\
            "        for (i = 0; i < e->nout; ++i)\n"\
            "          puts(e->out[i]->path->s);\n"\
            "      }\n"\
            "    }\n"\
            "  } else if (strcmp(mode, \"all\") == 0 && argc == 2) {\n"\
            "    for (e = alledges; e; e = e->allnext) {\n"\
            "      for (i = 0; i < e->nout; ++i)\n"\
            "        printf(\"%s: %s\\n\", e->out[i]->path->s, e->rule->name);\n"\
            "    }\n"\
            "  } else {\n"\
            "    targetsusage();\n"\
            "  }\n"\
            "  if (fflush(stdout) || ferror(stdout)) fatal(\"write failed\");\n"\
            "  return 0;\n"\
            "}\n"\
            "static const struct tool tools[] = {\n"\
            "    {\"clean\", clean},\n"\
            "    {\"commands\", commands},\n"\
            "    {\"compdb\", compdb},\n"\
            "    {\"graph\", graph},\n"\
            "    {\"query\", query},\n"\
            "    {\"targets\", targets},\n"\
            "};\n"\
            "const struct tool *toolget(const char *name) {\n"\
            "  const struct tool *t;\n"\
            "  size_t i;\n"\
            "  t = NULL;\n"\
            "  for (i = 0; i < LEN(tools); ++i) {\n"\
            "    if (strcmp(name, tools[i].name) == 0) {\n"\
            "      t = &tools[i];\n"\
            "      break;\n"\
            "    }\n"\
            "  }\n"\
            "  if (!t) fatal(\"unknown tool '%s'\", name);\n"\
            "  return t;\n"\
            "}\n"\
            "#define MAXH (sizeof(void *) * 8 * 3 / 2)\n"\
            "void deltree(struct treenode *n, void delkey(void *), void delval(void *)) {\n"\
            "  if (!n) return;\n"\
            "  if (delkey) delkey(n->key);\n"\
            "  if (delval) delval(n->value);\n"\
            "  deltree(n->child[0], delkey, delval);\n"\
            "  deltree(n->child[1], delkey, delval);\n"\
            "  free(n);\n"\
            "}\n"\
            "static inline int height(struct treenode *n) {\n"\
            "  return n ? n->height : 0;\n"\
            "}\n"\
            "static int rot(struct treenode **p, struct treenode *x, int dir) {\n"\
            "  struct treenode *y = x->child[dir];\n"\
            "  struct treenode *z = y->child[!dir];\n"\
            "  int hx = x->height;\n"\
            "  int hz = height(z);\n"\
            "  if (hz > height(y->child[dir])) {\n"\
            "    x->child[dir] = z->child[!dir];\n"\
            "    y->child[!dir] = z->child[dir];\n"\
            "    z->child[!dir] = x;\n"\
            "    z->child[dir] = y;\n"\
            "    x->height = hz;\n"\
            "    y->height = hz;\n"\
            "    z->height = hz + 1;\n"\
            "  } else {\n"\
            "    x->child[dir] = z;\n"\
            "    y->child[!dir] = x;\n"\
            "    x->height = hz + 1;\n"\
            "    y->height = hz + 2;\n"\
            "    z = y;\n"\
            "  }\n"\
            "  *p = z;\n"\
            "  return z->height - hx;\n"\
            "}\n"\
            "static int balance(struct treenode **p) {\n"\
            "  struct treenode *n = *p;\n"\
            "  int h0 = height(n->child[0]);\n"\
            "  int h1 = height(n->child[1]);\n"\
            "  if (h0 - h1 + 1u < 3u) {\n"\
            "    int old = n->height;\n"\
            "    n->height = h0 < h1 ? h1 + 1 : h0 + 1;\n"\
            "    return n->height - old;\n"\
            "  }\n"\
            "  return rot(p, n, h0 < h1);\n"\
            "}\n"\
            "struct treenode *treefind(struct treenode *n, const char *key) {\n"\
            "  int c;\n"\
            "  while (n) {\n"\
            "    c = strcmp(key, n->key);\n"\
            "    if (c == 0) return n;\n"\
            "    n = n->child[c > 0];\n"\
            "  }\n"\
            "  return NULL;\n"\
            "}\n"\
            "void *treeinsert(struct treenode **rootp, char *key, void *value) {\n"\
            "  struct treenode **a[MAXH], *n = *rootp, *r;\n"\
            "  void *old;\n"\
            "  int i = 0, c;\n"\
            "  a[i++] = rootp;\n"\
            "  while (n) {\n"\
            "    c = strcmp(key, n->key);\n"\
            "    if (c == 0) {\n"\
            "      old = n->value;\n"\
            "      n->value = value;\n"\
            "      return old;\n"\
            "    }\n"\
            "    a[i++] = &n->child[c > 0];\n"\
            "    n = n->child[c > 0];\n"\
            "  }\n"\
            "  r = xmalloc(sizeof(*r));\n"\
            "  r->key = key;\n"\
            "  r->value = value;\n"\
            "  r->child[0] = r->child[1] = NULL;\n"\
            "  r->height = 1;\n"\
            "  *a[--i] = r;\n"\
            "  while (i && balance(a[--i]))\n"\
            "    ;\n"\
            "  return NULL;\n"\
            "}\n"\
            "extern const char *argv0;\n"\
            "static void vwarn(const char *fmt, va_list ap) {\n"\
            "  fprintf(stderr, \"%s: \", argv0);\n"\
            "  vfprintf(stderr, fmt, ap);\n"\
            "  if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {\n"\
            "    putc(' ', stderr);\n"\
            "    perror(NULL);\n"\
            "  } else {\n"\
            "    putc('\\n', stderr);\n"\
            "  }\n"\
            "}\n"\
            "void warn(const char *fmt, ...) {\n"\
            "  va_list ap;\n"\
            "  va_start(ap, fmt);\n"\
            "  vwarn(fmt, ap);\n"\
            "  va_end(ap);\n"\
            "}\n"\
            "void fatal(const char *fmt, ...) {\n"\
            "  va_list ap;\n"\
            "  va_start(ap, fmt);\n"\
            "  vwarn(fmt, ap);\n"\
            "  va_end(ap);\n"\
            "  exit(1);\n"\
            "}\n"\
            "void *xmalloc(size_t n) {\n"\
            "  void *p;\n"\
            "  p = malloc(n);\n"\
            "  if (!p) fatal(\"malloc:\");\n"\
            "  return p;\n"\
            "}\n"\
            "static void *reallocarray_(void *p, size_t n, size_t m) {\n"\
            "  if (m && n > SIZE_MAX / m) {\n"\
            "    errno = ENOMEM;\n"\
            "    return NULL;\n"\
            "  }\n"\
            "  return realloc(p, n * m);\n"\
            "}\n"\
            "void *xreallocarray(void *p, size_t n, size_t m) {\n"\
            "  p = reallocarray_(p, n, m);\n"\
            "  if (!p) fatal(\"reallocarray:\");\n"\
            "  return p;\n"\
            "}\n"\
            "char *xmemdup(const char *s, size_t n) {\n"\
            "  char *p;\n"\
            "  p = xmalloc(n);\n"\
            "  memcpy(p, s, n);\n"\
            "  return p;\n"\
            "}\n"\
            "int xasprintf(char **s, const char *fmt, ...) {\n"\
            "  va_list ap;\n"\
            "  int ret;\n"\
            "  size_t n;\n"\
            "  va_start(ap, fmt);\n"\
            "  ret = vsnprintf(NULL, 0, fmt, ap);\n"\
            "  va_end(ap);\n"\
            "  if (ret < 0) fatal(\"vsnprintf:\");\n"\
            "  n = ret + 1;\n"\
            "  *s = xmalloc(n);\n"\
            "  va_start(ap, fmt);\n"\
            "  ret = vsnprintf(*s, n, fmt, ap);\n"\
            "  va_end(ap);\n"\
            "  if (ret < 0 || (size_t)ret >= n) fatal(\"vsnprintf:\");\n"\
            "  return ret;\n"\
            "}\n"\
            "void bufadd(struct buffer *buf, char c) {\n"\
            "  if (buf->len >= buf->cap) {\n"\
            "    buf->cap = buf->cap ? buf->cap * 2 : 1 << 8;\n"\
            "    buf->data = realloc(buf->data, buf->cap);\n"\
            "    if (!buf->data) fatal(\"realloc:\");\n"\
            "  }\n"\
            "  buf->data[buf->len++] = c;\n"\
            "}\n"\
            "struct string *mkstr(size_t n) {\n"\
            "  struct string *str;\n"\
            "  str = xmalloc(sizeof(*str) + n + 1);\n"\
            "  str->n = n;\n"\
            "  return str;\n"\
            "}\n"\
            "void delevalstr(void *ptr) {\n"\
            "  struct evalstring *str = ptr, *p;\n"\
            "  while (str) {\n"\
            "    p = str;\n"\
            "    str = str->next;\n"\
            "    if (p->var) free(p->var);\n"\
            "    else free(p->str);\n"\
            "    free(p);\n"\
            "  }\n"\
            "}\n"\
            "void canonpath(struct string *path) {\n"\
            "  char *component[60];\n"\
            "  int n;\n"\
            "  char *s, *d, *end;\n"\
            "  if (path->n == 0) fatal(\"empty path\");\n"\
            "  s = d = path->s;\n"\
            "  end = path->s + path->n;\n"\
            "  n = 0;\n"\
            "  if (*s == '/') {\n"\
            "    ++s;\n"\
            "    ++d;\n"\
            "  }\n"\
            "  while (s < end) {\n"\
            "    switch (s[0]) {\n"\
            "    case '/':\n"\
            "      ++s;\n"\
            "      continue;\n"\
            "    case '.':\n"\
            "      switch (s[1]) {\n"\
            "      case '\\0':\n"\
            "      case '/':\n"\
            "        s += 2;\n"\
            "        continue;\n"\
            "      case '.':\n"\
            "        if (s[2] != '/' && s[2] != '\\0') break;\n"\
            "        if (n > 0) {\n"\
            "          d = component[--n];\n"\
            "        } else {\n"\
            "          *d++ = s[0];\n"\
            "          *d++ = s[1];\n"\
            "          *d++ = s[2];\n"\
            "        }\n"\
            "        s += 3;\n"\
            "        continue;\n"\
            "      }\n"\
            "    }\n"\
            "    if (n == LEN(component)) fatal(\"path has too many components: %s\", path->s);\n"\
            "    component[n++] = d;\n"\
            "    while (*s != '/' && *s != '\\0')\n"\
            "      *d++ = *s++;\n"\
            "    *d++ = *s++;\n"\
            "  }\n"\
            "  if (d == path->s) {\n"\
            "    *d++ = '.';\n"\
            "    *d = '\\0';\n"\
            "  } else {\n"\
            "    *--d = '\\0';\n"\
            "  }\n"\
            "  path->n = d - path->s;\n"\
            "}\n"\
            "int writefile(const char *name, struct string *s) {\n"\
            "  FILE *f;\n"\
            "  int ret;\n"\
            "  f = fopen(name, \"w\");\n"\
            "  if (!f) {\n"\
            "    warn(\"open %s:\", name);\n"\
            "    return -1;\n"\
            "  }\n"\
            "  ret = 0;\n"\
            "  if (s && (fwrite(s->s, 1, s->n, f) != s->n || fflush(f) != 0)) {\n"\
            "    warn(\"write %s:\", name);\n"\
            "    ret = -1;\n"\
            "  }\n"\
            "  fclose(f);\n"\
            "  return ret;\n"\
            "}\n"\
            "void osgetcwd(char *buf, size_t len) {\n"\
            "  if (!getcwd(buf, len)) fatal(\"getcwd:\");\n"\
            "}\n"\
            "void oschdir(const char *dir) {\n"\
            "  if (chdir(dir) < 0) fatal(\"chdir %s:\", dir);\n"\
            "}\n"\
            "int osmkdirs(struct string *path, bool parent) {\n"\
            "  int ret;\n"\
            "  struct stat st;\n"\
            "  char *s, *end;\n"\
            "  ret = 0;\n"\
            "  end = path->s + path->n;\n"\
            "  for (s = end - parent; s > path->s; --s) {\n"\
            "    if (*s != '/' && *s) continue;\n"\
            "    *s = '\\0';\n"\
            "    if (stat(path->s, &st) == 0) break;\n"\
            "    if (errno != ENOENT) {\n"\
            "      warn(\"stat %s:\", path->s);\n"\
            "      ret = -1;\n"\
            "      break;\n"\
            "    }\n"\
            "  }\n"\
            "  if (s > path->s && s < end) *s = '/';\n"\
            "  while (++s <= end - parent) {\n"\
            "    if (*s != '\\0') continue;\n"\
            "    if (ret == 0 && mkdir(path->s, 0777) < 0 && errno != EEXIST) {\n"\
            "      warn(\"mkdir %s:\", path->s);\n"\
            "      ret = -1;\n"\
            "    }\n"\
            "    if (s < end) *s = '/';\n"\
            "  }\n"\
            "  return ret;\n"\
            "}\n"\
            "int64_t osmtime(const char *name) {\n"\
            "  struct stat st;\n"\
            "  if (stat(name, &st) < 0) {\n"\
            "    if (errno != ENOENT) fatal(\"stat %s:\", name);\n"\
            "    return MTIME_MISSING;\n"\
            "  } else {\n"\
            "#ifdef __APPLE__\n"\
            "    return (int64_t)st.st_mtime * 1000000000 + st.st_mtimensec;\n"\
            "#elif defined(__sun)\n"\
            "    return (int64_t)st.st_mtim.__tv_sec * 1000000000 + st.st_mtim.__tv_nsec;\n"\
            "#else\n"\
            "    return (int64_t)st.st_mtim.tv_sec * 1000000000 + st.st_mtim.tv_nsec;\n"\
            "#endif\n"\
            "  }\n"\
            "}\n"\
            ""
// --- SAMURAI END ---

/* MIT License
   mate.h - Mate Implementations start here
   Guide on the `README.md`
*/
#ifdef MATE_IMPLEMENTATION
// TODO: state -> mateState
static MateConfig state = {0};

// TODO: add all to mateState
static String includes = {0};
static String libs = {0};
static StringVector sources = {0};

static Executable executable = {0};
static StaticLib staticLib = {0};

static const String MSVCStr = {.data = "cl.exe", .length = 6};
bool isMSVC() {
  return StrEq(state.compiler, MSVCStr);
}

static const String GCCStr = {.data = "gcc", .length = 3};
bool isGCC() {
  return StrEq(state.compiler, GCCStr);
}

static const String ClangStr = {.data = "clang", .length = 5};
bool isClang() {
  return StrEq(state.compiler, ClangStr);
}

static const String TCCStr = {.data = "tcc", .length = 3};
bool isTCC() {
  return StrEq(state.compiler, TCCStr);
}

bool isLinux() {
#if defined(PLATFORM_LINUX)
  return true;
#else
  return false;
#endif
}

bool isWindows() {
#if defined(PLATFORM_WIN)
  return true;
#else
  return false;
#endif
}

static String fixPathExe(String str) {
  String path = NormalizeExePath(state.arena, str);
#if defined(PLATFORM_WIN)
  return F(state.arena, "%s\\%s", GetCwd(), path.data);
#else
  return F(state.arena, "%s/%s", GetCwd(), path.data);
#endif
}

static String fixPath(String str) {
  String path = NormalizePath(state.arena, str);
#if defined(PLATFORM_WIN)
  return F(state.arena, "%s\\%s", GetCwd(), path.data);
#else
  return F(state.arena, "%s/%s", GetCwd(), path.data);
#endif
}

String ConvertNinjaPath(String str) {
#if defined(PLATFORM_WIN)
  String copy = StrNewSize(state.arena, str.data, str.length + 1);
  memmove(&copy.data[2], &copy.data[1], str.length - 1);
  copy.data[1] = '$';
  copy.data[2] = ':';
  return copy;
#else
  return str;
#endif
}

static void setDefaultState() {
  state.arena = ArenaCreate(20000 * sizeof(String));
  state.compiler = GetCompiler();

  state.mateExe = fixPathExe(S("./mate"));
  state.mateSource = fixPath(S("./mate.c"));
  state.buildDirectory = fixPath(S("./build"));
}

static MateConfig parseMateConfig(MateOptions options) {
  MateConfig result;
  result.mateExe = StrNew(state.arena, options.mateExe);
  result.compiler = StrNew(state.arena, options.compiler);
  result.mateSource = StrNew(state.arena, options.mateSource);
  result.buildDirectory = StrNew(state.arena, options.buildDirectory);
  return result;
}

void CreateConfig(MateOptions options) {
  setDefaultState();
  MateConfig config = parseMateConfig(options);

  if (!StrIsNull(config.mateExe)) {
    state.mateExe = fixPathExe(config.mateExe);
  }

  if (!StrIsNull(config.mateSource)) {
    state.mateSource = fixPath(config.mateSource);
  }

  if (!StrIsNull(config.buildDirectory)) {
    state.buildDirectory = fixPath(config.buildDirectory);
  }

  if (!StrIsNull(config.compiler)) {
    state.compiler = config.compiler;
  }

  state.initConfig = true;
}

static void readCache() {
  String mateCachePath = F(state.arena, "%s/mate-cache.ini", state.buildDirectory.data);
  errno_t err = IniParse(mateCachePath, &state.cache);
  Assert("MateReadCache: failed reading MateCache at %s, err: %d", mateCachePath.data, err);

  state.mateCache.lastBuild = IniGetLong(&state.cache, S("modify-time"));
  if (state.mateCache.lastBuild == 0) {
    state.mateCache.firstBuild = true;
    state.mateCache.lastBuild = TimeNow() / 1000;

    String modifyTime = F(state.arena, "%ld", state.mateCache.lastBuild);
    IniSet(&state.cache, S("modify-time"), modifyTime);
  }

#if defined(PLATFORM_WIN)
  if (state.mateCache.firstBuild) {
    errno_t ninjaCheck = RunCommand(S("ninja --version > nul 2> nul"));
    Assert(ninjaCheck == SUCCESS, "MateReadCache: Ninja build system not found. Please install Ninja and add it to your PATH.");
  }
#else
  state.mateCache.samuraiBuild = IniGetBool(&state.cache, S("samurai-build"));
  if (state.mateCache.samuraiBuild == false) {
    Assert(state.mateCache.firstBuild, "MateCache: This is not the first build and samurai is not compiled, could be a cache error, delete `./build` folder and rebuild `./mate.c`");

    String samuraiAmalgam = s(SAMURAI_AMALGAM);
    String sourcePath = F(state.arena, "%s/samurai.c", state.buildDirectory.data);
    errno_t errFileWrite = FileWrite(sourcePath, samuraiAmalgam);
    Assert(errFileWrite == SUCCESS, "MateReadCache: failed writing samurai source code to path %s", sourcePath.data);

    String outputPath = F(state.arena, "%s/samurai", state.buildDirectory.data);
    String compileCommand = F(state.arena, "%s \"%s\" -o \"%s\" -lrt -std=c99", state.compiler.data, sourcePath.data, outputPath.data);

    errno_t err = RunCommand(compileCommand);
    Assert(err == SUCCESS, "MateReadCache: Error meanwhile compiling samurai at %s, if you are seeing this please make an issue at github.com/TomasBorquez/mate.h", sourcePath.data);

    LogSuccess("Successfully compiled samurai");
    state.mateCache.samuraiBuild = true;
    IniSet(&state.cache, S("samurai-build"), S("true"));
  }
#endif

  err = IniWrite(mateCachePath, &state.cache);
  Assert(err == SUCCESS, "MateReadCache: Failed writing cache, err: %d", err);
}

void StartBuild() {
  LogInit();
  if (!state.initConfig) {
    setDefaultState();
  }

  state.initConfig = true;
  state.startTime = TimeNow();

  Mkdir(state.buildDirectory);
  readCache();
  reBuild();
}

static bool needRebuild() {
  File stats = {0};
  errno_t err = FileStats(state.mateSource, &stats);
  Assert(err == SUCCESS, "Aborting rebuild: Could not read fileStats for %s, error: %d", state.mateSource.data, err);

  if (stats.modifyTime <= state.mateCache.lastBuild) {
    return false;
  }

  String mateCachePath = F(state.arena, "%s/mate-cache.ini", state.buildDirectory.data);
  String modifyTime = F(state.arena, "%ld", stats.modifyTime);
  IniSet(&state.cache, S("modify-time"), modifyTime);

  err = IniWrite(mateCachePath, &state.cache);
  Assert(err == SUCCESS, "Aborting rebuild: Could not write cache for path %s, error: %d", mateCachePath.data, err);

  return true;
}

void reBuild() {
  if (state.mateCache.firstBuild || !needRebuild()) {
    return;
  }

  String mateExeNew = NormalizeExePath(state.arena, F(state.arena, "%s/mate-new", state.buildDirectory.data));
  String mateExeOld = NormalizeExePath(state.arena, F(state.arena, "%s/mate-old", state.buildDirectory.data));
  String mateExe = NormalizeExePath(state.arena, state.mateExe);

  String compileCommand;
  if (isMSVC()) {
    compileCommand = F(state.arena, "cl.exe \"%s\" /Fe:\"%s\"", state.mateSource.data, mateExeNew.data);
  } else {
    compileCommand = F(state.arena, "%s \"%s\" -o \"%s\"", state.compiler.data, state.mateSource.data, mateExeNew.data);
  }

  LogWarn("%s changed rebuilding...", state.mateSource.data);
  errno_t rebuildErr = RunCommand(compileCommand);
  Assert(rebuildErr == SUCCESS, "MateRebuild: failed command %s, err: %d", compileCommand.data, rebuildErr);

  errno_t renameErr = FileRename(mateExe, mateExeOld);
  Assert(renameErr == SUCCESS, "MateRebuild: failed renaming original executable failed, err: %d", renameErr);

  renameErr = FileRename(mateExeNew, mateExe);
  Assert(renameErr == SUCCESS, "MateRebuild: failed renaming new executable into old: %d", renameErr);

  LogInfo("Rebuild finished, running %s", mateExe.data);
  errno_t err = RunCommand(mateExe);
  exit(err);
}

void defaultStaticLib() {
  staticLib.output = S("");
  staticLib.flags = S("");
  staticLib.arFlags = S("rcs");
}

static StaticLib parseStaticLibOptions(StaticLibOptions options) {
  StaticLib result = {0};
  result.output = StrNew(state.arena, options.output);
  Assert(!StrIsNull(result.output),
         "MateParseStaticLibOptions: failed, StaticLibOptions.output should never be null, please define the output name like this: \n"
         "\n"
         "CreateStaticLib((StaticLibOptions) { .output = \"libexample\"});");
  result.flags = StrNew(state.arena, options.flags);
  result.arFlags = StrNew(state.arena, options.arFlags);
  return result;
}

String CreateStaticLib(StaticLibOptions staticLibOptions) {
  Assert(!isMSVC(), "CreateStaticLib: MSVC compiler not yet implemented for static libraries");
  Assert(state.initConfig,
         "CreateStaticLib: before creating a static library you must use StartBuild(), like this: \n"
         "\n"
         "StartBuild()\n"
         "{\n"
         " // ...\n"
         "}\n"
         "EndBuild()");

  defaultStaticLib();
  StaticLib options = parseStaticLibOptions(staticLibOptions);

  String staticLibOutput = NormalizeStaticLibPath(state.arena, options.output);
  staticLib.output = NormalizePath(state.arena, staticLibOutput);

  String flagsStr = options.flags;
  if (flagsStr.data == NULL) {
    flagsStr = S("");
  }

  if (staticLibOptions.warnings != 0) {
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
    switch (staticLibOptions.warnings) {
    case FLAG_WARNINGS_MINIMAL:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-Wall");
      break;
    case FLAG_WARNINGS:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-Wall -Wextra");
      break;
    case FLAG_WARNINGS_VERBOSE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-Wall -Wextra -Wpedantic");
      break;
    }
#elif defined(COMPILER_MSVC)
    switch (staticLibOptions.warnings) {
    case FLAG_WARNINGS_MINIMAL:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/W3");
      break;
    case FLAG_WARNINGS:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/W4");
      break;
    case FLAG_WARNINGS_VERBOSE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/Wall");
      break;
    }
#endif
  }

  if (staticLibOptions.debug != 0) {
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
    switch (staticLibOptions.debug) {
    case FLAG_DEBUG_MINIMAL:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-g1");
      break;
    case FLAG_DEBUG_MEDIUM:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-g2");
      break;
    case FLAG_DEBUG:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-g3");
      break;
    }
#elif defined(COMPILER_MSVC)
    switch (staticLibOptions.debug) {
    case FLAG_DEBUG_MINIMAL:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/Zi");
      break;
    case FLAG_DEBUG_MEDIUM:
    case FLAG_DEBUG:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/ZI");
      break;
    }
#endif
  }

  if (staticLibOptions.optimization != 0) {
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
    switch (staticLibOptions.optimization) {
    case FLAG_OPTIMIZATION_NONE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-O0");
      break;
    case FLAG_OPTIMIZATION_BASIC:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-O1");
      break;
    case FLAG_OPTIMIZATION:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-O2");
      break;
    case FLAG_OPTIMIZATION_SIZE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-Os");
      break;
    case FLAG_OPTIMIZATION_AGGRESSIVE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-O3");
      break;
    }
#elif defined(COMPILER_MSVC)
    switch (staticLibOptions.optimization) {
    case FLAG_OPTIMIZATION_NONE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/Od");
      break;
    case FLAG_OPTIMIZATION_BASIC:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/O1");
      break;
    case FLAG_OPTIMIZATION:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/O2");
      break;
    case FLAG_OPTIMIZATION_SIZE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/O1");
      break;
    case FLAG_OPTIMIZATION_AGGRESSIVE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/Ox");
      break;
    }
#endif
  }

  if (staticLibOptions.std != 0) {
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
    switch (staticLibOptions.std) {
    case FLAG_STD_C99:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-std=c99");
      break;
    case FLAG_STD_C11:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-std=c11");
      break;
    case FLAG_STD_C17:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-std=c17");
      break;
    case FLAG_STD_C23:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-std=c2x");
      break;
    case FLAG_STD_C2X:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-std=c2x");
      break;
    }
#elif defined(COMPILER_MSVC)
    switch (staticLibOptions.std) {
    case FLAG_STD_C99:
    case FLAG_STD_C11:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/std:c11");
      break;
    case FLAG_STD_C17:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/std:c17");
      break;
    case FLAG_STD_C23:
    case FLAG_STD_C2X:
      // NOTE: MSVC doesn't have C23 yet
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/std:clatest");
      break;
    }
#endif
  }

  if (!StrIsNull(flagsStr)) {
    staticLib.flags = flagsStr;
  }
  if (!StrIsNull(options.arFlags)) {
    staticLib.arFlags = options.arFlags;
  }

  String optionIncludes = s(staticLibOptions.includes);
  if (!StrIsNull(optionIncludes)) {
    includes = optionIncludes;
  }

  String optionLibs = s(staticLibOptions.libs);
  if (!StrIsNull(optionLibs)) {
    includes = optionLibs;
  }

  staticLib.ninjaBuildPath = F(state.arena, "%s/static-%s.ninja", state.buildDirectory.data, NormalizeExtension(state.arena, staticLib.output).data);
  return staticLib.ninjaBuildPath;
}

void defaultExecutable() {
  String executableOutput = NormalizeExePath(state.arena, S("main"));
  executable.output = NormalizePath(state.arena, executableOutput);
  executable.linkerFlags = S("");
  executable.flags = S("");
}

static Executable parseExecutableOptions(ExecutableOptions options) {
  Executable result;
  result.output = StrNew(state.arena, options.output);
  result.flags = StrNew(state.arena, options.flags);
  result.linkerFlags = StrNew(state.arena, options.linkerFlags);
  return result;
}

String CreateExecutable(ExecutableOptions executableOptions) {
  Assert(state.initConfig,
         "CreateExecutable: before creating an executable you must use StartBuild(), like this: \n"
         "\n"
         "StartBuild()\n"
         "{\n"
         " // ...\n"
         "}\n"
         "EndBuild()");

  defaultExecutable();
  Executable options = parseExecutableOptions(executableOptions);
  if (!StrIsNull(options.output)) {
    String executableOutput = NormalizeExePath(state.arena, options.output);
    executable.output = NormalizePath(state.arena, executableOutput);
  }

  String flagsStr = options.flags;
  if (flagsStr.data == NULL) {
    flagsStr = S("");
  }

  if (executableOptions.warnings != 0) {
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
    switch (executableOptions.warnings) {
    case FLAG_WARNINGS_MINIMAL:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-Wall");
      break;
    case FLAG_WARNINGS:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-Wall -Wextra");
      break;
    case FLAG_WARNINGS_VERBOSE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-Wall -Wextra -Wpedantic");
      break;
    }
#elif defined(COMPILER_MSVC)
    switch (executableOptions.warnings) {
    case FLAG_WARNINGS_MINIMAL:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/W3");
      break;
    case FLAG_WARNINGS:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/W4");
      break;
    case FLAG_WARNINGS_VERBOSE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/Wall");
      break;
    }
#endif
  }

  if (executableOptions.debug != 0) {
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
    switch (executableOptions.debug) {
    case FLAG_DEBUG_MINIMAL:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-g1");
      break;
    case FLAG_DEBUG_MEDIUM:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-g2");
      break;
    case FLAG_DEBUG:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-g3");
      break;
    }
#elif defined(COMPILER_MSVC)
    switch (executableOptions.debug) {
    case FLAG_DEBUG_MINIMAL:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/Zi");
      break;
    case FLAG_DEBUG_MEDIUM:
    case FLAG_DEBUG:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/ZI");
      break;
    }
#endif
  }

  if (executableOptions.optimization != 0) {
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
    switch (executableOptions.optimization) {
    case FLAG_OPTIMIZATION_NONE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-O0");
      break;
    case FLAG_OPTIMIZATION_BASIC:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-O1");
      break;
    case FLAG_OPTIMIZATION:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-O2");
      break;
    case FLAG_OPTIMIZATION_SIZE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-Os");
      break;
    case FLAG_OPTIMIZATION_AGGRESSIVE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-O3");
      break;
    }
#elif defined(COMPILER_MSVC)
    switch (executableOptions.optimization) {
    case FLAG_OPTIMIZATION_NONE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/Od");
      break;
    case FLAG_OPTIMIZATION_BASIC:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/O1");
      break;
    case FLAG_OPTIMIZATION:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/O2");
      break;
    case FLAG_OPTIMIZATION_SIZE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/O1");
      break;
    case FLAG_OPTIMIZATION_AGGRESSIVE:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/Ox");
      break;
    }
#endif
  }

  if (executableOptions.std != 0) {
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
    switch (executableOptions.std) {
    case FLAG_STD_C99:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-std=c99");
      break;
    case FLAG_STD_C11:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-std=c11");
      break;
    case FLAG_STD_C17:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-std=c17");
      break;
    case FLAG_STD_C23:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-std=c2x");
      break;
    case FLAG_STD_C2X:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "-std=c2x");
      break;
    }
#elif defined(COMPILER_MSVC)
    switch (executableOptions.std) {
    case FLAG_STD_C99:
    case FLAG_STD_C11:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/std:c11");
      break;
    case FLAG_STD_C17:
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/std:c17");
      break;
    case FLAG_STD_C23:
    case FLAG_STD_C2X:
      // NOTE: MSVC doesn't have C23 yet
      flagsStr = F(state.arena, "%s %s", flagsStr.data, "/std:clatest");
      break;
    }
#endif
  }

  if (!StrIsNull(flagsStr)) {
    executable.flags = flagsStr;
  }
  if (!StrIsNull(options.linkerFlags)) {
    executable.linkerFlags = options.linkerFlags;
  }

  String optionIncludes = s(executableOptions.includes);
  if (!StrIsNull(optionIncludes)) {
    includes = optionIncludes;
  }

  String optionLibs = s(executableOptions.libs);
  if (!StrIsNull(optionLibs)) {
    includes = optionLibs;
  }

  executable.ninjaBuildPath = F(state.arena, "%s/exe-%s.ninja", state.buildDirectory.data, NormalizeExtension(state.arena, executable.output).data);
  return executable.ninjaBuildPath;
}

errno_t CreateCompileCommands(String ninjaBuildPath) {
  FILE *ninjaPipe;
  FILE *outputFile;
  char buffer[4096];
  size_t bytes_read;

  String compileCommandsPath = NormalizePath(state.arena, F(state.arena, "%s/compile_commands.json", state.buildDirectory.data));
  errno_t err = fopen_s(&outputFile, compileCommandsPath.data, "w");
  if (err != SUCCESS || outputFile == NULL) {
    LogError("CreateCompileCommands: Failed to open file %s, err: %d", compileCommandsPath.data, err);
    return COMPILE_COMMANDS_FAILED_OPEN_FILE;
  }

  String compdbCommand;
  if (state.mateCache.samuraiBuild == true) {
    String samuraiOutputPath = F(state.arena, "%s/samurai", state.buildDirectory.data);
    compdbCommand = NormalizePath(state.arena, F(state.arena, "%s -f %s -t compdb", samuraiOutputPath.data, ninjaBuildPath.data));
  }

  if (state.mateCache.samuraiBuild == false) {
    compdbCommand = NormalizePath(state.arena, F(state.arena, "ninja -f %s -t compdb", ninjaBuildPath.data));
  }

  ninjaPipe = popen(compdbCommand.data, "r");
  if (ninjaPipe == NULL) {
    LogError("CreateCompileCommands: Failed to run compdb command, %s", compdbCommand.data);
    fclose(outputFile);
    return COMPILE_COMMANDS_FAILED_COMPDB;
  }

  while ((bytes_read = fread(buffer, 1, sizeof(buffer), ninjaPipe)) > 0) {
    fwrite(buffer, 1, bytes_read, outputFile);
  }

  fclose(outputFile);
  errno_t status = pclose(ninjaPipe);
  if (status != SUCCESS) {
    LogError("CreateCompileCommands: Command failed with status %d\n", status);
    return COMPILE_COMMANDS_FAILED_COMPDB;
  }

  LogSuccess("Successfully created %s", NormalizePathEnd(state.arena, compileCommandsPath).data);
  return SUCCESS;
}

static bool globMatch(String pattern, String text) {
  if (pattern.length == 1 && pattern.data[0] == '*') {
    return true;
  }

  size_t p = 0;
  size_t t = 0;
  size_t starP = -1;
  size_t starT = -1;
  while (t < text.length) {
    if (p < pattern.length && pattern.data[p] == text.data[t]) {
      p++;
      t++;
    } else if (p < pattern.length && pattern.data[p] == '*') {
      starP = p;
      starT = t;
      p++;
    } else if (starP != (size_t)-1) {
      p = starP + 1;
      t = ++starT;
    } else {
      return false;
    }
  }

  while (p < pattern.length && pattern.data[p] == '*') {
    p++;
  }

  return p == pattern.length;
}

static void addFile(String source) {
  bool isGlob = false;
  for (size_t i = 0; i < source.length; i++) {
    if (source.data[i] == '*') {
      isGlob = true;
      break;
    }
  }

  Assert(source.length > 2 && source.data[0] == '.' && source.data[1] == '/',
         "AddFile: failed to a add file, to add a file it has to "
         "contain the relative path, for example AddFile(\"./main.c\")");

  Assert(source.data[source.length - 1] != '/',
         "AddFile: failed to add a file, you can't add a slash at the end of a path.\n"
         "For example, valid: AddFile(\"./main.c\"), invalid: AddFile(\"./main.c/\")");

  if (!isGlob) {
    VecPush(sources, source);
    return;
  }

  String directory = {0};
  i32 lastSlash = -1;
  for (size_t i = 0; i < source.length; i++) {
    if (source.data[i] == '/') {
      lastSlash = i;
    }
  }

  directory = StrSlice(state.arena, source, 0, lastSlash);
  String pattern = StrSlice(state.arena, source, lastSlash + 1, source.length);

  StringVector files = ListDir(state.arena, directory);
  for (size_t i = 0; i < files.length; i++) {
    String file = VecAt(files, i);

    if (globMatch(pattern, file)) {
      String finalSource = F(state.arena, "%s/%s", directory.data, file.data);
      VecPush(sources, finalSource);
    }
  }
}

static bool removeFile(String source) {
  Assert(sources.length > 0, "RemoveFile: Before removing a file you must first add a file, use: AddFile()");

  for (size_t i = 0; i < sources.length; i++) {
    String *currValue = VecAtPtr(sources, i);
    if (StrEq(source, *currValue)) {
      currValue->data = NULL;
      currValue->length = 0;
      return true;
    }
  }
  return false;
}

// TODO: Create something like NormalizeOutput
static StringVector outputTransformer(StringVector vector) {
  StringVector result = {0};

  if (isMSVC()) {
    for (size_t i = 0; i < vector.length; i++) {
      String currentExecutable = VecAt(vector, i);
      if (StrIsNull(currentExecutable)) {
        VecPush(result, S(""));
        continue;
      }

      size_t lastCharIndex = 0;
      for (size_t j = currentExecutable.length - 1; j > 0; j--) {
        char currentChar = currentExecutable.data[j];
        if (currentChar == '/') {
          lastCharIndex = j;
          break;
        }
      }

      Assert(lastCharIndex != 0, "MateOutputTransformer: failed to transform %s, to an object file", currentExecutable.data);
      char *filenameStart = currentExecutable.data + lastCharIndex + 1;
      size_t filenameLength = currentExecutable.length - (lastCharIndex + 1);

      String objOutput = StrNewSize(state.arena, filenameStart, filenameLength + 2);
      objOutput.data[objOutput.length - 3] = 'o';
      objOutput.data[objOutput.length - 2] = 'b';
      objOutput.data[objOutput.length - 1] = 'j';

      VecPush(result, objOutput);
    }
    return result;
  }

  for (size_t i = 0; i < vector.length; i++) {
    String currentExecutable = VecAt(vector, i);
    if (StrIsNull(currentExecutable)) {
      VecPush(result, S(""));
      continue;
    }

    size_t lastCharIndex = 0;
    for (size_t j = currentExecutable.length - 1; j > 0; j--) {
      char currentChar = currentExecutable.data[j];
      if (currentChar == '/') {
        lastCharIndex = j;
        break;
      }
    }
    Assert(lastCharIndex != 0, "MateOutputTransformer: failed to transform %s, to an object file", currentExecutable.data);
    String output = StrSlice(state.arena, currentExecutable, lastCharIndex + 1, currentExecutable.length);
    output.data[output.length - 1] = 'o';
    VecPush(result, output);
  }
  return result;
}

void ResetExecutable() {
  executable = (Executable){0};
  VecFree(sources);
}

String InstallExecutable() {
  Assert(sources.length != 0, "InstallExecutable: Executable has zero sources, add at least one with AddFile(\"./main.c\")");
  Assert(!StrIsNull(executable.output), "InstallExecutable: Before installing executable you must first CreateExecutable()");

  StringBuilder builder = StringBuilderReserve(state.arena, 1024);

  // Compiler
  StringBuilderAppend(state.arena, &builder, &S("cc = "));
  StringBuilderAppend(state.arena, &builder, &state.compiler);
  StringBuilderAppend(state.arena, &builder, &S("\n"));

  // Linker flags
  if (executable.linkerFlags.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S("linker_flags = "));
    StringBuilderAppend(state.arena, &builder, &executable.linkerFlags);
    StringBuilderAppend(state.arena, &builder, &S("\n"));
  }

  // Compiler flags
  if (executable.flags.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S("flags = "));
    StringBuilderAppend(state.arena, &builder, &executable.flags);
    StringBuilderAppend(state.arena, &builder, &S("\n"));
  }

  // Include paths
  if (includes.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S("includes = "));
    StringBuilderAppend(state.arena, &builder, &includes);
    StringBuilderAppend(state.arena, &builder, &S("\n"));
  }

  // Libraries
  if (libs.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S("libs = "));
    StringBuilderAppend(state.arena, &builder, &libs);
    StringBuilderAppend(state.arena, &builder, &S("\n"));
  }

  // Current working directory
  String cwd_path = ConvertNinjaPath(s(GetCwd()));
  StringBuilderAppend(state.arena, &builder, &S("cwd = "));
  StringBuilderAppend(state.arena, &builder, &cwd_path);
  StringBuilderAppend(state.arena, &builder, &S("\n"));

  // Build directory
  String build_dir_path = ConvertNinjaPath(state.buildDirectory);
  StringBuilderAppend(state.arena, &builder, &S("builddir = "));
  StringBuilderAppend(state.arena, &builder, &build_dir_path);
  StringBuilderAppend(state.arena, &builder, &S("\n"));

  // Target
  StringBuilderAppend(state.arena, &builder, &S("target = $builddir/"));
  StringBuilderAppend(state.arena, &builder, &executable.output);
  StringBuilderAppend(state.arena, &builder, &S("\n\n"));

  // Link command
  StringBuilderAppend(state.arena, &builder, &S("rule link\n  command = $cc"));
  if (executable.flags.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S(" $flags"));
  }

  if (executable.linkerFlags.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S(" $linker_flags"));
  }

  if (isMSVC()) {
    StringBuilderAppend(state.arena, &builder, &S(" /Fe:$out $in"));
  } else {
    StringBuilderAppend(state.arena, &builder, &S(" -o $out $in"));
  }

  if (libs.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S(" $libs"));
  }
  StringBuilderAppend(state.arena, &builder, &S("\n\n"));

  // Compile command
  StringBuilderAppend(state.arena, &builder, &S("rule compile\n  command = $cc"));
  if (executable.flags.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S(" $flags"));
  }
  if (includes.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S(" $includes"));
  }

  if (isMSVC()) {
    StringBuilderAppend(state.arena, &builder, &S(" /c $in /Fo:$out\n\n"));
  } else {
    StringBuilderAppend(state.arena, &builder, &S(" -c $in -o $out\n\n"));
  }

  // Build individual source files
  StringVector outputFiles = outputTransformer(sources);
  StringBuilder outputBuilder = StringBuilderCreate(state.arena);
  for (size_t i = 0; i < sources.length; i++) {
    String currSource = VecAt(sources, i);
    if (StrIsNull(currSource)) continue;

    String outputFile = VecAt(outputFiles, i);
    String sourceFile = NormalizePathStart(state.arena, currSource);

    // Source build command
    StringBuilderAppend(state.arena, &builder, &S("build $builddir/"));
    StringBuilderAppend(state.arena, &builder, &outputFile);
    StringBuilderAppend(state.arena, &builder, &S(": compile $cwd/"));
    StringBuilderAppend(state.arena, &builder, &sourceFile);
    StringBuilderAppend(state.arena, &builder, &S("\n"));

    // Add to output files list
    if (outputBuilder.buffer.length == 0) {
      StringBuilderAppend(state.arena, &outputBuilder, &S("$builddir/"));
      StringBuilderAppend(state.arena, &outputBuilder, &outputFile);
    } else {
      StringBuilderAppend(state.arena, &outputBuilder, &S(" $builddir/"));
      StringBuilderAppend(state.arena, &outputBuilder, &outputFile);
    }
  }

  // Build target
  StringBuilderAppend(state.arena, &builder, &S("build $target: link "));
  StringBuilderAppend(state.arena, &builder, &outputBuilder.buffer);
  StringBuilderAppend(state.arena, &builder, &S("\n\n"));

  // Default target
  StringBuilderAppend(state.arena, &builder, &S("default $target\n"));

  String ninjaBuildPath = executable.ninjaBuildPath;
  errno_t errWrite = FileWrite(ninjaBuildPath, builder.buffer);
  Assert(errWrite == SUCCESS, "InstallExecutable: failed to write build.ninja for %s, err: %d", ninjaBuildPath.data, errWrite);

  String buildCommand;
  if (state.mateCache.samuraiBuild) {
    String samuraiOutputPath = F(state.arena, "%s/samurai", state.buildDirectory.data);
    buildCommand = F(state.arena, "%s -f %s", samuraiOutputPath.data, ninjaBuildPath.data);
  } else {
    buildCommand = F(state.arena, "ninja -f %s", ninjaBuildPath.data);
  }

  i64 err = RunCommand(buildCommand);
  Assert(err == SUCCESS, "InstallExecutable: Ninja file compilation failed with code: %lu", err);

  LogSuccess("Ninja file compilation done for %s", NormalizePathEnd(state.arena, ninjaBuildPath).data);
  state.totalTime = TimeNow() - state.startTime;

#if defined(PLATFORM_WIN)
  String path = F(state.arena, "%s\\%s", state.buildDirectory.data, executable.output.data);
#else
  String path = F(state.arena, "%s/%s", state.buildDirectory.data, executable.output.data);
#endif

  ResetExecutable();
  return path;
}

void ResetStaticLib() {
  staticLib = (StaticLib){0};
  VecFree(sources);
}

String InstallStaticLib() {
  Assert(sources.length != 0, "InstallStaticLib: Static Library has zero sources, add at least one with AddFile(\"./main.c\")");
  Assert(!StrIsNull(staticLib.output), "InstallStaticLib: Before installing static library you must first CreateStaticLib()");

  StringBuilder builder = StringBuilderReserve(state.arena, 1024);

  // Compiler
  StringBuilderAppend(state.arena, &builder, &S("cc = "));
  StringBuilderAppend(state.arena, &builder, &state.compiler);
  StringBuilderAppend(state.arena, &builder, &S("\n"));

  // Archive
  StringBuilderAppend(state.arena, &builder, &S("ar = ar\n")); // TODO: Add different ar for MSVC

  // Compiler flags
  if (staticLib.flags.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S("flags = "));
    StringBuilderAppend(state.arena, &builder, &staticLib.flags);
    StringBuilderAppend(state.arena, &builder, &S("\n"));
  }

  // Archive flags
  if (staticLib.arFlags.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S("ar_flags = "));
    StringBuilderAppend(state.arena, &builder, &staticLib.arFlags);
    StringBuilderAppend(state.arena, &builder, &S("\n"));
  }

  // Include paths
  if (includes.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S("includes = "));
    StringBuilderAppend(state.arena, &builder, &includes);
    StringBuilderAppend(state.arena, &builder, &S("\n"));
  }

  // Current working directory
  String cwd_path = ConvertNinjaPath(s(GetCwd()));
  StringBuilderAppend(state.arena, &builder, &S("cwd = "));
  StringBuilderAppend(state.arena, &builder, &cwd_path);
  StringBuilderAppend(state.arena, &builder, &S("\n"));

  // Build directory
  String build_dir_path = ConvertNinjaPath(state.buildDirectory);
  StringBuilderAppend(state.arena, &builder, &S("builddir = "));
  StringBuilderAppend(state.arena, &builder, &build_dir_path);
  StringBuilderAppend(state.arena, &builder, &S("\n"));

  // Target
  StringBuilderAppend(state.arena, &builder, &S("target = $builddir/"));
  StringBuilderAppend(state.arena, &builder, &staticLib.output);
  StringBuilderAppend(state.arena, &builder, &S("\n\n"));

  // Archive command
  StringBuilderAppend(state.arena, &builder, &S("rule archive\n  command = $ar $ar_flags $out $in\n\n"));

  // Compile command
  StringBuilderAppend(state.arena, &builder, &S("rule compile\n  command = $cc"));
  if (staticLib.flags.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S(" $flags"));
  }
  if (includes.length > 0) {
    StringBuilderAppend(state.arena, &builder, &S(" $includes"));
  }
  StringBuilderAppend(state.arena, &builder, &S(" -c $in -o $out\n\n"));

  // Build individual source files
  StringVector outputFiles = outputTransformer(sources);
  StringBuilder outputBuilder = StringBuilderCreate(state.arena);
  for (size_t i = 0; i < sources.length; i++) {
    String currSource = VecAt(sources, i);
    if (StrIsNull(currSource)) continue;

    String outputFile = VecAt(outputFiles, i);
    String sourceFile = NormalizePathStart(state.arena, currSource);

    // Source build command
    StringBuilderAppend(state.arena, &builder, &S("build $builddir/"));
    StringBuilderAppend(state.arena, &builder, &outputFile);
    StringBuilderAppend(state.arena, &builder, &S(": compile $cwd/"));
    StringBuilderAppend(state.arena, &builder, &sourceFile);
    StringBuilderAppend(state.arena, &builder, &S("\n"));

    // Add to output files list
    if (outputBuilder.buffer.length == 0) {
      StringBuilderAppend(state.arena, &outputBuilder, &S("$builddir/"));
      StringBuilderAppend(state.arena, &outputBuilder, &outputFile);
    } else {
      StringBuilderAppend(state.arena, &outputBuilder, &S(" $builddir/"));
      StringBuilderAppend(state.arena, &outputBuilder, &outputFile);
    }
  }

  // Build target
  StringBuilderAppend(state.arena, &builder, &S("build $target: archive "));
  StringBuilderAppend(state.arena, &builder, &outputBuilder.buffer);
  StringBuilderAppend(state.arena, &builder, &S("\n\n"));

  // Default target
  StringBuilderAppend(state.arena, &builder, &S("default $target\n"));

  String ninjaBuildPath = staticLib.ninjaBuildPath;
  errno_t errWrite = FileWrite(ninjaBuildPath, builder.buffer);
  Assert(errWrite == SUCCESS, "InstallStaticLib: failed to write build-static-library.ninja for %s, err: %d", ninjaBuildPath.data, errWrite);

  String buildCommand;
  if (state.mateCache.samuraiBuild) {
    String samuraiOutputPath = F(state.arena, "%s/samurai", state.buildDirectory.data);
    buildCommand = F(state.arena, "%s -f %s", samuraiOutputPath.data, ninjaBuildPath.data);
  } else {
    buildCommand = F(state.arena, "ninja -f %s", ninjaBuildPath.data);
  }

  i64 err = RunCommand(buildCommand);
  Assert(err == SUCCESS, "InstallStaticLib: Ninja file compilation failed with code: %lu", err);

  LogSuccess("Ninja file compilation done for %s", NormalizePathEnd(state.arena, ninjaBuildPath).data);
  state.totalTime = TimeNow() - state.startTime;

#if defined(PLATFORM_WIN)
  String path = F(state.arena, "%s\\%s", state.buildDirectory.data, staticLib.output.data);
#else
  String path = F(state.arena, "%s/%s", state.buildDirectory.data, staticLib.output.data);
#endif

  ResetStaticLib();
  return path;
}

errno_t RunCommand(String command) {
#if defined(PLATFORM_LINUX)
  return system(command.data) >> 8;
#else
  return system(command.data);
#endif
}

static void addLibraryPaths(StringVector *vector) {
  StringBuilder builder = StringBuilderCreate(state.arena);

  if (isMSVC() && libs.length == 0) {
    StringBuilderAppend(state.arena, &builder, &S("/link"));
  }

  if (libs.length) {
    StringBuilderAppend(state.arena, &builder, &libs);
  }

  if (isMSVC()) {
    // MSVC format: /LIBPATH:"path"
    for (size_t i = 0; i < vector->length; i++) {
      String currLib = VecAt((*vector), i);
      String buffer = F(state.arena, " /LIBPATH:\"%s\"", currLib.data);
      StringBuilderAppend(state.arena, &builder, &buffer);
    }
  } else {
    // GCC/Clang format: -L"path"
    for (size_t i = 0; i < vector->length; i++) {
      String currLib = VecAt((*vector), i);
      if (i == 0 && builder.buffer.length == 0) {
        String buffer = F(state.arena, "-L\"%s\"", currLib.data);
        StringBuilderAppend(state.arena, &builder, &buffer);
        continue;
      }
      String buffer = F(state.arena, " -L\"%s\"", currLib.data);
      StringBuilderAppend(state.arena, &builder, &buffer);
    }
  }

  libs = builder.buffer;
}

static void linkSystemLibraries(StringVector *vector) {
  StringBuilder builder = StringBuilderCreate(state.arena);

  if (isMSVC() && libs.length == 0) {
    StringBuilderAppend(state.arena, &builder, &S("/link"));
  }

  if (libs.length) {
    StringBuilderAppend(state.arena, &builder, &libs);
  }

  if (isMSVC()) {
    // MSVC format: library.lib
    for (size_t i = 0; i < vector->length; i++) {
      String currLib = VecAt((*vector), i);
      String buffer = F(state.arena, " %s.lib", currLib.data);
      StringBuilderAppend(state.arena, &builder, &buffer);
    }
  } else {
    // GCC/Clang format: -llib
    for (size_t i = 0; i < vector->length; i++) {
      String currLib = VecAt((*vector), i);
      if (i == 0 && builder.buffer.length == 0) {
        String buffer = F(state.arena, "-l%s", currLib.data);
        StringBuilderAppend(state.arena, &builder, &buffer);
        continue;
      }
      String buffer = F(state.arena, " -l%s", currLib.data);
      StringBuilderAppend(state.arena, &builder, &buffer);
    }
  }

  libs = builder.buffer;
}

static void addIncludePaths(StringVector *vector) {
  StringBuilder builder = StringBuilderCreate(state.arena);

  if (includes.length) {
    StringBuilderAppend(state.arena, &builder, &includes);
    StringBuilderAppend(state.arena, &builder, &S(" "));
  }

  if (isMSVC()) {
    // MSVC format: /I"path"
    for (size_t i = 0; i < vector->length; i++) {
      String currInclude = VecAt((*vector), i);
      if (i == 0 && builder.buffer.length == 0) {
        String buffer = F(state.arena, "/I\"%s\"", currInclude.data);
        StringBuilderAppend(state.arena, &builder, &buffer);
        continue;
      }
      String buffer = F(state.arena, " /I\"%s\"", currInclude.data);
      StringBuilderAppend(state.arena, &builder, &buffer);
    }
  } else {
    // GCC/Clang format: -I"path"
    for (size_t i = 0; i < vector->length; i++) {
      String currInclude = VecAt((*vector), i);
      if (i == 0 && builder.buffer.length == 0) {
        String buffer = F(state.arena, "-I\"%s\"", currInclude.data);
        StringBuilderAppend(state.arena, &builder, &buffer);
        continue;
      }
      String buffer = F(state.arena, " -I\"%s\"", currInclude.data);
      StringBuilderAppend(state.arena, &builder, &buffer);
    }
  }

  includes = builder.buffer;
}

void EndBuild() {
  LogInfo("Build took: %ldms", state.totalTime);
  ArenaFree(state.arena);
}
#endif
// --- MATE.H END ---
