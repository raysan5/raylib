/*******************************************************************************************
*
*   rexm [raylib examples manager] - A simple command-line tool to manage raylib examples
*
*   Supported processes:
*    - create <new_example_name>
*    - add <example_name>
*    - rename <old_examples_name> <new_example_name>
*    - remove <example_name>
*
*   Files involved in the processes:
*    - raylib/examples/<category>/<category>_example_name.c
*    - raylib/examples/<category>/<category>_example_name.png
*    - raylib/examples/<category>/resources/..
*    - raylib/examples/Makefile
*    - raylib/examples/Makefile.Web
*    - raylib/examples/README.md
*    - raylib/projects/VS2022/examples/<category>_example_name.vcxproj
*    - raylib/projects/VS2022/raylib.sln
*    - raylib.com/common/examples.js
*    - raylib.com/examples/<category>/<category>_example_name.html
*    - raylib.com/examples/<category>/<category>_example_name.data
*    - raylib.com/examples/<category>/<category>_example_name.wasm
*    - raylib.com/examples/<category>/<category>_example_name.js
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2025 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>
#include <stdio.h>      // Required for: rename(), remove()
#include <string.h>     // Required for: strcmp(), strcpy()

#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO) && defined(_DEBUG)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// Example management operations
typedef enum {
    OP_NONE     = 0,
    OP_CREATE   = 1,
    OP_ADD      = 2,
    OP_RENAME   = 3,
    OP_REMOVE   = 4
} ExampleOperation;

//----------------------------------------------------------------------------------
// Module specific functions declaration
//----------------------------------------------------------------------------------
static int FileTextReplace(const char *fileName, const char *textLookUp, const char *textReplace);
static int FileCopy(const char *srcPath, const char *dstPath);
static int FileRename(const char *fileName, const char *fileRename); // TODO: Implement, make sure to deal with paths moving
static int FileRemove(const char *fileName); // TODO: Implement

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // Examples path for building
    // TODO: Avoid hardcoding path values...
    char *exBasePath = "C:/GitHub/raylib/examples";
    char *exWebPath = "C:/GitHub/raylib.com/examples";
    char *exTemplateFilePath = "C:/GitHub/raylib/examples/examples_template.c";
    
    char inFileName[1024] = { 0 };  // Example input filename

    char exName[1024] = { 0 };      // Example name, without extension: core_basic_window
    char exCategory[32] = { 0 };    // Example category: core
    char exRename[1024] = { 0 };    // Example re-name, without extension
    char exPath[1024] = { 0 };      // Example path -NOT USED-
    char exFullPath[1024] = { 0 };  // Example full path -NOT USED-
    
    int opCode = OP_NONE;           // Operation code: 0-None(Help), 1-Create, 2-Add, 3-Rename, 4-Remove

    // Command-line usage mode
    //--------------------------------------------------------------------------------------
    if (argc > 1)
    {
        // Supported commands:
        //    help                          : Provides command-line usage information (default)
        //    create <new_example_name>     : Creates an empty example, from internal template
        //    add <example_name>            : Add existing example, category extracted from name
        //    rename <old_examples_name> <new_example_name> : Rename an existing example
        //    remove <example_name>         : Remove an existing example
        if (strcmp(argv[1], "create") == 0)
        {
            // Check for valid upcoming argument
            if (argc == 2) LOG("WARNING: No filename provided to create\n");
            else if (argc > 3) LOG("WARNING: Too many arguments provided\n");
            else
            {
                // TODO: Additional security checks for file name?
                
                strcpy(inFileName, argv[2]); // Register filename for creation
                opCode = 1;
            }
        }
        else if (strcmp(argv[1], "add") == 0)
        {
            // Check for valid upcoming argument
            if (argc == 2) LOG("WARNING: No filename provided to create\n");
            else if (argc > 3) LOG("WARNING: Too many arguments provided\n");
            else
            {
                if (IsFileExtension(argv[2], ".c")) // Check for valid file extension: input
                {
                    // TODO: Parse category name from filename provided!
                    
                    strcpy(inFileName, argv[2]); // Register filename for creation
                    opCode = 2;
                }
                else LOG("WARNING: Input file extension not recognized (.c)\n");
            }
        }
        else if (strcmp(argv[1], "rename") == 0)
        {
            if (argc == 2) LOG("WARNING: No filename provided to create\n");
            else if (argc == 3) LOG("WARNING: No enough arguments provided\n");
            else if (argc > 4) LOG("WARNING: Too many arguments provided\n");
            else
            {
                // TODO: Register exName, exCategory and exRename
                
                opCode = 3;
            }
        }
        else if (strcmp(argv[1], "remove") == 0)
        {
            // Check for valid upcoming argument
            if (argc == 2) LOG("WARNING: No filename provided to create\n");
            else if (argc > 3) LOG("WARNING: Too many arguments provided\n");
            else
            {
                strcpy(inFileName, argv[2]); // Register filename for removal
                opCode = 4;
            }
        }
    }
    
    switch (opCode)
    {
        case 1:     // Create: New example from template
        {
            // Copy template file as new example
            FileCopy(exTemplateFilePath, TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));
        }
        case 2:     // Add: Example from command-line input filename
        {
            if ((opCode != 1) && FileExists(inFileName))
            {
                FileCopy(inFileName, TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName));
            }                
            
            // Generate all required files
            //--------------------------------------------------------------------------------
            // Create: raylib/examples/<category>/<category>_example_name.c
            // Create: raylib/examples/<category>/<category>_example_name.png
            FileCopy("C:/GitHub/raylib/examples/examples_template.png", 
                TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName)); // To be updated manually!
            
            // Copy: raylib/examples/<category>/resources/*.*  ---> To be updated manually!
            
            // TODO: Update the required files to add new example in the required position (ordered by category and name),
            // it could require some logic to make it possible...
            
            // Edit: raylib/examples/Makefile --> Add new example
            // Edit: raylib/examples/Makefile.Web --> Add new example
            // Edit: raylib/examples/README.md --> Add new example
            
            // Create: raylib/projects/VS2022/examples/<category>_example_name.vcxproj
            // Edit: raylib/projects/VS2022/raylib.sln --> Add new example
            // Edit: raylib.com/common/examples.js --> Add new example
            
            // Compile to: raylib.com/examples/<category>/<category>_example_name.html
            // Compile to: raylib.com/examples/<category>/<category>_example_name.data
            // Compile to: raylib.com/examples/<category>/<category>_example_name.wasm
            // Compile to: raylib.com/examples/<category>/<category>_example_name.js
            
            // Recompile example (on raylib side)
            // NOTE: Tools requirements: emscripten, w64devkit
            system(TextFormat("%s/../build_example_web.bat %s\%s", exBasePath, exCategory, exName));

            // Copy results to web side
            FileCopy(TextFormat("%s/%s/%s.html", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.html", exWebPath, exCategory, exName));
            FileCopy(TextFormat("%s/%s/%s.data", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.data", exWebPath, exCategory, exName));
            FileCopy(TextFormat("%s/%s/%s.wasm", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.wasm", exWebPath, exCategory, exName));
            FileCopy(TextFormat("%s/%s/%s.js", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.js", exWebPath, exCategory, exName));
        } break;
        case 3:     // Rename
        {
            // Rename all required files
            rename(TextFormat("%s/%s/%s.c", exBasePath, exCategory, exName), 
                TextFormat("%s/%s/%s.c", exBasePath, exCategory, exRename));
            rename(TextFormat("%s/%s/%s.png", exBasePath, exCategory, exName), 
                TextFormat("%s/%s/%s.png", exBasePath, exCategory, exRename));
            
            FileTextReplace(TextFormat("%s/Makefile", exBasePath), exName, exRename);
            FileTextReplace(TextFormat("%s/Makefile.Web", exBasePath), exName, exRename);
            FileTextReplace(TextFormat("%s/README.md", exBasePath), exName, exRename);
            
            rename(TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exName), 
                TextFormat("%s/../projects/VS2022/examples/%s.vcxproj", exBasePath, exRename));
            FileTextReplace(TextFormat("%s/../projects/VS2022/raylib.sln", exBasePath), exName, exRename);
            
            // Remove old web compilation
            FileTextReplace(TextFormat("%s/../common/examples.js", exWebPath), exName, exRename);
            remove(TextFormat("%s/%s/%s.html", exWebPath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.data", exWebPath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.wasm", exWebPath, exCategory, exName));
            remove(TextFormat("%s/%s/%s.js", exWebPath, exCategory, exName));

            // Recompile example (on raylib side)
            // NOTE: Tools requirements: emscripten, w64devkit
            system(TextFormat("%s/../build_example_web.bat %s\%s", exBasePath, exCategory, exName));

            // Copy results to web side
            FileCopy(TextFormat("%s/%s/%s.html", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.html", exWebPath, exCategory, exName));
            FileCopy(TextFormat("%s/%s/%s.data", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.data", exWebPath, exCategory, exName));
            FileCopy(TextFormat("%s/%s/%s.wasm", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.wasm", exWebPath, exCategory, exName));
            FileCopy(TextFormat("%s/%s/%s.js", exBasePath, exCategory, exName),
                TextFormat("%s/%s/%s.js", exWebPath, exCategory, exName));
        } break;
        case 4:     // Remove
        {
            // TODO: Remove and update all required files...

        } break;
        default:    // Help
        {
            // Supported commands:
            //    help                          : Provides command-line usage information
            //    create <new_example_name>     : Creates an empty example, from internal template
            //    add <example_name>            : Add existing example, category extracted from name
            //    rename <old_examples_name> <new_example_name> : Rename an existing example
            //    remove <example_name>         : Remove an existing example
            
            printf("\n////////////////////////////////////////////////////////////////////////////////////////////\n");
            printf("//                                                                                        //\n");
            printf("// rexm [raylib examples manager] - A simple command-line tool to manage raylib examples  //\n");
            printf("// powered by raylib v5.6-dev                                                             //\n");
            printf("//                                                                                        //\n");
            printf("// Copyright (c) 2025 Ramon Santamaria (@raysan5)                                         //\n");
            printf("//                                                                                        //\n");
            printf("////////////////////////////////////////////////////////////////////////////////////////////\n\n");

            printf("USAGE:\n\n");
            printf("    > rexm help|create|add|rename|remove <example_name> [<example_rename>]\n");

            printf("\nOPTIONS:\n\n");
            printf("    help                          : Provides command-line usage information\n");
            printf("    create <new_example_name>     : Creates an empty example, from internal template\n");
            printf("    add <example_name>            : Add existing example, category extracted from name\n");
            printf("                                    Supported categories: core, shapes, textures, text, models\n");
            printf("    rename <old_examples_name> <new_example_name> : Rename an existing example\n");
            printf("    remove <example_name>         : Remove an existing example\n\n");
            printf("\nEXAMPLES:\n\n");
            printf("    > rexm add shapes_custom_stars\n");
            printf("        Add and updates new example provided <shapes_custom_stars>\n\n");
            printf("    > rexm rename core_basic_window core_cool_window\n");
            printf("        Renames and updates example <core_basic_window> to <core_cool_window>\n\n");
        } break;
    }
    
    return 0;
}

//----------------------------------------------------------------------------------
// Module specific functions definition
//----------------------------------------------------------------------------------
static int FileTextReplace(const char *fileName, const char *textLookUp, const char *textReplace)
{
    int result = 0;
    char *fileText = NULL;
    char *fileTextUpdated = { 0 };

    fileText = LoadFileText(fileName);
    fileTextUpdated = TextReplace(fileText, textLookUp, textReplace);
    result = SaveFileText(fileName, fileTextUpdated);
    MemFree(fileTextUpdated);
    UnloadFileText(fileText);
    
    return result;
}

static int FileCopy(const char *srcPath, const char *dstPath)
{
    int result = 0;
    int srcDataSize = 0;
    unsigned char *srcFileData = LoadFileData(srcPath, &srcDataSize);
    
    if ((srcFileData != NULL) && (srcDataSize > 0)) result = SaveFileData(dstPath, srcFileData, srcDataSize);

    UnloadFileData(srcFileData);
    
    return result;
}

static int FileRename(const char *fileName, const char *fileRename)
{
    int result = 0;
    
    // TODO: Make sure to deal with paths properly for file moving if required
    
    if (FileExists(fileName)) rename(fileName, TextFormat("%s/%s", GetDirectoryPath(fileName), fileRename));

    return result;
}

static int FileRemove(const char *fileName)
{
    int result = 0;
    
    if (FileExists(fileName)) remove(fileName);

    return result;
}
