# This file should be in the main folder of your project

# Replace 'projectname' with the name of your project
# Replace '1.0' with its version
project('core_basic_window', 'c', version: '1.0',
        meson_version: '>= 0.39.1')

# We want a C Compiler to be present
cc = meson.get_compiler('c')

# Find dependencies
glfw_dep = dependency('glfw3')
gl_dep = dependency('gl')
openal_dep = dependency('openal')
m_dep = cc.find_library('m', required : false)
raylib_dep = cc.find_library('raylib', required : false)

# List your source files here
source_c = [
  '../../../examples/core/core_basic_window.c',
]

# Build executable
core_basic_window = executable('core_basic_window',
  source_c,
  dependencies : [ raylib_dep, glfw_dep, gl_dep, openal_dep, m_dep ],
  install : true)
