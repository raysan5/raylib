# GLFW (modified for raylib)

This directory contains a modification of GLFW, whose official website and
upstream repository are https://glfw.org and https://github.com/glfw/glfw,
respectively.

In this modification, some static functions sharing the same name in different
platforms have been renamed so all of GLFW's source files can be combined into
one (as done by ``rglfw.c``). Also, the Null platform, which is not used by
raylib, has been disabled. The renamed functions are:

``
createKeyTables()
translateKey()
acquireMonitor()
releaseMonitor()
``

