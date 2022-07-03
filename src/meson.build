raylib_src = [
  'raudio.c',
  'rcore.c',
  'rglfw.c',
  'rmodels.c',
  'rshapes.c',
  'rtext.c',
  'rtextures.c',
  'utiles.c'
]

ray_lib = static_library('raylib,
  raylib_src,
  include_directories: '.',
  install: true)

ray_dep = declare_dependency(link_with: ray_lib, include_directories: '.')
