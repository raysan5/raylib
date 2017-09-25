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

# use 'meson --default-library=static builddir' to build as static, if no builddir yet exists
# use 'mesonconf -Ddefault_library=static builddir' to change the type
raylib = library('raylib',
                  source_c,
                  dependencies : [ glfw_dep, gl_dep, openal_dep, m_dep, x11_dep],
                  install : true,
                  version : '1.8.0')

