cmake_minimum_required(VERSION 3.10)
project(raylib_parser)

# Set the directory for Raylib
get_filename_component(parent_directory "${CMAKE_CURRENT_SOURCE_DIR}/../" ABSOLUTE)
set(RAYLIB_DIR ${parent_directory} CACHE STRING "Raylib directory")
set(LIBS_DIR "${RAYLIB_DIR}/../")

# Define the list of supported formats and their corresponding extensions
set(FORMATS)

# Define options for each format
option(USE_FORMAT_TXT "Enable TXT format" OFF)
option(USE_FORMAT_JSON "Enable JSON format" ON)
option(USE_FORMAT_XML "Enable XML format" OFF)
option(USE_FORMAT_LUA "Enable LUA format" OFF)

# Add selected formats to the list
message(STATUS "\nSelected formats:")
if(USE_FORMAT_TXT)
    message(STATUS " - TXT")
    list(APPEND FORMATS DEFAULT txt)
endif()
if(USE_FORMAT_JSON)
    message(STATUS " - JSON")
    list(APPEND FORMATS JSON json)
endif()
if(USE_FORMAT_XML)
    message(STATUS " - XML")
    list(APPEND FORMATS XML xml)
endif()
if(USE_FORMAT_LUA)
    message(STATUS " - LUA")
    list(APPEND FORMATS LUA lua)
endif()

# Define the list of headers with their respective targets and parameters
set(HEADERS
    raylib_api "${RAYLIB_DIR}/src/raylib.h" "-d RLAPI"
    raymath_api "${RAYLIB_DIR}/src/raymath.h" "-d RMAPI"
    rlgl_api "${RAYLIB_DIR}/src/rlgl.h" "-d RLAPI -t \"RLGL IMPLEMENTATION\""
    reasings_api "${RAYLIB_DIR}/examples/others/reasings.h" "-d EASEDEF"
)

# Set variables for library directories
set(RMEM_DIR "${LIBS_DIR}/rmem")
set(RRES_DIR "${LIBS_DIR}/rres")
set(PHYSAC_DIR "${LIBS_DIR}/Physac")
set(RAYGUI_DIR "${LIBS_DIR}/raygui")
set(RAUDIO_DIR "${LIBS_DIR}/raudio")

# Check if RMEM directory exists
if(EXISTS "${RMEM_DIR}")
    option(USE_LIBRARY_RMEM "Use RMEM library" ON)
else()
    option(USE_LIBRARY_RMEM "Use RMEM library" OFF)
endif()

# Check if RRES directory exists
if(EXISTS "${RRES_DIR}")
    option(USE_LIBRARY_RRES "Use RRES library" ON)
else()
    option(USE_LIBRARY_RRES "Use RRES library" OFF)
endif()

# Check if Physac directory exists
if(EXISTS "${PHYSAC_DIR}")
    option(USE_LIBRARY_PHYSAC "Use PHYSAC library" ON)
else()
    option(USE_LIBRARY_PHYSAC "Use PHYSAC library" OFF)
endif()

# Check if raygui directory exists
if(EXISTS "${RAYGUI_DIR}")
    option(USE_LIBRARY_RAYGUI "Use RAYGUI library" ON)
else()
    option(USE_LIBRARY_RAYGUI "Use RAYGUI library" OFF)
endif()

# Check if raudio directory exists
if(EXISTS "${RAUDIO_DIR}")
    option(USE_LIBRARY_RAUDIO "Use RAUDIO library" ON)
else()
    option(USE_LIBRARY_RAUDIO "Use RAUDIO library" OFF)
endif()

# Print out selected libraries
message(STATUS "\nSelected libraries:")
if(USE_LIBRARY_RMEM)
    message(STATUS " - RMEM library")
    list(APPEND HEADERS rmem_api "${RMEM_DIR}/src/rmem.h" "-d RMEMAPI -t \"RMEM IMPLEMENTATION\"")
endif()
if(USE_LIBRARY_RRES)
    message(STATUS " - RRES library")
    list(APPEND HEADERS rres_api "${RRES_DIR}/src/rres.h" "-d RRESAPI -t \"RRES IMPLEMENTATION\"")
endif()
if(USE_LIBRARY_PHYSAC)
    message(STATUS " - PHYSAC library")
    list(APPEND HEADERS physac_api "${PHYSAC_DIR}/src/physac.h" "-d PHYSACDEF -t \"PHYSAC IMPLEMENTATION\"")
endif()
if(USE_LIBRARY_RAYGUI)
    message(STATUS " - RAYGUI library")
    list(APPEND HEADERS raygui_api "${RAYGUI_DIR}/src/raygui.h" "-d RAYGUIAPI -t \"RAYGUI IMPLEMENTATION\"")
endif()
if(USE_LIBRARY_RAUDIO)
    message(STATUS " - RAUDIO library")
    list(APPEND HEADERS raudio_api "${RAUDIO_DIR}/src/raudio.h" "-t \"RAUDIO IMPLEMENTATION\"")
endif()

# Add the executable
add_executable(raylib_parser raylib_parser.c)

# Function to parse a header for a specific format
function(parse_header format_name extension target file params)
    add_custom_command(
        OUTPUT ${target}.${extension}
        COMMAND raylib_parser -i ${file} -o ${target}.${extension} -f ${format_name} ${params}
        DEPENDS raylib_parser ${file}
    )
    add_custom_target(${target}_${format_name} DEPENDS ${target}.${extension})
    add_dependencies(parse_${format_name} ${target}_${format_name})
endfunction()

# Custom target to parse all formats
add_custom_target(parse)

# Define headers to be parsed for each format
set(format_index 0)
set(format_group "")
set(format_group_size 2)
foreach(format ${FORMATS})
    list(APPEND format_group ${format})
    math(EXPR format_index "${format_index} + 1")
    if(format_index EQUAL format_group_size)
        message(STATUS "")
    
        list(GET format_group 0 format_name)
        message(STATUS "format_name: ${format_name}")

        list(GET format_group 1 extension)
        message(STATUS "extension: ${extension}")

        message(STATUS "")

        add_custom_target(parse_${format_name})

        set(header_index 0)
        set(header_group "")
        set(header_group_size 3)
        foreach(header ${HEADERS})
            list(APPEND header_group ${header})
            math(EXPR header_index "${header_index} + 1")
            if(header_index EQUAL header_group_size)
                list(GET header_group 0 target)
                message(STATUS "target: ${target}")

                list(GET header_group 1 file)
                message(STATUS "file: ${file}")

                list(GET header_group 2 params)
                message(STATUS "params: ${params}")

                message(STATUS "")

                parse_header(${format_name} ${extension} ${target} ${file} "${params}")
                
                set(header_index 0)
                set(header_group "")
            endif()
        endforeach()
        add_dependencies(parse parse_${format_name})

        set(format_index 0)
        set(format_group "")
    endif()
endforeach()

# Clean target
add_custom_target(clean_files
    COMMAND ${CMAKE_COMMAND} -E remove
        raylib_parser *.json *.txt *.xml *.lua
)
