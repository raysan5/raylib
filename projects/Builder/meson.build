# This file should be in the main folder of your project

# Replace 'projectname' with the name of your project
# Replace '1.0' with its version
project('projectname', 'c', version: '1.0',
        meson_version: '>= 0.39.1')

# We want a C Compiler to be present
cc = meson.get_compiler('c')

# Find dependencies
gl_dep = dependency('gl')
m_dep = cc.find_library('m', required : false)
raylib_dep = cc.find_library('raylib', required : false)

# List your source files here
source_c = [
  'src/main.c',
]

# Build executable
projectname = executable('projectname',
  source_c,
  dependencies : [ raylib_dep, gl_dep, m_dep ],
  install : true)
