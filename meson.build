project('raylib', 'c', version:	'1.8.0',
		license: 'zlib',
		meson_version: '>= 0.39.1',
		default_options : 'c_std=gnu99')

cc = meson.get_compiler('c')

glfw_dep = dependency('glfw3', version : '>=3.2')
gl_dep = dependency('gl')
openal_dep = dependency('openal')
x11_dep = dependency('x11')
m_dep = cc.find_library('m', required : false)

subdir('src')

