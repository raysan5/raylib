# Build and package each Apple platform and simulator variant from the raylib sources
cmake_minimum_required(VERSION 3.22)
find_program(XCODEBUILD xcodebuild REQUIRED)

set(xcframework_args)
foreach (sdk IN ITEMS macosx iphoneos iphonesimulator appletvos appletvsimulator)
    set(platform Desktop)
    set(system_args)
    set(deployment_target 17.0)
    set(architectures arm64)
    if (sdk STREQUAL "macosx")
        set(deployment_target 11.0)
    elseif (sdk MATCHES "^iphone")
        set(platform iOS)
        set(system_args -DCMAKE_SYSTEM_NAME=iOS)
    else ()
        set(platform tvOS)
        set(system_args -DCMAKE_SYSTEM_NAME=tvOS)
    endif ()
    if (sdk MATCHES "simulator$" OR sdk STREQUAL "macosx")
        list(APPEND architectures x86_64)
    endif ()

    set(build_dir "${RAYLIB_APPLE_OUTPUT}/${sdk}")
    set(install_dir "${build_dir}/install")
    execute_process(COMMAND "${CMAKE_COMMAND}" -G "Unix Makefiles"
        -S "${RAYLIB_APPLE_SOURCE}" -B "${build_dir}"
        ${system_args}
        "-DPLATFORM=${platform}"
        "-DCMAKE_BUILD_TYPE=${RAYLIB_APPLE_CONFIG}"
        "-DCMAKE_OSX_SYSROOT=${sdk}"
        "-DCMAKE_OSX_DEPLOYMENT_TARGET=${deployment_target}"
        "-DCMAKE_OSX_ARCHITECTURES=${architectures}"
        -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_INCLUDEDIR=include
        -DBUILD_EXAMPLES=OFF -DBUILD_SHARED_LIBS=OFF -DCUSTOMIZE_BUILD=ON
        "-DSUPPORT_MODULE_RAUDIO=${RAYLIB_APPLE_AUDIO}" "-DSUPPORT_MODULE_RMODELS=${RAYLIB_APPLE_MODELS}"
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(COMMAND "${CMAKE_COMMAND}" --build "${build_dir}" --target raylib COMMAND_ERROR_IS_FATAL ANY)
    execute_process(COMMAND "${CMAKE_COMMAND}" --install "${build_dir}" --prefix "${install_dir}" COMMAND_ERROR_IS_FATAL ANY)
    list(APPEND xcframework_args -library "${install_dir}/lib/libraylib.a" -headers "${install_dir}/include")
endforeach ()

set(staging_dir "${RAYLIB_APPLE_OUTPUT}/staging")
set(output_dir "${RAYLIB_APPLE_OUTPUT}/raylib.xcframework")
file(REMOVE_RECURSE "${staging_dir}/raylib.xcframework")
execute_process(COMMAND "${XCODEBUILD}" -create-xcframework ${xcframework_args}
    -output "${staging_dir}/raylib.xcframework" COMMAND_ERROR_IS_FATAL ANY
)
file(REMOVE_RECURSE "${output_dir}")
file(RENAME "${staging_dir}/raylib.xcframework" "${output_dir}")
message(STATUS "Created ${output_dir}")
