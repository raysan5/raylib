
# `rexm` - raylib examples manager

rexm is a raylib support tool to help to manage the raylib examples collection.

## Why do we need a tool to manage raylib examples?

raylib has a big collection of examples to learn how to use the library. It started with just a few examples and it has grown to +160 in the last 12 years, with many of those examples contributed by the community (~50%).

**Managing those examples has become a considerable amount of work.** Some years ago an [examples template](https://github.com/raysan5/raylib/blob/master/examples/examples_template.c) was created to standarize examples format and contents, it simplified the review process but implementing a new example is not enough; adding/modifying an new example from the collection implies a series of actions:

 - Create the example `.c` code file, **following provided [examples template](https://github.com/raysan5/raylib/blob/master/examples/examples_template.c) guidelines**.
    - Make sure example header lists all required metadata information
    - Make sure example follows required structure and sections
 - Copy required resources into `resource` directory, make sure they are based in a permissive license
 - Create a screenshot for the example, illustrating main functionality.
 - Add example to several raylib build systems:
    - Examples `Makefile`: multi-platform build configuration
    - Examples `Makefile.Web`: specific Web building configuration
    - raylib Visual Studio 2022 solution: Include a separate project with example, in correct category
 - Add example to examples `README.md` table, in corerct category
 - Compile example for Web and upload to `raylib.com` open-source repository
 - Update `raylib.com` webpage to include example running on real-time
 
All the following files need to be require to be review every time an examples is added or modified:

```
  - Add: raylib/examples/<category>/<category>_example_name.c
  - Add: raylib/examples/<category>/<category>_example_name.png
  - Add: raylib/examples/<category>/resources/..
  - Edit: raylib/examples/Makefile
  - Edit: raylib/examples/Makefile.Web
  - Add: raylib/projects/VS2022/examples/<category>_example_name.vcxproj
  - Edit: raylib/projects/VS2022/raylib.sln
  - Edit: raylib/examples/README.md
  - Edit: raylib.com/common/examples.js
  - Generate: raylib.com/examples/<category>/<category>_example_name.html
  - Generate: raylib.com/examples/<category>/<category>_example_name.data
  - Generate: raylib.com/examples/<category>/<category>_example_name.wasm
  - Generate: raylib.com/examples/<category>/<category>_example_name.js
```

This process has been done manually for many years and required a lot of time, it was also very prone to errors, like missing some edits or adding typos.

Finally I decided to automatize the process and create a pipeline to `add`, `rename`, `remove`, `validate` and `build` examples automatically edited. Additionally I decided to a `validation` process to verify the integrity of all the required files and even fix some of the potential issues.

## `rexm` supported commands

The following commands are supported by `rexm` to manage examples:

```
  help                          : Provides command-line usage information
  create <new_example_name>     : Creates an empty example, from internal template
  add <example_name>            : Add existing example, category extracted from name
                                  Supported categories: core, shapes, textures, text, models
  rename <old_examples_name> <new_example_name> : Rename an existing example
  remove <example_name>         : Remove an existing example
  build <example_name>          : Build example for Desktop and Web platforms
  validate                      : Validate examples collection, generates report
  update                        : Validate and update examples collection, generates report
```

## `rexm` examples **validation** and **update**

Two commands have been added to `rexm` to validate all the examples, checking if the examples follow all requirements and all the files needed are available, that way the examples review has been automated, generating complete tables on examples status.

As we were able to detect possible examples errors and inconsistencies, an additional command was implemented to **update** all examples with inconsistencies.

## `rexm` technology used

Usually this kind of pipelines are implemented with high-level scripting languages like Python due to all the functionality already provided, specially useful for files/paths management or text files editing but those languages also require the interpreter available to run. I decided to follow a less common route, implementing the tool directly in C, the language I use for raylib and all my tools. 

C standard library does not provide a lot of the high-level functionality required for files and text management but here is where raylib came to the rescue. raylib already contained some functions to work with the file system and to manage text, so I decided to implement the required additional ones to use raylib on pipelines building. 

The main benefit of using C and raylib is that the pipeline can be created in C and it can be build int an independent multi-platform dependency-free executable tool; it also open the door for a possible expansion into a UI interface, if required in the future.

As mentioned, implementing the pipeline in C with raylib required some additional support functions to be added, to simplify some tasks and align with functionality provided by high-level languages; but it was not dramatic and the truth is that the development process just flowed smoothly.

Some of the support functions implemented:

```c
// File management functions
int FileRename(const char *fileName, const char *fileRename); // Rename file (if exists)
int FileRemove(const char *fileName); // Remove file (if exists)
int FileCopy(const char *srcPath, const char *dstPath); // Copy file from one path to another, dstPath created if it doesn't exist
int FileMove(const char *srcPath, const char *dstPath); // Move file from one directory to another, dstPath created if it doesn't exist
int FileTextReplace(const char *fileName, const char *search, const char *replacement); // Replace text in an existing file
iint FileTextFindIndex(const char *fileName, const char *search); // Find text in existing file

// Text management functions
char **LoadTextLines(const char *text, int *count);  // Load text lines from text block, separate by '\n'
void UnloadTextLines(char **text); // Unload text lines
const char *TextRemoveSpaces(const char *text); // Remove text spaces, concat words
char *GetTextBetween(const char *text, const char *begin, const char *end); // Get text between two strings
char *TextReplace(const char *text, const char *search, const char *replacement); // Replace text string
char *TextReplaceBetween(const char *text, const char *begin, const char *end, const char *replacement); // Replace text between two specific strings
```

**These functions have been added to raylib main library.**

## License

`rexm` is an **open source** project, licensed under an unmodified [zlib/libpng license](LICENSE)

*Copyright (c) 2025-2026 Ramon Santamaria ([@raysan5](https://github.com/raysan5))*
