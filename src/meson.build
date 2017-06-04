install_headers('raylib.h')

source_c = [
  'audio.c',
  'core.c',
  'models.c',
  'rlgl.c',
  'shapes.c',
  'text.c',
  'textures.c',
  'utils.c',
  'external/stb_vorbis.c',
]

raylib = shared_library('raylib',
                        source_c,
                        dependencies : [ glfw_dep, gl_dep, openal_dep, m_dep, x11_dep],
                        install : true)

