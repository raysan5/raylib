
# NOTE: The order of this list determines the order of items in the Guides
#       (i.e. Pages) list in the generated documentation
set(GLFW_DOXYGEN_SOURCES
    "include/GLFW/glfw3.h"
    "include/GLFW/glfw3native.h"
    "docs/main.dox"
    "docs/news.dox"
    "docs/quick.dox"
    "docs/moving.dox"
    "docs/compile.dox"
    "docs/build.dox"
    "docs/intro.dox"
    "docs/context.dox"
    "docs/monitor.dox"
    "docs/window.dox"
    "docs/input.dox"
    "docs/vulkan.dox"
    "docs/compat.dox"
    "docs/internal.dox")

# Format the source list into a Doxyfile INPUT value that Doxygen can parse
foreach(path IN LISTS GLFW_DOXYGEN_SOURCES)
    set(GLFW_DOXYGEN_INPUT "${GLFW_DOXYGEN_INPUT} \\\n\"${GLFW_SOURCE_DIR}/${path}\"")
endforeach()

configure_file(Doxyfile.in Doxyfile @ONLY)

add_custom_target(docs ALL "${DOXYGEN_EXECUTABLE}"
                  WORKING_DIRECTORY "${GLFW_BINARY_DIR}/docs"
                  COMMENT "Generating HTML documentation" VERBATIM)

set_target_properties(docs PROPERTIES FOLDER "GLFW3")

