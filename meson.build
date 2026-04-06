project('raylib', 'c',
  version: '6.0.0',
  default_options: [
    'default_library=static',
  ]
)

version_array = meson.project_version().split('.')
version_major = version_array[0]
version_minor = version_array[1]
version_patch = version_array[2]
api_version = '@0@.@1@'.format(version_major, version_minor)

raylib_sources = [
  'src/raudio.c',
  'src/rcore.c',
  'src/rmodels.c',
  'src/rshapes.c',
  'src/rtext.c',
  'src/rtextures.c',
]

raylib_public_headers = [
  'src/raylib.h',
  'src/rcamera.h',
  'src/rlgl.h',
  'src/raymath.h',
]

platform = get_option('platform')
graphics = get_option('graphics')
build_shared_libs = get_option('default_library') == 'shared'

compiler_flags = []
link_flags = []
dependencies = []
include_dirs = ['src']
defines = []

if platform == 'desktop'
  defines += 'PLATFORM_DESKTOP'
  
  if host_machine.system() == 'windows'
    dependencies += cc.find_library('winmm', required: true)
    dependencies += cc.find_library('opengl32', required: true)
  elif host_machine.system() == 'darwin'
    compiler_flags += '-DGL_SILENCE_DEPRECATION'
    dependencies += dependency('appleframeworks', modules: ['OpenGL', 'Cocoa', 'IOKit', 'CoreVideo', 'CoreAudio'])
  elif host_machine.system() == 'linux'
    if get_option('glfw_backend') == 'x11'
      dependencies += dependency('x11', required: true)
    elif get_option('glfw_backend') == 'wayland'
      dependencies += dependency('wayland-client', required: true)
      dependencies += dependency('wayland-cursor', required: true)
      dependencies += dependency('xkbcommon', required: true)
    endif
    dependencies += dependency('opengl', required: true)
  endif

  dependencies += dependency('threads', required: true)

  if get_option('use_external_glfw')
    dependencies += dependency('glfw3', required: true)
  else
    glfw_sources = [
      'src/external/glfw/src/context.c',
      'src/external/glfw/src/egl_context.c',
      'src/external/glfw/src/init.c',
      'src/external/glfw/src/input.c',
      'src/external/glfw/src/monitor.c',
      'src/external/glfw/src/platform.c',
      'src/external/glfw/src/vulkan.c',
      'src/external/glfw/src/window.c',
    ]

    if host_machine.system() == 'windows'
      glfw_sources += [
        'src/external/glfw/src/win32_init.c',
        'src/external/glfw/src/win32_joystick.c',
        'src/external/glfw/src/win32_module.c',
        'src/external/glfw/src/win32_monitor.c',
        'src/external/glfw/src/win32_thread.c',
        'src/external/glfw/src/win32_time.c',
        'src/external/glfw/src/win32_window.c',
        'src/external/glfw/src/wgl_context.c',
      ]
    elif host_machine.system() == 'darwin'
      glfw_sources += [
        'src/external/glfw/src/cocoa_init.m',
        'src/external/glfw/src/cocoa_joystick.m',
        'src/external/glfw/src/cocoa_monitor.m',
        'src/external/glfw/src/cocoa_time.c',
        'src/external/glfw/src/cocoa_window.m',
        'src/external/glfw/src/nsgl_context.m',
      ]
    elif host_machine.system() == 'linux'
      glfw_sources += [
        'src/external/glfw/src/posix_module.c',
        'src/external/glfw/src/posix_poll.c',
        'src/external/glfw/src/posix_thread.c',
        'src/external/glfw/src/posix_time.c',
      ]

      if get_option('glfw_backend') == 'x11'
        glfw_sources += [
          'src/external/glfw/src/glx_context.c',
          'src/external/glfw/src/x11_init.c',
          'src/external/glfw/src/x11_monitor.c',
          'src/external/glfw/src/x11_window.c',
          'src/external/glfw/src/xkb_unicode.c',
        ]
      elif get_option('glfw_backend') == 'wayland'
        glfw_sources += [
          'src/external/glfw/src/wl_init.c',
          'src/external/glfw/src/wl_monitor.c',
          'src/external/glfw/src/wl_window.c',
        ]
      endif
    else
      glfw_sources += [
        'src/external/glfw/src/null_init.c',
        'src/external/glfw/src/null_joystick.c',
        'src/external/glfw/src/null_monitor.c',
        'src/external/glfw/src/null_window.c',
      ]
    endif

    if get_option('use_audio')
      dependencies += dependency('dl', required: false)
    endif
  endif

elif platform == 'web'
  defines += 'PLATFORM_WEB'
  
  if graphics == 'opengles2'
    defines += 'GRAPHICS_API_OPENGL_ES2'
  elif graphics == 'opengles3'
    defines += 'GRAPHICS_API_OPENGL_ES3'
  else
    defines += 'GRAPHICS_API_OPENGL_ES2'
  endif

elif platform == 'android'
  defines += 'PLATFORM_ANDROID'
  defines += 'GRAPHICS_API_OPENGL_ES2'
  
  dependencies += dependency('log', required: true)
  dependencies += dependency('android', required: true)
  dependencies += dependency('egl', required: true)
  dependencies += dependency('glesv2', required: true)
  dependencies += dependency('openal', required: false)
  dependencies += cc.find_library('atomic', required: false)

elif platform == 'drm'
  defines += 'PLATFORM_DRM'
  defines += '_DEFAULT_SOURCE'
  
  dependencies += cc.find_library('drm', required: true)
  dependencies += cc.find_library('gbm', required: false)
  dependencies += dependency('egl', required: false)
  dependencies += dependency('glesv2', required: false)
  dependencies += dependency('threads', required: true)

  if graphics == 'opengles2'
    defines += 'GRAPHICS_API_OPENGL_ES2'
    defines += 'EGL_NO_X11'
  elif graphics == 'software'
    defines += 'GRAPHICS_API_OPENGL_SOFTWARE'
  else
    defines += 'GRAPHICS_API_OPENGL_ES2'
    defines += 'EGL_NO_X11'
  endif

elif platform == 'sdl'
  defines += 'PLATFORM_DESKTOP_SDL'
  
  sdl_dep = dependency('sdl3', required: false)
  if not sdl_dep.found()
    sdl_dep = dependency('sdl2', required: true)
  endif
  dependencies += sdl_dep
  dependencies += dependency('opengl', required: true)

elif platform == 'rgfw'
  defines += 'PLATFORM_DESKTOP_RGFW'
  
  if host_machine.system() == 'darwin'
    dependencies += dependency('appleframeworks', modules: ['OpenGL', 'Cocoa'])
  elif host_machine.system() == 'windows'
    dependencies += cc.find_library('gdi32', required: true)
    dependencies += dependency('opengl', required: true)
  elif host_machine.system() == 'linux'
    dependencies += dependency('x11', required: true)
    dependencies += dependency('opengl', required: true)
  endif
endif

if graphics == 'opengl33'
  defines += 'GRAPHICS_API_OPENGL_33'
elif graphics == 'opengl43'
  defines += 'GRAPHICS_API_OPENGL_43'
elif graphics == 'opengles2'
  defines += 'GRAPHICS_API_OPENGL_ES2'
elif graphics == 'opengles3'
  defines += 'GRAPHICS_API_OPENGL_ES3'
else
  if platform == 'desktop'
    defines += 'GRAPHICS_API_OPENGL_33'
  endif
endif

if get_option('support_module_rshapes')
  defines += 'SUPPORT_MODULE_RSHAPES=1'
else
  defines += 'SUPPORT_MODULE_RSHAPES=0'
endif

if get_option('support_module_rtextures')
  defines += 'SUPPORT_MODULE_RTEXTURES=1'
else
  defines += 'SUPPORT_MODULE_RTEXTURES=0'
endif

if get_option('support_module_rtext')
  defines += 'SUPPORT_MODULE_RTEXT=1'
else
  defines += 'SUPPORT_MODULE_RTEXT=0'
endif

if get_option('support_module_rmodels')
  defines += 'SUPPORT_MODULE_RMODELS=1'
else
  defines += 'SUPPORT_MODULE_RMODELS=0'
endif

if get_option('support_module_raudio') and get_option('use_audio')
  defines += 'SUPPORT_MODULE_RAUDIO=1'
  dependencies += dependency('openal', required: false)
else
  defines += 'SUPPORT_MODULE_RAUDIO=0'
endif

if get_option('support_tracelog')
  defines += 'SUPPORT_TRACELOG=1'
else
  defines += 'SUPPORT_TRACELOG=0'
endif

if get_option('support_camera_system')
  defines += 'SUPPORT_CAMERA_SYSTEM=1'
else
  defines += 'SUPPORT_CAMERA_SYSTEM=0'
endif

if get_option('support_gestures_system')
  defines += 'SUPPORT_GESTURES_SYSTEM=1'
else
  defines += 'SUPPORT_GESTURES_SYSTEM=0'
endif

if get_option('support_fileformat_tga')
  defines += 'SUPPORT_FILEFORMAT_TGA=1'
endif

if get_option('support_fileformat_jpg')
  defines += 'SUPPORT_FILEFORMAT_JPG=1'
endif

if get_option('support_fileformat_bmp')
  defines += 'SUPPORT_FILEFORMAT_BMP=1'
endif

if get_option('support_fileformat_hdr')
  defines += 'SUPPORT_FILEFORMAT_HDR=1'
endif

if get_option('support_fileformat_ktx')
  defines += 'SUPPORT_FILEFORMAT_KTX=1'
endif

if get_option('support_fileformat_astc')
  defines += 'SUPPORT_FILEFORMAT_ASTC=1'
endif

if get_option('support_fileformat_pvr')
  defines += 'SUPPORT_FILEFORMAT_PVR=1'
endif

if get_option('support_fileformat_obj')
  defines += 'SUPPORT_FILEFORMAT_OBJ=1'
endif

if get_option('support_fileformat_gltf')
  defines += 'SUPPORT_FILEFORMAT_GLTF=1'
endif

if get_option('support_fileformat_vox')
  defines += 'SUPPORT_FILEFORMAT_VOX=1'
endif

if get_option('support_fileformat_flac')
  defines += 'SUPPORT_FILEFORMAT_FLAC=1'
endif

if get_option('support_compression_api')
  defines += 'SUPPORT_COMPRESSION_API=1'
else
  defines += 'SUPPORT_COMPRESSION_API=0'
endif

if get_option('support_automation_events')
  defines += 'SUPPORT_AUTOMATION_EVENTS=1'
else
  defines += 'SUPPORT_AUTOMATION_EVENTS=0'
endif

if build_shared_libs
  defines += 'BUILD_LIBTYPE_SHARED'
  compiler_flags += '-fvisibility=hidden'
endif

if get_option('use_audio')
  dependencies += dependency('m', required: false)
endif

if not get_option('use_external_glfw') and platform == 'desktop'
  glfw_args = []
  if host_machine.system() == 'linux' and get_option('glfw_backend') == 'x11'
    glfw_args = ['-D_GLFW_X11']
  elif host_machine.system() == 'linux' and get_option('glfw_backend') == 'wayland'
    glfw_args = ['-D_GLFW_WAYLAND']
  elif host_machine.system() == 'windows'
    glfw_args = ['-D_GLFW_WIN32']
  elif host_machine.system() == 'darwin'
    glfw_args = ['-D_GLFW_COCOA']
  else
    glfw_args = ['-D_GLFW_null']
  endif
  
  glfw_dep = static_library('glfw', glfw_sources,
    include_directories: ['src/external/glfw/include'],
    c_args: glfw_args + ['-DGLFW_BUILD_DLL=0'],
    install: false,
  )

  dependencies += declare_dependency(include_directories: ['src/external/glfw/include'],
    link_with: glfw_dep)
  include_dirs += 'src/external/glfw/include'
endif

raylib_kwargs = {
  'sources': raylib_sources,
  'dependencies': dependencies,
  'include_directories': include_dirs,
  'c_args': compiler_flags,
  'defines': defines,
}

defines_args = []
foreach d : defines
  defines_args += '-D' + d
endforeach
all_c_args = compiler_flags + defines_args

if build_shared_libs
  raylib_lib = shared_library('raylib',
    sources: raylib_sources,
    dependencies: dependencies,
    include_directories: include_dirs,
    c_args: all_c_args,
    version: meson.project_version(),
    soversion: api_version,
  )
else
  raylib_lib = static_library('raylib',
    sources: raylib_sources,
    dependencies: dependencies,
    include_directories: include_dirs,
    c_args: all_c_args,
  )
endif

raylib_dep = declare_dependency(
  include_directories: include_dirs,
  sources: raylib_public_headers,
  dependencies: dependencies,
  link_with: raylib_lib,
)

install_headers(raylib_public_headers, subdir: 'raylib')

pkgconfig = import('pkgconfig')
pkgconfig.generate(raylib_lib,
  name: 'raylib',
  description: 'Simple and easy-to-use library to enjoy videogames programming',
  version: meson.project_version(),
  filebase: 'raylib',
  libraries: dependencies,
)
