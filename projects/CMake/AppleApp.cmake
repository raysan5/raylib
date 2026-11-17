# Package an existing C game as a UIKit app
function(raylib_add_apple_app target game_source)
    add_executable(${target} MACOSX_BUNDLE "${game_source}" "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/apple_entry.c")
    set_property(SOURCE "${game_source}" APPEND PROPERTY COMPILE_DEFINITIONS main=RaylibGameMain)
    target_link_libraries(${target} PRIVATE raylib)

    if (ARGC GREATER 2 AND IS_DIRECTORY "${ARGV2}")
        file(GLOB_RECURSE app_resources CONFIGURE_DEPENDS "${ARGV2}/*")
        foreach (resource IN LISTS app_resources)
            file(RELATIVE_PATH relative_path "${ARGV2}" "${resource}")
            get_filename_component(resource_dir "${relative_path}" DIRECTORY)
            # A top-level Resources directory can be mistaken for a macOS bundle layout
            set_source_files_properties("${resource}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/raylib_resources/${resource_dir}")
        endforeach ()
        target_sources(${target} PRIVATE ${app_resources})
    endif ()

    string(REPLACE "_" "-" bundle_name "${target}")
    if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(device_family "1,2")
    else ()
        set(device_family "3")
    endif ()
    set_target_properties(${target} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/apple_info.plist"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.raylib.${bundle_name}"
        MACOSX_BUNDLE_BUNDLE_VERSION "1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "${device_family}"
    )
    if (CMAKE_GENERATOR STREQUAL "Xcode")
        set_property(TARGET ${target} PROPERTY MACOSX_BUNDLE_GUI_IDENTIFIER "$(PRODUCT_BUNDLE_IDENTIFIER)")
        if (NOT DEFINED CMAKE_XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER)
            set_property(TARGET ${target} PROPERTY XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "org.raylib.${bundle_name}")
        endif ()
    endif ()
endfunction()
