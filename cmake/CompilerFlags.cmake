include(AddIfFlagCompiles)

# Makes +/- operations on void pointers be considered an error
# https://gcc.gnu.org/onlinedocs/gcc/Pointer-Arith.html
add_if_flag_compiles(-Werror=pointer-arith CMAKE_C_FLAGS)

# Generates error whenever a function is used before being declared
# https://gcc.gnu.org/onlinedocs/gcc-4.0.1/gcc/Warning-Options.html
add_if_flag_compiles(-Werror=implicit-function-declaration CMAKE_C_FLAGS)

# Allows some casting of pointers without generating a warning
add_if_flag_compiles(-fno-strict-aliasing CMAKE_C_FLAGS)

if (ENABLE_MSAN AND ENABLE_ASAN)
    # MSAN and ASAN both work on memory - ASAN does more things
    MESSAGE(WARNING "Compiling with both AddressSanitizer and MemorySanitizer is not recommended")
endif()

if (ENABLE_ASAN)
    
    # If enabled it would generate errors/warnings for all kinds of memory errors
    # (like returning a stack variable by reference)
    # https://clang.llvm.org/docs/AddressSanitizer.html
    
    add_if_flag_compiles(-fno-omit-frame-pointer CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    add_if_flag_compiles(-fsanitize=address CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    
endif()

if (ENABLE_UBSAN)
    
    # If enabled this will generate errors for undefined behavior points
    # (like adding +1 to the maximum int value)
    # https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html
    
    add_if_flag_compiles(-fno-omit-frame-pointer CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    add_if_flag_compiles(-fsanitize=undefined CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    
endif()

if (ENABLE_MSAN)
    
    # If enabled this will generate warnings for places where uninitialized memory is used
    # https://clang.llvm.org/docs/MemorySanitizer.html
    
    add_if_flag_compiles(-fno-omit-frame-pointer CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    add_if_flag_compiles(-fsanitize=memory CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    
endif()

if(CMAKE_VERSION VERSION_LESS "3.1")
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        add_if_flag_compiles(-std=gnu99 CMAKE_C_FLAGS)
    endif()
else()
    set (CMAKE_C_STANDARD 99)
endif()

if(${PLATFORM} MATCHES "Android")
    
    # If enabled will remove dead code during the linking process
    # https://gcc.gnu.org/onlinedocs/gnat_ugn/Compilation-options.html
    add_if_flag_compiles(-ffunction-sections CMAKE_C_FLAGS)
    
    # If enabled will generate some exception data (usually disabled for C programs)
    # https://gcc.gnu.org/onlinedocs/gcc-4.2.4/gcc/Code-Gen-Options.html
    add_if_flag_compiles(-funwind-tables CMAKE_C_FLAGS)
    
    # If enabled adds stack protection guards around functions that allocate memory
    # https://www.keil.com/support/man/docs/armclang_ref/armclang_ref_cjh1548250046139.htm
    add_if_flag_compiles(-fstack-protector-strong CMAKE_C_FLAGS)
    
    # Marks that the library will not be compiled with an executable stack
    add_if_flag_compiles(-Wa,--noexecstack CMAKE_C_FLAGS)
    
    # Do not expand symbolic links or resolve paths like "/./" or "/../", etc.
    # https://gcc.gnu.org/onlinedocs/gcc/Directory-Options.html
    add_if_flag_compiles(-no-canonical-prefixes CMAKE_C_FLAGS)
endif()
