include(AddIfFlagCompiles)

add_if_flag_compiles(-Werror=pointer-arith CMAKE_C_FLAGS)
add_if_flag_compiles(-Werror=implicit-function-declaration CMAKE_C_FLAGS)
# src/external/jar_xm.h does shady stuff
add_if_flag_compiles(-fno-strict-aliasing CMAKE_C_FLAGS)

if (ENABLE_ASAN)
    add_if_flag_compiles(-fno-omit-frame-pointer CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    add_if_flag_compiles(-fsanitize=address CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
endif()
if (ENABLE_UBSAN)
    add_if_flag_compiles(-fno-omit-frame-pointer CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    add_if_flag_compiles(-fsanitize=undefined CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
endif()
if (ENABLE_MSAN)
    add_if_flag_compiles(-fno-omit-frame-pointer CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
    add_if_flag_compiles(-fsanitize=memory CMAKE_C_FLAGS CMAKE_LINKER_FLAGS)
endif()

if (ENABLE_MSAN AND ENABLE_ASAN)
    MESSAGE(WARNING "Compiling with both AddressSanitizer and MemorySanitizer is not recommended")
endif()

add_definitions("-DRAYLIB_CMAKE=1")

if(CMAKE_VERSION VERSION_LESS "3.1")
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        add_if_flag_compiles(-std=gnu99 CMAKE_C_FLAGS)
    endif()
else()
    set (CMAKE_C_STANDARD 99)
endif()
