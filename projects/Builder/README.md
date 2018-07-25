# Builder project template

This is a project template to be used with [GNOME Builder](https://raw.githubusercontent.com/jubalh/raymario/master/meson.build).
We use the [meson](https://raw.githubusercontent.com/jubalh/raymario/master/meson.build) build system here.

We can compile our project via the command line:
```
meson build
cd build
ninja
ninja install
```

Or can simply click on the `meson.build` file to open it with Builder.
Alternatively you can open Builder first and click on the `open` button and the left top.

We added comments to the file to give you an idea which values you should edit.
For a full overview of options please check the [meson manual](http://mesonbuild.com/Manual.html).

In the provided file we assume that the build file is located at the root folder of your project, and that all your sources are in a `src` subfolder.

Check out the `examples` directory for a simple example on how to use this template.
You can also look at [raymario](https://github.com/jubalh/raymario) for a slightly more complex example which also installs resource files.
