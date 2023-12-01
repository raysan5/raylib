For the sake of this PR, this folder was added to demonstrate what a custom platform look like as well
as how to use a custom platform

* `custom_platforms/custom_platform_1`: clone of the web platform
* `custom_platforms/custom_platform_2`: clone of the desktop platform


* `examples/example_for_custom_platform_1`: example using the custom_platform_1

  How to build (use emscripten since it is a web platform: make sure you use the proper locations)
    ```text
    mkdir build
    cd build
    cmake -DCMAKE_TOOLCHAIN_FILE=/usr/local/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DEMSDK=/usr/local/emsdk -DPLATFORM=Custom -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build . --target example_for_custom_platform_1
    ```

* `examples/example_for_custom_platform_2`: example using the custom_platform_2

  How to build:
    ```text
    mkdir build
    cd build
    cmake -DPLATFORM=Custom -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build . --target example_for_custom_platform_2
    ```
