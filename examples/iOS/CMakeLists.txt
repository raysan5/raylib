cmake_minimum_required(VERSION 3.25)

project(raylib_ios_example LANGUAGES C OBJC)

set(raylib_ios_example_sources
    ${CMAKE_CURRENT_LIST_DIR}/main.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/rcore.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/rmodels.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/rshapes.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/rtext.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/rtextures.c
    ${CMAKE_CURRENT_LIST_DIR}/../../src/rcore_ios_main.m
    ${CMAKE_CURRENT_LIST_DIR}/../../logo/raylib_1024x1024.png
    ${CMAKE_CURRENT_LIST_DIR}/../../logo/raylib_512x512.png
    ${CMAKE_CURRENT_LIST_DIR}/../../logo/raylib_180x180.png
)

find_library(UIKIT_LIBRARY UIKit)
find_library(QUARTZCORE_LIBRARY QuartzCore)
find_library(OPENGLES_LIBRARY OpenGLES)
find_library(COREMOTION_LIBRARY CoreMotion)
find_library(FOUNDATION_LIBRARY Foundation)

add_executable(raylib-ios-example MACOSX_BUNDLE ${raylib_ios_example_sources})

target_include_directories(raylib-ios-example PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/../../src
)

target_compile_definitions(raylib-ios-example PRIVATE
    PLATFORM_IOS
    GRAPHICS_API_OPENGL_ES3
    SUPPORT_MODULE_RAUDIO=0 # FIXME: Currently the mini_audio backend is not working on iOS, so we disable audio support for this example
    GLES_SILENCE_DEPRECATION=1
)

target_compile_options(raylib-ios-example PRIVATE -Wno-deprecated-declarations)

set_source_files_properties(
    ${CMAKE_CURRENT_LIST_DIR}/../../logo/raylib_1024x1024.png
    ${CMAKE_CURRENT_LIST_DIR}/../../logo/raylib_512x512.png
    ${CMAKE_CURRENT_LIST_DIR}/../../logo/raylib_180x180.png
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources
)

target_link_libraries(raylib-ios-example PRIVATE
    ${UIKIT_LIBRARY}
    ${QUARTZCORE_LIBRARY}
    ${OPENGLES_LIBRARY}
    ${COREMOTION_LIBRARY}
    ${FOUNDATION_LIBRARY}
)

set_target_properties(raylib-ios-example PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/Info.plist"
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
    XCODE_ATTRIBUTE_GENERATE_INFOPLIST_FILE "NO"
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "13.0"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.raylib.raylib-ios-example"
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
)