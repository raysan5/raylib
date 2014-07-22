<img src="http://www.raylib.com/img/fb_raylib_logo.png" width=256>

about
-----

raylib is a simple and easy-to-use library to learn videogames programming.

raylib is highly inspired by Borland BGI graphics lib (more specifically WinBGI) and by XNA framework. 
Allegro and SDL have also been analyzed for reference.

Want to see how easy is making games with raylib? Jump to [code examples!] (http://www.raylib.com/examples.htm)

history
-------

I've developed videogames for some years and last year I had to taught videogames development 
to young people with artistic profile, most of them had never written a single line of code.

I started with C language basis and, after searching for the most simple and easy-to-use library to teach 
videogames programming, I found WinBGI; it was great and it worked very well with students, in just a 
couple of weeks that people that had never written a single line of code were able to program (and understand) 
a simple PONG and some of them even a BREAKOUT!

But WinBGI was not the clearer and most organized lib. There were a lot of things I found useless and 
confusing and some function names were not clear enough for most of the students; not to mention points 
like no transparencies support or no hardware acceleration.

So, I decided to create my own lib, hardware accelerated, clear function names, quite organized, well structured, 
plain C coding and, the most important, primarily intended to LEARN videogames programming.

I've coded quite a lot in C# and XNA and I really love it (in fact, my students learn C# after C), 
so, I decided to use C# language notation and XNA naming conventions. That way, students can jump from 
raylib to XNA, MonoGame or similar libs extremely easily.

raylib started as a weekend project and after three months of hard work, first version was published.

Enjoy it.

notes on raylib 1.1
-------------------

On April 2014, after 6 month of first raylib release, raylib 1.1 has been released. This new version presents a
complete internal redesign of the library to support OpenGL 1.1, OpenGL 3.3+ and OpenGL ES 2.0.

A new module named [rlgl] (https://github.com/raysan5/raylib/blob/master/src/rlgl.h) has been added to the library. This new module translate raylib-OpenGL-style 
immediate mode functions (i.e. rlVertex3f(), rlBegin(), ...) to different versions of OpenGL (1.1, 3.3+, ES2), selectable by one define.

[rlgl] (https://github.com/raysan5/raylib/blob/master/src/rlgl.h) also comes with a second new module named [raymath] (https://github.com/raysan5/raylib/blob/master/src/raymath.h), which includes
a bunch of useful functions for 3d-math with vectors, matrices and quaternions.

Some other big changes of this new version have been the support for OGG files loading and stream playing, and the
support of DDS texture files (compressed and uncompressed) along with mipmaps support.

Lots of code changes and lot of testing have concluded in this amazing new raylib 1.1.

Enjoy it.

features
--------
 
   *  Written in plain C code (C99)
   *  Uses C# PascalCase/camelCase notation
   *  Hardware accelerated with OpenGL (1.1, 3.3+ or ES2)
   *  Unique OpenGL abstraction layer [rlgl]
   *  Powerful fonts module with SpriteFonts support
   *  Multiple textures support, including DDS and mipmaps generation
   *  Basic 3d support for Shapes, Models, Heightmaps and Billboards
   *  Powerful math module for Vector and Matrix operations [raymath]
   *  Audio loading and playing with streaming support
   *  Custom color palette for fancy visuals on raywhite background

raylib uses on its core module the outstanding [GLFW3] (http://www.glfw.org/) library. The best option by far I found for 
window/context and input management (clean, focused, great license, well documented, modern, ...). 

raylib is licensed under a zlib/libpng license like GLFW3. View [LICENSE] (https://github.com/raysan5/raylib/blob/master/LICENSE.md).

tool requirements
------------------

raylib has been developed using exclusively two tools: 

   * Notepad++ (text editor) - [http://notepad-plus-plus.org/](http://notepad-plus-plus.org/)
   * MinGW (GCC compiler) - [http://www.mingw.org/](http://www.mingw.org/)
   
Those are the tools I recommend to develop with raylib, in fact, those are the tools my students use. 
I believe those are the best tools to train spartan-programmers.

Someone could argue about debugging. raylib is a library intended for learning and I think C it's a clear enough language
to allow writing small-mid size programs with a printf-based debugging. All raylib examples have also been written this way.

Since raylib v1.1, you can download a windows Installer package for easy installation and configuration. Check [raylib Webpage](http://www.raylib.com/)

building
--------

raylib could be build with the following command lines (Using GCC compiler):

	cd raylib/src
	gcc -c core.c -std=c99 -Wall
	gcc -c shapes.c -std=c99 -Wall
	gcc -c textures.c -std=c99 -Wall
	gcc -c text.c -std=c99 -Wall
	gcc -c models.c -std=c99 -Wall
	gcc -c raymath.c -std=c99 -Wall
    gcc -c rlgl.c -std=c99 -Wall
	gcc -c audio.c -std=c99 -Wall
    gcc -c utils.c -std=c99 -Wall
    gcc -c stb_image.c -std=c99 -Wall
    gcc -c stb_vorbis.c -std=c99 -Wall
    
	ar rcs libraylib.a core.o shapes.o textures.o stb_image.o text.o models.o raymath.o rlgl.o utils.o stb_vorbis.o audio.o

To compile examples, make sure raylib.h is placed in the include path and the following libraries are placed in the libraries path:

    libraylib.a   - raylib
    libglfw3.a    - GLFW3 (static version)
    libglew32.a   - GLEW, OpenGL extension loading, only required if using OpenGL 3.3+ or ES2
    libopenal32.a - OpenAL, audio device management
    
It's also recommended to link with file icon.o for fancy raylib icon usage. Linking command:

	cd raylib/examples
	gcc -o test_code.exe test_code.c icon.o -lraylib -lglfw3 -lglew32 -lopenal32 -lopengl32 -lgdi32 -std=c99 -Wl,--subsystem,windows
    
If you have any doubt, [let me know][raysan5].

contact
-------

   * Webpage: [http://www.raylib.com](http://www.raylib.com)
   * Twitter: [http://www.twitter.com/raysan5](http://www.twitter.com/raysan5)
   * Facebook: [http://www.facebook.com/raylibgames](http://www.facebook.com/raylibgames)

If you are using raylib and you enjoy it, please, [let me know][raysan5].

If you feel you can help, then, [helpme!] (http://www.raylib.com/helpme.htm)

acknowledgments
---------------

The following people have contributed in some way to make raylib project a reality. Big thanks to them!

 - [Zopokx](https://github.com/Zopokx)
 - [Elendow](http://www.elendow.com)
 - Victor Dual
 - Marc Palau
	
[raysan5]: mailto:raysan@raysanweb.com "Ramon Santamaria - Ray San"
