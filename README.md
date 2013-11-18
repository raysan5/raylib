<img src="http://www.raylib.com/favicon.ico" width=256>

about
-----

raylib is a simple and easy-to-use library to learn C videogames programming.

raylib is highly inspired by Borland BGI graphics lib (more specifically WinBGI) and by XNA framework. 
Allegro and SDL have also been analyzed for reference.

Want to see how easy is making games with raylib? Jump to [code examples!] (http://www.raylib.com/examples.php)

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

I've coded quite a lot in C# and XNA and I really love it (in fact, my students learn C# with XNA after C), 
so, I decided to use C# language notation and XNA naming conventions. That way, students can jump from 
raylib to XNA (or MonoGame) extremely easily.

raylib started as a weekend project and after three months of hard work, here it is the first version. 

Enjoy it.

features
--------

   * Written in plain C code (C99)
   * Uses C# PascalCase/camelCase notation
   * Hardware accelerated using OpenGL 1.1
   * Transparencies support (RGBA Colors)
   * Custom color palette for better use on white background
   * Basic 3D Support (camera, basic models, OBJ models, etc)
   * Powerful Text module with SpriteFonts support

raylib uses on its core module the outstanding [GLFW3] (http://www.glfw.org/) library. The best option by far I found for 
window/context and input management (clean, focused, great license, well documented, modern, ...). 

raylib is licensed under a zlib/libpng license like GLFW3. View [LICENSE] (https://github.com/raysan5/raylib/blob/master/LICENSE.md).

tool requirements
------------------

raylib has been developed using exclusively two tools: 

   * Notepad++ (text editor) - [http://notepad-plus-plus.org/](http://notepad-plus-plus.org/)
   * MinGW (GCC compiler) - [http://www.mingw.org/](http://www.mingw.org/)
   
Those are the tools I recommended to develop with raylib, actually, my students develop using this tools. 
I believe those are the best tools to train spartan-programmers.

Someone could argue about debugging. raylib is a library intended for learning and I think C it's a clear enough language
to allow writing small-mid size programs with a printf-based debugging. All raylib examples have also been written this way.

building
--------

raylib could be build with the following command lines (Using GCC compiler):

	cd raylib/src
	gcc -c core.c -std=c99 -Wall
	gcc -c shapes.c -std=c99 -Wall
	gcc -c textures.c -std=c99 -Wall
	gcc -c stb_image.c -std=c99 -Wall
	gcc -c text.c -std=c99 -Wall
	gcc -c models.c -std=c99 -Wall
	gcc -c vector3.c -std=c99 -Wall
	gcc -c audio.c -std=c99 -Wall
	ar rcs raylib.a core.o shapes.o textures.o stb_image.o text.o models.o vector3.o audio.o

To compile examples, make sure raylib.h is placed in include path and libraries raylib (libraylib.a) and glfw3 (libglfw3.a) 
are placed in the libraries path. It's also recommended to link with file icon.o for fancy raylib icon usage.

	cd raylib/src/examples
	gcc -o test_code.exe test_code.c icon.o -lraylib -lglfw3 -lopengl32 -lgdi32 -std=c99 -Wl,--subsystem,windows

contact
-------

   * Webpage: [http://www.raylib.com](http://www.raylib.com)
   * Twitter: [http://www.twitter.com/raysan5](http://www.twitter.com/raysan5)
   * Facebook: [http://www.facebook.com/raylibgames](http://www.facebook.com/raylibgames)

If you are using raylib and you enjoy it, please, [let me know][raysan5].

If you feel you can help, then, [helpme!] (http://www.raylib.com/helpme.php)

acknowledgments
---------------

The following people have contributed in some way to make raylib project a reality. Big thanks to them!

 - Zopokx
 - Elendow

	
[raysan5]: mailto:raysan@raysanweb.com "Ramon Santamaria - Ray San"
