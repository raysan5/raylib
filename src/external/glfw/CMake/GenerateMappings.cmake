# Usage:
# cmake -P GenerateMappings.cmake <path/to/mappings.h.in> <path/to/mappings.h>

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
foreach(line ${lines})
    if (line MATCHES "^[0-9a-fA-F]")
        if (line MATCHES "platform:Windows")
            if (GLFW_WIN32_MAPPINGS)
                set(GLFW_WIN32_MAPPINGS "${GLFW_WIN32_MAPPINGS}\n")
            endif()
            set(GLFW_WIN32_MAPPINGS "${GLFW_WIN32_MAPPINGS}\"${line}\",")
        elseif (line MATCHES "platform:Mac OS X")
            if (GLFW_COCOA_MAPPINGS)
                set(GLFW_COCOA_MAPPINGS "${GLFW_COCOA_MAPPINGS}\n")
            endif()
            set(GLFW_COCOA_MAPPINGS "${GLFW_COCOA_MAPPINGS}\"${line}\",")
        elseif (line MATCHES "platform:Linux")
            if (GLFW_LINUX_MAPPINGS)
                set(GLFW_LINUX_MAPPINGS "${GLFW_LINUX_MAPPINGS}\n")
            endif()
            set(GLFW_LINUX_MAPPINGS "${GLFW_LINUX_MAPPINGS}\"${line}\",")
        endif()
    endif()
endforeach()

configure_file("${template_path}" "${target_path}" @ONLY NEWLINE_STYLE UNIX)
file(REMOVE "${source_path}")

