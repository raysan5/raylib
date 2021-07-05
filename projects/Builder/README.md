# Builder project template

This is a project template to be used with [GNOME Builder](https://raw.githubusercontent.com/jubalh/raymario/master/meson.build).
It uses the [meson](https://raw.githubusercontent.com/jubalh/raymario/master/meson.build) build system.

Project can be compiled via the command line using:
```
meson build
cd build
ninja
ninja install
```

Or it can be opened with Building simply clicking on the `meson.build` file.
Alternatively, open Builder first and click on the `open` button at the top-left.

There are comments to the `meson.build` file to note the values that should be edited.
For a full overview of options please check the [meson manual](http://mesonbuild.com/Manual.html).

In the provided file, it's assumed that the build file is located at the root folder of the project, and that all the sources are in a `src` subfolder.

Check out the `examples` directory for a simple example on how to use this template.

You can also look at [raymario](https://github.com/jubalh/raymario) for a slightly more complex example which also installs resource files.
