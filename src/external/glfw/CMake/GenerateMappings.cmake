# Usage:
# cmake -P GenerateMappings.cmake <path/to/mappings.h.in> <path/to/mappings.h>

cmake_policy(VERSION 3.16)

set(source_url "https://raw.githubusercontent.com/gabomdq/SDL_GameControllerDB/master/gamecontrollerdb.txt")
set(source_path "${CMAKE_CURRENT_BINARY_DIR}/gamecontrollerdb.txt")
set(template_path "${CMAKE_ARGV3}")
set(target_path "${CMAKE_ARGV4}")

if (NOT EXISTS "${template_path}")
    message(FATAL_ERROR "Failed to find template file ${template_path}")
endif()

file(DOWNLOAD "${source_url}" "${source_path}"
     STATUS download_status
     TLS_VERIFY on)

list(GET download_status 0 status_code)
list(GET download_status 1 status_message)

if (status_code)
    message(FATAL_ERROR "Failed to download ${source_url}: ${status_message}")
endif()

file(STRINGS "${source_path}" lines)
list(FILTER lines INCLUDE REGEX "^[0-9a-fA-F]")

foreach(line IN LISTS lines)
    if (line MATCHES "platform:Windows")
        if (GLFW_WIN32_MAPPINGS)
            string(APPEND GLFW_WIN32_MAPPINGS "\n")
        endif()
        string(APPEND GLFW_WIN32_MAPPINGS "\"${line}\",")
    elseif (line MATCHES "platform:Mac OS X")
        if (GLFW_COCOA_MAPPINGS)
            string(APPEND GLFW_COCOA_MAPPINGS "\n")
        endif()
        string(APPEND GLFW_COCOA_MAPPINGS "\"${line}\",")
    elseif (line MATCHES "platform:Linux")
        if (GLFW_LINUX_MAPPINGS)
            string(APPEND GLFW_LINUX_MAPPINGS "\n")
        endif()
        string(APPEND GLFW_LINUX_MAPPINGS "\"${line}\",")
    endif()
endforeach()

configure_file("${template_path}" "${target_path}" @ONLY NEWLINE_STYLE UNIX)
file(REMOVE "${source_path}")

